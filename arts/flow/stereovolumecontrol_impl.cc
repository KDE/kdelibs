#include <math.h>
#include "artsflow.h"
#include "stdsynthmodule.h"

class StereoVolumeControl_impl : virtual public StereoVolumeControl_skel,
								 virtual public StdSynthModule
{
	float _scaleFactor;
	float _currentVolumeLeft;
	float _currentVolumeRight;
public:
	StereoVolumeControl_impl() :_scaleFactor(1.0), _currentVolumeLeft(0.0),
													_currentVolumeRight(0.0)
	{
		//
	}

	/*attribute float scaleFactor;*/
	void scaleFactor(float newFactor) { _scaleFactor = newFactor; }
	float scaleFactor() { return _scaleFactor; }

	/*
	  readonly attribute float currentVolumeLeft;
	  readonly attribute float currentVolumeRight;
	*/
	float currentVolumeLeft() { return _currentVolumeLeft; }
	float currentVolumeRight() { return _currentVolumeRight; }

	/*
	  in audio stream inleft, inright;
	  out audio stream outleft, outright;
	*/
	void calculateBlock(unsigned long samples)
	{
		unsigned long i;

		/*
		 * make sure that we reach zero sometimes - otherwise there are
		 * performance issues with calculations close to zero
		 */
		if(fabs(_currentVolumeLeft) < 0.001)	_currentVolumeLeft = 0.0;
		if(fabs(_currentVolumeRight) < 0.001)	_currentVolumeRight = 0.0;

		for(i=0;i<samples;i++)
		{
			/* scale */
			outleft[i] = inleft[i] * _scaleFactor;
			outright[i] = inright[i] * _scaleFactor;

			/* measure volume */
			float delta;

				// left
			delta = fabs(outleft[i])-_currentVolumeLeft;
			if(delta > 0.0)
				_currentVolumeLeft += 0.01 * delta;
			else
				_currentVolumeLeft += 0.0003 * delta;

				// right
			delta = fabs(outright[i])-_currentVolumeRight;
			if(delta > 0.0)
				_currentVolumeRight += 0.01 * delta;
			else
				_currentVolumeRight += 0.0003 * delta;

			/* clipping */
			if(outleft[i] > 1.0) outleft[i] = 1.0;
			if(outleft[i] < -1.0) outleft[i] = -1.0;
			if(outright[i] > 1.0) outright[i] = 1.0;
			if(outright[i] < -1.0) outright[i] = -1.0;
		}
	}
};

REGISTER_IMPLEMENTATION(StereoVolumeControl_impl);
