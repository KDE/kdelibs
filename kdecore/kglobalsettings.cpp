/* This file is part of the KDE libraries
   Copyright (C) 2000 David Faure <faure@kde.org>

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

#include "kglobalsettings.h"

#include <kglobal.h>
#include <kconfig.h>

// kdebase/kcontrol/input/mousedefaults.h
#define DEFAULT_SINGLECLICK true
#define DEFAULT_AUTOSELECTDELAY 700
#define DEFAULT_CHANGECURSOR true

int KGlobalSettings::dndEventDelay()
{
  // Don't cache the value. The point of this method is to
  // detect changes in the configuration...
  //static int delay = -1;

  //if(delay == -1){
    KConfig *c = KGlobal::config();
    c->setGroup("General");
    return c->readNumEntry("DndDelay", 2);
  //}
  //return(delay);
}

bool KGlobalSettings::singleClick()
{
  // No caching here, too
  // static int doubleClicks = -1;
  // if(doubleClicks == -1)
  //{
    KConfig *c = KGlobal::config();
    KConfigGroupSaver cgs( c, "KDE" );
    return c->readBoolEntry("SingleClick", DEFAULT_SINGLECLICK);
  //}
  //return(doubleClicks == 1);
}

bool KGlobalSettings::changeCursorOverIcon()
{
  KConfig *c = KGlobal::config();
  KConfigGroupSaver cgs( c, "KDE" );
  return c->readBoolEntry("ChangeCursor", DEFAULT_CHANGECURSOR);
}

int KGlobalSettings::autoSelectDelay()
{
  KConfig *c = KGlobal::config();
  KConfigGroupSaver cgs( c, "KDE" );
  return c->readBoolEntry("AutoSelectDelay", DEFAULT_AUTOSELECTDELAY);
}

KGlobalSettings::Completion KGlobalSettings::completionMode()
{
  // No caching here, too
  //static int completion = -1;
  int completion;
  //if (completion == -1)
  //{
    KConfig *c = KGlobal::config();
    c->setGroup("General");
    completion = c->readNumEntry("completionMode", -1);
    if ((completion < (int) CompletionNone) ||
	(completion > (int) CompletionShell))
      {
	completion = (int) CompletionShell; // Default
      }
  //}
  return (Completion) completion;
}

