/* This file is part of the KDE libraries
   Copyright (C) 2001 Holger Freyther <freyther@kde.org>

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

class KGuiItem;

#ifndef __kstdguiitem_h
#define __kstdguiitem_h

#include <qstring.h>

class KStdGuiItem
{
 public:
  enum StdItem {
    Ok=1, Cancel, Yes, No, Discard, Save, DontSave, SaveAs,
    Apply, Clear, Help, Defaults, Close, Back, Forward
  };
  static KGuiItem guiItem ( StdItem ui_enum );
  static QString  stdItem ( StdItem ui_enum );
  static KGuiItem ok();
  static KGuiItem cancel();
  static KGuiItem yes();
  static KGuiItem no();
  static KGuiItem discard();
  static KGuiItem save();
  static KGuiItem help();
  static KGuiItem dontSave();
  static KGuiItem saveAs();
  static KGuiItem apply();
  static KGuiItem clear();
  static KGuiItem defaults();
  static KGuiItem close();

  /**
   * Return a GUI item for a 'back' action, like Konqueror's back button.
   * This GUI item by default honours the user's setting for BiDi, so the
   * icon for right-to-left languages (Hebrew and Arab) has the arrow
   * pointing in the opposite direction.
   * If you want the arrow to force in the Western 'back' direction (i.e.
   * to the left), set the boolean useBiDi to false.
   */
  static KGuiItem back( bool useBidi = true );
  /**
   * Return a GUI item for a 'forward' action, like Konqueror's forward button.
   * This GUI item by default honours the user's setting for BiDi, so the
   * icon for right-to-left languages (Hebrew and Arab) has the arrow
   * pointing in the opposite direction.
   * If you want the arrow to force in the Western 'forward' direction (i.e.
   * to the right), set the boolean useBiDi to false.
   */
  static KGuiItem forward( bool useBidi = true );

};

#endif
