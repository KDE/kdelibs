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

#ifndef PHONON_IFACES_VISUALIZATION_H
#define PHONON_IFACES_VISUALIZATION_H

#include "base.h"
#include <QtCore/QtGlobal>

class QWidget;

namespace Phonon
{
namespace Ifaces
{
	class AudioPath;
	class AbstractVideoOutput;

	class Visualization : public Base
	{
		public:
			virtual int visualization() const = 0;
			virtual void setVisualization( int newVisualization ) = 0;
			virtual void setAudioPath( AudioPath* audioPath ) = 0;
			virtual void setVideoOutput( AbstractVideoOutput* videoOutput ) = 0;

			virtual bool hasParameterWidget() const { return false; }
			virtual QWidget* createParameterWidget( QWidget* parent ) { Q_UNUSED( parent ); return 0; }
	};
}} // namespace Phonon::Ifaces

// vim: sw=4 ts=4 tw=80 noet
#endif // PHONON_IFACES_VISUALIZATION_H
