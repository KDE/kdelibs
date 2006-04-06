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

#ifndef Phonon_IFACES_UI_BACKEND_H
#define Phonon_IFACES_UI_BACKEND_H

#include <QObject>
#include <kdelibs_export.h>

namespace Phonon
{
namespace Ui
{
/**
 * \short The interfaces that depend on a GUI.
 *
 * This class is currently only used to create new VideoWidget instances.
 *
 * \see Phonon::Ifaces
 * \author Matthias Kretz <kretz@kde.org>
 */
namespace Ifaces
{
	class VideoWidget;

	/**
	 * \brief Central class for all GUI dependent parts of Phonon backends.
	 *
	 * Exactly one instance of this class is created for the Phonon fronted
	 * class which utilizes it to create your VideoWidget implementation.
	 *
	 * \author Matthias Kretz <kretz@kde.org>
	 */
	class PHONONUI_EXPORT Backend : public QObject
	{
		Q_OBJECT
		public:
			/**
			 * Standard QObject constructor.
			 */
			Backend( QObject* parent = 0 );
			virtual ~Backend();

			/**
			 * Creates an instance of VideoWidget
			 */
			virtual VideoWidget*  createVideoWidget( QWidget* parent ) = 0;

		private:
			class Private;
			Private * d;
	};
}}} // namespace Phonon::Ifaces::Ui

// vim: sw=4 ts=4 noet tw=80
#endif // Phonon_IFACES_UI_BACKEND_H
