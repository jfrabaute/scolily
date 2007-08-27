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

extern "C" char Name[200] = "ABC Music plugin";
extern "C" bool Type = 1; //0 = input, 1 = output
extern "C" bool Mode = 0; // Note-based plugin

// Données propres au plugin :
ofstream FileOutputStream;
char uLongChars [11][4] = {"64", "48", "32", "24", "16", "12", "8", "6", "4", "3", "2" }; //1/64
unsigned int uLongValues [11] = {64, 48, 32, 24, 16, 12, 8, 6, 4, 3, 2 }; //Ronde = 64 ; Max = 64
char notes[13][4];
unsigned int LastBarTime = 0;
bool NotesState[13] = {0,0,0,0,0,0,0,0,0,0,0,0,0}; //État des notes ( altérations )
bool Recursive;

// Fonctions obligatoires

extern "C" int CreateFile(char * FileName)
{
    FileOutputStream.open(FileName);
    if(!FileOutputStream) {
        std::cerr << "Unable to open file\n";
        return 1;
    }
    strcpy(notes[0],"C");	//do
    strcpy(notes[1],"^C");	//do dièse
    strcpy(notes[2],"D");	//ré
    strcpy(notes[3],"^D");	//ré dièse
    strcpy(notes[4],"E");	//mi
    strcpy(notes[5],"F");	//fa
    strcpy(notes[6],"^F");	//fa dièse
    strcpy(notes[7],"G");	//sol
    strcpy(notes[8],"^G");	//sol dièse
    strcpy(notes[9],"A");	//la
    strcpy(notes[10],"^A");	//la dièse
    strcpy(notes[11],"B");	//si
}
extern "C" void StartRecording()
{
    FileOutputStream << "T:Scolily Output\nM:C\nK:C\nL:1/64\n\n"; //Début du fichier ABC
}
extern "C" void AddNote(int Note, unsigned int NoteDuration, int Octave)
{
    Octave -= 1;
    if(uLongValues[NoteDuration]+LastBarTime > 64) {
        unsigned int Duration2 = uLongValues[NoteDuration]+LastBarTime-64;
        unsigned int BestMatch = 0;
        for(int i=0;i<11;i++)
        {
            if(uLongValues[i] <= (64-LastBarTime)) {
                BestMatch = i;
                break;
            }
        }
        if(BestMatch > 0 && !Recursive) {
            Recursive = 1;
            AddNote(Note,BestMatch,Octave+1);
            Recursive = 0;
        }
        for(int i=0;i<11;i++)
        {
            if(uLongValues[i] <= Duration2) {
                BestMatch = i;
                break;
            }
        }
        FileOutputStream << "|";
        NoteDuration = BestMatch;
        LastBarTime = 0;
    }
    else FileOutputStream << " ";
    LastBarTime += uLongValues[NoteDuration];
    //si Note == -1, alors silence. Si NoteDuration = 1, ronde ; 2, blanche ; ... 
    if(Note >= 0) {
        if(notes[Note][0] == '^')
            NotesState[Note] = 1;
        else
            if(Note < 11)
                if(notes[Note+1][0] == '^')
                    if(NotesState[Note+1] == 1) {
                        FileOutputStream << "=";
                        NotesState[Note+1] = 0;
                    }
        FileOutputStream << notes[Note];
        if(Octave > 0)
            for(unsigned int z=0;z<Octave;z++) FileOutputStream << "'";
        else if(Octave < 0)
            for(unsigned int z=0;z<(-Octave);z++) FileOutputStream << ",";
    }
    else FileOutputStream << "z";
    FileOutputStream << uLongChars[NoteDuration] << "";
}
extern "C" void CloseFile()
{
    FileOutputStream << "|]";
    FileOutputStream.close();
}

