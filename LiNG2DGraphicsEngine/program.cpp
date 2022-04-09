#include <Windows.h>
#include <mmsystem.h>
#include <mmreg.h>
#include <conio.h>
#include "BasicChart.h"
#include <mutex>
#include <ipps.h>

#pragma comment(lib, "winmm.lib")

#define BUFSIZE				655360
#define SAMPLERATE			6400
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
	waveform.nSamplesPerSec = SAMPLERATE;//采样率，16000次/秒  
	waveform.wBitsPerSample = BITS_PER_SAMPLE;//采样比特，16bits/次  
	waveform.nChannels = CHANNEL_COUNT;//采样声道数，2声道  
	waveform.nAvgBytesPerSec = SAMPLERATE * CHANNEL_COUNT * BITS_PER_SAMPLE / 8;//每秒的数据率，就是每秒能采集多少字节的数据  
	waveform.nBlockAlign = CHANNEL_COUNT * BITS_PER_SAMPLE / 8;//一个块的大小，采样bit的字节数乘以声道数  
	waveform.cbSize = 0;//通常为0 
	wait = CreateEvent(NULL, 0, 0, NULL);
	//使用waveInOpen函数开启音频采集  
	waveInOpen(&hWaveIn, WAVE_MAPPER, &waveform, (DWORD_PTR)wait, 0L, CALLBACK_EVENT);
	//创建两个数组（这里能够创建多个数组）用来缓冲音频数据  
	DWORD bufsize = BUFSIZE;//每次开辟10k的缓存存储录音数据  
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
			waveInPrepareHeader(hWaveIn, &wHdr1, sizeof(WAVEHDR));//准备一个波形数据块头用于录音  
			waveInAddBuffer(hWaveIn, &wHdr1, sizeof(WAVEHDR));//指定波形数据块为录音输入缓存  
			waveInStart(hWaveIn);//开始录音  
			Sleep(10);
			ippsConvert_16s32f((short*)(pBuffer1), fTempData + BUFSIZE + (pos_tail / 2), wHdr1.dwBytesRecorded / 2);
			//ippsLn_32f_I(fTempData + BUFSIZE + (pos_tail / 2), wHdr1.dwBytesRecorded / 2);
			ippsDivC_32f_I(640.f, fTempData + BUFSIZE + (pos_tail / 2), wHdr1.dwBytesRecorded / 2);
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