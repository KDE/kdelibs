/*
 *  This file is part of the KDE libraries
 *  Copyright (C) 1999-2000 Harri Porten (porten@kde.org)
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
#include "types.h"
#include "operations.h"
#include "error_object.h"
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
  ~KJScriptLock() { /* KJScript::current()->setCurrent(0L); */ }
};

KJScript::KJScript()
  : initialized(false)
{
  KJScriptLock lock(this);
  Lexer::setCurrent(new Lexer());
  init();
}

KJScript::~KJScript()
{
  clear();

  KJScriptLock lock(this);

  delete Lexer::curr();
  Lexer::setCurrent(0L);

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
  return evaluate((QChar*)UString(code).data(), strlen(code));
}

bool KJScript::evaluate(const UString &code)
{
  return evaluate((QChar*)code.data(), code.size());
}

bool KJScript::evaluate(const QChar *code, unsigned int length)
{
  init();

  // maintain lock on global "current" pointer while running
  KJScriptLock lock(this);

  Lexer::curr()->setCode((UChar*)code, length);
  int parseError = kjsyyparse();

  if (parseError) {
    fprintf(stderr, "JavaScript parse error.\n");
    KJS::Node::deleteAllNodes();
    return false;
  }

  // leak ?
  KJSO::setError(0L);

  Ptr res = KJS::Node::progNode()->evaluate();
  res.release();

  if (KJSO::error()) {
    /* TODO */
    errType = 99;
    errMsg = "Error";
    //    KJSO::error()->deref();
    KJSO::setError(0L);
  }

  if (KJS::Node::progNode())
    KJS::Node::progNode()->deleteStatements();

  return true;
}

void KJScript::clear()
{
  if (initialized) {
    KJScriptLock lock(this);

    KJS::Node::deleteAllNodes();

    delete Context::current();
    global()->deref();

    initialized = false;
  }
}

/**
 * @short Print to stderr for debugging purposes.
 */
class DebugPrint : public InternalFunction {
public:
  KJSO* execute(const List &args)
    {
      Ptr v = args[0];
      Ptr s = toString(v);
      fprintf(stderr, "---> %s\n", s->stringVal().cstring().c_str());

      return newCompletion(Normal);
    }
};

void KJScript::enableDebug()
{
  KJScriptLock lock(this);
  global()->put("debug", zeroRef(new DebugPrint()));
}

void KJScript::init()
{
  if (!initialized) {
    KJScriptLock lock(this);
    setGlobal(new Global());
    Context::setCurrent(new Context());
    initialized = true;
  }
}
