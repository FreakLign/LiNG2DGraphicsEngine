#define _CRT_SECURE_NO_WARNINGS
#include <Windows.h>
#include <mmsystem.h>
#include <mmreg.h>
#include <conio.h>
#include "BasicChart.h"
#include <mutex>
#include <ipps.h>
#include <io.h>

#pragma comment(lib, "winmm.lib")

#define BUFSIZE				655360
#define SAMPLERATE			64000
#define BITS_PER_SAMPLE		16
#define CHANNEL_COUNT		1

int main() {
	BasicChart* chart;
	chart = new BasicChart(2560, 500, "Test");
	int datCount = BUFSIZE;
	chart->SetVisualParas(datCount, -1, 1);

	HANDLE          wait;
	HWAVEIN hWaveIn;
	WAVEFORMATEX waveform;
	char* pBuffer1;
	WAVEHDR wHdr1;
	waveform.wFormatTag = WAVE_FORMAT_PCM;
	waveform.nSamplesPerSec = SAMPLERATE;
	waveform.wBitsPerSample = BITS_PER_SAMPLE;
	waveform.nChannels = CHANNEL_COUNT;
	waveform.nAvgBytesPerSec = SAMPLERATE * CHANNEL_COUNT * BITS_PER_SAMPLE / 8;
	waveform.nBlockAlign = CHANNEL_COUNT * BITS_PER_SAMPLE / 8;
	waveform.cbSize = 0;
	wait = CreateEvent(NULL, 0, 0, NULL);
	waveInOpen(&hWaveIn, WAVE_MAPPER, &waveform, (DWORD_PTR)wait, 0L, CALLBACK_EVENT);
	DWORD bufsize = BUFSIZE;
	FILE* f_towrite = fopen("dat0.dat", "wb");
	thread t = thread([&] {
		pBuffer1 = new char[SAMPLERATE * CHANNEL_COUNT * BITS_PER_SAMPLE / 80];
		wHdr1.dwBufferLength = SAMPLERATE * CHANNEL_COUNT * BITS_PER_SAMPLE / 80;
		wHdr1.dwBytesRecorded = 0;
		wHdr1.dwUser = 0;
		wHdr1.dwFlags = 0;
		wHdr1.dwLoops = 1;
		int pos_tail = 0;
		float* fIndex = new float[BUFSIZE];
		float* fPoints = new float[BUFSIZE * 2];
		float* fTempData = new float[BUFSIZE * 2];
		memset(fTempData, 0, BUFSIZE * 8);
		for (size_t i = 0; i < BUFSIZE; i++)
		{
			fIndex[i] = (float(i) - float(BUFSIZE) / 2.f) / (float(BUFSIZE) / 2.f);
		}
		wHdr1.lpData = (LPSTR)pBuffer1 + pos_tail;
		while (true) {
			waveInPrepareHeader(hWaveIn, &wHdr1, sizeof(WAVEHDR));
			waveInAddBuffer(hWaveIn, &wHdr1, sizeof(WAVEHDR));
			waveInStart(hWaveIn);
			Sleep(10);
			/*ippsConvert_16s32f((short*)(pBuffer1), fTempData + BUFSIZE + (pos_tail / 2), wHdr1.dwBytesRecorded / 2);
			for (auto fitem = fTempData + BUFSIZE + (pos_tail / 2); fitem < (fTempData + BUFSIZE + (pos_tail / 2) + wHdr1.dwBytesRecorded / 2); fitem++)
			{
				auto fv = (*fitem) / 1000.0;
				if (fv > 0) {
					(*fitem) = (1.0f - expf(0 - 4.0f * fv)) / 2.0f;
				}
				else {
					(*fitem) = (expf(4.0f * fv) - 1.0f) / 2.0f;
				}
			}*/
			//fwrite(pBuffer1, wHdr1.dwBytesRecorded, 1, f_towrite);
			//fwrite(fTempData + BUFSIZE + (pos_tail / 2), wHdr1.dwBytesRecorded * 2, 1, f_towrite);
			//ippsRealToCplx_32f(fIndex, fTempData + (pos_tail / 2), (Ipp32fc*)fPoints, BUFSIZE);
			ippsConvert_16s32f((short*)(pBuffer1), fTempData, wHdr1.dwBytesRecorded / 2);
			for (auto fitem = fTempData; fitem < (fTempData + wHdr1.dwBytesRecorded / 2); fitem++)
			{
				auto fv = (*fitem) / 1000.0;
				if (fv > 0) {
					(*fitem) = (1.0f - expf(0 - 4.0f * fv)) / 2.0f;
				}
				else {
					(*fitem) = (expf(4.0f * fv) - 1.0f) / 2.0f;
				}
			}
			chart->InputData(fTempData, wHdr1.dwBytesRecorded / 2);
			/*pos_tail += wHdr1.dwBytesRecorded;
			if (pos_tail >= BUFSIZE * 2) {
				ippsCopy_32f(fTempData + BUFSIZE, fTempData, BUFSIZE);
				pos_tail -= BUFSIZE * 2;
			}*/
			waveInReset(hWaveIn);//ÖÐÖ¹Â¼Òô 
		}
		delete pBuffer1;
		});
	chart->Start();
	chart->Stop();
}