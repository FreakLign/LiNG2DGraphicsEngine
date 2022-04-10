#pragma once
#include <ipps.h>
#include <math.h>
#include <Windows.h>

typedef enum { WinNone = -1, WinBartlett, WinBlackman, WinHamming, WinHann, WinKaiser } enumWinType;
typedef enum { DT_float, DT_float_comp } enumDataType;

constexpr double f20ln10 = 8.685889638f;
constexpr double M_2PI = 6.28318530717958647692;
constexpr double M_PI = 6.28318530717958647692 / 2;

typedef int BOOL;

typedef struct
{
	int Len;
	enumWinType WinType;
	float WinAlpha;
	float* Win;
	Ipp32fc* Winfc;
	Ipp32fc* tData1;
	Ipp32fc* tData2;
	Ipp8u* WorkBuffer;
	Ipp8u* pSpecBuffer_C, * pSpecBuffer_R;
	IppsFFTSpec_C_32fc* FFTSpec_C;
	IppsDFTSpec_C_32fc* DFTSpec_C;
	IppsFFTSpec_R_32f* FFTSpec_R;
	IppsDFTSpec_R_32f* DFTSpec_R;

}strDFTSpec;

template <class T> struct COMPLEX
{
public:
	T re;
	T im;
};

typedef COMPLEX<short> sc16;
typedef COMPLEX<int> sc32;
typedef COMPLEX<float> fc32;

class CDFT
{
public:
	CDFT();
	~CDFT();
	BOOL Init(int Len, enumWinType WinType, float WinAlpha, BOOL DevideByN);
	void DFT(fc32* Data, fc32* FFTOut, float* Phase, float* Amp, float AmpPlus);
	void DFT(sc16* Data, fc32* FFTOut, float* Phase, float* Amp, float AmpPlus);
	void DFT(float* Data, fc32* FFTOut, float* Phase, float* Amp, float AmpPlus);
	void DFT(short* Data, fc32* FFTOut, float* Phase, float* Amp, float AmpPlus);
private:
	strDFTSpec m_DFTSpec;
	void Rlease();
};

