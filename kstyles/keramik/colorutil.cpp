/* Keramik Style for KDE3, color utility routines..
   Copyright (c) 2002 Malte Starostik <malte@kde.org>
                  (c) 2002 Maksim Orlovich <mo002j@mail.rochester.edu>

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
// $Id:$

#include <qcolor.h>

#include "colorutil.h"

namespace Keramik
{
	QColor ColorUtil::lighten(QColor in, int factor)
	{
		if (factor > 100)
		{
			int h, s, v;
			in.hsv(&h, &s, &v);
			
			float mShare = v/230.0;
			if (mShare > 1) mShare = 1;
			
			mShare *= mShare;
			
			int diff = factor - 100;
			int hd  = int(mShare*diff);
			int delta  =  int((diff - hd)*7.55);
			
			QColor wrk = in.light(100+hd);
			
			int r = wrk.red();
			int g = wrk.green();
			int b = wrk.blue();

			r+=delta;
			g+=delta;
			b+=delta;
			
			if (r>255) r=255;
			if (g>255) g=255;
			if (b>255) b=255;
			
			return QColor(r,g,b);
		}
		
		return in;
	}
};

// vim: ts=4 sw=4 noet
