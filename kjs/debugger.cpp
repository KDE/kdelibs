/*
 *  This file is part of the KDE libraries
 *  Copyright (C) 1999-2001 Harri Porten (porten@kde.org)
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifdef KJS_DEBUGGER

#include "debugger.h"
#include "kjs.h"

using namespace KJS;

Debugger::Debugger(KJScript *engine)
{
  attach(engine);
}

Debugger::~Debugger()
{
  detach();
}

void Debugger::attach(KJScript *e)
{
  dmode = Disabled;
  if (e) {
    eng = e->rep;
    eng->attachDebugger(this);
  } else {
    eng = 0L;
  }
}

void Debugger::detach()
{
  if (!eng)
    return;
  eng->attachDebugger(0L);
  eng = 0L;
}

void Debugger::setMode(Mode m)
{
  dmode = m;
}

Debugger::Mode Debugger::mode() const
{
  return dmode;
}

// supposed to be overriden by the user
bool Debugger::stopEvent(int)
{
  return true;
}

// called from the scripting engine each time a statement node is hit.
bool Debugger::hit(int line)
{
  if (!eng || mode() == Continue || mode() == Disabled)
      return true;

  return stopEvent(line);
}

#endif
