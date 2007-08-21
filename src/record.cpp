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

#include "record.h"


extern double Threshold;

#define MAX_FFT_LENGTH 48000
#define MAX_PEAKS 4
unsigned int rate = MAX_FFT_LENGTH;
typedef struct {
	double freq;
	double db;
} Peak;


CFft::CFft(int size)
{
	fftSize = rate/size;
	fftIn = (float*)fftwf_malloc(sizeof(float) * 2 * (fftSize/2+1));
	fftOut = (fftwf_complex *)fftIn;
	fftPlan = fftwf_plan_dft_r2c_1d(fftSize, fftIn, fftOut, FFTW_MEASURE);
	fftSampleBuffer = (float *)malloc(fftSize * sizeof(float));
	fftSample = NULL;
	fftLastPhase = (float *)malloc((fftSize/2+1) * sizeof(float));
	memset(fftSampleBuffer, 0, fftSize*sizeof(float));
	memset(fftLastPhase, 0, (fftSize/2+1)*sizeof(float));
	fftFrameCount = 0;
}
CFft::~CFft()
{
	fftwf_destroy_plan(fftPlan);
	fftwf_free(fftIn);
	free(fftSampleBuffer);
	free(fftLastPhase);
}

void CFft::measure(int nframes, int overlap, float *indata)
{
	int i, stepSize = fftSize/overlap;
	double freqPerBin = rate/(double)fftSize, phaseDifference = 2.*M_PI*(double)stepSize/(double)fftSize;

	if (!fftSample) fftSample = fftSampleBuffer + (fftSize-stepSize);

	for (i=0; i<nframes; i++) {
		*fftSample++ = indata[i];
		if (fftSample-fftSampleBuffer >= fftSize) {
			int k;
			Peak peaks[MAX_PEAKS];

			for (k=0; k<MAX_PEAKS; k++) {
				peaks[k].db = -200.;
				peaks[k].freq = 0.;
			}

			fftSample = fftSampleBuffer + (fftSize-stepSize);

			for (k=0; k<fftSize; k++) {
				double window = -.5*cos(2.*M_PI*(double)k/(double)fftSize)+.5;
				fftIn[k] = fftSampleBuffer[k] * window;
			}
			fftwf_execute(fftPlan);

			for (k=0; k<=fftSize/2; k++) {
				long qpd;
				float real = fftOut[k][0];
				float imag = fftOut[k][1];
				float magnitude = 20.*log10(2.*sqrt(real*real + imag*imag)/fftSize);
				float phase = atan2(imag, real);
				float tmp, freq;

				/* compute phase difference */
				tmp = phase - fftLastPhase[k];
				fftLastPhase[k] = phase;

				/* subtract expected phase difference */
				tmp -= (double)k*phaseDifference;

				/* map delta phase into +/- Pi interval */
				qpd = (long) (tmp / M_PI);
				if (qpd >= 0) qpd += qpd&1;
				else qpd -= qpd&1;
				tmp -= M_PI*(double)qpd;

				/* get deviation from bin frequency from the +/- Pi interval */
				tmp = overlap*tmp/(2.*M_PI);

				/* compute the k-th partials' true frequency */
				freq = (double)k*freqPerBin + tmp*freqPerBin;

				if (freq > 0.0 && magnitude > peaks[0].db) {
					memmove(peaks+1, peaks, sizeof(Peak)*(MAX_PEAKS-1));
					peaks[0].freq = freq;
					peaks[0].db = magnitude;
				}
			}
			fftFrameCount++;
			if (fftFrameCount > 0 && fftFrameCount % overlap == 0) {
				int l, maxharm = 0;
				k = 0;
				for (l=1; l<MAX_PEAKS && peaks[l].freq > 0.0; l++) {
					int harmonic;

					for (harmonic=5; harmonic>1; harmonic--) {
						if (peaks[0].freq / peaks[l].freq < harmonic+.02 && peaks[0].freq / peaks[l].freq > harmonic-.02) {
							if (harmonic > maxharm && peaks[0].db < peaks[l].db/2) {
								maxharm = harmonic;
								k = l;
							}
						}
					}
				}
				if( peaks[k].freq > 65. && peaks[k].freq < 1000. && peaks[k].db > Threshold )
					m_freq = peaks[k].freq;
				else
					m_freq = 0.0;
			}
			memmove(fftSampleBuffer, fftSampleBuffer+stepSize, (fftSize-stepSize)*sizeof(float));
		}
	}
}

void CFft::compute(int nframes,signed short int *indata)
{
	float buf[nframes];
	int i;
	for (i=0; i<nframes; i++) {
		buf[i] = indata[i]/32768.;
	}
	if( nframes > 0 )
		measure(nframes, 4, buf);
}

CRecord::CRecord( char * deviceName )
{
	fft = new CFft(10);
	int result;
	snd_pcm_hw_params_t *hw_params;

	if ((result = snd_pcm_open(&alsaHandle, deviceName, SND_PCM_STREAM_CAPTURE, 0)) < 0) {
		fprintf(stderr, "Cannot open audio device %s: %s\n",deviceName, snd_strerror(result));
		exit(EXIT_FAILURE);
	}

	snd_pcm_hw_params_malloc(&hw_params);
	snd_pcm_hw_params_any(alsaHandle, hw_params);
	snd_pcm_hw_params_set_access(alsaHandle, hw_params,SND_PCM_ACCESS_RW_INTERLEAVED);
	snd_pcm_hw_params_set_format(alsaHandle, hw_params,SND_PCM_FORMAT_S16_LE);
	snd_pcm_hw_params_set_rate_near(alsaHandle, hw_params,&rate, 0);
	if( rate != MAX_FFT_LENGTH )
		fprintf(stderr, "rate as changed, please report this as a bug\n");
	snd_pcm_hw_params_set_channels(alsaHandle, hw_params, 1);
	snd_pcm_hw_params(alsaHandle, hw_params);
	snd_pcm_hw_params_free(hw_params);
	snd_pcm_prepare(alsaHandle);
}

CRecord::~CRecord()
{
	delete fft;
	snd_pcm_drain(alsaHandle);
	snd_pcm_close(alsaHandle);
}

void CRecord::compute( void )
{
	int frames  = 1;
	int nFrames = 0;
	nFrames = snd_pcm_readi(alsaHandle, buf, frames);

	if(nFrames == -EPIPE)
		snd_pcm_prepare(alsaHandle);
	else if(nFrames < 0)
		fprintf(stderr,"error from read: %s\n",snd_strerror(nFrames));
	else if(nFrames != frames)
		fprintf(stderr, "short read, read %d frames\n", nFrames);
	fft->compute(nFrames, buf);
}

