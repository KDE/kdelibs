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

#include "kstdguiitem.h"

#include <kiconloader.h>
#include <kguiitem.h>
#include <klocale.h>

#include <qiconset.h>

KGuiItem KStdGuiItem::guiItem ( StdItem ui_enum )
{
  switch (ui_enum ) {
  case Ok       : return ok();
  case Cancel   : return cancel();
  case Yes      : return yes();
  case No       : return no();
  case Discard  : return discard();
  case Save     : return save();
  case DontSave : return dontSave();
  case SaveAs   : return saveAs();
  case Apply    : return apply();
  default       : return KGuiItem(); 
  };
}

QString KStdGuiItem::stdItem( StdItem ui_enum ) 
{
  switch (ui_enum ) {
  case Ok       : return QString::fromLatin1("ok");
  case Cancel   : return QString::fromLatin1("cancel");
  case Yes      : return QString::fromLatin1("yes");
  case No       : return QString::fromLatin1("no");
  case Discard  : return QString::fromLatin1("discard");
  case Save     : return QString::fromLatin1("save");
  case DontSave : return QString::fromLatin1("dontSave");
  case SaveAs   : return QString::fromLatin1("saveAs");
  case Apply    : return QString::fromLatin1("apply");
  default       : return QString::null; 
  };
}

KGuiItem KStdGuiItem::ok()
{
  return KGuiItem(i18n("&OK"), QIconSet(), "ok", i18n("Accept settings"),
          i18n(""  "If you press the <b>OK<b> button, all changes\n"
                    "you made will be used to proceed ") );
}

KGuiItem KStdGuiItem::cancel()
{
  return KGuiItem(i18n("&Cancel"), QIconSet(), "cancel", i18n("Cancel operation") );
}

KGuiItem KStdGuiItem::yes()
{
  return KGuiItem(i18n("&Yes"), QIconSet(), "yes" );
}

KGuiItem KStdGuiItem::no()
{
  return KGuiItem(i18n("&No"), QIconSet(), "no"  );
}

KGuiItem KStdGuiItem::discard()
{
  return KGuiItem(i18n("&Discard"), QIconSet(), "discard", i18n("Discard changes"),
          i18n("Pressing this button will discard all recent changes") );
}

KGuiItem KStdGuiItem::save()
{
  return KGuiItem(i18n("&Save"), QIconSet(), "save", i18n("Save data") );
}

KGuiItem KStdGuiItem::dontSave()
{
  return KGuiItem(i18n("&Don't save"), QIconSet(), "dontSave", i18n("Don't save data") );
}

KGuiItem KStdGuiItem::saveAs()
{
  return KGuiItem(i18n("Save &As..."), QIconSet(), "saveAs" );
}

KGuiItem KStdGuiItem::apply()
{
  return KGuiItem(i18n("&Apply"), QIconSet(), "apply", i18n("Apply settings"),
          i18n(""
               "When clicking <b>Apply<b>, the settings will be\n"
               "handed over to the program, but the dialog\n"
               "will no be closed. "
               "Use this to try different settings. " ) );
}
