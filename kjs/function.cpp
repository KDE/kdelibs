// -*- c-basic-offset: 2 -*-
/*
 *  This file is part of the KDE libraries
 *  Copyright (C) 1999-2002 Harri Porten (porten@kde.org)
 *  Copyright (C) 2001 Peter Kelly (pmk@post.com)
 *  Copyright (C) 2003 Apple Computer, Inc.
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
 *
 */

#include "function.h"

#include "internal.h"
#include "function_object.h"
#include "lexer.h"
#include "nodes.h"
#include "operations.h"
#include "debugger.h"
#include "context.h"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <errno.h>
#include <math.h>
#include <ctype.h>

using namespace KJS;

// ----------------------------- FunctionImp ----------------------------------

const ClassInfo FunctionImp::info = {"Function", &InternalFunctionImp::info, 0, 0};

namespace KJS {
  class Parameter {
  public:
    Parameter(const Identifier &n) : name(n), next(0L) { }
    ~Parameter() { delete next; }
    Identifier name;
    Parameter *next;
  };
};

FunctionImp::FunctionImp(ExecState *exec, const Identifier &n)
  : InternalFunctionImp(
      static_cast<FunctionPrototypeImp*>(exec->interpreter()->builtinFunctionPrototype().imp())
      ), param(0L), line0(-1), line1(-1), sid(-1)
{
  //fprintf(stderr,"FunctionImp::FunctionImp this=%p\n");
  ident = n;
}

FunctionImp::~FunctionImp()
{
  delete param;
}

bool FunctionImp::implementsCall() const
{
  return true;
}

Value FunctionImp::call(ExecState *exec, Object &thisObj, const List &args)
{
  Object &globalObj = exec->interpreter()->globalObject();

  // enter a new execution context
  ContextImp ctx(globalObj, exec->interpreter()->imp(), thisObj, sid, codeType(),
                 exec->context().imp(), this, &args);
  ExecState newExec(exec->interpreter(), &ctx);
  newExec._exception = exec->exception(); // could be null

  // assign user supplied arguments to parameters
  processParameters(&newExec, args);
  // add variable declarations (initialized to undefined)
  processVarDecls(&newExec);

  ctx.setLines(line0,line0);
  Debugger *dbg = exec->interpreter()->imp()->debugger();
  if (dbg) {
    if (!dbg->enterContext(&newExec)) {
      // debugger requested we stop execution
      dbg->imp()->abort();
      return Undefined();
    }
  }

  Completion comp = execute(&newExec);

  ctx.setLines(line1,line1);
  if (dbg) {
    Object func(this);
    // ### lineno is inaccurate - we really want the end of the function _body_ here
    // line1 is suppoed to be the end of the function start, just before the body
    if (!dbg->exitContext(&newExec,comp)) {
      // debugger requested we stop execution
      dbg->imp()->abort();
      return Undefined();
    }
  }

  // if an exception occured, propogate it back to the previous execution object
  if (newExec.hadException())
    exec->_exception = newExec.exception();

#ifdef KJS_VERBOSE
  CString n = ident.isEmpty() ? CString("(internal)") : ident.ustring().cstring();
  if (comp.complType() == Throw) {
    n += " throws";
    printInfo(exec, n.c_str(), comp.value());
  } else if (comp.complType() == ReturnValue) {
    n += " returns";
    printInfo(exec, n.c_str(), comp.value());
  } else
    fprintf(stderr, "%s returns: undefined\n", n.c_str());
#endif

  if (comp.complType() == Throw) {
    exec->_exception = comp.value();
    return comp.value();
  }
  else if (comp.complType() == ReturnValue)
    return comp.value();
  else
    return Undefined();
}

void FunctionImp::addParameter(const Identifier &n)
{
  Parameter **p = &param;
  while (*p)
    p = &(*p)->next;

  *p = new Parameter(n);
}

Identifier FunctionImp::parameterProperty(int index) const
{
  // Find the property name corresponding to the given parameter
  int pos = 0;
  Parameter *p;
  for (p = param; p && pos < index; p = p->next)
    pos++;

  if (!p)
    return Identifier::null();

  // Are there any subsequent parameters with the same name?
  Identifier name = p->name;
  for (p = p->next; p; p = p->next)
    if (p->name == name)
      return Identifier::null();

  return name;
}

UString FunctionImp::parameterString() const
{
  UString s;
  const Parameter *p = param;
  while (p) {
    if (!s.isEmpty())
        s += ", ";
    s += p->name.ustring();
    p = p->next;
  }

  return s;
}


// ECMA 10.1.3q
void FunctionImp::processParameters(ExecState *exec, const List &args)
{
  Object variable = exec->context().imp()->variableObject();

#ifdef KJS_VERBOSE
  fprintf(stderr, "---------------------------------------------------\n"
	  "processing parameters for %s call\n",
	  name().isEmpty() ? "(internal)" : name().ascii());
#endif

  if (param) {
    ListIterator it = args.begin();
    Parameter *p = param;
    while (p) {
      if (it != args.end()) {
#ifdef KJS_VERBOSE
	fprintf(stderr, "setting parameter %s ", p->name.ascii());
	printInfo(exec,"to", *it);
#endif
	variable.put(exec, p->name, *it);
	it++;
      } else
	variable.put(exec, p->name, Undefined());
      p = p->next;
    }
  }
#ifdef KJS_VERBOSE
  else {
    for (int i = 0; i < args.size(); i++)
      printInfo(exec,"setting argument", args[i]);
  }
#endif
}

void FunctionImp::processVarDecls(ExecState */*exec*/)
{
}

Value FunctionImp::get(ExecState *exec, const Identifier &propertyName) const
{
    // Find the arguments from the closest context.
    if (propertyName == argumentsPropertyName) {
// delme
        ContextImp *context = exec->context().imp();
// fixme
//         ContextImp *context = exec->_context;
        while (context) {
            if (context->function() == this)
                return static_cast<ActivationImp *>
                    (context->activationObject())->get(exec, propertyName);
            context = context->callingContext();
        }
        return Null();
    }
    
    // Compute length of parameters.
    if (propertyName == lengthPropertyName) {
        const Parameter * p = param;
        int count = 0;
        while (p) {
            ++count;
            p = p->next;
        }
        return Number(count);
    }
    
    return InternalFunctionImp::get(exec, propertyName);
}

void FunctionImp::put(ExecState *exec, const Identifier &propertyName, const Value &value, int attr)
{
    if (propertyName == argumentsPropertyName || propertyName == lengthPropertyName)
        return;
    InternalFunctionImp::put(exec, propertyName, value, attr);
}

bool FunctionImp::hasProperty(ExecState *exec, const Identifier &propertyName) const
{
    if (propertyName == argumentsPropertyName || propertyName == lengthPropertyName)
        return true;
    return InternalFunctionImp::hasProperty(exec, propertyName);
}

bool FunctionImp::deleteProperty(ExecState *exec, const Identifier &propertyName)
{
    if (propertyName == argumentsPropertyName || propertyName == lengthPropertyName)
        return false;
    return InternalFunctionImp::deleteProperty(exec, propertyName);
}

// ------------------------------ DeclaredFunctionImp --------------------------

// ### is "Function" correct here?
const ClassInfo DeclaredFunctionImp::info = {"Function", &FunctionImp::info, 0, 0};

DeclaredFunctionImp::DeclaredFunctionImp(ExecState *exec, const Identifier &n,
					 FunctionBodyNode *b, const ScopeChain &sc)
  : FunctionImp(exec,n), body(b)
{
  Value protect(this);
  body->ref();
  setScope(sc);
  line0 = body->firstLine();
  line1 = body->lastLine();
  sid = body->sourceId();
}

DeclaredFunctionImp::~DeclaredFunctionImp()
{
  if ( body->deref() )
    delete body;
}

bool DeclaredFunctionImp::implementsConstruct() const
{
  return true;
}

// ECMA 13.2.2 [[Construct]]
Object DeclaredFunctionImp::construct(ExecState *exec, const List &args)
{
  Object proto;
  Value p = get(exec,prototypePropertyName);
  if (p.type() == ObjectType)
    proto = Object(static_cast<ObjectImp*>(p.imp()));
  else
    proto = exec->interpreter()->builtinObjectPrototype();

  Object obj(new ObjectImp(proto));

  Value res = call(exec,obj,args);

  if (res.type() == ObjectType)
    return Object::dynamicCast(res);
  else
    return obj;
}

Completion DeclaredFunctionImp::execute(ExecState *exec)
{
  Completion result = body->execute(exec);

  if (result.complType() == Throw || result.complType() == ReturnValue)
      return result;
  return Completion(Normal, Undefined()); // TODO: or ReturnValue ?
}

void DeclaredFunctionImp::processVarDecls(ExecState *exec)
{
  body->processVarDecls(exec);
}

// ------------------------------- ShadowImp -----------------------------------

// Acts as a placeholder value to indicate that the actual value is kept
// in the activation object
class ShadowImp : public ObjectImp {
public:
  ShadowImp(ObjectImp *_obj, Identifier _prop) : obj(_obj), prop(_prop) {}
  virtual void mark();

  virtual const ClassInfo *classInfo() const { return &info; }
  static const ClassInfo info;

  ObjectImp *obj;
  Identifier prop;
};

const ClassInfo ShadowImp::info = {"Shadow", 0, 0, 0};

void ShadowImp::mark()
{
  if (!obj->marked())
    obj->mark();
}

// ------------------------------ ArgumentsImp ---------------------------------

const ClassInfo ArgumentsImp::info = {"Arguments", 0, 0, 0};

// ECMA 10.1.8
ArgumentsImp::ArgumentsImp(ExecState *exec, FunctionImp *func, const List &args,
			   ActivationImp *act)
  : ObjectImp(exec->interpreter()->builtinObjectPrototype()), activation(act)
{
  Value protect(this);
  putDirect(calleePropertyName, func, DontEnum);
  putDirect(lengthPropertyName, args.size(), DontEnum);
  if (!args.isEmpty()) {
    ListIterator arg = args.begin();
    for (int i = 0; arg != args.end(); arg++, i++) {
      Identifier prop = func->parameterProperty(i);
      if (!prop.isEmpty()) {
	Object shadow(new ShadowImp(act,prop));
	ObjectImp::put(exec,Identifier::from(i), shadow, DontEnum);
      }
      else {
	ObjectImp::put(exec,Identifier::from(i), *arg, DontEnum);
      }
    }
  }
}

void ArgumentsImp::mark()
{
  ObjectImp::mark();
  if (!activation->marked())
    activation->mark();
}

Value ArgumentsImp::get(ExecState *exec, const Identifier &propertyName) const
{
  Value val = ObjectImp::get(exec,propertyName);
  Object obj = Object::dynamicCast(val);
  if (obj.isValid() && obj.inherits(&ShadowImp::info)) {
    ShadowImp *shadow = static_cast<ShadowImp*>(val.imp());
    return activation->get(exec,shadow->prop);
  }
  else {
    return val;
  }
}

void ArgumentsImp::put(ExecState *exec, const Identifier &propertyName,
		       const Value &value, int attr)
{
  Value val = ObjectImp::get(exec,propertyName);
  Object obj = Object::dynamicCast(val);
  if (obj.isValid() && obj.inherits(&ShadowImp::info)) {
    ShadowImp *shadow = static_cast<ShadowImp*>(val.imp());
    activation->put(exec,shadow->prop,value,attr);
  }
  else {
    ObjectImp::put(exec,propertyName,value,attr);
  }
}

// ------------------------------ ActivationImp --------------------------------

const ClassInfo ActivationImp::info = {"Activation", 0, 0, 0};

// ECMA 10.1.6
ActivationImp::ActivationImp(FunctionImp *function, const List &arguments)
    : _function(function), _arguments(true), _argumentsObject(0)
{
  _arguments = arguments.copy();
  // FIXME: Do we need to support enumerating the arguments property?
}

Value ActivationImp::get(ExecState *exec, const Identifier &propertyName) const
{
  if (propertyName == argumentsPropertyName) {
    ValueImp *imp = getDirect(propertyName);
    if (imp)
      return Value(imp);

    if (!_argumentsObject)
      _argumentsObject = new ArgumentsImp(exec, _function, _arguments, const_cast<ActivationImp*>(this));
    return Value(_argumentsObject);
  }
  return ObjectImp::get(exec, propertyName);
}

bool ActivationImp::hasProperty(ExecState *exec, const Identifier &propertyName) const
{
  if (propertyName == argumentsPropertyName)
    return true;
  return ObjectImp::hasProperty(exec, propertyName);
}

bool ActivationImp::deleteProperty(ExecState *exec, const Identifier &propertyName)
{
  if (propertyName == argumentsPropertyName)
    return false;
  return ObjectImp::deleteProperty(exec, propertyName);
}

void ActivationImp::mark()
{
  ObjectImp::mark();
  if (_function && !_function->marked()) 
    _function->mark();
  _arguments.mark();
  if (_argumentsObject && !_argumentsObject->marked())
    _argumentsObject->mark();
}

// ------------------------------ GlobalFunc -----------------------------------


GlobalFuncImp::GlobalFuncImp(ExecState */*exec*/, FunctionPrototypeImp *funcProto,
			     int i, int len, const Identifier &_ident)
  : InternalFunctionImp(funcProto), id(i)
{
  Value protect(this);
  putDirect(lengthPropertyName, len, DontDelete|ReadOnly|DontEnum);
  ident = _ident;
}

CodeType GlobalFuncImp::codeType() const
{
  return id == Eval ? EvalCode : codeType();
}

bool GlobalFuncImp::implementsCall() const
{
  return true;
}

Value GlobalFuncImp::call(ExecState *exec, Object &thisObj, const List &args)
{
  Value res;

  static const char non_escape[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
				   "abcdefghijklmnopqrstuvwxyz"
				   "0123456789@*_+-./";

  switch (id) {
  case Eval: { // eval()
    Value x = args[0];
    if (x.type() != StringType)
      return x;
    else {
      UString s = x.toString(exec);

      int errLine;
      UString errMsg;
#ifdef KJS_VERBOSE
      fprintf(stderr, "eval(): %s\n", s.ascii());
#endif
      SourceCode *source;
      FunctionBodyNode *progNode = Parser::parse(s.data(),s.size(),&source,&errLine,&errMsg);
      if (progNode)
	progNode->setProgram(true);

      // notify debugger that source has been parsed
      Debugger *dbg = exec->interpreter()->imp()->debugger();
      if (dbg) {
	bool cont = dbg->sourceParsed(exec,source->sid,s,errLine);
	if (!cont) {
	  source->deref();
	  dbg->imp()->abort();
	  if (progNode)
	    delete progNode;
	  return Undefined();
	}
      }

      exec->interpreter()->imp()->addSourceCode(source);

      // no program node means a syntax occurred
      if (!progNode) {
	Object err = Error::create(exec,SyntaxError,errMsg.ascii(),errLine);
        err.put(exec,"sid",Number(source->sid));
        exec->setException(err);
	source->deref();
        return err;
      }

      source->deref();
      progNode->ref();

      // enter a new execution context
      ContextImp ctx(exec->interpreter()->globalObject(),
                     exec->interpreter()->imp(),
                     thisObj,
                     source->sid,
                     EvalCode,
                     exec->context().imp());

      ExecState newExec(exec->interpreter(), &ctx);
      newExec.setException(exec->exception()); // could be null

      ctx.setLines(progNode->firstLine(),progNode->firstLine());
      if (dbg) {
	if (!dbg->enterContext(&newExec)) {
	  // debugger requested we stop execution
	  dbg->imp()->abort();

	  if (progNode->deref())
	    delete progNode;
	  return Undefined();
	}
      }

      // execute the code
      Completion c = progNode->execute(&newExec);

      res = Undefined();

      ctx.setLines(progNode->lastLine(),progNode->lastLine());
      if (dbg && !dbg->exitContext(&newExec,c))
	// debugger requested we stop execution
	dbg->imp()->abort();
      else if (newExec.hadException()) // propagate back to parent context
	exec->_exception = newExec.exception(); 
      else if (c.complType() == Throw)
	exec->setException(c.value());
      else if (c.isValueCompletion())
	res = c.value();

      if (progNode->deref())
	delete progNode;

      return res;
    }
    break;
  }
  case ParseInt: { // ECMA 15.1.2.2
    CString cstr = args[0].toString(exec).cstring();
    const char* startptr = cstr.c_str();
    while ( *startptr && isspace( *startptr ) ) // first, skip leading spaces
      ++startptr;
    char* endptr;
    errno = 0;
    //fprintf( stderr, "ParseInt: parsing string %s\n", startptr );
    int base = 0;
    // Figure out the base
    if ( args.size() > 1 )
      base = args[ 1 ].toInt32( exec );
    if ( base == 0 ) {
      // default base is 10, unless the number starts with 0x or 0X
      if ( *startptr == '0' && toupper( *(startptr+1) ) == 'X' )
        base = 16;
      else
        base = 10;
    }
    //fprintf( stderr, "base=%d\n",base );
    if ( base != 10 )
    {
      // We can't use strtod if a non-decimal base was specified...
#ifdef HAVE_FUNC_STRTOLL
      long long llValue = strtoll(startptr, &endptr, base);
      double value = llValue;
#else
      long value = strtol(startptr, &endptr, base);
#endif
      if (errno || endptr == startptr)
        res = Number(NaN);
      else
        res = Number(value);
    } else {
      // Parse into a double, not an int or long. We must be able to parse
      // huge integers like 16-digits ones (credit card numbers ;)
      // But first, check that it only has digits (after the +/- sign if there's one).
      // That's because strtod will accept .5, but parseInt shouldn't.
      // Also, strtod will parse 0x7, but it should fail here (base==10)
      bool foundSign = false;
      bool foundDot = false;
      bool ok = false;
      for ( const char* ptr = startptr; *ptr; ++ptr ) {
        if ( *ptr >= '0' && *ptr <= '9' )
          ok = true;
        else if ( !foundSign && ( *ptr == '-' || *ptr == '+' ) )
          foundSign = true;
        else if ( ok && !foundDot && *ptr == '.' ) // only accept one dot, and after a digit
          foundDot = true;
        else {
          *const_cast<char *>(ptr) = '\0';   // this will prevent parseInt('0x7',10) from returning 7.
          break; // something else -> stop here.
        }
      }

      double value = strtod(startptr, &endptr);
      if (!ok || errno || endptr == startptr)
        res = Number(NaN);
      else
        res = Number(floor(value));
    }
    break;
  }
  case ParseFloat:
    res = Number(args[0].toString(exec).toDouble( true /*tolerant*/ ));
    break;
  case IsNaN:
    res = Boolean(isNaN(args[0].toNumber(exec)));
    break;
  case IsFinite: {
    double n = args[0].toNumber(exec);
    res = Boolean(!isNaN(n) && !isInf(n));
    break;
  }
  case Escape: {
    UString r = "", s, str = args[0].toString(exec);
    const UChar *c = str.data();
    for (int k = 0; k < str.size(); k++, c++) {
      int u = c->uc;
      if (u > 255) {
	char tmp[7];
	sprintf(tmp, "%%u%04X", u);
	s = UString(tmp);
      } else if (strchr(non_escape, (char)u)) {
	s = UString(c, 1);
      } else {
	char tmp[4];
	sprintf(tmp, "%%%02X", u);
	s = UString(tmp);
      }
      r += s;
    }
    res = String(r);
    break;
  }
  case UnEscape: {
    UString s, str = args[0].toString(exec);
    int k = 0, len = str.size();
    while (k < len) {
      const UChar *c = str.data() + k;
      UChar u;
      if (*c == UChar('%') && k <= len - 6 && *(c+1) == UChar('u')) {
	u = Lexer::convertUnicode((c+2)->uc, (c+3)->uc,
				  (c+4)->uc, (c+5)->uc);
	c = &u;
	k += 5;
      } else if (*c == UChar('%') && k <= len - 3) {
	u = UChar(Lexer::convertHex((c+1)->uc, (c+2)->uc));
	c = &u;
	k += 2;
      }
      k++;
      s += UString(c, 1);
    }
    res = String(s);
    break;
  }
  case KJSPrint: {
#ifndef NDEBUG
    UString str = args[0].toString(exec);
    puts(str.ascii());
#endif
    break;
  }
  }

  return res;
}
