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

using namespace std;

// Données obligatoires

char Name[200] = "Lilypond plugin";
bool Type = 1; //0 = input, 1 = output
bool Mode = 0; // Note-based plugin

// Données propres au plugin :
ofstream FileOutputStream;
char uLongChars [11][4] = {"1", "2.", "2", "4.", "4", "8.", "8", "16.", "16", "32.", "32" };
char notes[13][4];

// Fonctions obligatoires

int CreateFile(char * FileName)
{
    FileOutputStream.open(FileName);
    if(!FileOutputStream) {
        std::cerr << "Unable to open file\n";
        return 1;
    }
    strcpy(notes[0],"c");	//do
    strcpy(notes[1],"cis");	//do dièse
    strcpy(notes[2],"d");	//ré
    strcpy(notes[3],"dis");	//ré dièse
    strcpy(notes[4],"e");	//mi
    strcpy(notes[5],"f");	//fa
    strcpy(notes[6],"fis");	//fa dièse
    strcpy(notes[7],"g");	//sol
    strcpy(notes[8],"gis");	//sol dièse
    strcpy(notes[9],"a");	//la
    strcpy(notes[10],"ais");	//la dièse
    strcpy(notes[11],"b");	//si
}
void StartRecording()
{
    FileOutputStream << "\n\n{ "; //Début du fichier Lilypond
}
void AddNote(int Note, unsigned int NoteDuration, int Octave)
{
    //si Note == -1, alors silence. Si NoteDuration = 1, ronde ; 2, blanche ; ... 
    if(Note >= 0) {
        FileOutputStream << notes[Note];
        if(Octave > 0)
            for(unsigned int z=0;z<Octave;z++) FileOutputStream << "'";
        else if(Octave < 0)
            for(unsigned int z=0;z<(-Octave);z++) FileOutputStream << ",";
    }
    else FileOutputStream << "r";
    FileOutputStream << uLongChars[NoteDuration] << "\n";
}
void CloseFile()
{
    FileOutputStream << "\n}\n";
    FileOutputStream.close();
}

