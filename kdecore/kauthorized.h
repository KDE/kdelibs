/* This file is part of the KDE libraries
    Copyright (C) 1997 Matthias Kalle Dalheimer (kalle@kde.org)
    Copyright (c) 1998, 1999 KDE Team

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

#ifndef KAUTHORIZED_H
#define KAUTHORIZED_H

#include "kdelibs_export.h"

class KURL;
class QString;
class QStringList;

/**
* Extracted from kapplication (3.x). Kiosk authorization framework
*/
class KDECORE_EXPORT KAuthorized
{
public:

  /**
   * Returns whether a certain action is authorized
   * @param genericAction The name of a generic  action
   * @return true if the action is authorized
   * @todo what are the generic actions?
   */
  static bool authorize(const QString& genericAction);

  /**
   * Returns whether a certain KAction is authorized.
   *
   * @param action The name of a KAction action. The name is prepended
   * with "action/" before being passed to authorize()
   * @return true if the KAction is authorized
   */
  static bool authorizeKAction(const char* action);

  /**
   * Returns whether a certain URL related action is authorized.
   *
   * @param action The name of the action. Known actions are
   *  - list (may be listed (e.g. in file selection dialog)),
   *  - link (may be linked to),
   *  - open (may open) and
   *  - redirect (may be redirected to)
   * @param baseURL The url where the action originates from
   * @param destURL The object of the action
   * @return true when the action is authorized, false otherwise.
   * @since 3.1
   */
  static bool authorizeURLAction(const QString& action, const KURL& baseURL, const KURL& destURL);

  /**
   * Allow a certain URL action. This can be useful if your application
   * needs to ensure access to an application specific directory that may
   * otherwise be subject to KIOSK restrictions.
   * @param action The name of the action.
   * @param baseURL The url where the action originates from
   * @param _destURL The object of the action
   * @since 3.2
   */
  static void allowURLAction(const QString& action, const KURL& baseURL, const KURL&  _destURL);

  /**
   * Returns whether access to a certain control module is authorized.
   *
   * @param menuId identifying the control module, e.g. kde-mouse.desktop
   * @return true if access to the module is authorized, false otherwise.
   * @since 3.2
   */
  static bool authorizeControlModule(const QString& menuId);

  /**
   * Returns whether access to a certain control modules is authorized.
   *
   * @param menuIds list of menu-ids of control module,
   * an example of a menu-id is kde-mouse.desktop.
   * @return Those control modules for which access has been authorized.
   * @since 3.2
   */
  static QStringList authorizeControlModules(const QStringList& menuIds);

};

#endif
