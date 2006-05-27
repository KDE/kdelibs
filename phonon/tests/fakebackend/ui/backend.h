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

#ifndef Phonon_FAKE_UI_BACKEND_H
#define Phonon_FAKE_UI_BACKEND_H

#include <QObject>
#include <kdelibs_export.h>
#include <phonon/ui/ifaces/backend.h>

namespace Phonon
{
namespace Ifaces
{
	class VideoWidget;
}
namespace Fake
{
	class PHONON_FAKEUI_EXPORT UiBackend : public Ifaces::UiBackend
	{
		Q_OBJECT
		public:
			UiBackend( QObject* parent, const QStringList& args );
			virtual ~UiBackend();

			virtual Ifaces::VideoWidget*  createVideoWidget( QWidget* parent );
	};
}} // namespace Phonon::Fake

// vim: sw=4 ts=4 noet tw=80
#endif // Phonon_FAKE_UI_BACKEND_H
