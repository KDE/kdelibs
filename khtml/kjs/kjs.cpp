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

#include <ltdl.h>

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
bool KJScript::ltdlInit = false;

class KJScriptLock {
  friend KJScript;
  KJScriptLock(KJScript *s) { s->setCurrent(s); }
  ~KJScriptLock() { KJScript::current()->setCurrent(0L); }
};

KJScript::KJScript()
{
  printf("KJScript::KJScript()\n");

  KJScriptLock lock(this);
  setGlobal(new KJSGlobal());
  setContext(new KJSContext());
}

KJScript::~KJScript()
{
  KJScriptLock lock(this);

  delete context();
  global()->deref();

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

  if (ltdlInit)
    lt_dlexit();
}

bool KJScript::evaluate(const char *code)
{
  return evaluate(UString(code).unicode(), strlen(code));
}

bool KJScript::evaluate(const KJS::UnicodeChar *code, unsigned int length)
{
  // maintain lock on global "current" pointer while running
  KJScriptLock lock(this);

  setLexer(new KJSLexer(UString(code, length)));

  int parseError = kjsyyparse();

  // we can safely get rid of the parser now
  delete lexer();
  setLexer(0L);

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

  KJS::Node::deleteAllNodes();

  return true;
}

bool KJScript::useModule(const char *module, void *arg)
{
  if (!module) {
    fprintf(stderr, "KJScript::useModule(): module == NULL\n");
    return false;
  }

  KJScriptLock lock(this);

  // initialize libtool's dlopen wrapper
  if (!ltdlInit) {
    ltdlInit = true;
    lt_dlinit();
  }

  CString lib = module;
  CString name;
  // did we get an absolute path ?
  if (module[0] == '/') {
    const char *p = strrchr(module, '/') + 1;
    name = p;
    const char *e = strchr(p, '.');
    if (e)
      name.resize(e-p+1);
  } else {
    name = module;
    if (strchr(module, '.'))
      name.resize(strchr(module, '.')-module+1);
    else
      lib += ".la";
  }

  // try to dlopen the module
  lt_dlhandle handle = lt_dlopen(lib.ascii());
  if (!handle) {
    fprintf(stderr, "error loading %s: %s\n", lib.ascii(), lt_dlerror());
    return false;
  }

  // look for a C symbol called {name}_init
  CString init = name;
  init += "_init";
  lt_ptr_t sym = lt_dlsym(handle, init.ascii());
  const char *error;
  if ((error = lt_dlerror()) != 0L) {
    fprintf(stderr, "error finding init symbol: %s\n", error);
    return false;
  }

  initFunction initSym = (initFunction) sym;

  if ((*initSym)(arg)) {
    fprintf(stderr, "initialization of module %s failed\n", name);
    return false;
  }

  return true;
}


