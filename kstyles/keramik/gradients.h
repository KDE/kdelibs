/* Keramik Style for KDE3, gradient routines..
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
// $Id: $

#ifndef KERAMIK_GRADIENTS_H
#define KERAMIK_GRADIENTS_H

class QPainter;


namespace Keramik
{
	class GradientPainter
	{
	public:
		static void renderGradient( QPainter* p, const QRect& r, QColor cr, bool horizontal, bool menu = false);
	};
};

#endif
