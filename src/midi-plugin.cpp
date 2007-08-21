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

#include <iostream>
#include <fstream>
#include <stdio.h>
#include <stdlib.h>

using namespace std;

// Données obligatoires

extern "C" char Name[200] = "Midifile plugin";
extern "C" bool Type = 1; //0 = input, 1 = output
extern "C" bool Mode = 0; //Mode : 0 = note-based infos, 1 = event-based infos

// Données propres au plugin :
ofstream FileOutputStream;
char OutputFileName[200];
//char uLongChars [11][4] = {"1", "2.", "2", "4.", "4", "8.", "8", "16.", "16", "32.", "32" };
unsigned long uLongValues [] = { 4*192,		// ronde
                                 1.5*2*192,	// blanche pointée
                                 2*192,		// blanche
                                 1.5*1*192,	// noir pointée
                                 1*192,		// noir
                                 1.5*96,	// croche pointée
                                 96,		// croche
                                 1.5*48,	// double croche pointée
                                 48,		// double croche
                                 24*1.5,
                                 24 };
char notes[13][4];
unsigned long DeltaTime = 0;

// Fonctions supplémentaires

unsigned long WriteVarLen(unsigned long value)
{
   unsigned long buffer;
   buffer = value & 0x7F;

   while ( (value >>= 7) )
   {
     buffer <<= 8;
     buffer |= ((value & 0x7F) | 0x80);
   }

   return buffer;
}

// Fonctions obligatoires


extern "C" int CreateFile(char * FileName)
{
    FileOutputStream.open(FileName,ios::binary);
    if(!FileOutputStream) {
        std::cerr << "Unable to open file\n";
        return 1;
    }

    // Header midi
    FileOutputStream << "MThd";
    FileOutputStream.put(0);
    FileOutputStream.put(0);
    FileOutputStream.put(0);
    FileOutputStream.put(6);

    FileOutputStream.put(0); // Format 0
    FileOutputStream.put(0);

    FileOutputStream.put(0); // Une piste
    FileOutputStream.put(1);

    FileOutputStream.put(0);
    FileOutputStream.put(192); // Une noire = 192 delta

    // Header track
    FileOutputStream << "MTrk";
    FileOutputStream.put(0);
    FileOutputStream.put(0);
    FileOutputStream.put(0);
    FileOutputStream.put(0);  
}
extern "C" void StartRecording()
{

}
extern "C" void AddNote(int Note, unsigned int NoteDuration, int Octave)
{
    //si Note == -1, alors silence. Si NoteDuration = 1, ronde ; 2, blanche ; ... 
    if(Note >= 0) {
        unsigned int Delta2 = WriteVarLen(DeltaTime);
        while (true)
        {
            FileOutputStream.put(Delta2);
            if (Delta2 & 0x80)
            Delta2 >>= 8;
            else break;
        }

        FileOutputStream.put( ((1<<7) | (1<<4) | 1) ); //Note on, canal 1
        FileOutputStream.put( Note + 12*(Octave+4) ); //Note on data : Note
        FileOutputStream.put( 127 ); //Note on data : Vélocité : 127
        DeltaTime = uLongValues[NoteDuration];
        Delta2 = WriteVarLen(DeltaTime);
        while (true)
        {
            FileOutputStream.put(Delta2);
            if (Delta2 & 0x80)
            Delta2 >>= 8;
            else break;
        }
        FileOutputStream.put( ((1<<7) | (1<<4) | 1) ); //Note on, canal 1
        FileOutputStream.put( Note + 12*(Octave+4) ); //Note on data : Note
        FileOutputStream.put( 0 ); //Note on data : Vélocité : 0
        DeltaTime = 0;
    }
    else DeltaTime = uLongValues[NoteDuration];
}
extern "C" void CloseFile()
{
    // Fin de piste
    FileOutputStream.put((char)(0));
    FileOutputStream.put((char)(0xFF));
    FileOutputStream.put((char)(0x2F));
    FileOutputStream.put((char)(0x00));

    // Calcul de la taille de la piste
    unsigned long FileLong = (long)FileOutputStream.tellp();
    FileLong -= 22;

    // Renseignement de la taille de la piste
    FileOutputStream.seekp(18);
    FileOutputStream.put((char)(FileLong>>8*3)%256);
    FileOutputStream.put((char)(FileLong>>8*2)%256);
    FileOutputStream.put((char)(FileLong>>8*1)%256);
    FileOutputStream.put((char)(FileLong)%256);

    FileOutputStream.close();
}

