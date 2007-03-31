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

#ifndef ABSTRACTMEDIAPRODUCER_P_H
#define ABSTRACTMEDIAPRODUCER_P_H

#include "abstractmediaproducer.h"
#include "base_p.h"
#include <QHash>
#include <QString>
#include <QMultiMap>

namespace Phonon
{
class AbstractMediaProducerPrivate : public BasePrivate, private BaseDestructionHandler
{
	K_DECLARE_PUBLIC( AbstractMediaProducer )
	PHONON_PRIVATEABSTRACTCLASS
	protected:
		AbstractMediaProducerPrivate()
			: state( Phonon::LoadingState )
			, currentTime( 0 )
            , tickInterval(0),
            videoPaths(),
            audioPaths(),
            selectedAudioStream(),
            selectedVideoStream(),
            selectedSubtitleStream(),
            metaData(),
            errorOverride(false),
            errorString(),
            errorType(Phonon::NormalError)
		{ }

		State state;
		qint64 currentTime;
		qint32 tickInterval;
		QList<VideoPath*> videoPaths;
		QList<AudioPath*> audioPaths;
		QHash<AudioPath*, QString> selectedAudioStream;
		QHash<VideoPath*, QString> selectedVideoStream;
		QHash<VideoPath*, QString> selectedSubtitleStream;
		QMultiMap<QString, QString> metaData;
        bool errorOverride;
        QString errorString;
        ErrorType errorType;

	private:
		void _k_resumePlay();
		void _k_resumePause();
		void _k_metaDataChanged( const QMultiMap<QString, QString>& );

		virtual void phononObjectDestroyed( Base* );
};
} //namespace Phonon

#endif // ABSTRACTMEDIAPRODUCER_P_H
// vim: sw=4 ts=4 tw=80
