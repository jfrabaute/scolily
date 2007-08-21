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

#ifndef _UI_H
#define _UI_H

#include <gtkmm.h>

class FinishDialog : public Gtk::MessageDialog
{
public:
    FinishDialog(Gtk::Window& window);
    virtual ~FinishDialog();
    
    void ShowDialog();
    
    Gtk::ProgressBar Progression;
    
protected:

};

class SettingsDialog : public Gtk::MessageDialog
{
public:
    SettingsDialog(Gtk::Window& window);
    virtual ~SettingsDialog();
    
    void ShowDialog();
    
    Gtk::SpinButton ThresholdInput;
    Gtk::ComboBoxText ComboBoxOutput;
protected:
    Gtk::Label ThresholdLabel, OutputPluginLabel;
    Gtk::HBox m_hbox1, m_hbox2;
};

class MainWindow : public Gtk::Window
{

public:
    MainWindow();
    virtual ~MainWindow();
    
    Gtk::Button ButtonStart, ButtonStop;
    Gtk::Entry InputFile;
    Gtk::SpinButton InputBPM, InputDuration, InputShift;
    Gtk::CheckButton DurationCheck;
    Gtk::ProgressBar TimeBar, Metronome;
    Gtk::Widget* pMenubar;

protected:
    //Signal handlers:
    //virtual void on_button_clicked();
    virtual void on_menu_file_quit();
    virtual void on_menu_file_settings();
    virtual void on_menu_file_about();
    virtual void on_menu_file_help();
    virtual void on_toggle_duration();
    virtual void on_button_start();
    virtual void on_button_stop();

    //Member widgets:
    Gtk::VBox m_vbox;
    Gtk::HBox m_hbox, m_hbox2, m_hbox3, m_hbox4, m_hbox5;
    
    Gtk::Label FileLabel, BPMLabel, ShiftLabel, DurationLabel;
    
    Glib::RefPtr<Gtk::UIManager> m_refUIManager;
    Glib::RefPtr<Gtk::ActionGroup> m_refActionGroup;
    Glib::RefPtr<Gtk::RadioAction> m_refChoiceOne, m_refChoiceTwo;
};

#endif

