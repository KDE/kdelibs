/*
 *  This file is part of the KDE libraries
 *  Copyright (C) 1999-2001 Harri Porten (porten@kde.org)
 *  Copyright (C) 2001 Peter Kelly (pmk@post.com)
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

class KJScript;

namespace KJS {

  class DebuggerImp;

  //
  // NOTE: this interface is not ready, yet. Do not use unless you
  // don't mind source and binary incompatible changes that may arise
  // before the final version is released.
  //

  class Context;
  class UString;
  class FunctionImp;
  class List;
  class KJSO;

  class ExecutionContext {
    friend class DebuggerImp;
    friend class FunctionImp;
  public:
    KJSO resolveVar(const UString &varName) const;
    KJSO executeCall(KJScript *script, KJSO &func, const KJSO &thisV, const List *args) const;
    KJSO thisValue() const;
  private:
    Context *rep;
    ExecutionContext(Context *c);
  };

  class Debugger {
    friend class KJScriptImp;
    friend class StatementNode;
    friend class DeclaredFunctionImp;
    friend class FunctionImp;
    friend class DebuggerImp;
    friend class FunctionObject;
  public:
    /**
     * Construct a debugger and attach it to the scripting engine s.
     */
    Debugger();
    /**
     * Destruct the debugger and detach from the scripting engine we
     * might have been attached to.
     */
    virtual ~Debugger();
    /**
     * Attaches the debugger to specified scripting engine.
     */
    void attach(KJScript *script);
    /**
     * Detach the debugger from a scripting engine (or all if script == 0)
     */
    void detach(KJScript *script);
    /**
     * Returns the value of ident out of the current context in string form
     */
    //    UString varInfo(const UString &ident);
    /**
     * Set variable ident to value. Returns true if successful, false if
     * the specified variable doesn't exist or isn't writable.
     */
    //    bool setVar(const UString &ident, const KJSO &value);
  protected:
    /**
     * Called to notify the debugger that some javascript source code has
     * been parsed. For calls to KJScript::evaluate(), this will be called
     * with the supplied source code before any other code is parsed.
     * Other situations in which this may be called include creation of a
     * function using the Function() constructor, or the eval() function.
     *
     * @param script The interpreter which parsed the script
     * @param sourceId The ID of the source code (corresponds to the
     * sourceId supplied in other functions such as atLine()
     * @param source The source code that was parsed
     * @param errorLine The line number at which parsing encountered an
     * error, or -1 if the source code was valid and parsed succesfully
     * @return true if execution should be continue, false if it should
     * be aborted
     */
    virtual bool sourceParsed(KJScript *script, int sourceId,
			      const UString &source, int errorLine);
    /**
     * Called when all functions/programs associated with a particular
     * sourceId have been deleted. After this function has been called for
     * a particular sourceId, that sourceId will not be used again.
     * @return true if execution should be continue, false if it should
     * be aborted
     */
    virtual bool sourceUnused(KJScript *script, int sourceId);
    /**
     * Called when an error occurs during script execution.
     *
     * @param script The interpreter which is running the script
     * @param sourceId The ID of the source code being executed
     * @param lineno The line at which the error occurred
     * @param errorType The type of error
     * @param errorMessage A string containing a description of the error
     * @return true if execution should be continue, false if it should
     * be aborted
     */
    virtual bool error(KJScript *script, int sourceId, int lineno,
		       int errorType, const UString &errorMessage);
    /**
     * Called when a line of the script is reached (before it is executed)
     *
     * The default implementation does nothing. Overload this method if
     * you want to process this event. After returning, execution of the
     * script will continue.
     *
     * @param script The interpreter which is running the script
     * @param sourceId The ID of the source code being executed
     * @param lineno The line that is about to be executed
     * @param execContext The execution context within which the current
     * line is being executed
     * @return true if execution should be continue, false if it should
     * be aborted
     */
    virtual bool atLine(KJScript *script, int sourceId, int lineno,
			const ExecutionContext *execContext);
    /**
     * Called on each function call. Use together with @ref #returnEvent
     * if you want to keep track of the call stack.
     *
     * The default implementation does nothing. Overload this method if
     * you want to process this event. After returning, execution of the
     * script will continue.
     *
     * @param script The interpreter which is running the script
     * @param sourceId The ID of the source code being executed
     * @param lineno The line that is about to be executed
     * @param execContext The execution context within which the current
     * @param function The function being called
     * @param args The arguments that were passed to the function
     * line is being executed
     * @return true if execution should be continue, false if it should
     * be aborted
     */
    virtual bool callEvent(KJScript *script, int sourceId, int lineno,
			   const ExecutionContext *execContext,
			   FunctionImp *function, const List *args);
    /**
     * Called on each function exit. The function being returned from is that
     * which was supplied in the last callEvent().
     *
     * The default implementation does nothing. Overload this method if
     * you want to process this event. After returning, execution of the
     * script will continue.
     *
     * @param script The interpreter which is running the script
     * @param sourceId The ID of the source code being executed
     * @param lineno The line that is about to be executed
     * @param execContext The execution context within which the current
     * @param function The function being called
     * line is being executed
     * @return true if execution should be continue, false if it should
     * be aborted
     */
    virtual bool returnEvent(KJScript *script, int sourceId, int lineno,
			   const ExecutionContext *execContext,
			   FunctionImp *function);

  private:
    //    UString objInfo(const KJSO &obj) const;

    DebuggerImp *rep;
  };

};

#endif
