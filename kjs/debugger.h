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

#ifndef _KJSDEBUGGER_H_
#define _KJSDEBUGGER_H_

#include "internal.h"

namespace KJS {

#ifdef KJS_DEBUGGER
  class Debugger {
    friend class KJScriptImp;
    friend class StatementNode;
  public:
    /**
     * Available modes of the debugger.
     */
    enum Mode { Disabled = 0, Next, Step, Continue, Stop };
    /**
     * Construct a debugger and attach it to the scripting engine s.
     */
    Debugger(KJScript *s);
    /**
     * Destruct the debugger and detach from the scripting engine we
     * might have been attached to.
     */
    virtual ~Debugger();
    /**
     * Attaches the debugger to specified scripting engine.
     */
    void attach(KJScript *e);
    /**
     * Detach he debugger from any scripting engine.
     */
    void detach();
    /**
     * Set debugger into specified mode. This will influence further behaviour
     * if execution of the programm is started or continued.
     */
    virtual void setMode(Mode m);
    /**
     * Returns the current operation mode.
     */
    Mode mode() const;

  protected:
    /**
     * Invoked in case a breakpoint or the next statement is reached in step
     * mode. The return value decides whether execution will continue. True
     * denotes continuation, false an abortion, respectively.
     *
     * The default implementation does nothing. Overload this method if
     * you want to process this event.
     */
    virtual bool stopEvent(int line);

  private:
    bool hit(int line);

    KJScriptImp *eng;
    Mode dmode;
  };
#endif

};

#endif
