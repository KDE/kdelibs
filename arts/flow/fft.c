#include "gsl/gslfft.h"
#include "fft.h"

void fft_float (
    unsigned  NumSamples,
    int       InverseTransform,
    float    *RealIn,
    float    *ImagIn,
    float    *RealOut,
    float    *ImagOut )
{
	double *ri_in = g_newa(double, NumSamples*4);
	double *ri_out = ri_in + NumSamples*2;
	unsigned int i;

	for(i = 0; i < NumSamples; i++)
	{
		ri_in[2*i] = RealIn[i];
		ri_in[2*i+1] = (ImagIn?ImagIn[i]:0.0);
	}

	if(InverseTransform == 0)
		gsl_power2_fftac (NumSamples, ri_in, ri_out);
	else
		gsl_power2_fftsc (NumSamples, ri_in, ri_out);

	for(i = 0; i < NumSamples; i++)
	{
		RealOut[i] = ri_out[2*i];
		ImagOut[i] = ri_out[2*i+1];
	}
}
