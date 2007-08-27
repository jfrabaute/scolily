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

#ifdef GUI_ENABLED

#include "scolily.h"


#include "ui.h"

#include <iostream>

#include <libintl.h>
#define _(String) gettext (String)

extern char FileName[800];
extern unsigned int Duration;
extern void LoadPlugin();
extern unsigned short int Stop;
extern int OctaveShift;
extern float BPM;
extern CRecord * record;
extern MainWindow * pMainwindow;
extern void * DlPlugin;

pthread_t UILoopID;
extern pthread_t ComputeLoopID, WriteNotesID;

void * UILoopTh( void * );


MainWindow::MainWindow()
:
    m_vbox(false, 0)
{
    set_title("Scolily v" VERSION);
    set_resizable(false);
    
    try
    {
        Glib::RefPtr<Gdk::Pixbuf> Icon = Gdk::Pixbuf::create_from_file(ICONDIR "/scolily.svg");
    
        set_default_icon(Icon);
    }
    catch(Glib::Error& e)
    {
        std::cerr << "Error while loading icon " ICONDIR "/scolily.svg \n";
    }
    
    add(m_vbox);
    
    FileLabel.set_text(_("File"));
    BPMLabel.set_text(_("Beat per minute"));
    ShiftLabel.set_text(_("Shift, in octave"));
    DurationLabel.set_text(_("Duration (in seconds) :"));
    ButtonStart.set_label(_("Start"));
    ButtonStop.set_label(_("Stop"));
    
    m_refActionGroup = Gtk::ActionGroup::create(); // Create menubar
    
    m_refActionGroup->add( Gtk::Action::create("FileMenu", _("File")) );
    
    m_refActionGroup->add( Gtk::Action::create("FileSettings", Gtk::Stock::PREFERENCES),
                sigc::mem_fun(*this, &MainWindow::on_menu_file_settings) ); // Create File -> Settings
    m_refActionGroup->add( Gtk::Action::create("FileQuit", Gtk::Stock::QUIT), 
                sigc::mem_fun(*this, &MainWindow::on_menu_file_quit) ); // Creatie File -> Quit
            
            
    m_refActionGroup->add( Gtk::Action::create("HelpMenu", _("?")) ); // Create ?
    m_refActionGroup->add( Gtk::Action::create("HelpAbout", Gtk::Stock::ABOUT),
                sigc::mem_fun(*this, &MainWindow::on_menu_file_about) ); // Create ? -> About
    m_refActionGroup->add( Gtk::Action::create("HelpHelp", Gtk::Stock::HELP),
                sigc::mem_fun(*this, &MainWindow::on_menu_file_help) ); // Create ? -> Aide
    
    m_refUIManager = Gtk::UIManager::create();
    m_refUIManager->insert_action_group(m_refActionGroup);

    add_accel_group(m_refUIManager->get_accel_group());
    
    Glib::ustring ui_info = 
          "<ui>"
          "  <menubar name='MenuBar'>"
          "    <menu action='FileMenu'>"
          "      <menuitem action='FileSettings' />"
          "      <menuitem action='FileQuit' />"
          "    </menu>"
          "    <menu action='HelpMenu'>"
          "      <menuitem action='HelpAbout'/>"
//          "      <menuitem action='HelpHelp'/>"
          "    </menu>"
          "  </menubar>"
          "</ui>";
          
    #ifdef GLIBMM_EXCEPTIONS_ENABLED
    try
    {
        m_refUIManager->add_ui_from_string(ui_info);
    }
    catch(const Glib::Error& ex)
    {
        std::cerr << "building menus failed: " <<  ex.what();
    }
    #else
    std::auto_ptr<Glib::Error> ex;
    m_refUIManager->add_ui_from_string(ui_info, ex);
    if(ex.get())
    {
        std::cerr << "building menus failed: " <<  ex->what();
    }
    #endif //GLIBMM_EXCEPTIONS_ENABLED
    
    pMenubar = m_refUIManager->get_widget("/MenuBar"); // Retrieve menubar
    if(pMenubar)
        m_vbox.pack_start(*pMenubar, Gtk::PACK_SHRINK); // Ajout du menu si non nul
    
    // Limitations des SpinButton
    InputBPM.set_range(20,200);
    InputBPM.set_width_chars(3);
    InputBPM.set_increments(1,5);
    InputShift.set_range(-4,4);
    InputShift.set_width_chars(1);
    InputShift.set_increments(1,2);
    InputDuration.set_range(5,500);
    InputDuration.set_width_chars(3);
    InputDuration.set_increments(5,15);
    
    // Ajout des widgets
    m_vbox.add(m_hbox);
    m_hbox.add(FileLabel);   
    m_hbox.add(InputFile);
    
    m_vbox.add(m_hbox2);
    m_hbox2.add(BPMLabel);   
    m_hbox2.add(InputBPM); 
    
    m_vbox.add(m_hbox3);
    m_hbox3.add(ShiftLabel);   
    m_hbox3.add(InputShift); 
    

    // Signal sur la case à cocher
    DurationCheck.signal_clicked().connect( sigc::mem_fun(*this,
                &MainWindow::on_toggle_duration) );
    DurationCheck.set_active(true);

    m_vbox.add(m_hbox4);
    m_hbox4.add(DurationCheck);  
    m_hbox4.add(DurationLabel);   
    m_hbox4.add(InputDuration); 
    
    m_vbox.add(m_hbox5);
    m_hbox5.add(ButtonStart);  
    m_hbox5.add(ButtonStop);   

    ButtonStart.signal_clicked().connect( sigc::mem_fun(*this,
                &MainWindow::on_button_start) );
                
    ButtonStop.signal_clicked().connect( sigc::mem_fun(*this,
                &MainWindow::on_button_stop) );
                
    m_vbox.add(Metronome);
    m_vbox.add(TimeBar);
    TimeBar.set_text("00:00");
    
    InputFile.set_text((gchar *) FileName);
    InputDuration.set_value(Duration);
    InputBPM.set_value(BPM);
    InputShift.set_value(OctaveShift);
    
    ButtonStop.set_sensitive(false);

    show_all_children(); // On affiche tout, et on laisse l'utilisateur
    
}

MainWindow::~MainWindow() {
}

void MainWindow::on_menu_file_quit() {
    hide();
}

void MainWindow::on_menu_file_settings() {
    SettingsDialog settings(*pMainwindow);
    settings.ShowDialog();
}

void MainWindow::on_menu_file_about() {

    Gtk::AboutDialog aboutDialog;

    
    std::list<Glib::ustring> authors;
    authors.push_back("Thibaut GIRKA <thibaut.girka@gmail.com>");
    
    std::list<Glib::ustring> artists;
    artists.push_back("Thibaut GIRKA <thibaut.girka@gmail.com>");
    artists.push_back("PMdomine");
    
    aboutDialog.set_authors(authors);
    aboutDialog.set_artists(artists);
    aboutDialog.set_comments(_("Scolily listen to the music you play and write scores."));
    aboutDialog.set_copyright(_("Copyright (C) 2007 Thibaut GIRKA <thibaut.girka@gmail.com>"));
    aboutDialog.set_license(_("Copyright (C) 2007 Thibaut GIRKA <thibaut.girka@gmail.com>\n"
          "Scolily is free software; you can redistribute it and/or modify it\n"
          "under the terms of the GNU General Public License as published by\n"
          "the Free Software Foundation; either version 2 of the License, or\n"
          "(at your option) any later version."));
    aboutDialog.set_website(_("http://www.codingteam.net/scolily-aff_en.html"));
    aboutDialog.set_version(VERSION);
    aboutDialog.set_name("Scolily");
    aboutDialog.set_translator_credits(_("translator-credits"));
    
    try
    {
        Glib::RefPtr<Gdk::Pixbuf> Logo = Gdk::Pixbuf::create_from_file(ICONDIR "/scolily-logo.svg");
    
        aboutDialog.set_logo(Logo);
    }
    catch(Glib::Error& e)
    {
        std::cerr << "Error while loading icon " ICONDIR "/scolily-logo.svg \n";
    }
    
    aboutDialog.run();
}


void MainWindow::on_menu_file_help() {
    //FIXME
}


void MainWindow::on_toggle_duration() {
    InputDuration.set_sensitive(DurationCheck.get_active());
}

void MainWindow::on_button_start() {
    Stop = 0;
    LoadPlugin();
    if(Stop == 1) return;
    
    Duration = InputDuration.get_value_as_int();
    OctaveShift = InputShift.get_value_as_int();
    BPM = InputBPM.get_value_as_int();
    InputFile.get_text().copy(FileName,800);
    ButtonStart.set_sensitive(false);
    ButtonStop.set_sensitive(true);
    
    record = new CRecord();
    
    InputFile.set_sensitive(false);
    InputBPM.set_sensitive(false);
    InputShift.set_sensitive(false);
    InputDuration.set_sensitive(false);
    DurationCheck.set_sensitive(false);
    pMenubar->set_sensitive(false);
    
    TimeBar.set_fraction(0);
    TimeBar.set_text("00:00");
    
    Metronome.set_orientation(Gtk::PROGRESS_LEFT_TO_RIGHT);
    Metronome.set_fraction(0);
    
    Stop = 0;
    
    pthread_create (&UILoopID, NULL, UILoopTh, NULL);
}

void MainWindow::on_button_stop() {
    Stop = 1;
}

void * UILoopTh( void * )
{
    pthread_create (&ComputeLoopID, NULL, ComputeLoopTh, NULL);

    gdk_threads_enter();
    pMainwindow->Metronome.set_text("1");
    gdk_threads_leave();
    
    usleep(MINUTE_USEC/BPM);
    
    gdk_threads_enter();
    pMainwindow->Metronome.set_text("2");
    gdk_threads_leave();
    
    usleep(MINUTE_USEC/BPM);
    
    gdk_threads_enter();
    pMainwindow->Metronome.set_text("3");
    gdk_threads_leave();
    
    usleep(MINUTE_USEC/BPM);
    
    gdk_threads_enter();
    pMainwindow->Metronome.set_text(_("and..."));
    gdk_threads_leave();
    
    usleep(MINUTE_USEC/BPM);
    
    gdk_threads_enter();
    pMainwindow->Metronome.set_text(" ");
    gdk_threads_leave();

    pthread_create (&WriteNotesID, NULL, WriteNotesTh, NULL);

    bool DurationChecked;

    gdk_threads_enter ();
    DurationChecked = pMainwindow->DurationCheck.get_active();
    gdk_threads_leave ();

    long Time = 0;

    char * TimeText = (char *) malloc(sizeof(char)*6);
    timeval StartTime, StopTime, CurrentTime;

    gettimeofday(&StartTime,NULL);
    StopTime.tv_sec = StartTime.tv_sec + Duration;
    StopTime.tv_usec = StartTime.tv_usec;
    
    gettimeofday(&CurrentTime,NULL);

    if(DurationChecked) {
        do {
            unsigned long Duration2 = Duration*1000000;
            if(Stop == 0) {
                gdk_threads_enter ();
                unsigned long Time = CurrentTime.tv_sec*1000000+CurrentTime.tv_usec-StartTime.tv_sec*1000000-StartTime.tv_usec;
                if(Time != 0) pMainwindow->TimeBar.set_fraction(((double)Time)/((double)Duration2));
                sprintf(TimeText,"%02d:%02d",Time/1000000/60,(Time/1000000)%60);
                pMainwindow->TimeBar.set_text(TimeText);
                float TimePercent = Time/(MINUTE_USEC/BPM);
                pMainwindow->Metronome.set_orientation(((int)(TimePercent)%2)?Gtk::PROGRESS_RIGHT_TO_LEFT:Gtk::PROGRESS_LEFT_TO_RIGHT);
                pMainwindow->Metronome.set_fraction(TimePercent-(int)TimePercent);
                gdk_threads_leave ();
                usleep(25000);
                gettimeofday(&CurrentTime,NULL);
            }
            else break;
        }
        while((CurrentTime.tv_sec*1000000+CurrentTime.tv_usec) < (StopTime.tv_sec*1000000+StopTime.tv_usec));
    }
   
    
    else {
        while(Stop == 0) {
            unsigned long Duration2 = Duration*1000000;
            gdk_threads_enter ();
            unsigned long Time = CurrentTime.tv_sec*1000000+CurrentTime.tv_usec-StartTime.tv_sec*1000000-StartTime.tv_usec;
            pMainwindow->TimeBar.set_text(TimeText);
            float TimePercent = Time/(MINUTE_USEC/BPM);
            pMainwindow->Metronome.set_orientation(((int)(TimePercent)%2)?Gtk::PROGRESS_RIGHT_TO_LEFT:Gtk::PROGRESS_LEFT_TO_RIGHT);
            pMainwindow->Metronome.set_fraction(TimePercent-(int)TimePercent);
            gdk_threads_leave ();
            usleep(25000);
            gettimeofday(&CurrentTime,NULL);
        }
    }

    Stop = 1;

    pthread_join(WriteNotesID,NULL);
    pthread_join(ComputeLoopID,NULL);

    gdk_threads_enter ();
    pMainwindow->ButtonStart.set_sensitive(true);
    pMainwindow->ButtonStop.set_sensitive(false);

    pMainwindow->InputFile.set_sensitive(true);
    pMainwindow->InputBPM.set_sensitive(true);
    pMainwindow->InputShift.set_sensitive(true);
    pMainwindow->InputDuration.set_sensitive(pMainwindow->DurationCheck.get_active());
    pMainwindow->DurationCheck.set_sensitive(true);
    pMainwindow->pMenubar->set_sensitive(true);
    
    void (*Plugin_AfterProcessing)(void);
    if(DlPlugin) if (!(Plugin_AfterProcessing = (void (*)(void)) dlsym(DlPlugin, "AfterProcessing"))) {
            Gtk::MessageDialog infoDialog(*pMainwindow,_("Finished"), false, Gtk::MESSAGE_INFO, Gtk::BUTTONS_OK, true);
            infoDialog.set_secondary_text(_("Scolily  just finished to export the file. But your plugin doesn't specify any after-processing option. It meens there is nothing to do, or you have to do it yourself."));
            infoDialog.run();
        }
        else {
            Plugin_AfterProcessing();
        }
    else {
        FinishDialog finished(*pMainwindow);
        finished.ShowDialog();
    }
   gdk_threads_leave ();
}

#endif


