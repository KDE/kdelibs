    /*

    Copyright (C) 2000 Stefan Westerfeld
                       stefan@space.twc.de

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.
  
    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.

    */

#include <math.h>
#include "artsflow.h"
#include "stdsynthmodule.h"

using namespace Arts;

namespace Arts {

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
	void scaleFactor(float newFactor) {
		if(_scaleFactor != newFactor)
		{
			_scaleFactor = newFactor;
			scaleFactor_changed(newFactor);
		}
	}
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
		}
	}

	AutoSuspendState autoSuspend() {
		return (_currentVolumeLeft+_currentVolumeRight<0.001)? 
			asSuspend : asNoSuspend;
	}

};

REGISTER_IMPLEMENTATION(StereoVolumeControl_impl);

};
