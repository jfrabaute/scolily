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

#include <dirent.h>

#include <libintl.h>
#define _(String) gettext (String)

extern char PluginSearchPath[2][200];
extern char PluginPath[200];
extern double Threshold;

int Choosen = -1;

SettingsDialog::SettingsDialog(Gtk::Window& window) : Gtk::MessageDialog(window, _("Scolily Settings"), false, Gtk::MESSAGE_INFO, Gtk::BUTTONS_OK_CANCEL , true)
{
    set_title(_("Settings"));

    set_secondary_text(_("Scolily can use many plugins to export your partitions, in addition of lilypond built-in abilities."));

    
    ThresholdLabel.set_text(_("Threshold:"));
    OutputPluginLabel.set_text(_("Output plugin:"));
    
    ThresholdInput.set_range(-90,0);
    ThresholdInput.set_width_chars(3);
    ThresholdInput.set_increments(1,5);
    ThresholdInput.set_value(Threshold);
    
    get_vbox()->add(m_hbox1);
    m_hbox1.add(ThresholdLabel);
    m_hbox1.add(ThresholdInput);
    
    get_vbox()->add(m_hbox2);
    m_hbox2.add(OutputPluginLabel);
    m_hbox2.add(ComboBoxOutput);
    
    
    show_all_children(); // On affiche tout, et on laisse l'utilisateur
}
void SettingsDialog::ShowDialog()
{

    ComboBoxOutput.append_text(_("Internal lilypond"));//get_active_row_number ()
    
    char PluginFiles[50][200];
    unsigned int Index = 0;
    DIR *dir;
    for(unsigned int i=0;i<2;i++)
    {
        dir = opendir(PluginSearchPath[i]);
        struct dirent *DirContent;
        if(dir)
        {
            while(DirContent = readdir(dir))
            {
                char TempFileName[200], FullTempFilePath[200];
                strcpy(TempFileName,DirContent->d_name);
                if(strcmp((char *)TempFileName+strlen(TempFileName)-3,".so") == 0) {
                    void *DlPlugTemp;
                    char *Name;
                    int Type;
                    sprintf(FullTempFilePath,"%s/%s",PluginSearchPath[i],TempFileName);
                    DlPlugTemp = dlopen(FullTempFilePath, RTLD_LAZY);
                    if(DlPlugTemp)
                    {
                        if(Name = (char*) dlsym(DlPlugTemp, "Name"))
                            if(Type = *(int*) dlsym(DlPlugTemp, "Type"))
                                if(Type == 1) {
                                    strcpy(PluginFiles[Index++],FullTempFilePath);
                                    ComboBoxOutput.append_text(Name);
                                }
                        dlclose(DlPlugTemp);
                    }
                }
            }
            closedir(dir);
        }
    }
    
    ComboBoxOutput.set_active(Choosen+1);

    if(run() == Gtk::RESPONSE_OK)
    {
        Choosen = ComboBoxOutput.get_active_row_number();
        Choosen -= 1;

        if(Choosen == -1) strcpy(PluginPath,"/dev/null");
        else strcpy(PluginPath,PluginFiles[Choosen]);
        Threshold = ThresholdInput.get_value_as_int();
    }
}
SettingsDialog::~SettingsDialog()
{
}

#endif

