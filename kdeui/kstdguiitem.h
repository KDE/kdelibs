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

class KStdGuiItem
{
 public:
  enum StdItem {
    Ok=1, Cancel, Yes, No, Discard, Save, DontSave, SaveAs,
    Apply,NULL1,
    NULL2, NULL3, NULL4, NULL5, NULL6, NULL7, NULL8, NULL9,
    NULL10, NULL11, NULL12, NULL13, NULL14, NULL15, NULL16,
    NULL17, NULL18, NULL19, NULL20
  };

  KStdGuiItem();
  ~KStdGuiItem();
  static KGuiItem guiitem( StdItem ui_enum);

  static const char* stdItem( StdItem ui_enum );
  static KGuiItem ok( );
  static KGuiItem cancel( );
  static KGuiItem yes();
  static KGuiItem no();
  static KGuiItem discard();
  static KGuiItem save();
  static KGuiItem dontsave();
  static KGuiItem saveas();
  static KGuiItem apply();

 private:
  class KStdGuiItemPrivate;
  KStdGuiItem *d;
};
#endif

