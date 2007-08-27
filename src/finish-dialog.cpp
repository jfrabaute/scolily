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

extern char FileName[800];

FinishDialog::FinishDialog(Gtk::Window& window) : Gtk::MessageDialog(window, _("Finish"), false, Gtk::MESSAGE_INFO, Gtk::BUTTONS_YES_NO, true)
{
    set_title(_("Finish"));
    set_message(_("Finish"));
    set_secondary_text(_("Scolily  just finished to export the file, you may want to run lilypond, now.\nRun lilypond ?"));

    get_vbox()->add(Progression);
    
    show_all_children(); // On affiche tout, et on laisse l'utilisateur
    
}

void FinishDialog::ShowDialog()
{
    switch(run())
        {
            case Gtk::RESPONSE_YES:
                FILE *fPipe;
                char command[810];
                char line[100];
                int result;

                sprintf(command,"lilypond %s 2>&1", FileName);
                if(!(fPipe = (FILE*)popen(command,"r")))
                {
                    GtkWidget *pInfoDialog;
                    Gtk::MessageDialog errorDialog(*this, _("Error"), false, Gtk::MESSAGE_ERROR, Gtk::BUTTONS_OK, true);
                    errorDialog.set_secondary_text(_("Scolily did not find lilypond, please install it."));
                }
                
                else {
                
                    gdk_threads_leave();

                    while ( fgets( line, sizeof line, fPipe))
                    {
                        printf("%s", line);
                        gdk_threads_enter();
                        Progression.pulse();
                        gdk_threads_leave();
                        usleep(100000);
                    }
                    result = pclose(fPipe);
                
                    gdk_threads_enter();

                    Progression.set_fraction(1);

                    if(result == 0) {
                        Gtk::MessageDialog infoDialog(*this, _("Finished"), false, Gtk::MESSAGE_INFO, Gtk::BUTTONS_OK, true);
                        infoDialog.set_secondary_text(_("Convertion finished, you should now be able to open the PDF file."));
                        infoDialog.run();
                    }
                    else {
                        Gtk::MessageDialog errorDialog(*this, _("Error"), false, Gtk::MESSAGE_ERROR, Gtk::BUTTONS_OK, true);
                        errorDialog.set_secondary_text(_("Convertion failed, check the console for details."));
                        errorDialog.run();
                    }
                }
    
                break;
            case Gtk::RESPONSE_NO:
                break;
        }
}

FinishDialog::~FinishDialog()
{
}

#endif

