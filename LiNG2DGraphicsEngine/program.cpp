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
	chart = new BasicChart(2560, 1080, "Test");
	int datCount = BUFSIZE;
	chart->SetVisualParas(datCount);

	HANDLE          wait;
	HWAVEIN hWaveIn;  //输入设备  
	WAVEFORMATEX waveform; //采集音频的格式，结构体  
	char* pBuffer1;//采集音频时的数据缓存  
	WAVEHDR wHdr1; //采集音频时包含数据缓存的结构体
	waveform.wFormatTag = WAVE_FORMAT_PCM;//声音格式为PCM  
	waveform.nSamplesPerSec = SAMPLERATE;//采样率
	waveform.wBitsPerSample = BITS_PER_SAMPLE;//采样比特，16bits/次  
	waveform.nChannels = CHANNEL_COUNT;//采样声道数，1声道  
	waveform.nAvgBytesPerSec = SAMPLERATE * CHANNEL_COUNT * BITS_PER_SAMPLE / 8;//每秒的数据率，就是每秒能采集多少字节的数据  
	waveform.nBlockAlign = CHANNEL_COUNT * BITS_PER_SAMPLE / 8;//一个块的大小，采样bit的字节数乘以声道数  
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
			ippsConvert_16s32f((short*)(pBuffer1), fTempData + BUFSIZE + (pos_tail / 2), wHdr1.dwBytesRecorded / 2);
			for (auto fitem = fTempData + BUFSIZE + (pos_tail / 2); fitem < (fTempData + BUFSIZE + (pos_tail / 2) + wHdr1.dwBytesRecorded / 2); fitem++)
			{
				auto fv = (*fitem) / 1000.0;
				if (fv > 0) {
					(*fitem) = (1.0f - expf(0 - fv)) / 2.0f;
				}
				else {
					(*fitem) = (expf(fv) - 1.0f) / 2.0f;
				}
			}
			//fwrite(pBuffer1, wHdr1.dwBytesRecorded, 1, f_towrite);
			//fwrite(fTempData + BUFSIZE + (pos_tail / 2), wHdr1.dwBytesRecorded * 2, 1, f_towrite);
			ippsRealToCplx_32f(fIndex, fTempData + (pos_tail / 2), (Ipp32fc*)fPoints, BUFSIZE);
			chart->InputData(fPoints, BUFSIZE);
			pos_tail += wHdr1.dwBytesRecorded;
			if (pos_tail >= BUFSIZE * 2) {
				ippsCopy_32f(fTempData + BUFSIZE, fTempData, BUFSIZE);
				pos_tail -= BUFSIZE * 2;
			}
			waveInReset(hWaveIn);//中止录音 
		}
		delete pBuffer1;
		});
	chart->Start();
	chart->Stop();
}