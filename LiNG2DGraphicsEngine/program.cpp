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
	HWAVEIN hWaveIn;  //�����豸  
	WAVEFORMATEX waveform; //�ɼ���Ƶ�ĸ�ʽ���ṹ��  
	char* pBuffer1;//�ɼ���Ƶʱ�����ݻ���  
	WAVEHDR wHdr1; //�ɼ���Ƶʱ�������ݻ���Ľṹ��
	waveform.wFormatTag = WAVE_FORMAT_PCM;//������ʽΪPCM  
	waveform.nSamplesPerSec = SAMPLERATE;//�����ʣ�16000��/��  
	waveform.wBitsPerSample = BITS_PER_SAMPLE;//�������أ�16bits/��  
	waveform.nChannels = CHANNEL_COUNT;//������������2����  
	waveform.nAvgBytesPerSec = SAMPLERATE * CHANNEL_COUNT * BITS_PER_SAMPLE / 8;//ÿ��������ʣ�����ÿ���ܲɼ������ֽڵ�����  
	waveform.nBlockAlign = CHANNEL_COUNT * BITS_PER_SAMPLE / 8;//һ����Ĵ�С������bit���ֽ�������������  
	waveform.cbSize = 0;//ͨ��Ϊ0 
	wait = CreateEvent(NULL, 0, 0, NULL);
	//ʹ��waveInOpen����������Ƶ�ɼ�  
	waveInOpen(&hWaveIn, WAVE_MAPPER, &waveform, (DWORD_PTR)wait, 0L, CALLBACK_EVENT);
	//�����������飨�����ܹ�����������飩����������Ƶ����  
	DWORD bufsize = BUFSIZE;//ÿ�ο���10k�Ļ���洢¼������  
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
			waveInPrepareHeader(hWaveIn, &wHdr1, sizeof(WAVEHDR));//׼��һ���������ݿ�ͷ����¼��  
			waveInAddBuffer(hWaveIn, &wHdr1, sizeof(WAVEHDR));//ָ���������ݿ�Ϊ¼�����뻺��  
			waveInStart(hWaveIn);//��ʼ¼��  
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
			waveInReset(hWaveIn);//��ֹ¼�� 
		}
		delete pBuffer1;
		});
	chart->Start();
	chart->Stop();
}