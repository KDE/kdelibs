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
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.

*/

#ifndef Phonon_FAKE_UI_BACKEND_H
#define Phonon_FAKE_UI_BACKEND_H

#include <QObject>
#include <kdelibs_export.h>
#include <phonon/ifaces/ui/backend.h>

namespace Phonon
{
namespace Ui
{
namespace Ifaces
{
	class VideoWidget;
}
namespace Fake
{
	class PHONON_EXPORT Backend : public Ui::Ifaces::Backend
	{
		Q_OBJECT
		public:
			Backend( QObject* parent, const char*, const QStringList& args );
			virtual ~Backend();

			virtual Ifaces::VideoWidget*  createVideoWidget( QWidget* parent );
	};
}}} // namespace Phonon::Ui::Fake

// vim: sw=4 ts=4 noet tw=80
#endif // Phonon_FAKE_UI_BACKEND_H
