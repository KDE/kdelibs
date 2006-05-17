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

#ifndef PHONON_FAKE_VISUALIZATION_H
#define PHONON_FAKE_VISUALIZATION_H

#include <QObject>
#include <phonon/ifaces/visualization.h>

namespace Phonon
{
namespace Ifaces
{
	class AudioPath;
	class AbstractVideoOutput;
}
namespace Fake
{
	class AudioPath;
	class AbstractVideoOutput;

class Visualization : public QObject, virtual public Ifaces::Visualization
{
	Q_OBJECT
	public:
		Visualization( QObject* parent = 0 );
		virtual int visualization() const;
		virtual void setVisualization( int newVisualization );
		virtual void setAudioPath( Ifaces::AudioPath* audioPath );
		virtual void setVideoOutput( Ifaces::AbstractVideoOutput* videoOutput );

	public:
		virtual QObject* qobject() { return this; }
		virtual const QObject* qobject() const { return this; }

	private:
		int m_visualization;
		AudioPath* m_audioPath;
		AbstractVideoOutput* m_videoOutput;
};

}} //namespace Phonon::Fake

#endif // PHONON_FAKE_VISUALIZATION_H
