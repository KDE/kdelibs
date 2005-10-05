/*  This file is part of the KDE project
    Copyright (C) 2003 Matthias Kretz <kretz@kde.org>

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

#ifndef KAUDIOMANAGERPLAY_H
#define KAUDIOMANAGERPLAY_H

#include <artsflow.h>
#include <qstring.h>
#include <kdelibs_export.h>

class KArtsServer;
class QString;

/**
 * KDE Wrapper for Arts::Synth_AMAN_PLAY. Use this class to create an entry in
 * the AudioManager - that's the list you see when opening the AudioManager view
 * in artscontrol.
 *
 * @author Matthias Kretz <kretz@kde.org>
 * @since 3.2
 */
class KDE_ARTS_EXPORT KAudioManagerPlay
{
	public:
		KAudioManagerPlay( KArtsServer * server, const QString & title = QString::null );
		~KAudioManagerPlay();

		/**
		 * Returns the internal Arts::Synth_AMAN_PLAY
		 */
		Arts::Synth_AMAN_PLAY amanPlay();

		/**
		 * return true if this == 0 or amanPlay().isNull()
		 *
		 * in essence, ((KDE::PlayObject*)0)->isNull() will not
		 * crash
		 **/
		bool isNull() const;

		/**
		 * Set the name of the output in the AudioManager
		 */
		void setTitle( const QString & title );

		/**
		 * returns the name of the output as it appears in the AudioManager
		 */
		QString title();

		void setAutoRestoreID( const QString & autoRestoreID );
		QString autoRestoreID();

		void start();
		void stop();

	private:
		struct PrivateData {
			Arts::Synth_AMAN_PLAY amanPlay;
			bool started;
		};
		PrivateData* d;
};


#endif // KAUDIOMANAGERPLAY_H

// vim: sw=4 ts=4
