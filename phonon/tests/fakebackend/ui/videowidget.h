/*  This file is part of the KDE project
    Copyright (C) 2005-2006 Matthias Kretz <kretz@kde.org>

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
#ifndef Phonon_UI_FAKE_VIDEOWIDGET_H
#define Phonon_UI_FAKE_VIDEOWIDGET_H

#include <QWidget>
#include <phonon/ui/ifaces/videowidget.h>

class QString;

namespace Phonon
{
namespace Ui
{
namespace Fake
{
	class VideoWidget : public QWidget, virtual public Ui::Ifaces::VideoWidget
	{
		Q_OBJECT
		public:
			VideoWidget( QWidget* parent = 0 );

		public:
			virtual QObject* qobject() { return this; }
			virtual const QObject* qobject() const { return this; }

		private:
			bool m_fullscreen;
	};
}}} //namespace Phonon::Ui::Fake

// vim: sw=4 ts=4 tw=80 noet
#endif // Phonon_UI_FAKE_VIDEOWIDGET_H
