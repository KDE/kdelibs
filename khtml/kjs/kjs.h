/*
 *  This file is part of the KDE libraries
 *  Copyright (C) 1999 Harri Porten (porten@kde.org)
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 *  Boston, MA 02111-1307, USA.
 */

#ifndef _KJS_H_
#define _KJS_H_

class KHTMLWidget;
class KJScriptLock;

namespace KJS {
  class UnicodeChar;
  class KJSLexer;
  class KJSContext;
  class KJSGlobal;
  class ProgramNode;
  class KJSError;
  class KJSInternal;
};

/**
 * @short ECMAScript interpreter
 *
 * This library implements ECMAScript. Currently its main aim is to add
 * JavaScript support to KHTMLWidget.
 */
class KJScript {
  friend KJScriptLock;
public:
  /**
   * Create a new ECMAScript interpreter. You can later ask it to interprete
   * code by pass it via @ref #evaluate.
   * @param w is pointing to the KHTMLWidget you wish the script to operate on.
   */
  KJScript(KHTMLWidget *w);
  /**
   *  Destructor
   */
  ~KJScript();
  /**
   * Asks the interpreter to evaluate a piece of code. If called more than
   * once the state (global variables, functions etc.) will be preserved
   * between each call.
   * @param code is a string containing the code to be executed.
   */
  bool evaluate(const char *code);
  /**
   * Same as above. Only differing in the arguments accepted.
   * KJS::UnicodeChar is a data type compatible to XChar2b and QChar.
   * @param code is an Unicode string containing the code to be executed.
   * @param length of the string.
   */
  bool evaluate(const KJS::UnicodeChar *code, unsigned int length);
public:
  static KJScript *current() { return curr; }
  void setCurrent(KJScript *c) { curr = c; }

  static KJS::KJSError *error() { return current()->err; }
  static void setError(KJS::KJSError *e) { current()->err = e; }

  static KJS::KJSContext *context() { return current()->con; }
  static void setContext(KJS::KJSContext *c) { current()->con = c; }

  static KJS::KJSGlobal *global() { return current()->glob; }
  static void setGlobal(KJS::KJSGlobal *g) { current()->glob = g; }

  static KJS::KJSLexer *lexer() { return current()->lex; }
  static void setLexer(KJS::KJSLexer *l) { current()->lex = l; }
private:
  static KJScript *curr;
  KJS::KJSLexer *lex;
  KJS::KJSInternal *internal;
  KJS::KJSError *err;
  KJS::KJSContext *con;
  KJS::KJSGlobal *glob;
  KHTMLWidget *htmlw;
};

#endif
