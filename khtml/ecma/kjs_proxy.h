/*
 *  This file is part of the KDE libraries
 *  Copyright (C) 1999 Harri Porten (porten@kde.org)
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

#ifndef _KJS_PROXY_H_
#define _KJS_PROXY_H_

class QChar;
class QEvent;
class KJScript;

// callback functions for KJSProxy
typedef bool (KJSEvalFunc)(KJScript *script, const QChar *, unsigned int);
typedef void (KJSClearFunc)(KJScript *script);
typedef bool (KJSEventFunc)(KJScript *script, QEvent *, void *);
typedef bool (KJSMaskFunc)(KJScript *script, int);
typedef const char* (KJSSpecialFunc)(KJScript *script, const char *);
typedef void (KJSDestroyFunc)(KJScript *script);
extern "C" {
  KJSEvalFunc kjs_eval;
  KJSClearFunc kjs_clear;
  KJSSpecialFunc kjs_special;
  KJSEventFunc kjs_event;
  KJSMaskFunc kjs_mask;
  KJSDestroyFunc kjs_destroy;
}

/**
 * @short Proxy class serving as interface when being dlopen'ed.
 */
class KJSProxy {
public:
  KJSProxy(KJScript *s, KJSEvalFunc e, KJSClearFunc c,
	   KJSEventFunc ev, KJSMaskFunc m,
	   KJSSpecialFunc sp, KJSDestroyFunc d)
    : script(s), eval(e), clr(c), event(ev), mask(m), spec(sp), destr(d) {};
  ~KJSProxy() { (*destr)(script); }
  bool evaluate(const QChar *c, unsigned int l) {
    return (*eval)(script, c, l);
  }
  bool processEvent(QEvent *e, void *unknown) {
    return (*event)(script, e, unknown);
  }
  bool eventMask(int e) {
    return (*mask)(script, e);
  }
  const char *special(const char *c) {
    return (*spec)(script, c);
  }
  void clear() {
    (*clr)(script);
  }
private:
  KJScript *script;
  KJSEvalFunc *eval;
  KJSClearFunc *clr;
  KJSEventFunc *event;
  KJSMaskFunc *mask;
  KJSSpecialFunc *spec;
  KJSDestroyFunc *destr;
};

#endif
