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

#include <stdio.h>

#include <qmessagebox.h>
#include <klocale.h>

#include "kjs.h"
#include "object.h"
#include "nodes.h"
#include "lexer.h"

extern int yyparse();

using namespace KJS;

int          KJSWorld::nodeCount = 0;
KJSLexer*    KJSWorld::lexer     = 0L;
KJSContext*  KJSWorld::context   = 0L;
KJSGlobal*   KJSWorld::global    = 0L;
ProgramNode* KJSWorld::prog      = 0L;

KJSWorld::KJSWorld(KHTMLWidget *w)
  : htmlw(w)
{
  printf("KJSWorld::KJSWorld()\n");

  // due to yacc's C nature the parser isn't reentrant
  assert(nodeCount == 0);
  assert(lexer == 0);
  assert(context == 0);
  assert(global == 0);
  assert(prog == 0);
}

void KJSWorld::evaluate(const QString &code)
{
  fprintf(stderr, "KJSWorld::evaluate(%s)\n", code.ascii());

  lexer = new KJSLexer(code);

  int parseError = yyparse();

  // we can safely get rid of the parser now
  delete lexer;
  lexer = 0L;

  if (!parseError) {
    fprintf(stderr, "nodeCount = %d\n", nodeCount);

    global = new KJSGlobal();
    context = new KJSContext();
    context->insertScope(global);

    assert(prog);
    prog->evaluate();

    // TODO: delete context and global

    delete prog;
    prog = 0L;

    //    assert(KJSWorld::nodeCount == 0);
    if(KJSWorld::nodeCount != 0)
      fprintf(stderr, "remain nodeCount = %d. Looks like a leak.\n",
	      KJSWorld::nodeCount);
  } else
    QMessageBox::warning((QWidget *)htmlw, i18n("JavaScript"), i18n("Parsing error"));
}


