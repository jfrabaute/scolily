/***********************************************************************************
This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA
************************************************************************************/

#include "scolily.h"

#include <sstream>

#include <dirent.h>

#include <libintl.h>
#define _(String) gettext (String)

#ifdef GUI_ENABLED
#include <gtkmm/main.h>
#include "ui.h"
#endif

extern int CreateFile (char *);
extern void StartRecording(void);
extern void AddNote(int, unsigned int, int);
extern void CloseFile(void);


char PluginPath[200] = "/dev/null";
char PluginSearchPath[2][200] = {LIBDIR,"~/.scolily"};

void * DlPlugin;

int (*Plugin_CreateFile)(char *);
void (*Plugin_StartRecording)(void);
void (*Plugin_AddNote)(int, unsigned int, int);
void (*Plugin_StartNote)(int, int);
void (*Plugin_StopNote)(int, int);
void (*Plugin_CloseFile)(void);
bool Plugin_Mode;

float frequences[] = { 65.406, 69.295, 73.416, 77.781, 82.406, 87.307, 92.498, 97.998, 103.826, 110.000, 116.540, 123.470 };

pthread_t ComputeLoopID, WriteNotesID;

CRecord * record;

int OctaveShift;

unsigned short int Stop = 0;
unsigned short int useUI;

unsigned int Duration;

float BPM;
// n = MINUTE_USEC/BPM = délai en secondes		; Par exemple, pour 60BPM = 1s
// durée d'une croche = MINUTE_USEC/BPM/2		; 0.5s = 500000µs
// durée d'une double croche = MINUTE_USEC/BPM/4	; 0.25s = 250000µs 
// durée d'une triple croche = MINUTE_USEC/BPM/8	; 0.125s = 125000µs
// en dessous ( MINUTE_USEC/BPM/16 ), on ignore		; 0.0625s = 62500µs


char FileName[800];

double Threshold;

void LoadPlugin();
void printHelp(void);
void printError(void);
void showError(char *);
void printVersion(void);

#ifdef GUI_ENABLED
MainWindow * pMainwindow;
#endif

int main(int argc, char **argv)
{
#ifdef GUI_ENABLED
   useUI = 1;
#else
   useUI = 0;
#endif
    setlocale (LC_ALL, "");
    bindtextdomain (PACKAGE, LOCALEDIR);
    bind_textdomain_codeset(PACKAGE, "UTF-8");
    textdomain (PACKAGE);

    BPM = 60;

    OctaveShift = 0;

    Threshold = -45.;

    Duration = 10;

    strcpy(FileName,"Test.ly");

    for(unsigned int i = 1;i<argc;i++)
    {
        if(strcmp(argv[i],"-h") == 0) {
            printHelp();
            return 0;
        }
        if(strcmp(argv[i],"-v") == 0) {
            printVersion();
            return 0;
        }
       if(strcmp(argv[i],"-o") == 0)
           if(i+1 < argc) strcpy(FileName,argv[++i]);
           else printError();
       if(strcmp(argv[i],"-d") == 0)
           if(i+1 < argc) {
               std::istringstream DurationStream(argv[++i]);
               DurationStream >> Duration;
           }
           else printError();

        if(strcmp(argv[i],"-t") == 0)
            if(i+1 < argc) {
                std::istringstream ThresholdStream(argv[++i]);
                ThresholdStream >> Threshold;
            }
            else printError();

        if(strcmp(argv[i],"-s") == 0)
            if(i+1 < argc) {
                std::istringstream ShiftStream(argv[++i]);
                ShiftStream >> OctaveShift;
            }
            else printError();
        if(strcmp(argv[i],"-b") == 0)
            if(i+1 < argc) {
                std::istringstream BPMStream(argv[++i]);
                BPMStream >> BPM;
            }
            else printError();
#ifdef GUI_ENABLED
        if(strcmp(argv[i],"-g") == 0)
            useUI = 0;
#endif
    }


    if(useUI == 0) {

        record = new CRecord();

        LoadPlugin();

        if(Stop == 0)
        {

            pthread_create (&ComputeLoopID, NULL, ComputeLoopTh, NULL);


            // On prépare l'utilisateur
            printf("1\n");
            usleep(MINUTE_USEC/BPM);
            printf("2\n");
            usleep(MINUTE_USEC/BPM);
            printf("3\n");
            usleep(MINUTE_USEC/BPM);
            printf(_("and...\n"));
            usleep(MINUTE_USEC/BPM);

            pthread_create (&WriteNotesID, NULL, WriteNotesTh, NULL);

            long Duration2 = Duration*1000000;
            long Time = 0;

            for(Time;Time<=Duration2;Time+=25000)
                if(Stop == 0) {
                    usleep(25000);
                }
                else break;

            Stop = 1;

            pthread_join(WriteNotesID,NULL);
            pthread_join(ComputeLoopID,NULL);

        }
        delete record;
    }
#ifdef GUI_ENABLED
    else {
        g_thread_init (NULL);
        gdk_threads_init ();
        gdk_threads_enter ();
        
        Gtk::Main Application(argc, argv);
        
        MainWindow mainwindow;
        pMainwindow = &mainwindow;
        
        
        Gtk::Main::run(mainwindow);

        gdk_threads_leave ();
    }
#endif
    if(DlPlugin) dlclose(DlPlugin);
    return 0;
}

void LoadPlugin()
{
        if(DlPlugin) dlclose(DlPlugin);
        if (!(DlPlugin = dlopen(PluginPath, RTLD_LAZY)))
        {
            if(strcmp("/dev/null",PluginPath) != 0) showError(_("Unable to open plugin. Using internal lilypond converter.\n"));
            Plugin_CreateFile = (int (*)(char *)) &CreateFile;
            Plugin_StartRecording = (void (*)(void)) &StartRecording;
            Plugin_AddNote = (void (*)(int, unsigned int, int)) &AddNote;
            Plugin_CloseFile = (void (*)(void)) &CloseFile;
            Plugin_Mode = 0;
        }
        else if (!(Plugin_CreateFile = (int (*)(char *)) dlsym(DlPlugin, "CreateFile")))
        {
            Stop = 1;
            showError(_("Unable to find CreateFile symbol in plugin.\n"));
        }
        else if (!(Plugin_StartRecording = (void (*)(void)) dlsym(DlPlugin, "StartRecording")))
        {
            Stop = 1;
            showError(_("Unable to find StartRecording symbol in plugin.\n"));
        }
        else if (!(Plugin_CloseFile = (void (*)(void)) dlsym(DlPlugin, "CloseFile")))
        {
            Stop = 1;
            showError(_("Unable to find CloseFile symbol in plugin.\n"));
        }
        else {
            if(!dlsym(DlPlugin, "Mode")) Plugin_Mode = 0;
            else if(!(Plugin_Mode = *(bool *) dlsym(DlPlugin, "Mode"))) Plugin_Mode = 0;
            if(Plugin_Mode == 0) {
                if (!(Plugin_AddNote = (void (*)(int, unsigned int, int)) dlsym(DlPlugin, "AddNote")))
                {
                    Stop = 1;
                    showError(_("Unable to find AddNote symbol in plugin.\n"));
                }
            }
            if(Plugin_Mode == 1) {
                if (!(Plugin_StartNote = (void (*)(int, int)) dlsym(DlPlugin, "StartNote")))
                {
                    Stop = 1;
                    showError(_("Unable to find StartNote symbol in plugin.\n"));
                }
                if (!(Plugin_StopNote = (void (*)(int, int)) dlsym(DlPlugin, "StopNote")))
                {
                    Stop = 1;
                    showError(_("Unable to find StopNote symbol in plugin.\n"));
                }
            }
        }
}

void showError(char * Error) {
    std::cerr << Error;
#ifdef GUI_ENABLED
    if(useUI == 1)
    {
            
        Gtk::MessageDialog errorDialog(*pMainwindow, _("An error has occured"), false, Gtk::MESSAGE_ERROR, Gtk::BUTTONS_OK);

        errorDialog.set_secondary_text( Error );

        errorDialog.run();
    }
#endif
}

void printError(void) {
    printf(_("Syntax error!\n"));
    printHelp();
    exit(1);
}

void printHelp(void) {
    printf(_("Usage: Scolily [OPTIONS]\n"));
    printf(_("Scolily generates lilypond files by processing audio input\n"));
    printf(      "  -h\t%s", _("Display this help screen\n"));
    printf(      "  -o\t%s", _("Write output to specified file (instead of Test.ly)\n"));
    printf(      "  -v\t%s", _("Show version information\n"));
    printf(      "  -d\t%s", _("Choose execution time\n"));
    printf(      "  -t\t%s", _("Choose the recording threshold\n"));
    printf(      "  -s\t%s", _("Shift, in octave\n"));
    printf(      "  -b\t%s", _("Specify the number of beats per minute\n"));
    printf(      "  -g\t%s %s\n", _("Do not use a GUI "),_("(Not available yet)"));
    printVersion();
}

void printVersion(void) {
    printf(_("Scolily v%s\n"), VERSION);
    printf("Copyright © 2007 Thibaut GIRKA <thibaut.girka@gmail.com>\n");
    printf(_("This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details.\n"));

}


