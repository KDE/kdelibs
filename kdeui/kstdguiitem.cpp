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

#include <kaction.h>
#include <kiconloader.h>
#include <kguiitem.h>
#include <klocale.h>

#include <qiconset.h>

KStdGuiItem::KStdGuiItem()
{
}

KStdGuiItem::~KStdGuiItem()
{
}
KGuiItem *KStdGuiItem::guiitem( StdItem ui_enum )
{
  KGuiItem *item;
  switch(ui_enum ) 
  {
  case Ok:
    item = ok();
    break;
  case Cancel:
    item = cancel();
    break;
  case Yes:
    item = yes();
    break;
  case No:
    item = no();
    break;
  case Discard:
    item = discard();
    break;
  case Save:
    item = save();
    break;
  case DontSave:
    item = dontsave();
    break;
  case SaveAs:
    item = saveas();
    break;
  case Apply:
    item = apply();
  default:
    item=0;
  }
  return item;
}

const char* KStdGuiItem::stdItem(StdItem ui_enum )
{
  const char *ret;
  switch (ui_enum)
  {
  case Ok:
    ret = "ok";
    break;
  default:
    ret = "";
    break;
  }
  return ret;
}

KGuiItem *KStdGuiItem::ok() {
  return new KGuiItem(i18n("&OK"), QIconSet(), "ok",i18n("Accept settings"), i18n( "" "If you press the <b>OK</b> button, all changes\n you made will be used to proceed" )    );
}

KGuiItem *KStdGuiItem::cancel(){
  return new KGuiItem(i18n("&Cancel"), QIconSet(), "cancel" );
}
KGuiItem *KStdGuiItem::yes(){
  return new KGuiItem(i18n("&Yes"), QIconSet(), "yes" );
}
KGuiItem *KStdGuiItem::no(){
  return new KGuiItem(i18n("&No"), QIconSet(), "no" );
}
KGuiItem *KStdGuiItem::discard(){
  return new KGuiItem(i18n("&Discard") );
}
KGuiItem *KStdGuiItem::save(){
  return new KGuiItem(i18n("&Save") );
}
KGuiItem *KStdGuiItem::dontsave(){
  return new KGuiItem(i18n("&Don't save") );
}
KGuiItem *KStdGuiItem::saveas(){
  return new KGuiItem(i18n("&Save As") );
}
KGuiItem *KStdGuiItem::apply(){
  return new KGuiItem(i18n("&Apply"), QIconSet(), "apply", i18n("Apply settings"),
  i18n( ""
    "When clicking <b>Apply</b>, the settings will be\n"
    "handed over to the program, but the dialog\n"
    "will not be closed. "
    "Use this to try different settings. ") );
}
