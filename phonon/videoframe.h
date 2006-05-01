/*  This file is part of the KDE project
    Copyright (C) 2006 Matthias Kretz <kretz@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.

*/

#ifndef PHONON_FRAME_H
#define PHONON_FRAME_H

#include "videodataoutput.h"
#include <QByteArray>

namespace Phonon
{
	class VideoFrame
	{
		public:
			Phonon::VideoDataOutput::Format format;
			QByteArray data;
			int width;
			int height;
			//zrusin: both format, depth and bpp are necessary. eg format could be rgb32, depth
			//        32 and bpp 8 ir format rgb32, depth 24 and bpp 8...
			int depth;
			int bpp;
	};
} // namespace Phonon

// vim: sw=4 ts=4 tw=80 noet
#endif // PHONON_FRAME_H
