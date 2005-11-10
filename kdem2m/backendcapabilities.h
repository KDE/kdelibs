/*  This file is part of the KDE project
    Copyright (C) 2005 Matthias Kretz <kretz@kde.org>

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

#ifndef Kdem2m_BACKENDCAPABILITIES_H
#define Kdem2m_BACKENDCAPABILITIES_H

#include <QObject>

#include <kstaticdeleter.h>
#include <kmimetype.h>

namespace Kdem2m
{

class BackendCapabilities : public QObject
{
	friend void ::KStaticDeleter<BackendCapabilities>::destructObject();

	Q_OBJECT
	public:
		/**
		 * Use this function to get an instance of Factory.
		 *
		 * @return a pointer to the BackendCapabilities instance. If no instance exists until now then
		 * one is created
		 */
		static BackendCapabilities* self();

		bool supportsVideo() const;
		bool supportsOSD() const;
		bool supportsSubtitles() const;
		KMimeType::List knownMimeTypes() const;
		QStringList availableSoundcardCaptureTypes() const;
		QStringList availableAudioEffects() const;
		QStringList availableVideoEffects() const;

	signals:
		/**
		 * This signal is emitted if the capabilites have changed. This can
		 * happen if the user has requested a backend change.
		 */
		void capabilitesChanged();

	protected:
		BackendCapabilities();
		~BackendCapabilities();

	private slots:
		void slotBackendChanged();

	private:
		static BackendCapabilities* m_self;
		class Private;
		Private* d;
};
} // namespace Kdem2m

#endif // Kdem2m_BACKENDCAPABILITIES_H
// vim: sw=4 ts=4 tw=80 noet
