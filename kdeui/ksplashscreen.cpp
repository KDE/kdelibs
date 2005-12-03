/* This file is part of the KDE libraries
    Copyright (C) 2003 Chris Howells (howells@kde.org)

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include <kconfig.h>
#include <kglobal.h>
#include <kglobalsettings.h>

#include <qpixmap.h>

#include "ksplashscreen.h"

KSplashScreen::KSplashScreen(const QPixmap &pixmap, Qt::WFlags f): QSplashScreen(pixmap, f)
{
	QRect desk = KGlobalSettings::splashScreenDesktopGeometry();
	resize(pixmap.width(), pixmap.height());
  setGeometry( ( desk.width() / 2 ) - ( width() / 2 ) + desk.left(),
      ( desk.height() / 2 ) - ( height() / 2 ) + desk.top(),
        width(), height() );

}

KSplashScreen::~KSplashScreen()
{
}

#include "ksplashscreen.moc"
