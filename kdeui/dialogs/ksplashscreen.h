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

#ifndef KSPLASHSCREEN_H
#define KSPLASHSCREEN_H

#include <kdeui_export.h>

#include <QtGui/QSplashScreen>

class QPixmap;

/**
 *  @short %KDE splash screen
 *
 *  This class is based on QSplashScreen and exists solely to make
 *  splash screens obey KDE's Xinerama settings.
 *
 *  For documentation on how to use the class, see the documentation
 *  for QSplashScreen.
 *
 * \image html ksplashscreen.png "KDE Splash Screen"
 *
 *  @author Chris Howells (howells@kde.org)
 */
class KDEUI_EXPORT KSplashScreen : public QSplashScreen //krazy:exclude=qclasses
{
  Q_OBJECT

public:

  /**
   *  Constructs a splash screen.
   */
  explicit KSplashScreen(const QPixmap &pixmap, Qt::WFlags f = 0);

  /**
   *  Destructor.
   *
   *  Deletes all internal objects.
   */
  ~KSplashScreen();

private:
    class Private;
    Private* const d;
    Q_DISABLE_COPY( KSplashScreen )
};

#endif //KSPLASHSCREEN_H

