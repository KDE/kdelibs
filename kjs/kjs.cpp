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
#include "internal.h"
#include "collector.h"

using namespace KJS;

KJScript::KJScript()
{
  Collector::detach();
  rep = new KJScriptImp();
  rep->init();
}

KJScript::~KJScript()
{
  delete rep;

#ifdef KJS_DEBUG_MEM
  printf("Imp::count: %d\n", Imp::count);
  assert(Imp::count == 0);
#endif
}

bool KJScript::evaluate(const char *code)
{
  return rep->evaluate((QChar*)UString(code).data(), strlen(code));
}

bool KJScript::evaluate(const UString &code)
{
  return rep->evaluate((QChar*)code.data(), code.size());
}

bool KJScript::evaluate(const Object &thisV,
			const QChar *code, unsigned int length)
{
  return rep->evaluate(code, length, thisV.imp());
}

void KJScript::clear()
{
  rep->clear();
}

int KJScript::errorType() const
{
  return rep->errType;
}

const char* KJScript::errorMsg() const
{
  return rep->errMsg;
}

/**
 * @short Print to stderr for debugging purposes.
 */
namespace KJS {
  class DebugPrint : public InternalFunctionImp {
  public:
    Completion execute(const List &args)
      {
	KJSO v = args[0];
	String s = v.toString();
	fprintf(stderr, "---> %s\n", s.value().cstring().c_str());

	return Completion(Normal);
      }
  };
};

void KJScript::enableDebug()
{
  //  KJScriptLock lock(this);
  rep->curr = rep;
  Global::current().put("debug", Function(new DebugPrint()));
}
