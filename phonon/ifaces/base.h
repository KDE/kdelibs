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
#ifndef Phonon_IFACES_BASE_H
#define Phonon_IFACES_BASE_H

#include <kdelibs_export.h>
#include <QtGlobal>
class QObject;

namespace Phonon
{
namespace Ifaces
{
	class BasePrivate;
	class PHONONCORE_EXPORT Base
	{
		Q_DECLARE_PRIVATE( Base )
		public:
			virtual ~Base();

			virtual QObject* qobject() = 0;
			virtual const QObject* qobject() const = 0;

		protected:
			Base();
			Base( BasePrivate& dd );

			BasePrivate* d_ptr;

		private:
			Base( const Base& );
			Base& operator=( const Base& );
	};
}} //namespace Phonon::Ifaces

// vim: sw=4 ts=4 tw=80 noet
#endif // Phonon_IFACES_BASE_H
