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
#include <string.h>

#include "kjs.h"
#include "object.h"
#include "nodes.h"
#include "lexer.h"

extern int kjsyyparse();

#ifdef KJS_DEBUG_MEM
extern const char* typeName[];
#endif

using namespace KJS;

KJScript* KJScript::curr = 0L;

class KJScriptLock {
  friend KJScript;
  KJScriptLock(KJScript *s) { s->setCurrent(s); }
  ~KJScriptLock() { KJScript::current()->setCurrent(0L); }
};

KJScript::KJScript()
  : initialized(false)
{
  KJScriptLock lock(this);
  setLexer(new KJSLexer());
  init();
}

KJScript::~KJScript()
{
  clear();

  KJScriptLock lock(this);

  delete lexer();
  setLexer(0L);

#ifdef KJS_DEBUG_MEM
  if (KJSO::count != 0) {
    fprintf(stderr, "MEMORY LEAK: %d unfreed objects\n", KJS::KJSO::count);
    KJSO *o = KJS::KJSO::firstObject;
    while (o) {
      fprintf(stderr, "id = %d type = %d %s refCount = %d\n",
	      o->objId, o->type(), typeName[o->type()], o->refCount);
      o = o->nextObject;
    }
  }
#endif
}

bool KJScript::evaluate(const char *code)
{
  return evaluate(UString(code).data(), strlen(code));
}

bool KJScript::evaluate(const UString &code)
{
  return evaluate(code.data(), code.size());
}

bool KJScript::evaluate(const KJS::UChar *code, unsigned int length)
{
  init();
  
  // maintain lock on global "current" pointer while running
  KJScriptLock lock(this);

  lexer()->setCode(code, length);
  int parseError = kjsyyparse();

  if (parseError) {
    fprintf(stderr, "JavaScript parse error.\n");
    KJS::Node::deleteAllNodes();
    return false;
  }

  setError(0L);

  Ptr res = KJS::Node::progNode()->evaluate();
  res.release();

  if (error())
    error()->deref();

  if (KJS::Node::progNode())
    KJS::Node::progNode()->deleteStatements();

  return true;
}

void KJScript::clear()
{
  if (initialized) {
    KJScriptLock lock(this);

    KJS::Node::deleteAllNodes();

    delete context();
    global()->deref();

    initialized = false;
  }
}

void KJScript::init()
{
  if (!initialized) {
    KJScriptLock lock(this);
    setGlobal(new KJSGlobal());
    setContext(new KJSContext());
    initialized = true;
  }
}
