#ifndef MAX30100_FILTERS_H_
#define MAX30100_FILTERS_H_

// http://www.schwietering.com/jayduino/filtuino/
// Low pass butterworth filter order=1 alpha1=0.1
// Fs=100Hz, Fc=6Hz
float v[2];

float butterworthStep(float x) //class II
{
	v[0] = v[1];
	v[1] = (2.452372752527856026e-1 * x)
		 + (0.50952544949442879485 * v[0]);
	return
		 (v[0] + v[1]);
}	

// http://sam-koblenski.blogspot.de/2015/11/everyday-dsp-for-programmers-dc-and.html
float alphaRed, alphaIr;
float dcwRed = 0, dcwIr = 0;

void setDCAlpha(float alpha_, char c)
{
	switch(c)
	{
		case 'R':
			alphaRed = alpha_;
			break;
		case 'I':
			alphaIr = alpha_;
			break;
		default:
			break;
	}
}

float getDCW(char c)
{
	switch(c)
	{
		case 'R':
			return dcwRed;
		case 'I':
			return dcwIr;
		default:
			return 0;
	}
}

float dcStepRed(float xRed)
{
	float olddcwRed = dcwRed;
	dcwRed = (float)xRed + alphaRed * dcwRed;

	return dcwRed - olddcwRed;
}

float dcStepIr(float xIr)
{
	float olddcwIr = dcwIr;
	dcwIr = (float)xIr + alphaIr * dcwIr;

	return dcwIr - olddcwIr;
}

#endif /* MAX30100_FILTERS_H_ */
