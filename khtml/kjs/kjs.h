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

namespace KJS {
  class UnicodeChar;
  class KJSLexer;
  class KJSContext;
  class KJSGlobal;
  class ProgramNode;
};

/**
 * @short ECMAScript interpreter
 *
 * This library implements ECMAScript. Currently its main aim is to add
 * JavaScript support to KHTMLWidget.
 */
class KJSWorld {
public:
  /**
   * Create a new ECMAScript interpreter. You can later ask it to interprete
   * code by pass it via @ref #evaluate.
   * @param w is pointing to the KHTMLWidget you wish the script to operate on.
   */
  KJSWorld(KHTMLWidget *w);
public:
  /**
   * Asks the interpreter to evaluate a piece of code. If called more than
   * once the state (global variables, functions etc.) will be preserved
   * between each call.
   * @param code is a string containing the code to be executed.
   */
  void evaluate(const char *code);
  /**
   * Same as above. Only differing in the arguments accepted.
   * KJS::UnicodeChar is a data type compatible to XChar2b and QChar.
   * @param code is an Unicode string containing the code to be executed.
   * @param length of the string.
   */
  void evaluate(const KJS::UnicodeChar *code, unsigned int length);
private:
  KHTMLWidget *htmlw;

public:
  static int nodeCount;
  static KJS::KJSLexer *lexer;
  static KJS::KJSContext *context;
  static KJS::KJSGlobal *global;
  static KJS::ProgramNode *prog;
};

#endif
