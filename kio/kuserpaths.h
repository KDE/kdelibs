/* This file is part of the KDE project
   Copyright (C) 1998, 1999 Torben Weis <weis@kde.org>
 
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

#ifndef _KUSERPATHS_H_
#define _KUSERPATHS_H_

#include <qstring.h>

/**
 * This is a little service class providing information on user's paths
 * <P>
 * Note: All these paths end with a '/'.
 */
class KUserPaths
{
public:
  /**
   * The path to the desktop directory of the current user.
   */
  static QString desktopPath() { initStatic(); return *s_desktopPath; }

  /**
   * The path to the templates directory of the current user.
   */
  static QString templatesPath() { initStatic(); return *s_templatesPath; }

  /**
   * The path to the autostart directory of the current user.
   */
  static QString autostartPath() { initStatic(); return *s_autostartPath; }

  /**
   * The path to the trash directory of the current user.
   */
  static QString trashPath() { initStatic(); return *s_trashPath; }

private:
  /**
   * reads in all paths from kfmrc
   */
  static void initStatic();

  static QString* s_desktopPath;
  static QString* s_templatesPath;
  static QString* s_autostartPath;
  static QString* s_trashPath;
};

#endif
