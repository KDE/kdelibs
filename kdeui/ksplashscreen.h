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
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#include <qglobal.h>
#if QT_VERSION >= 0x030200

#ifndef KSPLASHSCREEN_H
#define KSPLASHSCREEN_H

#include <qsplashscreen.h>

class QPixmap;

/**
 *  Display a splash screen.
 *
 *  This class is based on QSplashScreen and exists solely to provide
 *  compatability with KDE's Xinerama implementation.
 * 
 *  For documentation on how to use the class, please see the documentation
 *  for QSplashScreen.
 *
 *  @short KDE splash screen
 *  @author Chris Howells (howells@kde.org)
 *  @version $Id$
 *  @since 3.2
 */
class KSplashScreen : public QSplashScreen
{
  Q_OBJECT
    
public:
  
  /**
   *  Constructs a splash screen.
   */
  KSplashScreen(const QPixmap &pixmap, WFlags f = 0);

  /**
   *  Destructor.
   *
   *  Deletes all internal objects.
   */
  ~KSplashScreen();

};

#endif //KSPLASHSCREEN_H

#endif
