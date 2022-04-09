#include <Windows.h>
#include <mmsystem.h>
#include <mmreg.h>
#include <conio.h>
#include "BasicChart.h"
#include <mutex>
#include <ipps.h>

#pragma comment(lib, "winmm.lib")


BasicChart* chart;

class DataReceiver {
private:
	mutex m;
public:
	// Inherited via ICapturedAudioDataReceiver
	void OnReceiveAudioDataStreamChunk(unsigned char* audioData, unsigned long lengthInBytes)
	{
		float* dataArr = new float[lengthInBytes];
		for (size_t i = 0; i < lengthInBytes / 2; i++)
		{
			dataArr[2 * i] = float((int)(i)-(int)(lengthInBytes) / 4) / float(lengthInBytes / 4);
			dataArr[2 * i + 1] = float(((short*)audioData)[i]) / float(MAXSHORT);
		}
		chart->InputData(dataArr, lengthInBytes / 4);
		delete[] dataArr;
	}
};

int main() {
	HANDLE          wait;
	HWAVEIN hWaveIn;  //输入设备  
	WAVEFORMATEX waveform; //采集音频的格式，结构体  
	char* pBuffer1;//采集音频时的数据缓存  
	WAVEHDR wHdr1; //采集音频时包含数据缓存的结构体
	waveform.wFormatTag = WAVE_FORMAT_PCM;//声音格式为PCM  
	waveform.nSamplesPerSec = 25600;//采样率，16000次/秒  
	waveform.wBitsPerSample = 16;//采样比特，16bits/次  
	waveform.nChannels = 1;//采样声道数，2声道  
	waveform.nAvgBytesPerSec = 51200;//每秒的数据率，就是每秒能采集多少字节的数据  
	waveform.nBlockAlign = 2;//一个块的大小，采样bit的字节数乘以声道数  
	waveform.cbSize = 0;//通常为0 
	wait = CreateEvent(NULL, 0, 0, NULL);
	//使用waveInOpen函数开启音频采集  
	waveInOpen(&hWaveIn, WAVE_MAPPER, &waveform, (DWORD_PTR)wait, 0L, CALLBACK_EVENT);
#define BUFSIZE 8192
	//创建两个数组（这里能够创建多个数组）用来缓冲音频数据  
	DWORD bufsize = BUFSIZE;//每次开辟10k的缓存存储录音数据  
	thread t = thread([&] {
		pBuffer1 = new char[BUFSIZE * 4];
		wHdr1.dwBufferLength = BUFSIZE * 2;
		wHdr1.dwBytesRecorded = 0;
		wHdr1.dwUser = 0;
		wHdr1.dwFlags = 0;
		wHdr1.dwLoops = 1;
		int pos_tail = 0;
		float* fIndex = new float[BUFSIZE];
		float* fDatas = new float[BUFSIZE * 2];
		for (size_t i = 0; i < BUFSIZE; i++)
		{
			fIndex[i] = (float(i) - float(BUFSIZE) / 2.f) / (float(BUFSIZE) / 2.f);
		}
		while (true) {
			wHdr1.lpData = (LPSTR)pBuffer1 + pos_tail;
			waveInPrepareHeader(hWaveIn, &wHdr1, sizeof(WAVEHDR));//准备一个波形数据块头用于录音  
			waveInAddBuffer(hWaveIn, &wHdr1, sizeof(WAVEHDR));//指定波形数据块为录音输入缓存  
			waveInStart(hWaveIn);//开始录音  
			Sleep(10);
			cout << wHdr1.dwBytesRecorded << endl;
			pos_tail += wHdr1.dwBytesRecorded;
			if (pos_tail >= BUFSIZE * 2) {
				float* datIm = new float[BUFSIZE];
				ippsConvert_16s32f((short*)(pBuffer1), datIm, BUFSIZE);
				ippsDivC_32f_I(float(BUFSIZE) / 2.f, datIm, BUFSIZE);
				ippsRealToCplx_32f(fIndex, datIm, (Ipp32fc*)fDatas, BUFSIZE);
				ippsCopy_16s((short*)(pBuffer1 + BUFSIZE * 2), (short*)pBuffer1, (pos_tail - BUFSIZE * 2) / 2);
				pos_tail -= BUFSIZE * 2;
				chart->InputData(fDatas, BUFSIZE);
			}
			waveInReset(hWaveIn);//中止录音 
		}
		delete pBuffer1;
		});

	chart = new BasicChart(1200, 400, "Test");
	int datCount = BUFSIZE;
	chart->SetVisualParas(datCount);
	chart->Start();
	chart->Stop();
}