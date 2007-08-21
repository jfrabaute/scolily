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

extern int (*Plugin_CreateFile)(char *);
extern void (*Plugin_StartRecording)(void);
extern void (*Plugin_AddNote)(int, unsigned int, int);
extern void (*Plugin_StartNote)(int, int);
extern void (*Plugin_StopNote)(int, int);
extern void (*Plugin_CloseFile)(void);
extern bool Plugin_Mode;

extern float frequences[];// = { 65.406, 69.295, 73.416, 77.781, 82.406, 87.307, 92.498, 97.998, 103.826, 110.000, 116.540, 123.470 };
extern char notes[13][4];

extern CRecord * record;

extern unsigned short int Stop;

extern float BPM;
// n = MINUTE_USEC/BPM = délai en secondes		; Par exemple, pour 60BPM = 1s
// durée d'une croche = MINUTE_USEC/BPM/2		; 0.5s = 500000µs
// durée d'une double croche = MINUTE_USEC/BPM/4	; 0.25s = 250000µs 
// durée d'une triple croche = MINUTE_USEC/BPM/8	; 0.125s = 125000µs
// en dessous ( uLongs[10] ), on ignore			; 0.0625s = 62500µs

extern char FileName[200];

extern int OctaveShift;

using namespace std;

void * WriteNotesTh(void *) {

    if(Plugin_CreateFile(FileName) == -1) Stop = 1;

    if(Stop == 1) return (void *) 1;

    //Définition des valeurs de temps
    unsigned long uLongs[11] = {MINUTE_USEC/BPM*4,	//Ronde = 4*temps
                                MINUTE_USEC/BPM*3,	//Blanche pointée = 2*1.5*temps
                                MINUTE_USEC/BPM*2,	//Blanche = 2*temps
                                MINUTE_USEC/BPM*1.5,	//Noir pointée = 1*1.5*temps
                                MINUTE_USEC/BPM,	//Noir = 1*temps
                                MINUTE_USEC/BPM/2*1.5,	//Croche pointée = 1.5*temps/2
                                MINUTE_USEC/BPM/2,	//Croche = temps/2
                                MINUTE_USEC/BPM/4*1.5,	//Double croche pointée = 1.5*temps/4
                                MINUTE_USEC/BPM/4,	//Double croche = temps/4
                                MINUTE_USEC/BPM/8*1.5,	//Triple croche pointée = 1.5*temps/8
                                MINUTE_USEC/BPM/8};	//Triple croche pointée = temps/8

    int LastNote = -1, LastOctave = 0;
    unsigned long uLastNoteTime = 0, uSilenceTime = 0;

    Plugin_StartRecording();

    timeval Time2, StartSilence, StartNote;

    while(Stop == 0) {
        usleep(100);
        float Freq = record->getFreq();

        if(Freq == 0) { // Si pas de note
            if(uSilenceTime == 0) gettimeofday(&StartSilence,NULL); //Si la note précédente n'existait pas, le silence commence ici
            else if(uSilenceTime >= uLongs[0]) {
                if(Plugin_Mode == 0) Plugin_AddNote(-1,0,0);
                uSilenceTime = 0;
                gettimeofday(&StartSilence,NULL);
            }

            if(uSilenceTime >= uLongs[10]) {
                if(LastNote != -1 && uLastNoteTime >= uLongs[10]) {
                    int TempOctave = LastOctave - 1;

                    unsigned int Best = 0;
                    unsigned long BestScore = uLongs[0];
                    for(unsigned int i=0;i<11;i++) {
                        long Temp = uLastNoteTime-uLongs[i];
                        Temp = abs(Temp);
                        if(Temp < BestScore) {
                            BestScore = Temp;
                            Best = i;
                        }
                    }
                    if(Plugin_Mode == 0) Plugin_AddNote(LastNote,Best,TempOctave);
                    else Plugin_StopNote(LastNote,TempOctave);
                }
                LastNote = -1;
            }
            gettimeofday(&Time2,NULL);
            uSilenceTime = (Time2.tv_sec*1000000+Time2.tv_usec-StartSilence.tv_sec*1000000-StartSilence.tv_usec);
            continue;
        }

        if(uSilenceTime != 0) {
            if(LastNote != -1) uSilenceTime = 0;
            else {
                unsigned int Best = 0;
                unsigned long BestScore = uLongs[0];
                for(unsigned int i=0;i<11;i++) {
                    long Temp = uSilenceTime-uLongs[i];
                    Temp = abs(Temp);
                    if(Temp < BestScore) {
                        BestScore = Temp;
                        Best = i;
                    }
                }
                if(Plugin_Mode == 0) Plugin_AddNote(-1,Best,0);
                uSilenceTime = 0;
            }
        }

        unsigned int Octave;

        for(unsigned int octave = 0;octave<4;octave++) {
            float FrequenceStart, FrequenceEnd;
            if(octave > 0) FrequenceStart = (frequences[0]*pow(2,octave)*2.+frequences[11]*pow(2,octave-1))/3.;
            else FrequenceStart = frequences[0]*pow(2,octave);
            FrequenceEnd = (frequences[11]*pow(2,octave)*2.+frequences[0]*pow(2,octave+1))/3.;
            if(FrequenceStart <= Freq && Freq < FrequenceEnd) Octave = octave;
        }

        unsigned int best = 0;
        float bestScore = frequences[11]*pow(2,Octave);
        for(unsigned int j=0;j<12;j++) {
            float temp = Freq-(frequences[j]*pow(2,Octave));
            temp = sqrt(temp*temp);
            if(temp < bestScore) {
                bestScore = temp;
                best = j;
            }
        }

        Octave += OctaveShift;

        if(LastNote != best || LastOctave != Octave) {
            gettimeofday(&StartNote,NULL);
            if( LastNote != -1 && uLastNoteTime >= uLongs[10]) {
                int TempOctave = LastOctave - 1;

                unsigned int Best = 0;
                unsigned long BestScore = uLongs[0];
                for(unsigned int i=0;i<11;i++) {
                    long Temp = uLastNoteTime-uLongs[i];
                    Temp = abs(Temp);
                    if(Temp < BestScore) {
                        BestScore = Temp;
                        Best = i;
                    }
                }
                if(Plugin_Mode == 0) Plugin_AddNote(LastNote,Best,TempOctave);
                else Plugin_StopNote(LastNote,TempOctave);
            }

            if(Plugin_Mode == 1) Plugin_StartNote(best,Octave);

            LastNote = best;
            LastOctave = Octave;


        }
        else {
            gettimeofday(&Time2,NULL);
            uLastNoteTime = (Time2.tv_sec*1000000+Time2.tv_usec-StartNote.tv_sec*1000000-StartNote.tv_usec);
        }

    }

    if(LastNote == -1)
        if(uSilenceTime != 0) {
            if(uSilenceTime < uLongs[10]) uSilenceTime = 0;
            else {
                unsigned int Best = 0;
                unsigned long BestScore = uLongs[0];
                for(unsigned int i=0;i<11;i++) {
                    long Temp = uSilenceTime-uLongs[i];
                    Temp = abs(Temp);
                    if(Temp < BestScore) {
                        BestScore = Temp;
                        Best = i;
                    }
                }
                if(Plugin_Mode == 0) Plugin_AddNote(-1,Best,0);
                uSilenceTime = 0;
            }
        }
        else if(LastNote != -1 && uLastNoteTime >= uLongs[10]) {
            int TempOctave = LastOctave - 1;

            unsigned int Best = 0;
            unsigned long BestScore = uLongs[0];
            for(unsigned int i=0;i<11;i++) {
                long Temp = uLastNoteTime-uLongs[i];
                Temp = abs(Temp);
                if(Temp < BestScore) {
                    BestScore = Temp;
                    Best = i;
                }
            }
            if(Plugin_Mode == 0) Plugin_AddNote(LastNote,Best,TempOctave);
            else Plugin_StopNote(LastNote,TempOctave);
        }
    
    Plugin_CloseFile();
}

void * ComputeLoopTh(void *) {
 while( Stop == 0 ) {
     record->compute();
     // FIXME : usleep(100);
 } 
}
