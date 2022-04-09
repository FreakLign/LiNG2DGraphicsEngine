#include <Windows.h>
#include <mmsystem.h>
#include <mmreg.h>
#include <conio.h>
#include "BasicChart.h"
#include <IRR/irrKlang.h>
#include <mutex>

#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "x64/irrKlang.lib")

using namespace irrklang;

BasicChart* chart;

class DataReceiver :public ICapturedAudioDataReceiver {
private:
	mutex m;
public:
	// Inherited via ICapturedAudioDataReceiver
	virtual void OnReceiveAudioDataStreamChunk(unsigned char* audioData, unsigned long lengthInBytes) override
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
	ISoundEngine* engine = createIrrKlangDevice();
	IAudioRecorder* recorder = createIrrKlangAudioRecorder(engine);
	if (!engine || !recorder)
	{
		printf("Could not create audio engine or audio recoder\n");
		return 1;
	}
	DataReceiver* receiver = new DataReceiver();
	chart = new BasicChart(1200, 400, "Test");
	int datCount = 10'000;
	chart->SetVisualParas(datCount);
	recorder->startRecordingCustomHandledAudio(receiver, 9600, irrklang::ESF_S16, 2);
	/*thread t = thread([=] {
		float* dataArr = new float[datCount * 2];
		int looptime = 0;
		for (size_t i = 0; i < datCount; i++)
		{
			dataArr[2 * i] = (float(i) - datCount / 2) / float(datCount / 2);
		}
		while (1) {
			for (size_t i = 0; i < datCount; i++)
			{
				dataArr[2 * i + 1] = sin((float)(i + looptime) * 20.0f / datCount) * 0.5 + (float(rand() * rand() % 1000) / 100000.0f);
			}
			looptime += 100;
			chart->InputData(dataArr, datCount);
		}});*/
	chart->Start();
	chart->Stop();
}