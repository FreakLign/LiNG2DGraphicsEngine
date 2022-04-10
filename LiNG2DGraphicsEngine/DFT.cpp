#include "DFT.h"

#define SAFE_DELETE(value) if(value) ippsFree(value)

#define ippsFFTInitAlloc_C_32fc ippsFFTInit_C_32fc
#define ippsFFTInitAlloc_R_32f ippsFFTInit_R_32fc
#define ippsDFTInitAlloc_C_32fc ippsDFTInit_C_32fc
#define ippsDFTInitAlloc_R_32f ippsDFTInit_R_32f

#include <math.h>
template<typename T> int rectangle(int n, T Amp, T * WndBuff)
{
	return 0;
}
template<typename T> int bartlett(int n, T Amp, T * WndBuff)
{
	return 0;
}
template<typename T> int hanning(int n, T Amp, T * WndBuff)
{
	return 0;
}
template<typename T> int hamming(int n, T Amp, T * WndBuff)
{
	return 0;
}
template<typename T> int blackman(int n, T Amp, T * WndBuff)
{
	return 0;
}
#define MAXTERM 200
#define EPS 0.001
double I0(double alpha)
{
	double  J = 1.0, K = alpha / 2.0, iOld = 1.0, iNew;
	bool    converge = false;

	// Use series expansion definition of Bessel.
	for (int i = 1; i < MAXTERM; ++i)
	{
		J *= K / i;
		iNew = iOld + J * J;

		if ((iNew - iOld) < EPS)
		{
			converge = true;
			break;
		}
		iOld = iNew;
	}

	if (!converge)
		return double(0);
	return double(iNew);
}
template<typename T> int gauss(int n, T Amp, T * WndBuff, float param)
{
	return 0;
}
void kaiser(int N, float amp, double* win, float alpha)
{
	for (int i = 0; i < (N + 1) / 2; ++i)
	{
		double beta = 2 * alpha * sqrt(i * (N - i - 1.0)) / (N - 1.0);
		win[i] = I0(beta) / I0(alpha);
		win[N - 1 - i] = win[i];
	}
	double Sum = 0;
	for (int i = 0; i < N; i++)
		Sum += win[i];
	Sum /= N;
	Sum *= 4;
	for (int i = 0; i < N; i++)
		win[i] = win[i] * amp / Sum;
}
int kaiser(int N, float amp, short* Data, float alpha)
{
	double* win = new double[N];
	kaiser(N, amp, win, alpha);
	for (int i = 0; i < N; i++)
		Data[i] *= win[i];
	delete win;
	return 0;
}
int kaiser(int N, float amp, float* Data, float alpha)
{
	double* win = new double[N];
	kaiser(N, amp, win, alpha);
	for (int i = 0; i < N; i++)
		Data[i] *= win[i];
	delete win;
	return 0;
}
int kaiser(int N, float amp, sc16 * Data, float alpha)
{
	double* win = new double[N];
	kaiser(N, amp, win, alpha);
	for (int i = 0; i < N; i++)
	{
		Data[i].re *= win[i];
		Data[i].im *= win[i];
	}
	delete win;
	return 0;
}
int kaiser(int N, float amp, fc32 * Data, float alpha)
{
	double* win = new double[N];
	kaiser(N, amp, win, alpha);
	for (int i = 0; i < N; i++)
	{
		Data[i].re *= win[i];
		Data[i].im *= win[i];
	}
	delete win;
	return 0;
}

CDFT::CDFT()
{
	ZeroMemory(&m_DFTSpec, sizeof(m_DFTSpec));
}


CDFT::~CDFT()
{
	Rlease();
}
void CDFT::Rlease()
{
	SAFE_DELETE(m_DFTSpec.WorkBuffer);
	SAFE_DELETE(m_DFTSpec.tData2);
	SAFE_DELETE(m_DFTSpec.tData1);
	SAFE_DELETE(m_DFTSpec.Win);
	SAFE_DELETE(m_DFTSpec.Winfc);
	if (m_DFTSpec.DFTSpec_C)  ippsFree(m_DFTSpec.DFTSpec_C);
	if (m_DFTSpec.FFTSpec_C)  ippsFree(m_DFTSpec.FFTSpec_C);
	if (m_DFTSpec.DFTSpec_R)  ippsFree(m_DFTSpec.DFTSpec_R);
	if (m_DFTSpec.FFTSpec_R)  ippsFree(m_DFTSpec.FFTSpec_R);

	if (m_DFTSpec.pSpecBuffer_C)  ippsFree(m_DFTSpec.pSpecBuffer_C);
	if (m_DFTSpec.pSpecBuffer_R)  ippsFree(m_DFTSpec.pSpecBuffer_R);

	ZeroMemory(&m_DFTSpec, sizeof(m_DFTSpec));

}


BOOL CDFT::Init(int Len, enumWinType WinType, float WinAlpha, BOOL DevideByN)
{
	Rlease();
	m_DFTSpec.Len = Len;
	m_DFTSpec.WinType = WinType;
	m_DFTSpec.WinAlpha = WinAlpha;
	m_DFTSpec.tData1 = new Ipp32fc[Len];
	m_DFTSpec.tData2 = new Ipp32fc[Len];
	int Order = int(log(Len) / log(2) + 0.5);
	BOOL bFFT = Len == (1 << Order);
	int SpecSize, SpecBufferSize, BufferSize;
	int SpecSize2, SpecBufferSize2, BufferSize2;
	IppStatus a;
	if (bFFT)
	{
		ippsFFTGetSize_C_32fc(Order, DevideByN ? IPP_FFT_DIV_FWD_BY_N : IPP_FFT_NODIV_BY_ANY, ippAlgHintAccurate, &SpecSize, &SpecBufferSize, &BufferSize);
		ippsFFTGetSize_R_32f(Order, DevideByN ? IPP_FFT_DIV_FWD_BY_N : IPP_FFT_NODIV_BY_ANY, ippAlgHintAccurate, &SpecSize2, &SpecBufferSize2, &BufferSize2);
		Ipp8u* pSpec_C = ippsMalloc_8u(SpecSize);
		m_DFTSpec.pSpecBuffer_C = ippsMalloc_8u(SpecBufferSize);
		Ipp8u* pSpec_R = ippsMalloc_8u(SpecSize2);
		m_DFTSpec.pSpecBuffer_R = ippsMalloc_8u(SpecBufferSize2);
		m_DFTSpec.WorkBuffer = new Ipp8u[BufferSize > BufferSize2 ? BufferSize : BufferSize2];
		a = ippsFFTInit_C_32fc(&m_DFTSpec.FFTSpec_C, Order, DevideByN ? IPP_FFT_DIV_FWD_BY_N : IPP_FFT_NODIV_BY_ANY, ippAlgHintAccurate, pSpec_C, m_DFTSpec.pSpecBuffer_C);
		a = ippsFFTInit_R_32f(&m_DFTSpec.FFTSpec_R, Order, DevideByN ? IPP_FFT_DIV_FWD_BY_N : IPP_FFT_NODIV_BY_ANY, ippAlgHintAccurate, pSpec_R, m_DFTSpec.pSpecBuffer_R);

	}
	else
	{
		ippsDFTGetSize_C_32fc(Len, DevideByN ? IPP_FFT_DIV_FWD_BY_N : IPP_FFT_NODIV_BY_ANY, ippAlgHintAccurate, &SpecSize, &SpecBufferSize, &BufferSize);
		ippsDFTGetSize_R_32f(Len, DevideByN ? IPP_FFT_DIV_FWD_BY_N : IPP_FFT_NODIV_BY_ANY, ippAlgHintAccurate, &SpecSize2, &SpecBufferSize2, &BufferSize2);
		m_DFTSpec.DFTSpec_C = (IppsDFTSpec_C_32fc*)ippsMalloc_8u(SpecSize);
		m_DFTSpec.pSpecBuffer_C = ippsMalloc_8u(SpecBufferSize);
		m_DFTSpec.DFTSpec_R = (IppsDFTSpec_R_32f*)ippsMalloc_8u(SpecSize2);
		m_DFTSpec.pSpecBuffer_R = ippsMalloc_8u(SpecBufferSize2);
		m_DFTSpec.WorkBuffer = new Ipp8u[BufferSize > BufferSize2 ? BufferSize : BufferSize2];
		a = ippsDFTInit_C_32fc(Len, DevideByN ? IPP_FFT_DIV_FWD_BY_N : IPP_FFT_NODIV_BY_ANY, ippAlgHintAccurate, m_DFTSpec.DFTSpec_C, m_DFTSpec.pSpecBuffer_C);
		a = ippsDFTInit_R_32f(Len, DevideByN ? IPP_FFT_DIV_FWD_BY_N : IPP_FFT_NODIV_BY_ANY, ippAlgHintAccurate, m_DFTSpec.DFTSpec_R, m_DFTSpec.pSpecBuffer_R);
	}
	if (m_DFTSpec.WinType >= 0 && m_DFTSpec.WinType <= 4)
	{
		m_DFTSpec.Win = new float[Len];
		m_DFTSpec.Winfc = new Ipp32fc[Len];
		ippsSet_32f(2, m_DFTSpec.Win, Len);
		ippsSet_32fc(Ipp32fc{ 2,0 }, m_DFTSpec.Winfc, Len);
		switch (m_DFTSpec.WinType)
		{
		case WinBartlett:
			ippsWinBartlett_32fc_I(m_DFTSpec.Winfc, Len);
			ippsWinBartlett_32f_I(m_DFTSpec.Win, Len);
			break;
		case WinBlackman:
			ippsWinBlackman_32fc_I(m_DFTSpec.Winfc, Len, WinAlpha);
			ippsWinBlackman_32f_I(m_DFTSpec.Win, Len, WinAlpha);
			break;
		case WinHamming:
			ippsWinHamming_32fc_I(m_DFTSpec.Winfc, Len);
			ippsWinHamming_32f_I(m_DFTSpec.Win, Len);
			break;
		case WinHann:
			ippsWinHann_32fc_I(m_DFTSpec.Winfc, Len);
			ippsWinHann_32f_I(m_DFTSpec.Win, Len);
			break;
		case WinKaiser:
			kaiser(Len, 2, (fc32*)m_DFTSpec.Winfc, WinAlpha);
			kaiser(Len, 2, m_DFTSpec.Win, WinAlpha);
			break;
		}
	}
	return TRUE;
}
void CDFT::DFT(fc32* Data, fc32* FFTOut, float* Phase, float* Amp, float AmpPlus)
{
	Ipp32fc* pDataIn;
	if (m_DFTSpec.WinType >= 0 && m_DFTSpec.WinType <= 4)
	{
		pDataIn = m_DFTSpec.tData1;
		ippsMul_32fc((Ipp32fc*)Data, m_DFTSpec.Winfc, m_DFTSpec.tData1, m_DFTSpec.Len);
	}
	else
		pDataIn = (Ipp32fc*)Data;

	if (m_DFTSpec.FFTSpec_C)
		ippsFFTFwd_CToC_32fc(pDataIn, m_DFTSpec.tData2, m_DFTSpec.FFTSpec_C, m_DFTSpec.WorkBuffer);
	else
		ippsDFTFwd_CToC_32fc(pDataIn, m_DFTSpec.tData2, m_DFTSpec.DFTSpec_C, m_DFTSpec.WorkBuffer);
	Ipp32fc* pFFTOut = (FFTOut == NULL) ? m_DFTSpec.tData1 : (Ipp32fc*)FFTOut;
	CopyMemory(pFFTOut, m_DFTSpec.tData2 + m_DFTSpec.Len / 2, sizeof(Ipp32fc) * m_DFTSpec.Len / 2);
	CopyMemory(pFFTOut + m_DFTSpec.Len / 2, m_DFTSpec.tData2, sizeof(Ipp32fc) * m_DFTSpec.Len / 2);
	if (Amp)
	{
		ippsMagnitude_32fc((Ipp32fc*)pFFTOut, Amp, m_DFTSpec.Len);
		ippsLn_32f_I(Amp, m_DFTSpec.Len);
		ippsMulC_32f_I(f20ln10, Amp, m_DFTSpec.Len);
		//ippsAddC_32f_I(float(AmpPlus + 10 * log10(m_DFTSpec.Len)), Amp, m_DFTSpec.Len);//将因为分辨率下降导致的噪声下降加回去，让噪声恒定为0dB
		ippsAddC_32f_I(float(AmpPlus), Amp, m_DFTSpec.Len);//将因为分辨率下降导致的噪声下降加回去，让噪声恒定为0dB
	}
	if (Phase)
	{
		ippsPhase_32fc((Ipp32fc*)pFFTOut, Phase, m_DFTSpec.Len);
	}
}
void CDFT::DFT(float* Data, fc32* FFTOut, float* Phase, float* Amp, float AmpPlus)
{
	Ipp32f* pDataIn;
	if (m_DFTSpec.WinType >= 0 && m_DFTSpec.WinType <= 4)
	{
		pDataIn = (Ipp32f*)m_DFTSpec.tData1;
		ippsMul_32f((Ipp32f*)Data, m_DFTSpec.Win, (Ipp32f*)m_DFTSpec.tData1, m_DFTSpec.Len);
	}
	else
		pDataIn = (Ipp32f*)Data;

	if (m_DFTSpec.FFTSpec_C)
		ippsFFTFwd_RToCCS_32f(pDataIn, (Ipp32f*)m_DFTSpec.tData2, m_DFTSpec.FFTSpec_R, m_DFTSpec.WorkBuffer);
	else
		ippsDFTFwd_RToCCS_32f(pDataIn, (Ipp32f*)m_DFTSpec.tData2, m_DFTSpec.DFTSpec_R, m_DFTSpec.WorkBuffer);
	int LenDiv2 = m_DFTSpec.Len / 2;
	Ipp32fc* pFFTOut = (FFTOut == NULL) ? m_DFTSpec.tData1 : (Ipp32fc*)FFTOut;
	CopyMemory(pFFTOut, m_DFTSpec.tData2, sizeof(Ipp32fc) * LenDiv2);
	ippsMulC_32f_I(2, (Ipp32f*)pFFTOut + 1, m_DFTSpec.Len - 1);//双边功率谱 
	if (Amp)
	{
		ippsMagnitude_32fc((Ipp32fc*)pFFTOut, Amp, LenDiv2);
		ippsLn_32f_I(Amp, LenDiv2);
		ippsMulC_32f_I(f20ln10, Amp, LenDiv2);
		ippsAddC_32f_I(AmpPlus, Amp, LenDiv2);
	}
	if (Phase)
	{
		ippsPhase_32fc((Ipp32fc*)pFFTOut, Phase, LenDiv2);
	}
}
void CDFT::DFT(short* Data, fc32* FFTOut, float* Phase, float* Amp, float AmpPlus)
{
	float* fData = new float[m_DFTSpec.Len];
	ippsConvert_16s32f((Ipp16s*)Data, (Ipp32f*)fData, m_DFTSpec.Len);
	DFT(fData, FFTOut, Phase, Amp, AmpPlus);
	delete fData;
}
void CDFT::DFT(sc16* Data, fc32* FFTOut, float* Phase, float* Amp, float AmpPlus)
{
	fc32* fData = new fc32[m_DFTSpec.Len];
	ippsConvert_16s32f((Ipp16s*)Data, (Ipp32f*)fData, m_DFTSpec.Len * 2);
	DFT(fData, FFTOut, Phase, Amp, AmpPlus);
	delete fData;
}