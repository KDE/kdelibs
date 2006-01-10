// -*- c-basic-offset: 2 -*-
/*
 *  This file is part of the KDE libraries
 *  Copyright (C) 1999 Harri Porten (porten@kde.org)
 *  Copyright (C) 2001 Peter Kelly (pmk@post.com)
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
 *  You should have received a copy of the GNU Library General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#ifndef _KJS_PROXY_H_
#define _KJS_PROXY_H_

#include <qvariant.h>
#include <qstring.h>
#include <sys/time.h>

class KHTMLPart;

namespace DOM {
  class Node;
  class NodeImpl;
  class EventListener;
  class Event;
}

namespace KJS {
  class List;
  class Interpreter;
  class Completion;
  class KJSDebugWin;
}

namespace khtml {
  class ChildFrame;
}

/**
 * @internal
 *
 * @short Proxy class serving as interface when being dlopen'ed.
 */
class KJSProxy {
public:
  KJSProxy() { m_handlerLineno = 0; }
  virtual ~KJSProxy() { }
  virtual QVariant evaluate(QString filename, int baseLine, const QString &, const DOM::Node &n,
			    KJS::Completion *completion = 0) = 0;
  virtual void clear() = 0;
  virtual DOM::EventListener *createHTMLEventHandler(QString sourceUrl, QString name, QString code, DOM::NodeImpl* node) = 0;
  virtual void finishedWithEvent(const DOM::Event &event) = 0;
  virtual KJS::Interpreter *interpreter() = 0;

  virtual void setDebugEnabled(bool enabled) = 0;
  virtual void showDebugWindow(bool show=true) = 0;
  virtual bool paused() const = 0;
  virtual void dataReceived() = 0;

  void setEventHandlerLineno(int lineno) { m_handlerLineno = lineno; }

  khtml::ChildFrame *m_frame;
  int m_handlerLineno;

  // Helper method, to access the private KHTMLPart::jScript()
  static KJSProxy *proxy( KHTMLPart *part );
};

class KJSCPUGuard {
public:
  KJSCPUGuard() {}
  void start(unsigned int msec=5000, unsigned int i_msec=10000);
  void stop();
private:
  void (*oldAlarmHandler)(int);
  static void alarmHandler(int);
  static bool confirmTerminate();
#ifdef Q_WS_WIN
  //TODO
#else
  itimerval oldtv;
#endif
};

#endif
