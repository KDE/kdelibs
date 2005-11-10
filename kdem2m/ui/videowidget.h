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
#ifndef Kdem2m_VIDEOWIDGET_H
#define Kdem2m_VIDEOWIDGET_H

#include <QWidget>
#include <kdelibs_export.h>

class QString;

namespace Kdem2m
{
namespace Ifaces
{
namespace Ui
{
	class VideoWidget;
}
}
namespace Ui
{
	class KDEM2M_EXPORT VideoWidget : public QWidget
	{
		friend class VideoPath;
		Q_OBJECT
		public:
			VideoWidget( QWidget* parent = 0 );
			~VideoWidget();

		protected:
			VideoWidget( Ifaces::Ui::VideoWidget* iface, QWidget* parent );
			virtual bool aboutToDeleteIface();
			virtual Ifaces::Ui::VideoWidget* createIface();
			virtual void setupIface();

		protected slots:
			// when the Factory wants to change the Backend the following slots are used
			void slotDeleteIface();
			void slotCreateIface();

		private:
			Ifaces::Ui::VideoWidget* iface();
			Ifaces::Ui::VideoWidget* m_iface;
			class Private;
			Private* d;
	};
}} //namespace Kdem2m::Ui

// vim: sw=4 ts=4 tw=80 noet
#endif // Kdem2m_VIDEOWIDGET_H
