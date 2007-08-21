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

/*
 File originaly taken from UltraStar-ng
*/

#ifndef __RECORD_H_
#define __RECORD_H_

#include <math.h>
#include <fftw3.h>

#include <alsa/asoundlib.h>

class CFft {
	public:
	CFft(int size=10);
	~CFft();
	void compute(int nframes, signed short int *indata);
	float getFreq( void ) { return m_freq; }
	private:
	void measure (int nframes, int overlap, float *indata);
	float *fftSampleBuffer;
	float *fftSample;
	float *fftLastPhase;
	int fftSize;
	int fftFrameCount;
	float *fftIn;
	fftwf_complex *fftOut;
	fftwf_plan fftPlan;
	float m_freq;

};

class CRecord {
	public:
	CRecord(char * captureDevice = "default");
	~CRecord();
	void compute();
	float getFreq( void ) { return fft->getFreq(); }
	private:
	CFft * fft;
	snd_pcm_t *alsaHandle;
	signed short int buf[4096];
};

#endif
