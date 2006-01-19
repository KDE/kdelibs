// -*- c-basic-offset: 2 -*-
/*
 *  This file is part of the KDE libraries
 *  Copyright (C) 1999-2002 Harri Porten (porten@kde.org)
 *  Copyright (C) 2001,2003 Peter Kelly (pmk@post.com)
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
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
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
#include <errno.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <math.h>
#include <ctype.h>

using namespace KJS;

// ------------------------- URI handling functions ---------------------------

// ECMA 15.1.3
UString encodeURI(ExecState *exec, UString string, UString unescapedSet)
{
  char hexdigits[] = "0123456789ABCDEF";
  int encbufAlloc = 2;
  UChar *encbuf = (UChar*)malloc(encbufAlloc*sizeof(UChar));
  int encbufLen = 0;

  for (int k = 0; k < string.size(); k++) {

    UChar C = string[k];
    if (unescapedSet.find(C) >= 0) {
      if (encbufLen+1 >= encbufAlloc)
	encbuf = (UChar*)realloc(encbuf,(encbufAlloc *= 2)*sizeof(UChar));
      encbuf[encbufLen++] = C;
    }
    else {
      unsigned char octets[4];
      int octets_len = 0;
      if (C.uc <= 0x007F) {
	unsigned short zzzzzzz = C.uc;
	octets[0] = zzzzzzz;
	octets_len = 1;
      }
      else if (C.uc <= 0x07FF) {
	unsigned short zzzzzz = C.uc & 0x3F;
	unsigned short yyyyy = (C.uc >> 6) & 0x1F;
	octets[0] = 0xC0 | yyyyy;
	octets[1] = 0x80 | zzzzzz;
	octets_len = 2;
      }
      else if (C.uc >= 0xD800 && C.uc <= 0xDBFF) {

        // we need two chars
	if (k + 1 >= string.size()) {
	  Object err = Error::create(exec,URIError);
	  exec->setException(err);
	  free(encbuf);
	  return UString();
	}

	unsigned short Cnext = UChar(string[++k]).uc;

	if (Cnext < 0xDC00 || Cnext > 0xDFFF) {
	  Object err = Error::create(exec,URIError);
	  exec->setException(err);
	  free(encbuf);
	  return UString();
	}

	unsigned short zzzzzz = Cnext & 0x3F;
	unsigned short yyyy = (Cnext >> 6) & 0x0F;
	unsigned short xx = C.uc & 0x03;
	unsigned short wwww = (C.uc >> 2) & 0x0F;
	unsigned short vvvv = (C.uc >> 6) & 0x0F;
	unsigned short uuuuu = vvvv+1;
	octets[0] = 0xF0 | (uuuuu >> 2);
	octets[1] = 0x80 | ((uuuuu & 0x03) << 4) | wwww;
	octets[2] = 0x80 | (xx << 4) | yyyy;
	octets[3] = 0x80 | zzzzzz;
	octets_len = 4;
      }
      else if (C.uc >= 0xDC00 && C.uc <= 0xDFFF) {
	Object err = Error::create(exec,URIError);
	exec->setException(err);
	free(encbuf);
	return UString();
      }
      else {
	// 0x0800 - 0xD7FF or 0xE000 - 0xFFFF
	unsigned short zzzzzz = C.uc & 0x3F;
	unsigned short yyyyyy = (C.uc >> 6) & 0x3F;
	unsigned short xxxx = (C.uc >> 12) & 0x0F;
	octets[0] = 0xE0 | xxxx;
	octets[1] = 0x80 | yyyyyy;
	octets[2] = 0x80 | zzzzzz;
	octets_len = 3;
      }

      while (encbufLen+3*octets_len >= encbufAlloc)
	encbuf = (UChar*)realloc(encbuf,(encbufAlloc *= 2)*sizeof(UChar));

      for (int j = 0; j < octets_len; j++) {
	encbuf[encbufLen++] = '%';
	encbuf[encbufLen++] = hexdigits[octets[j] >> 4];
	encbuf[encbufLen++] = hexdigits[octets[j] & 0x0F];
      }
    }
  }

  UString encoded(encbuf,encbufLen);
  free(encbuf);
  return encoded;
}

static bool decodeHex(UChar hi, UChar lo, unsigned short *val)
{
  *val = 0;
  if (hi.uc >= '0' && hi.uc <= '9')
    *val = (hi.uc-'0') << 4;
  else if (hi.uc >= 'a' && hi.uc <= 'f')
    *val = 10+(hi.uc-'a') << 4;
  else if (hi.uc >= 'A' && hi.uc <= 'F')
    *val = 10+(hi.uc-'A') << 4;
  else
    return false;

  if (lo.uc >= '0' && lo.uc <= '9')
    *val |= (lo.uc-'0');
  else if (lo.uc >= 'a' && lo.uc <= 'f')
    *val |= 10+(lo.uc-'a');
  else if (lo.uc >= 'A' && lo.uc <= 'F')
    *val |= 10+(lo.uc-'A');
  else
    return false;

  return true;
}

UString decodeURI(ExecState *exec, UString string, UString reservedSet)
{
  int decbufAlloc = 2;
  UChar *decbuf = (UChar*)malloc(decbufAlloc*sizeof(UChar));
  int decbufLen = 0;

  for (int k = 0; k < string.size(); k++) {
    UChar C = string[k];

    if (C != UChar('%')) {
      // Normal unescaped character
      if (decbufLen+1 >= decbufAlloc)
	decbuf = (UChar*)realloc(decbuf,(decbufAlloc *= 2)*sizeof(UChar));
      decbuf[decbufLen++] = C;
      continue;
    }

    // We have % escape sequence... expect at least 2 more characters
    int start = k;
    if (k+2 >= string.size()) {
      Object err = Error::create(exec,URIError);
      exec->setException(err);
      free(decbuf);
      return UString();
    }

    unsigned short B;
    if (!decodeHex(string[k+1],string[k+2],&B)) {
      Object err = Error::create(exec,URIError);
      exec->setException(err);
      free(decbuf);
      return UString();
    }

    k += 2;

    if (decbufLen+2 >= decbufAlloc)
        decbuf = (UChar*)realloc(decbuf,(decbufAlloc *= 2)*sizeof(UChar));

    if ((B & 0x80) == 0) {
      // Single-byte character
      C = B;
    }
    else {
      // Multi-byte character
      int n = 0;
      while (((B << n) & 0x80) != 0)
	n++;

      if (n < 2 || n > 4) {
	Object err = Error::create(exec,URIError);
	exec->setException(err);
	free(decbuf);
	return UString();
      }

      if (k+3*(n-1) >= string.size()) {
	Object err = Error::create(exec,URIError);
	exec->setException(err);
	free(decbuf);
	return UString();
      }

      unsigned short octets[4];
      octets[0] = B;
      for (int j = 1; j < n; j++) {
	k++;
	if ((UChar(string[k]) != UChar('%')) ||
	    !decodeHex(string[k+1],string[k+2],&B) ||
	    ((B & 0xC0) != 0x80)) {
	  Object err = Error::create(exec,URIError);
	  exec->setException(err);
	  free(decbuf);
	  return UString();
	}

	k += 2;
	octets[j] = B;
      }

      // UTF-8 transform
      unsigned long V;
      if (n == 2) {
	unsigned long yyyyy = octets[0] & 0x1F;
	unsigned long zzzzzz = octets[1] & 0x3F;
	V = (yyyyy << 6) | zzzzzz;
	C = UChar((unsigned short)V);
      }
      else if (n == 3) {
	unsigned long xxxx = octets[0] & 0x0F;
	unsigned long yyyyyy = octets[1] & 0x3F;
	unsigned long zzzzzz = octets[2] & 0x3F;
	V = (xxxx << 12) | (yyyyyy << 6) | zzzzzz;
	C = UChar((unsigned short)V);
      }
      else {
	assert(n == 4);
	unsigned long uuuuu = ((octets[0] & 0x07) << 2) | ((octets[1] >> 4) & 0x03);
	unsigned long vvvv = uuuuu-1;
	if (vvvv > 0x0F) {
          Object err = Error::create(exec,URIError);
	  exec->setException(err);
	  free(decbuf);
	  return UString();
	}        
	unsigned long wwww = octets[1] & 0x0F;
	unsigned long xx = (octets[2] >> 4) & 0x03;
	unsigned long yyyy = octets[2] & 0x0F;
	unsigned long zzzzzz = octets[3] & 0x3F;
	unsigned short H = 0xD800 | (vvvv << 6) | (wwww << 2) | xx;
	unsigned short L = 0xDC00 | (yyyy << 6) | zzzzzz;
	decbuf[decbufLen++] = UChar(H);
	decbuf[decbufLen++] = UChar(L);
	continue;
      }
    }

    if (reservedSet.find(C) < 0) {
        decbuf[decbufLen++] = C;
    }
    else {
      while (decbufLen+k-start >= decbufAlloc)
	decbuf = (UChar*)realloc(decbuf,(decbufAlloc *= 2)*sizeof(UChar));
      for (int p = start; p < k; p++)
	decbuf[decbufLen++] = string[p];
    }
  }

  UString decoded(decbuf,decbufLen);
  free(decbuf);
  return decoded;
}

static UString uriReserved = ";/?:@&=+$,";
static UString uriAlpha = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
static UString DecimalDigit = "0123456789";
static UString uriMark = "-_.!~*'()";
static UString uriUnescaped = uriAlpha+DecimalDigit+uriMark;

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
}

FunctionImp::FunctionImp(ExecState *exec, const Identifier &n)
  : InternalFunctionImp(
      static_cast<FunctionPrototypeImp*>(exec->lexicalInterpreter()->builtinFunctionPrototype().imp())
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
  Object &globalObj = exec->dynamicInterpreter()->globalObject();

  // enter a new execution context
  ContextImp ctx(globalObj, exec->dynamicInterpreter()->imp(), thisObj, sid, codeType(),
                 exec->context().imp(), this, &args);
  ExecState newExec(exec->dynamicInterpreter(), &ctx);
  newExec.setException(exec->exception()); // could be null

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

  // if an exception occurred, propogate it back to the previous execution object
  if (newExec.hadException())
    exec->setException(newExec.exception());

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
    exec->setException(comp.value());
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

void FunctionImp::processVarDecls(ExecState * /*exec*/)
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
    proto = exec->lexicalInterpreter()->builtinObjectPrototype();

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

namespace KJS {

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

/*KDE_NOEXPORT*/ const ClassInfo ShadowImp::info = {"Shadow", 0, 0, 0};

void ShadowImp::mark()
{
  ObjectImp::mark();
  if (!obj->marked())
    obj->mark();
}

}

// ------------------------------ ArgumentsImp ---------------------------------

const ClassInfo ArgumentsImp::info = {"Arguments", 0, 0, 0};

// ECMA 10.1.8
ArgumentsImp::ArgumentsImp(ExecState *exec, FunctionImp *func, const List &args,
			   ActivationImp *act)
  : ObjectImp(exec->lexicalInterpreter()->builtinObjectPrototype()), activation(act)
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
  assert(SimpleNumber::is(val.imp()) || !val.imp()->isDestroyed());
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
        // check for locally declared arguments property
        ValueImp *v = getDirect(propertyName);
        if (v)
            return Value(v);

        // default: return builtin arguments array
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


GlobalFuncImp::GlobalFuncImp(ExecState * /*exec*/, FunctionPrototypeImp *funcProto,
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

  static const char do_not_escape[] =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    "abcdefghijklmnopqrstuvwxyz"
    "0123456789"
    "*+-./@_";

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
      ContextImp ctx(exec->dynamicInterpreter()->globalObject(),
                     exec->dynamicInterpreter()->imp(),
                     thisObj,
                     source->sid,
                     EvalCode,
                     exec->context().imp());

      ExecState newExec(exec->dynamicInterpreter(), &ctx);
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
      progNode->processVarDecls(&newExec);
      Completion c = progNode->execute(&newExec);

      res = Undefined();

      ctx.setLines(progNode->lastLine(),progNode->lastLine());
      if (dbg && !dbg->exitContext(&newExec,c))
	// debugger requested we stop execution
	dbg->imp()->abort();
      else if (newExec.hadException()) // propagate back to parent context
	exec->setException(newExec.exception());
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

    int base = 0;
    if (args.size() > 1)
      base = args[1].toInt32(exec);

    double sign = 1;
    if (*startptr == '-') {
      sign = -1;
      startptr++;
    }
    else if (*startptr == '+') {
      sign = 1;
      startptr++;
    }

    bool leading0 = false;
    if ((base == 0 || base == 16) &&
	(*startptr == '0' && (startptr[1] == 'x' || startptr[1] == 'X'))) {
      startptr += 2;
      base = 16;
    }
    else if (base == 0 && *startptr == '0') {
      base = 8;
      leading0 = true;
      startptr++;
    }
    else if (base == 0) {
      base = 10;
    }

    if (base < 2 || base > 36) {
      res = Number(NaN);
    }
    else {
      long double val = 0;
      int index = 0;
      for (; *startptr; startptr++) {
	int thisval = -1;
	if (*startptr >= '0' && *startptr <= '9')
	  thisval = *startptr - '0';
	else if (*startptr >= 'a' && *startptr <= 'z')
	  thisval = 10 + *startptr - 'a';
	else if (*startptr >= 'A' && *startptr <= 'Z')
	  thisval = 10 + *startptr - 'A';

	if (thisval < 0 || thisval >= base)
	  break;

	val *= base;
	val += thisval;
	index++;
      }

      if (index == 0 && !leading0)
	res = Number(NaN);
      else
	res = Number(double(val)*sign);
    }
    break;
  }
  case ParseFloat: {
    UString str = args[0].toString(exec);
    // don't allow hex numbers here
    bool isHex = false;
    if (str.is8Bit()) {
      const char *c = str.ascii();
      while (isspace(*c))
	c++;
      isHex = (c[0] == '0' && (c[1] == 'x' || c[1] == 'X'));
    }
    if (isHex)
      res = Number(0);
    else
      res = Number(str.toDouble( true /*tolerant*/, false ));
    }
    break;
  case IsNaN:
    res = Boolean(isNaN(args[0].toNumber(exec)));
    break;
  case IsFinite: {
    double n = args[0].toNumber(exec);
    res = Boolean(!isNaN(n) && !isInf(n));
    break;
  }
  case DecodeURI:
    res = String(decodeURI(exec,args[0].toString(exec),uriReserved+"#"));
    break;
  case DecodeURIComponent:
    res = String(decodeURI(exec,args[0].toString(exec),""));
    break;
  case EncodeURI:
    res = String(encodeURI(exec,args[0].toString(exec),uriReserved+uriUnescaped+"#"));
    break;
  case EncodeURIComponent:
    res = String(encodeURI(exec,args[0].toString(exec),uriUnescaped));
    break;
  case Escape: {
    UString r = "", s, str = args[0].toString(exec);
    const UChar *c = str.data();
    for (int k = 0; k < str.size(); k++, c++) {
      int u = c->uc;
      if (u > 255) {
	char tmp[7];
	sprintf(tmp, "%%u%04X", u);
	s = UString(tmp);
      } else if (u != 0 && strchr(do_not_escape, (char)u)) {
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
    UString s = "", str = args[0].toString(exec);
    int k = 0, len = str.size();
    while (k < len) {
      const UChar *c = str.data() + k;
      UChar u;
      if (*c == UChar('%') && k <= len - 6 && *(c+1) == UChar('u')) {
	if (Lexer::isHexDigit((c+2)->uc) && Lexer::isHexDigit((c+3)->uc) &&
	    Lexer::isHexDigit((c+4)->uc) && Lexer::isHexDigit((c+5)->uc)) {
	  u = Lexer::convertUnicode((c+2)->uc, (c+3)->uc,
				    (c+4)->uc, (c+5)->uc);
	  c = &u;
	  k += 5;
	}
      } else if (*c == UChar('%') && k <= len - 3 &&
		 Lexer::isHexDigit((c+1)->uc) && Lexer::isHexDigit((c+2)->uc)) {
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
  case KJSPrint:
#ifndef NDEBUG
    puts(args[0].toString(exec).ascii());
#endif
    break;
  }

  return res;
}
