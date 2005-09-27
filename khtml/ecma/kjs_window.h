// -*- c-basic-offset: 2 -*-
/*
 *  This file is part of the KDE libraries
 *  Copyright (C) 2000 Harri Porten (porten@kde.org)
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
 *  You should have received a copy of the GNU Library General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Steet, Fifth Floor, Boston, MA  02110-1301  USA
 */

#ifndef _KJS_WINDOW_H_
#define _KJS_WINDOW_H_

#include <qobject.h>
#include <qguardedptr.h>
#include <qmap.h>
#include <qptrlist.h>
#include <qdatetime.h>

#include "kjs_binding.h"

class QTimer;
class KHTMLView;
class KHTMLPart;

namespace KParts {
  class ReadOnlyPart;
}

namespace khtml {
  class ChildFrame;
}

namespace KJS {

  class WindowFunc;
  class WindowQObject;
  class Location;
  class History;
  class External;
  class FrameArray;
  class JSEventListener;
  class JSLazyEventListener;

  class Screen : public ObjectImp {
  public:
    Screen(ExecState *exec);
    enum {
      Height, Width, ColorDepth, PixelDepth, AvailLeft, AvailTop, AvailHeight,
      AvailWidth
    };
    virtual Value get(ExecState *exec, const Identifier &propertyName) const;
    Value getValueProperty(ExecState *exec, int token) const;
  private:
    KHTMLView *view;
    virtual const ClassInfo* classInfo() const { return &info; }
    static const ClassInfo info;
  };

  class KDE_EXPORT Window : public ObjectImp {
    friend QGuardedPtr<KHTMLPart> getInstance();
    friend class Location;
    friend class WindowFunc;
    friend class WindowQObject;
    friend class ScheduledAction;
  public:
    Window(khtml::ChildFrame *p);
  public:
    ~Window();
    /**
     * Returns and registers a window object. In case there's already a Window
     * for the specified part p this will be returned in order to have unique
     * bindings.
     */
    static Value retrieve(KParts::ReadOnlyPart *p);
    /**
     * Returns the Window object for a given part
     */
    static Window *retrieveWindow(KParts::ReadOnlyPart *p);
    /**
     * returns a pointer to the Window object this javascript interpreting instance
     * was called from.
     */
    static Window *retrieveActive(ExecState *exec);
    KParts::ReadOnlyPart *part() const;
    virtual void mark();
    virtual bool hasProperty(ExecState *exec, const Identifier &p) const;
    virtual Value get(ExecState *exec, const Identifier &propertyName) const;
    virtual void put(ExecState *exec, const Identifier &propertyName, const Value &value, int attr = None);
    virtual bool toBoolean(ExecState *exec) const;
    void scheduleClose();
    void closeNow();
    void delayedGoHistory(int steps);
    void goHistory(int steps);
    void goURL(ExecState* exec, const QString& url, bool lockHistory);
    Value openWindow(ExecState *exec, const List &args);
    Value executeOpenWindow(ExecState *exec, const KURL& url, const QString& frameName, const QString& features);
    void resizeTo(QWidget* tl, int width, int height);
    void afterScriptExecution();
    bool isSafeScript(ExecState *exec) const {
        KParts::ReadOnlyPart *activePart = static_cast<KJS::ScriptInterpreter *>(  exec->interpreter() )->part();
      if ( activePart == part() ) return true;
      return checkIsSafeScript( activePart );
    }
    Location *location() const;
    ObjectImp* frames( ExecState* exec ) const;
    JSEventListener *getJSEventListener(const Value &val, bool html = false);
    JSLazyEventListener *getJSLazyEventListener(const QString &code, const QString &name, DOM::NodeImpl* node);
    void clear( ExecState *exec );
    virtual UString toString(ExecState *exec) const;

    // Set the current "event" object
    void setCurrentEvent( DOM::Event *evt );

    QPtrDict<JSEventListener> jsEventListeners;
    virtual const ClassInfo* classInfo() const { return &info; }
    static const ClassInfo info;
    enum { Closed, Crypto, DefaultStatus, Status, Document, Node, EventCtor, Range,
           NodeFilter, DOMException, CSSRule, Frames, _History, _External, Event, InnerHeight,
           InnerWidth, Length, _Location, Navigate, Name, _Navigator, _Konqueror, ClientInformation,
           OffscreenBuffering, Opener, OuterHeight, OuterWidth, PageXOffset, PageYOffset,
           Parent, Personalbar, ScreenX, ScreenY, Scrollbars, Scroll, ScrollBy,
           ScreenTop, ScreenLeft, AToB, BToA, FrameElement,
           ScrollTo, ScrollX, ScrollY, MoveBy, MoveTo, ResizeBy, ResizeTo, Self, _Window, Top, _Screen,
           Image, Option, Alert, Confirm, Prompt, Open, SetTimeout, ClearTimeout,
           XMLHttpRequest, XMLSerializer, DOMParser,
           Focus, Blur, Close, SetInterval, ClearInterval, CaptureEvents, ReleaseEvents,
           Print, AddEventListener, RemoveEventListener, SideBar,
           Onabort, Onblur,
           Onchange, Onclick, Ondblclick, Ondragdrop, Onerror, Onfocus,
           Onkeydown, Onkeypress, Onkeyup, Onload, Onmousedown, Onmousemove,
           Onmouseout, Onmouseover, Onmouseup, Onmove, Onreset, Onresize,
           Onselect, Onsubmit, Onunload };
    WindowQObject *winq;

    void forgetSuppressedWindows();
    void showSuppressedWindows();

  protected:
    enum DelayedActionId { NullAction, DelayedClose, DelayedGoHistory };

    Value getListener(ExecState *exec, int eventId) const;
    void setListener(ExecState *exec, int eventId, Value func);
  private:
    struct DelayedAction;
    friend struct DelayedAction;

    bool checkIsSafeScript( KParts::ReadOnlyPart* activePart ) const;

    QGuardedPtr<khtml::ChildFrame> m_frame;
    Screen *screen;
    History *history;
    External *external;
    FrameArray *m_frames;
    Location *loc;
    DOM::Event *m_evt;

    struct DelayedAction {
      DelayedAction() : actionId(NullAction) {} // for QValueList
      DelayedAction( DelayedActionId id, QVariant p = QVariant() ) : actionId(id), param(p) {}
      DelayedActionId actionId;
      QVariant param; // just in case
    };
    QValueList<DelayedAction> m_delayed;

    struct SuppressedWindowInfo {
      SuppressedWindowInfo() {}  // for QValueList
      SuppressedWindowInfo( ExecState *e, KURL u, QString fr, QString fe ) : exec(e), url(u), frameName(fr), features(fe) {}
      ExecState *exec;
      KURL url;
      QString frameName;
      QString features;
    };
    QValueList<SuppressedWindowInfo> m_suppressedWindowInfo;
  };

  /**
   * An action (either function or string) to be executed after a specified
   * time interval, either once or repeatedly. Used for window.setTimeout()
   * and window.setInterval()
   */
  class ScheduledAction {
  public:
    ScheduledAction(Object _func, List _args, QTime _nextTime, int _interval, bool _singleShot, int _timerId);
    ScheduledAction(QString _code, QTime _nextTime, int _interval, bool _singleShot, int _timerId);
    ~ScheduledAction();
    bool execute(Window *window);
    void mark();

    ObjectImp *func;
    List args;
    QString code;
    bool isFunction;
    bool singleShot;

    QTime nextTime;
    int interval;
    bool executing;
    int timerId;
  };

  class KDE_EXPORT WindowQObject : public QObject {
    Q_OBJECT
  public:
    WindowQObject(Window *w);
    ~WindowQObject();
    int installTimeout(const Identifier &handler, int t, bool singleShot);
    int installTimeout(const Value &func, List args, int t, bool singleShot);
    void clearTimeout(int timerId);
    void mark();
    bool hasTimers() const;
  public slots:
    void timeoutClose();
  protected slots:
    void parentDestroyed();
  protected:
    void timerEvent(QTimerEvent *e);
    void setNextTimer();
  private:
    Window *parent;
    QPtrList<ScheduledAction> scheduledActions;
    int pausedTime;
    int lastTimerId;
    bool currentlyDispatching;
  };

  class Location : public ObjectImp {
  public:
    ~Location();
    virtual Value get(ExecState *exec, const Identifier &propertyName) const;
    virtual void put(ExecState *exec, const Identifier &propertyName, const Value &value, int attr = None);
    virtual Value toPrimitive(ExecState *exec, Type preferred) const;
    virtual UString toString(ExecState *exec) const;
    enum { Hash, Href, Hostname, Host, Pathname, Port, Protocol, Search, EqualEqual,
           Assign, Replace, Reload, ToString };
    KParts::ReadOnlyPart *part() const;
    virtual const ClassInfo* classInfo() const { return &info; }
    static const ClassInfo info;
  private:
    friend class Window;
    Location(khtml::ChildFrame *f);
    QGuardedPtr<khtml::ChildFrame> m_frame;
  };

#ifdef Q_WS_QWS
  class Konqueror : public ObjectImp {
    friend class KonquerorFunc;
  public:
    Konqueror(KHTMLPart *p) : part(p) { }
    virtual Value get(ExecState *exec, const Identifier &propertyName) const;
    virtual bool hasProperty(ExecState *exec, const Identifier &p) const;
    virtual UString toString(ExecState *exec) const;
    virtual const ClassInfo* classInfo() const { return &info; }
    static const ClassInfo info;
  private:
    KHTMLPart *part;
  };
#endif

} // namespace

#endif
