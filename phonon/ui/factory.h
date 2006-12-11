/*  This file is part of the KDE project
    Copyright (C) 2004-2006 Matthias Kretz <kretz@kde.org>

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

#ifndef Phonon_UiFACTORY_H
#define Phonon_UiFACTORY_H

#include <QObject>
#include <kdelibs_export.h>

class KUrl;

namespace Phonon
{

/**
 * \brief Factory to access the GUI dependent part of the Backend.
 *
 * This class is used internally to map the backends functionality to the API.
 *
 * \remarks
 * Extensions to the existing functionality can be added by adding a new interface e.g.
 * BackendV2 and creating a BackendV2 instance when the Backend instance is
 * created.
 *
 * \author Matthias Kretz <kretz@kde.org>
 * \internal
 */
class PHONONUI_EXPORT UiFactory : public QObject
{
	Q_OBJECT
	public:
		/**
		 * Returns a pointer to the factory.
		 * Use this function to get an instance of UiFactory.
		 *
		 * @return a pointer to the factory. If no factory exists until now then
		 * one is created
		 */
		static UiFactory* self();

        /**
         * Create a new backend object for a VideoWidget.
         *
         * \return a pointer to the VideoWidget the backend provides.
         */
		QObject* createVideoWidget( QWidget* parent = 0 );

		/**
		 * \return a pointer to the backend interface.
		 */
		QObject* backend() const;

	Q_SIGNALS:
		/**
		 * Emitted after the backend has successfully been changed.
		 */
		void backendChanged();

	protected:
		/**
		 * \internal
		 * Singleton constructor
		 */
		UiFactory();
		~UiFactory();

	private Q_SLOTS:
		/**
		 * deletes itself - called from the destructor of Phonon::Factory
		 */
		void deleteNow();

	private:
		static UiFactory * m_self;
		class Private;
		Private * d;
};
} // namespace Phonon

#endif // Phonon_UiFACTORY_H
// vim: sw=4 ts=4 tw=80
