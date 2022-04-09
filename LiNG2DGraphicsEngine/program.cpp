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
	HWAVEIN hWaveIn;  //�����豸  
	WAVEFORMATEX waveform; //�ɼ���Ƶ�ĸ�ʽ���ṹ��  
	char* pBuffer1;//�ɼ���Ƶʱ�����ݻ���  
	WAVEHDR wHdr1; //�ɼ���Ƶʱ�������ݻ���Ľṹ��
	waveform.wFormatTag = WAVE_FORMAT_PCM;//������ʽΪPCM  
	waveform.nSamplesPerSec = 25600;//�����ʣ�16000��/��  
	waveform.wBitsPerSample = 16;//�������أ�16bits/��  
	waveform.nChannels = 1;//������������2����  
	waveform.nAvgBytesPerSec = 51200;//ÿ��������ʣ�����ÿ���ܲɼ������ֽڵ�����  
	waveform.nBlockAlign = 2;//һ����Ĵ�С������bit���ֽ�������������  
	waveform.cbSize = 0;//ͨ��Ϊ0 
	wait = CreateEvent(NULL, 0, 0, NULL);
	//ʹ��waveInOpen����������Ƶ�ɼ�  
	waveInOpen(&hWaveIn, WAVE_MAPPER, &waveform, (DWORD_PTR)wait, 0L, CALLBACK_EVENT);
#define BUFSIZE 8192
	//�����������飨�����ܹ�����������飩����������Ƶ����  
	DWORD bufsize = BUFSIZE;//ÿ�ο���10k�Ļ���洢¼������  
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
			waveInPrepareHeader(hWaveIn, &wHdr1, sizeof(WAVEHDR));//׼��һ���������ݿ�ͷ����¼��  
			waveInAddBuffer(hWaveIn, &wHdr1, sizeof(WAVEHDR));//ָ���������ݿ�Ϊ¼�����뻺��  
			waveInStart(hWaveIn);//��ʼ¼��  
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
			waveInReset(hWaveIn);//��ֹ¼�� 
		}
		delete pBuffer1;
		});

	chart = new BasicChart(1200, 400, "Test");
	int datCount = BUFSIZE;
	chart->SetVisualParas(datCount);
	chart->Start();
	chart->Stop();
}