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

#define BUFSIZE				65536
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
			ippsConvert_16s32f((short*)(pBuffer1), fTempData, wHdr1.dwBytesRecorded / 2);
			for (auto fitem = fTempData; fitem < (fTempData + wHdr1.dwBytesRecorded / 2); fitem++)
			{
				auto fv = (*fitem) / 1000.0;
				if (fv > 0) {
					(*fitem) = (1.0f - expf(0 - fv));
				}
				else {
					(*fitem) = (expf(fv) - 1.0f);
				}
			}
			chart->InputData(fTempData, wHdr1.dwBytesRecorded / 2);
			waveInReset(hWaveIn);
		}
		delete pBuffer1;
		});
	chart->Start();
	chart->Stop();
}