// -*- c-basic-offset: 2 -*-
/*
 *  This file is part of the KDE libraries
 *  Copyright (C) 2000 Harri Porten (porten@kde.org)
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

#include <assert.h>
#include <qtimer.h>
#include <qinputdialog.h>
#include <qstringlist.h>
#include <qpaintdevicemetrics.h>
#include <qapplication.h>
#include <dom_string.h>
#include <kdebug.h>
#include <kurl.h>
#include <kmessagebox.h>
#include <klocale.h>
#include <kparts/browserextension.h>
#include <kparts/browserinterface.h>
#include <kwin.h>
#include <kwinmodule.h>
#include <kconfig.h>
#include <assert.h>

#include <kjs/kjs.h>
#include <kjs/operations.h>
#include <kjs/lookup.h>
#include "kjs_proxy.h"
#include "kjs_window.h"
#include "kjs_navigator.h"
#include "kjs_html.h"
#include "kjs_dom.h"
#include "kjs_range.h"
#include "kjs_traversal.h"
#include "kjs_events.h"

#include <qevent.h>
#include "khtmlview.h"
#include <html_element.h>
#include <html_documentimpl.h>
#include "khtml_part.h"
#include "xml/dom_docimpl.h"
#include "xml/dom2_eventsimpl.h"

using namespace KJS;

namespace KJS {

////////////////////// History Object ////////////////////////

class History : public HostImp {
  friend class HistoryFunc;
public:
  History(KHTMLPart *p) : part(p) { }
  virtual KJSO get(const UString &p) const;
  virtual bool hasProperty(const UString &p, bool recursive) const;
private:
   QGuardedPtr<KHTMLPart> part;
};

class HistoryFunc : public DOMFunction {
public:
  HistoryFunc(const History *h, int i) : history(h), id(i) { }
  Completion tryExecute(const List &args);
  enum { Back, Forward, Go };

private:
  const History *history;
  int id;
};

class FrameArray : public HostImp {
public:
  FrameArray(KHTMLPart *p) : part(p) { }
  KJSO get(const UString &p) const;
private:
  QGuardedPtr<KHTMLPart> part;
};

#ifdef Q_WS_QWS
class KonquerorFunc : public DOMFunction {
public:
  KonquerorFunc(const Konqueror* k, const char* name)
    : konqueror(k), m_name(name) { }
  Completion tryExecute(const List &args);

private:
  const Konqueror* konqueror;
  QCString m_name;
};
#endif
}; // namespace KJS

////////////////////// Screen Object ////////////////////////

// table for screen object
/*
@begin ecmaScreenTable Screen 8
height        DontEnum|ReadOnly
width         DontEnum|ReadOnly
colorDepth    DontEnum|ReadOnly
pixelDepth    DontEnum|ReadOnly
availLeft     DontEnum|ReadOnly
availTop      DontEnum|ReadOnly
availHeight   DontEnum|ReadOnly
availWidth    DontEnum|ReadOnly
@end
*/

static const struct HashEntry2 ecmaScreenTableEntries[] = {
   { "availLeft", Screen::availLeft, DontEnum|ReadOnly, 0 },
   { 0, 0, 0, 0 },
   { "availTop", Screen::availTop, DontEnum|ReadOnly, 0 },
   { "height", Screen::height, DontEnum|ReadOnly, 0 },
   { "width", Screen::width, DontEnum|ReadOnly, &ecmaScreenTableEntries[8] },
   { 0, 0, 0, 0 },
   { "availHeight", Screen::availHeight, DontEnum|ReadOnly, 0 },
   { "pixelDepth", Screen::pixelDepth, DontEnum|ReadOnly, &ecmaScreenTableEntries[9] },
   { "colorDepth", Screen::colorDepth, DontEnum|ReadOnly, 0 },
   { "availWidth", Screen::availWidth, DontEnum|ReadOnly, 0 }
};

static const struct HashTable2 ecmaScreenTable = { 2, 10, ecmaScreenTableEntries, 10 };

KJSO Screen::get(const UString &p) const
{
  int token = Lookup::find(&ecmaScreenTable, p);
  if (token < 0)
    return ObjectImp::get(p);

  KWinModule info;
  QPaintDeviceMetrics m(QApplication::desktop());

  switch( token ) {
  case height:
    return Number(QApplication::desktop()->height());
  case width:
    return Number(QApplication::desktop()->width());
  case colorDepth:
  case pixelDepth:
    return Number(m.depth());
  case availLeft:
    return Number(info.workArea().left());
  case availTop:
    return Number(info.workArea().top());
  case availHeight:
    return Number(info.workArea().height());
  case availWidth:
    return Number(info.workArea().width());
  default:
    assert(!"Screen::get: unhandled switch case");
    return Undefined();
  }
}

////////////////////// Window Object ////////////////////////

const TypeInfo Window::info = { "Window", HostType, 0, 0, 0 };

Window::Window(KHTMLPart *p)
  : m_part(p), screen(0), history(0), frames(0), loc(0), m_evt(0)
{
  winq = new WindowQObject(this);
  //kdDebug(6070) << "Window::Window this=" << this << " part=" << m_part << endl;
}

Window::~Window()
{
  kdDebug(6070) << "Window::~Window this=" << this << " part=" << m_part << endl;
  delete winq;
}

Window *Window::retrieveWindow(KHTMLPart *p)
{
  // prototype set in kjs_create()
  return (Window*)retrieve(p)->prototype();
}

Window *Window::retrieveActive()
{
  return static_cast<KJS::Window*>(KJS::Global::current().prototype().imp());
}

Imp *Window::retrieve(KHTMLPart *p)
{
  assert(p);
  KJSProxy *proxy = p->jScript();
  if (proxy)
    return proxy->jScript()->globalObject(); // the Global object is the "window"
  else
    return Null().imp();
}

Location *Window::location() const
{
  if (!loc)
    const_cast<Window*>(this)->loc = new Location(m_part);
  return loc;
}

// reference our special objects during garbage collection
void Window::mark(Imp *)
{
  HostImp::mark();
  if (screen && !screen->marked())
    screen->mark();
  if (history && !history->marked())
    history->mark();
  if (frames && !frames->marked())
    frames->mark();
  if (loc && !loc->marked())
    loc->mark();

#if 0
  // Mark all Window objects from the map. Necessary to keep
  // existing window properties, such as 'opener'.
  if (window_map)
  {
     WindowMap::Iterator it = window_map->begin();
     for ( ; it != window_map->end() ; ++it )
     {
       if (it.data() != this && !it.data()->refcount)
       {
         //kdDebug() << "Window::mark marking window from dict " << it.data() << endl;
         it.data()->mark();
       }
     }
  }
#endif
}

bool Window::hasProperty(const UString &/*p*/, bool /*recursive*/) const
{
  // emulate IE behaviour: it doesn't throw exceptions when undeclared
  // variables are used. Returning true here will lead to get() returning
  // 'undefined' in those cases.
  return true;

#if 0
  if (p == "closed")
    return true;

  // we don't want any operations on a closed window
  if (m_part.isNull())
    return false;

  // Properties
  if (p == "crypto" ||
      p == "defaultStatus" ||
      p == "document" ||
      p == "Node" ||
      p == "Range" ||
      p == "NodeFilter" ||
      p == "DOMException" ||
      p == "frames" ||
      p == "history" ||
    //  p == "event" ||
      p == "innerHeight" ||
      p == "innerWidth" ||
      p == "konqueror" ||
      p == "length" ||
      p == "location" ||
      p == "name" ||
      p == "navigator" ||
      p == "offscreenBuffering" ||
      p == "opener" ||
      p == "outerHeight" ||
      p == "outerWidth" ||
      p == "pageXOffset" ||
      p == "pageYOffset" ||
      p == "parent" ||
      p == "personalbar" ||
      p == "screen" ||
      p == "screenX" ||
      p == "screenY" ||
      p == "scrollbars" ||
      p == "self" ||
      p == "status" ||
      p == "top" ||
      p == "window" ||
      p == "Image" ||
      p == "Option" ||
  // Methods
      p == "alert" ||
      p == "confirm" ||
      p == "blur" ||
      p == "clearInterval" ||
      p == "clearTimeout" ||
      p == "close" ||
      p == "focus" ||
      p == "moveBy" ||
      p == "moveTo" ||
      p == "open" ||
      p == "prompt" ||
      p == "resizeBy" ||
      p == "resizeTo" ||
      p == "scroll" ||
      p == "scrollBy" ||
      p == "scrollTo" ||
      p == "setInterval" ||
      p == "setTimeout" ||
      p == "onabort" ||
      p == "onblur" ||
      p == "onchange" ||
      p == "onclick" ||
      p == "ondblclick" ||
      p == "ondragdrop" ||
      p == "onerror" ||
      p == "onfocus" ||
      p == "onkeydown" ||
      p == "onkeypress" ||
      p == "onkeyup" ||
      p == "onload" ||
      p == "onmousedown" ||
      p == "onmousemove" ||
      p == "onmouseout" ||
      p == "onmouseover" ||
      p == "onmouseup" ||
      p == "onmove" ||
      p == "onreset" ||
      p == "onresize" ||
      p == "onselect" ||
      p == "onsubmit" ||
      p == "onunload" ||
      HostImp::hasProperty(p,recursive) ||
      m_part->findFrame( p.qstring() ))
    return true;

  //  allow shortcuts like 'Image1' instead of document.images.Image1
  if (m_part->document().isHTMLDocument()) { // might be XML
    DOM::HTMLCollection coll = m_part->htmlDocument().all();
    DOM::HTMLElement element = coll.namedItem(p.string());
    if (!element.isNull()) {
        return true;
    }
  }

  return false;
#endif
}

String Window::toString() const
{
  return UString( "[object Window]" );
}

KJSO Window::get(const UString &p) const
{
#ifdef KJS_VERBOSE
  kdDebug(6070) << "Window::get " << p.qstring() << endl;
#endif
  if (p == "closed")
    return Boolean(m_part.isNull());

  // we don't want any operations on a closed window
  if (m_part.isNull())
    return Undefined();

  // Reimplement toString ourselves to avoid getting [object Object] from our prototype
  if (p == "toString")
    return Function(new WindowFunc(this, WindowFunc::ToString));

  if (Imp::hasProperty(p,true)) {
    if (isSafeScript())
      return Imp::get(p);
  }
  if (p == "crypto")
    return Undefined(); // ###
  else if (p == "defaultStatus" || p == "defaultstatus")
    return String(UString(m_part->jsDefaultStatusBarText()));
  else if (p == "status")
    return String(UString(m_part->jsStatusBarText()));
  else if (p == "document") {
    if (isSafeScript())
      return getDOMNode(m_part->document());
    else
      return Undefined();
  }
  else if (p == "Node")
    return getNodePrototype();
  else if (p == "Range")
    return getRangePrototype();
  else if (p == "NodeFilter")
    return getNodeFilterPrototype();
  else if (p == "DOMException")
    return getDOMExceptionPrototype();
  else if (p == "frames")
    return KJSO(frames ? frames :
		(const_cast<Window*>(this)->frames = new FrameArray(m_part)));
  else if (p == "history")
    return KJSO(history ? history :
		(const_cast<Window*>(this)->history = new History(m_part)));

  else if (p == "event") {
    if (m_evt)
      return getDOMEvent(*m_evt);
    else
      return Undefined();
  }

  else if (p == "innerHeight")
    return Number(m_part->view()->visibleHeight());
  else if (p == "innerWidth")
    return Number(m_part->view()->visibleWidth());
  else if (p == "length")
    return Number(m_part->frames().count());
  else if (p == "location") {
    if (isSafeScript())
      return KJSO(location());
    else
      return Undefined();
  }
  else if (p == "name")
    return String(m_part->name());
  else if (p == "navigator")
    return KJSO(new Navigator(m_part));
#ifdef Q_WS_QWS
  else if (p == "konqueror")
    return KJSO(new Konqueror(m_part));
#endif
  else if (p == "offscreenBuffering")
    return Boolean(true);
  else if (p == "opener")
    if (!m_part->opener())
      return Null(); 	// ### a null Window might be better, but == null
    else                // doesn't work yet
      return retrieve(m_part->opener());
  else if (p == "outerHeight" || p == "outerWidth") {
    if (!m_part->widget())
      return Number(0);
    KWin::Info inf = KWin::info(m_part->widget()->topLevelWidget()->winId());
    return Number(p == "outerHeight" ?
		  inf.geometry.height() : inf.geometry.width());
  } else if (p == "pageXOffset")
    return Number(m_part->view()->contentsX());
  else if (p == "pageYOffset")
    return Number(m_part->view()->contentsY());
  else if (p == "parent")
    return KJSO(retrieve(m_part->parentPart() ? m_part->parentPart() : (KHTMLPart*)m_part));
  else if (p == "personalbar")
    return Undefined(); // ###
  else if (p == "screenX")
    return Number(m_part->view() ? m_part->view()->mapToGlobal(QPoint(0,0)).x() : 0);
  else if (p == "screenY")
    return Number(m_part->view() ? m_part->view()->mapToGlobal(QPoint(0,0)).y() : 0);
  else if (p == "scrollbars")
    return Undefined(); // ###
  else if (p == "scroll")
    return Function(new WindowFunc(this, WindowFunc::ScrollTo)); // compatibility
  else if (p == "scrollBy")
    return Function(new WindowFunc(this, WindowFunc::ScrollBy));
  else if (p == "scrollTo")
    return Function(new WindowFunc(this, WindowFunc::ScrollTo));
  else if (p == "moveBy")
    return Function(new WindowFunc(this, WindowFunc::MoveBy));
  else if (p == "moveTo")
    return Function(new WindowFunc(this, WindowFunc::MoveTo));
  else if (p == "resizeBy")
    return Function(new WindowFunc(this, WindowFunc::ResizeBy));
  else if (p == "resizeTo")
    return Function(new WindowFunc(this, WindowFunc::ResizeTo));
  else if (p == "self" || p == "window")
    return KJSO(retrieve(m_part));
  else if (p == "top") {
    KHTMLPart *p = m_part;
    while (p->parentPart())
      p = p->parentPart();
    return KJSO(retrieve(p));
  }
  else if (p == "screen")
    return KJSO(screen ? screen :
		(const_cast<Window*>(this)->screen = new Screen()));
  else if (p == "Image")
    return KJSO(new ImageConstructor(Global::current(), m_part->document()));
  else if (p == "Option")
    return KJSO(new OptionConstructor(m_part->document()));
  else if (p == "alert")
    return Function(new WindowFunc(this, WindowFunc::Alert));
  else if (p == "confirm")
    return Function(new WindowFunc(this, WindowFunc::Confirm));
  else if (p == "prompt")
    return Function(new WindowFunc(this, WindowFunc::Prompt));
  else if (p == "open")
    return Function(new WindowFunc(this, WindowFunc::Open));
  else if (p == "setTimeout") {
    if (isSafeScript())
      return Function(new WindowFunc(this, WindowFunc::SetTimeout));
    else
      return Undefined();
  }
  else if (p == "clearTimeout") {
    if (isSafeScript())
      return Function(new WindowFunc(this, WindowFunc::ClearTimeout));
    else
      return Undefined();
  }
  else if (p == "focus")
    return Function(new WindowFunc(this, WindowFunc::Focus));
  else if (p == "blur")
    return Function(new WindowFunc(this, WindowFunc::Blur));
  else if (p == "close")
    return Function(new WindowFunc(this, WindowFunc::Close));
  else if (p == "setInterval") {
    if (isSafeScript())
      return Function(new WindowFunc(this, WindowFunc::SetInterval));
    else
      return Undefined();
  }
  else if (p == "clearInterval") {
    if (isSafeScript())
      return Function(new WindowFunc(this, WindowFunc::ClearInterval));
    else
      return Undefined();
  }
  else if (p == "onabort") {
    if (isSafeScript())
      return getListener(DOM::EventImpl::ABORT_EVENT);
    else
      return Undefined();
  }
  else if (p == "onblur") {
    if (isSafeScript())
      return getListener(DOM::EventImpl::BLUR_EVENT);
    else
      return Undefined();
  }
  else if (p == "onchange") {
    if (isSafeScript())
      return getListener(DOM::EventImpl::CHANGE_EVENT);
    else
      return Undefined();
  }
  else if (p == "onclick") {
    if (isSafeScript())
      return getListener(DOM::EventImpl::KHTML_CLICK_EVENT);
    else
      return Undefined();
  }
  else if (p == "ondblclick") {
    if (isSafeScript())
      return getListener(DOM::EventImpl::KHTML_DBLCLICK_EVENT);
    else
      return Undefined();
  }
  else if (p == "ondragdrop") {
    if (isSafeScript())
      return getListener(DOM::EventImpl::KHTML_DRAGDROP_EVENT);
    else
      return Undefined();
  }
  else if (p == "onerror") {
    if (isSafeScript())
      return getListener(DOM::EventImpl::KHTML_ERROR_EVENT);
    else
      return Undefined();
  }
  else if (p == "onfocus") {
    if (isSafeScript())
      return getListener(DOM::EventImpl::FOCUS_EVENT);
    else
      return Undefined();
  }
  else if (p == "onkeydown") {
    if (isSafeScript())
      return getListener(DOM::EventImpl::KHTML_KEYDOWN_EVENT);
    else
      return Undefined();
  }
  else if (p == "onkeypress") {
    if (isSafeScript())
      return getListener(DOM::EventImpl::KHTML_KEYPRESS_EVENT);
    else
      return Undefined();
  }
  else if (p == "onkeyup") {
    if (isSafeScript())
      return getListener(DOM::EventImpl::KHTML_KEYUP_EVENT);
    else
      return Undefined();
  }
  else if (p == "onload") {
    if (isSafeScript())
      return getListener(DOM::EventImpl::LOAD_EVENT);
    else
      return Undefined();
  }
  else if (p == "onmousedown") {
    if (isSafeScript())
      return getListener(DOM::EventImpl::MOUSEDOWN_EVENT);
    else
      return Undefined();
  }
  else if (p == "onmousemove") {
    if (isSafeScript())
      return getListener(DOM::EventImpl::MOUSEMOVE_EVENT);
    else
      return Undefined();
  }
  else if (p == "onmouseout") {
    if (isSafeScript())
      return getListener(DOM::EventImpl::MOUSEOUT_EVENT);
    else
      return Undefined();
  }
  else if (p == "onmouseover") {
    if (isSafeScript())
      return getListener(DOM::EventImpl::MOUSEOVER_EVENT);
    else
      return Undefined();
  }
  else if (p == "onmouseup") {
    if (isSafeScript())
      return getListener(DOM::EventImpl::MOUSEUP_EVENT);
    else
      return Undefined();
  }
  else if (p == "onmove") {
    if (isSafeScript())
      return getListener(DOM::EventImpl::KHTML_MOVE_EVENT);
    else
      return Undefined();
  }
  else if (p == "onreset") {
    if (isSafeScript())
      return getListener(DOM::EventImpl::RESET_EVENT);
    else
      return Undefined();
  }
  else if (p == "onresize") {
    if (isSafeScript())
      return getListener(DOM::EventImpl::RESIZE_EVENT);
    else
      return Undefined();
  }
  else if (p == "onselect") {
    if (isSafeScript())
      return getListener(DOM::EventImpl::SELECT_EVENT);
    else
      return Undefined();
  }
  else if (p == "onsubmit") {
    if (isSafeScript())
      return getListener(DOM::EventImpl::SUBMIT_EVENT);
    else
      return Undefined();
  }
  else if (p == "onunload") {
    if (isSafeScript())
      return getListener(DOM::EventImpl::UNLOAD_EVENT);
    else
      return Undefined();
  }

  KHTMLPart *kp = m_part->findFrame( p.qstring() );
  if (kp)
    return KJSO(retrieve(kp));

  // allow shortcuts like 'Image1' instead of document.images.Image1
  if (isSafeScript() &&
      m_part->document().isHTMLDocument()) { // might be XML
    DOM::HTMLCollection coll = m_part->htmlDocument().all();
    DOM::HTMLElement element = coll.namedItem(p.string());
    if (!element.isNull()) {
        return getDOMNode(element);
    }
  }

  return Undefined();
}

void Window::put(const UString &p, const KJSO &v)
{
#ifdef KJS_VERBOSE
  kdDebug(6070) << "Window::put " << p.qstring() << endl;
#endif
  if (p == "status") {
    String s = v.toString();
    m_part->setJSStatusBarText(s.value().qstring());
  }
  else if (p == "defaultStatus" || p == "defaultstatus") {
    String s = v.toString();
    m_part->setJSDefaultStatusBarText(s.value().qstring());
  }
  else if (p == "location") {
    QString str = v.toString().value().qstring();
    KHTMLPart* p = Window::retrieveActive()->m_part;
    if ( p )
      m_part->scheduleRedirection(0, p->htmlDocument().
                                  completeURL(str).string().prepend( "target://_self/?#" ));
  }
  else if (p == "onabort") {
    if (isSafeScript())
      setListener(DOM::EventImpl::ABORT_EVENT,v);
  }
  else if (p == "onblur") {
    if (isSafeScript())
      setListener(DOM::EventImpl::BLUR_EVENT,v);
  }
  else if (p == "onchange") {
    if (isSafeScript())
      setListener(DOM::EventImpl::CHANGE_EVENT,v);
  }
  else if (p == "onclick") {
    if (isSafeScript())
      setListener(DOM::EventImpl::KHTML_CLICK_EVENT,v);
  }
  else if (p == "ondblclick") {
    if (isSafeScript())
      setListener(DOM::EventImpl::KHTML_DBLCLICK_EVENT,v);
  }
  else if (p == "ondragdrop") {
    if (isSafeScript())
      setListener(DOM::EventImpl::KHTML_DRAGDROP_EVENT,v);
  }
  else if (p == "onerror") {
    if (isSafeScript())
      setListener(DOM::EventImpl::KHTML_ERROR_EVENT,v);
  }
  else if (p == "onfocus") {
    if (isSafeScript())
      setListener(DOM::EventImpl::FOCUS_EVENT,v);
  }
  else if (p == "onkeydown") {
    if (isSafeScript())
      setListener(DOM::EventImpl::KHTML_KEYDOWN_EVENT,v);
  }
  else if (p == "onkeypress") {
    if (isSafeScript())
      setListener(DOM::EventImpl::KHTML_KEYPRESS_EVENT,v);
  }
  else if (p == "onkeyup") {
    if (isSafeScript())
      setListener(DOM::EventImpl::KHTML_KEYUP_EVENT,v);
  }
  else if (p == "onload") {
    if (isSafeScript())
      setListener(DOM::EventImpl::LOAD_EVENT,v);
  }
  else if (p == "onmousedown") {
    if (isSafeScript())
      setListener(DOM::EventImpl::MOUSEDOWN_EVENT,v);
  }
  else if (p == "onmousemove") {
    if (isSafeScript())
      setListener(DOM::EventImpl::MOUSEMOVE_EVENT,v);
  }
  else if (p == "onmouseout") {
    if (isSafeScript())
      setListener(DOM::EventImpl::MOUSEOUT_EVENT,v);
  }
  else if (p == "onmouseover") {
    if (isSafeScript())
      setListener(DOM::EventImpl::MOUSEOVER_EVENT,v);
  }
  else if (p == "onmouseup") {
    if (isSafeScript())
      setListener(DOM::EventImpl::MOUSEUP_EVENT,v);
  }
  else if (p == "onmove") {
    if (isSafeScript())
      setListener(DOM::EventImpl::KHTML_MOVE_EVENT,v);
  }
  else if (p == "onreset") {
    if (isSafeScript())
      setListener(DOM::EventImpl::RESET_EVENT,v);
  }
  else if (p == "onresize") {
    if (isSafeScript())
      setListener(DOM::EventImpl::RESIZE_EVENT,v);
  }
  else if (p == "onselect") {
    if (isSafeScript())
      setListener(DOM::EventImpl::SELECT_EVENT,v);
  }
  else if (p == "onsubmit") {
    if (isSafeScript())
      setListener(DOM::EventImpl::SUBMIT_EVENT,v);
  }
  else if (p == "onunload") {
    if (isSafeScript())
      setListener(DOM::EventImpl::UNLOAD_EVENT,v);
  }
  else if (p == "name") {
    if (isSafeScript())
      m_part->setName( v.toString().value().qstring().local8Bit().data() );
  }
  else {
    if (isSafeScript())
      Imp::put(p, v);
  }
}

Boolean Window::toBoolean() const
{
  return Boolean(!m_part.isNull());
}

int Window::installTimeout(const UString &handler, int t, bool singleShot)
{
  return winq->installTimeout(handler, t, singleShot);
}

void Window::clearTimeout(int timerId)
{
  winq->clearTimeout(timerId);
}

void Window::scheduleClose()
{
  kdDebug(6070) << "WindowFunc::tryExecute window.close() " << m_part << endl;
  QTimer::singleShot( 0, winq, SLOT( timeoutClose() ) );
}

bool Window::isSafeScript() const
{
  KHTMLPart *act = (KHTMLPart*)KJS::Global::current().extra();
  if (!act)
      kdDebug(6070) << "Window::isSafeScript: KJS::Global::current().extra() is 0L!" << endl;
  return act && originCheck(m_part->url(), act->url());
}

void Window::setListener(int eventId, KJSO func)
{
  if (!isSafeScript())
    return;
  DOM::DocumentImpl *doc = static_cast<DOM::DocumentImpl*>(m_part->htmlDocument().handle());
  if (!doc)
    return;

  doc->setWindowEventListener(eventId,getJSEventListener(func,true));
}

KJSO Window::getListener(int eventId) const
{
  if (!isSafeScript())
    return Undefined();
  DOM::DocumentImpl *doc = static_cast<DOM::DocumentImpl*>(m_part->htmlDocument().handle());
  if (!doc)
    return Undefined();

  DOM::EventListener *listener = doc->getWindowEventListener(eventId);
  if (listener)
    return static_cast<JSEventListener*>(listener)->listenerObj();
  else
    return Null();
}


JSEventListener *Window::getJSEventListener(const KJSO &obj, bool html)
{
  if (obj.isA(KJS::NullType))
    return 0;

  QListIterator<JSEventListener> it(jsEventListeners);

  for (; it.current(); ++it)
    if (it.current()->listenerObj().imp() == obj.imp())
      return it.current();

  JSEventListener *listener = new JSEventListener(obj,KJSO(this),html);
//  jsEventListeners.append(listener);
  return listener;
}

void Window::clear()
{
  delete winq;
  winq = 0;
}

void Window::setCurrentEvent( DOM::Event *evt )
{
  m_evt = evt;
  kdDebug() << "Window(part=" << m_part << ")::setCurrentEvent m_evt=" << evt << endl;
}

Completion WindowFunc::tryExecute(const List &args)
{
  KJSO result;
  QString str, str2;

  if (!window->m_part)
    return Completion(Normal);

  KHTMLPart *part = window->m_part;
  if (!part)
    return Completion(Normal);

  KHTMLView *widget = part->view();
  KJSO v = args[0];
  String s = v.toString();
  str = s.value().qstring();

  switch (id) {
  case Alert:
    part->xmlDocImpl()->updateRendering();
    KMessageBox::error(widget, str, "JavaScript");
    result = Undefined();
    break;
  case Confirm:
    part->xmlDocImpl()->updateRendering();
    result = Boolean((KMessageBox::warningYesNo(widget, str, "JavaScript",
                                  i18n("OK"), i18n("Cancel")) == KMessageBox::Yes));
    break;
  case Prompt:
    part->xmlDocImpl()->updateRendering();
    if (args.size() >= 2)
      str2 = QInputDialog::getText("Konqueror: Prompt", str,
#if QT_VERSION >= 300
                                   QLineEdit::Normal,
#endif
                                   args[1].toString().value().qstring());
    else
      str2 = QInputDialog::getText("Konqueror: Prompt", str);
    result = String(str2);
    break;
  case Open:
  {
    KConfig *config = new KConfig("konquerorrc");
    config->setGroup("Java/JavaScript Settings");
    int policy = config->readUnsignedNumEntry( "WindowOpenPolicy", 0 );
    if ( policy == 1 ) {
      if ( KMessageBox::questionYesNo(widget,
                                      i18n( "This site is trying to open up a new browser "
                                            "window using Javascript.\n\n"
                                            "Do you want to allow this?" ),
                                      i18n( "Confirmation: Javascript Popup" ) ) == KMessageBox::Yes )
        policy = 0;
    }

    if ( policy ) {
      result = Undefined();
    } else {
      KParts::WindowArgs winargs;

      // scan feature argument
      v = args[2];
      QString features;
      if (v.isDefined()) {
	features = v.toString().value().qstring();
	// specifying window params means false defaults
	winargs.menuBarVisible = false;
	winargs.toolBarsVisible = false;
	winargs.statusBarVisible = false;
	QStringList flist = QStringList::split(',', features);
	QStringList::ConstIterator it = flist.begin();
	while (it != flist.end()) {
	  QString s = *it++;
	  QString key, val;
	  int pos = s.find('=');
	  if (pos >= 0) {
	    key = s.left(pos).stripWhiteSpace().lower();
	    val = s.mid(pos + 1).stripWhiteSpace().lower();
	    if (key == "left" || key == "screenx")
	      winargs.x = val.toInt();
	    else if (key == "top" || key == "screeny")
	      winargs.y = val.toInt();
	    else if (key == "height")
	      winargs.height = val.toInt() + 4;
	    else if (key == "width")
	      winargs.width = val.toInt() + 4;
	    else
	      goto boolargs;
	    continue;
	  } else {
	    // leaving away the value gives true
	    key = s.stripWhiteSpace().lower();
	    val = "1";
	  }
	boolargs:
	  if (key == "menubar")
	    winargs.menuBarVisible = (val == "1" || val == "yes");
	  else if (key == "toolbar")
	    winargs.toolBarsVisible = (val == "1" || val == "yes");
	  else if (key == "location")	// ### missing in WindowArgs
	    winargs.toolBarsVisible = (val == "1" || val == "yes");
	  else if (key == "status" || key == "statusbar")
	    winargs.statusBarVisible = (val == "1" || val == "yes");
	  else if (key == "resizable")
	    winargs.resizable = (val == "1" || val == "yes");
	  else if (key == "fullscreen")
	    winargs.fullscreen = (val == "1" || val == "yes");
	}
      }

        // prepare arguments
        KURL url;
	if (!str.isEmpty())
	  url = part->htmlDocument().completeURL(str).string();

        KParts::URLArgs uargs;
        uargs.frameName = args[1].isDefined() ?
			  args[1].toString().value().qstring()
			  : QString("_blank");
        uargs.serviceType = "text/html";

        // request new window
        KParts::ReadOnlyPart *newPart = 0L;
        emit part->browserExtension()->createNewWindow("", uargs,winargs,newPart);
        if (newPart && newPart->inherits("KHTMLPart")) {
	  KHTMLPart *khtmlpart = static_cast<KHTMLPart*>(newPart);
	    //qDebug("opener set to %p (this Window's part) in new Window %p  (this Window=%p)",part,win,window);
            khtmlpart->setOpener(part);
	    khtmlpart->setOpenedByJS(true);
	    uargs.serviceType = QString::null;
	    if (uargs.frameName == "_blank")
              uargs.frameName = QString::null;
	    if (!url.isEmpty())
	      emit khtmlpart->browserExtension()->openURLRequest(url,uargs);
	    result = Window::retrieve(khtmlpart); // global object
        } else
            result = Undefined();
     }
  delete config;
  break;
  }
  case ScrollBy:
    if(args.size() == 2 && widget)
      widget->scrollBy(args[0].toInt32(), args[1].toInt32());
    result = Undefined();
    break;
  case ScrollTo:
    if(args.size() == 2 && widget)
      widget->setContentsPos(args[0].toInt32(), args[1].toInt32());
    result = Undefined();
    break;
  case MoveBy:
    if(args.size() == 2 && widget)
    {
      QWidget * tl = widget->topLevelWidget();
      QPoint dest = tl->pos() + QPoint( args[0].toInt32(), args[1].toInt32() );
      // Security check (the spec talks about UniversalBrowserWrite to disable this check...)
      if ( dest.x() >= 0 && dest.y() >= 0 &&
           dest.x()+tl->width() <= QApplication::desktop()->width() &&
           dest.y()+tl->height() <= QApplication::desktop()->height() )
          tl->move( dest );
    }
    result = Undefined();
    break;
  case MoveTo:
    if(args.size() == 2 && widget)
    {
      QWidget * tl = widget->topLevelWidget();
      QPoint dest = QPoint( args[0].toInt32(), args[1].toInt32() );
      // Security check (the spec talks about UniversalBrowserWrite to disable this check...)
      if ( dest.x() >= 0 && dest.y() >= 0 &&
           dest.x()+tl->width() <= QApplication::desktop()->width() &&
           dest.y()+tl->height() <= QApplication::desktop()->height() )
          tl->move( dest );
    }
    result = Undefined();
    break;
  case ResizeBy:
    if(args.size() == 2 && widget)
    {
      QWidget * tl = widget->topLevelWidget();
      QSize dest = tl->size() + QSize( args[0].toInt32(), args[1].toInt32() );
      // Security check: within desktop limits and bigger than 100x100 (per spec)
      if ( tl->x()+dest.width() <= QApplication::desktop()->width() &&
           tl->y()+dest.height() <= QApplication::desktop()->height() &&
           dest.width() >= 100 && dest.height() >= 100 )
          tl->resize( dest );
    }
    result = Undefined();
    break;
  case ResizeTo:
    if(args.size() == 2 && widget)
    {
      QWidget * tl = widget->topLevelWidget();
      QSize dest = QSize( args[0].toInt32(), args[1].toInt32() );
      // Security check: within desktop limits and bigger than 100x100 (per spec)
      if ( tl->x()+dest.width() <= QApplication::desktop()->width() &&
           tl->y()+dest.height() <= QApplication::desktop()->height() &&
           dest.width() >= 100 && dest.height() >= 100 )
          tl->resize( dest );
    }
    result = Undefined();
    break;
  case SetTimeout:
    if (args.size() == 2 && v.isA(StringType)) {
      int i = args[1].toInt32();
      int r = (const_cast<Window*>(window))->installTimeout(s.value(), i, true /*single shot*/);
      result = Number(r);
    }
    else if (args.size() >= 2 && v.derivedFrom(FunctionType)) {
      KJSO func = args[0];
      int i = args[1].toInt32();
      List *funcArgs = args.copy();
      funcArgs->removeFirst(); // all args after 2 go to the function
      funcArgs->removeFirst();
      int r = (const_cast<Window*>(window))->installTimeout(s.value(), i, true /*single shot*/);
      result = Number(r);
    }
    else
      result = Undefined();
    break;
  case SetInterval:
    if (args.size() == 2 && v.isA(StringType)) {
      int i = args[1].toInt32();
      int r = (const_cast<Window*>(window))->installTimeout(s.value(), i, false);
      result = Number(r);
    }
    else if (args.size() >= 2 && v.derivedFrom(FunctionType)) {
      KJSO func = args[0];
      int i = args[1].toInt32();
      List *funcArgs = args.copy();
      funcArgs->removeFirst(); // all args after 2 go to the function
      funcArgs->removeFirst();
      int r = (const_cast<Window*>(window))->installTimeout(s.value(), i, false);
      result = Number(r);
    }
    else
      result = Undefined();
    break;
  case ClearTimeout:
  case ClearInterval:
    result = Undefined();
    (const_cast<Window*>(window))->clearTimeout(v.toInt32());
    break;
  case Focus:
    if (widget)
      widget->setActiveWindow();
    result = Undefined();
    break;
  case Blur:
    result = Undefined();
    // TODO
    break;
  case Close:
      /* From http://developer.netscape.com/docs/manuals/js/client/jsref/window.htm :
        The close method closes only windows opened by JavaScript using the open method.
        If you attempt to close any other window, a confirm is generated, which
        lets the user choose whether the window closes.
        This is a security feature to prevent "mail bombs" containing self.close().
        However, if the window has only one document (the current one) in its
        session history, the close is allowed without any confirm. This is a
        special case for one-off windows that need to open other windows and
        then dispose of themselves.
        */
    if (!window->m_part->openedByJS())
    {
        // To conform to the SPEC, we only ask if the window
        // has more than one entry in the history (NS does that too).
        History history(part);
        if ( history.get( "length" ).toInt32() <= 1 ||
             KMessageBox::questionYesNo( window->part()->widget(), i18n("Close window ?"), i18n("Confirmation required") ) == KMessageBox::Yes )
            (const_cast<Window*>(window))->scheduleClose();
    }
    else
    {
        (const_cast<Window*>(window))->scheduleClose();
    }

    result = Undefined();
    break;
  case ToString:
    result = window->toString();
    break;
  }
  return Completion(ReturnValue, result);

}

////////////////////// ScheduledAction ////////////////////////

ScheduledAction::ScheduledAction(KJSO _func, List *_args, bool _singleShot)
{
  func = _func;
  args = _args;
  isFunction = true;
  singleShot = _singleShot;
}

ScheduledAction::ScheduledAction(QString _code, bool _singleShot)
{
  func = 0;
  args = 0;
  code = _code;
  isFunction = false;
  singleShot = _singleShot;
}

void ScheduledAction::execute(Window *window)
{
  if (isFunction) {
    if (func.implementsCall()) {
      func.executeCall(window,args);
    }
  }
  else {
    window->m_part->executeScript(code);
  }
}

ScheduledAction::~ScheduledAction()
{
  if (isFunction)
    delete args;
}

////////////////////// WindowQObject ////////////////////////

WindowQObject::WindowQObject(Window *w)
  : parent(w)
{
  part = parent->m_part;
  connect( parent->m_part, SIGNAL( destroyed() ),
           this, SLOT( parentDestroyed() ) );
}

WindowQObject::~WindowQObject()
{
  killTimers();
}

void WindowQObject::parentDestroyed()
{
  killTimers();
  QMapIterator<int,ScheduledAction*> it;
  for (it = scheduledActions.begin(); it != scheduledActions.end(); ++it) {
    //ScheduledAction *action = *it;
    scheduledActions.remove(it);
    //    ### delete action;
  }
}

int WindowQObject::installTimeout(const UString &handler, int t, bool singleShot)
{
  int id = startTimer(t);
  scheduledActions.insert(id, new ScheduledAction(handler.qstring(),singleShot));
  return id;
}

int WindowQObject::installTimeout(const KJSO &func, List *args, int t, bool singleShot)
{
  int id = startTimer(t);
  scheduledActions.insert(id, new ScheduledAction(func,args,singleShot));
  return id;
}

void WindowQObject::clearTimeout(int timerId, bool delAction)
{
  killTimer(timerId);
  if (delAction) {
    QMapIterator<int,ScheduledAction*> it = scheduledActions.find(timerId);
    if (it != scheduledActions.end()) {
      //ScheduledAction *action = *it;
      scheduledActions.remove(it);
      //      delete action;
    }
  }
}

void WindowQObject::timerEvent(QTimerEvent *e)
{
  ScheduledAction *action = scheduledActions[e->timerId()];

  // remove single shots installed by setTimeout()
  if (action->singleShot)
    clearTimeout(e->timerId(),false);

  if (!parent->part().isNull())
    action->execute(parent);

  //  if (action->singleShot)
    //    delete action; // already cleared by now
}

void WindowQObject::timeoutClose()
{
  if (!parent->part().isNull())
  {
    kdDebug(6070) << "WindowQObject::timeoutClose -> closing window" << endl;
    delete parent->m_part;
  }
}

KJSO FrameArray::get(const UString &p) const
{
#ifdef KJS_VERBOSE
  kdDebug(6070) << "FrameArray::get " << p.qstring() << " part=" << (void*)part << endl;
#endif
  if (part.isNull())
    return Undefined();

  QList<KParts::ReadOnlyPart> frames = part->frames();
  int len = frames.count();
  if (p == "length")
    return Number(len);
  else if (p== "location") // non-standard property, but works in NS and IE
    return Window::retrieve( part )->get( "location" );

  // check for the name or number
  KParts::ReadOnlyPart *frame = part->findFrame(p.qstring());
  if (!frame) {
    int i = (int)p.toDouble();
    if (i >= 0 && i < len)
      frame = frames.at(i);
  }

  // we are potentially fetching a reference to a another Window object here.
  // i.e. we may be accessing objects from another interpreter instance.
  // Therefore we have to be a bit careful with memory managment.
  if (frame && frame->inherits("KHTMLPart")) {
    const KHTMLPart *khtml = static_cast<const KHTMLPart*>(frame);
    return KJSO(Window::retrieve(const_cast<KHTMLPart*>(khtml)));
  }

  return HostImp::get(p);
}

////////////////////// Location Object ////////////////////////

Location::Location(KHTMLPart *p) : m_part(p) { }

KJSO Location::get(const UString &p) const
{
#ifdef KJS_VERBOSE
  kdDebug(6070) << "Location::get " << p.qstring() << " m_part=" << (void*)m_part << endl;
#endif

  if (m_part.isNull())
    return Undefined();

  KURL url = m_part->url();
  QString str;

  if (p == "hash")
    str = url.ref().isEmpty() ? QString("") : "#" + url.ref();    
  else if (p == "host") {
    str = url.host();
    if (url.port())
      str += ":" + QString::number((int)url.port());
    // Note: this is the IE spec. The NS spec swaps the two, it says
    // "The hostname property is the concatenation of the host and port properties, separated by a colon."
    // Bleh.
  } else if (p == "hostname")
    str = url.host();
  else if (p == "href"){
      if (!url.hasPath())
          str = url.prettyURL()+"/";
      else
          str = url.prettyURL();
  }
  else if (p == "pathname")
    str = url.path().isEmpty() ? QString("/") : url.path();
  else if (p == "port")
    str = url.port() ? QString::number((int)url.port()) : QString::fromLatin1("");
  else if (p == "protocol")
    str = url.protocol()+":";
  else if (p == "search")
    str = url.query();
  else if (p == "[[==]]")
    return toString();
  else if (p == "toString")
    return Function(new LocationFunc(this, LocationFunc::ToString));
  else if (HostImp::hasProperty(p))
    return HostImp::get(p);
  else if (p == "replace")
    return Function(new LocationFunc(this, LocationFunc::Replace));
  else if (p == "reload")
    return Function(new LocationFunc(this, LocationFunc::Reload));
  else
    return Undefined();

  return String(str);
}

void Location::put(const UString &p, const KJSO &v)
{
#ifdef KJS_VERBOSE
  kdDebug(6070) << "Location::put " << p.qstring() << " m_part=" << (void*)m_part << endl;
#endif
  if (m_part.isNull())
    return;

  QString str = v.toString().value().qstring();
  KURL url;

  if (p == "href") {
    KHTMLPart* p = Window::retrieveActive()->part();
    if ( p )
      url = p->htmlDocument().completeURL( str ).string();
    else
      url = str;
  }
  else {
    url = m_part->url();
    if (p == "hash") url.setRef(str);
    else if (p == "host") {
      QString host = str.left(str.find(":"));
      QString port = str.mid(str.find(":")+1);
      url.setHost(host);
      url.setPort(port.toUInt());
    } else if (p == "hostname") url.setHost(str);
    else if (p == "pathname") url.setPath(str);
    else if (p == "port") url.setPort(str.toUInt());
    else if (p == "protocol")  url.setProtocol(str);
    else if (p == "search") url.setQuery(str);
    else {
      HostImp::put(p, v);
      return;
    }
  }
  m_part->scheduleRedirection(0, url.url().prepend( "target://_self/?#" ) );
}

KJSO Location::toPrimitive(Type) const
{
    return toString();
}

String Location::toString() const
{

 if (!m_part->url().hasPath())
        return String(m_part->url().prettyURL()+"/");
    else
        return String(m_part->url().prettyURL());
}

LocationFunc::LocationFunc( const Location* loc, int i) : location(loc), id(i) { };

Completion LocationFunc::tryExecute(const List &args)
{
  KHTMLPart *part = location->part();
  if (part) {
    switch (id) {
    case Replace:
      part->scheduleRedirection(0, args[0].toString().value().qstring().prepend( "target://_self/?#" ));
      break;
    case Reload:
      part->scheduleRedirection(0, part->url().url().prepend( "target://_self/?#" ) );
      break;
    case ToString:
      KJSO result = location->toString();
      return Completion(ReturnValue, result);
    }
  } else
    kdDebug(6070) << "LocationFunc::tryExecute - no part!" << endl;
  return Completion(Normal);
}

////////////////////// History Object ////////////////////////

bool History::hasProperty(const UString &p, bool recursive) const
{
  if (p == "back" ||
      p == "forward" ||
      p == "go" ||
      p == "length")
      return true;

  return HostImp::hasProperty(p, recursive);
}

KJSO History::get(const UString &p) const
{
  if (p == "back")
    return Function(new HistoryFunc(this, HistoryFunc::Back));
  else if (p == "forward")
    return Function(new HistoryFunc(this, HistoryFunc::Forward));
  else if (p == "go")
    return Function(new HistoryFunc(this, HistoryFunc::Go));
  else if (p == "length")
  {
      KParts::BrowserExtension *ext = part->browserExtension();
      if ( !ext )
          return Number( 0 );

      KParts::BrowserInterface *iface = ext->browserInterface();
      if ( !iface )
          return Number( 0 );

      QVariant length = iface->property( "historyLength" );

      if ( length.type() != QVariant::UInt )
          return Number( 0 );

      return Number( length.toUInt() );
  }
  else
    return HostImp::get(p);
}

Completion HistoryFunc::tryExecute(const List &args)
{
  KParts::BrowserExtension *ext = history->part->browserExtension();

  KJSO v = args[0];
  Number n;
  if(!v.isNull())
    n = v.toInteger();

  if(!ext)
    return Completion(Normal);

  KParts::BrowserInterface *iface = ext->browserInterface();

  if ( !iface )
    return Completion(Normal);

  switch (id) {
  case Back:
      iface->callMethod( "goHistory(int)", -1 );
//      emit ext->goHistory(-1);
      break;
  case Forward:
      iface->callMethod( "goHistory(int)", (int)1 );
//      emit ext->goHistory(1);
      break;
  case Go:
      iface->callMethod( "goHistory(int)", n.intValue() );
//      emit ext->goHistory(n.intValue());
      break;
  default:
    break;
  }

  return Completion(Normal);
}

/////////////////////////////////////////////////////////////////////////////

#ifdef Q_WS_QWS

bool Konqueror::hasProperty(const UString &p, bool recursive) const
{
  if ( p.qstring().startsWith( "goHistory" ) ) return false;

  return true;
}

KJSO Konqueror::get(const UString &p) const
{
  if ( p == "goHistory" || part->url().protocol() != "http" || part->url().host() != "localhost" )
    return Undefined();

  KParts::BrowserExtension *ext = part->browserExtension();
  if ( ext ) {
    KParts::BrowserInterface *iface = ext->browserInterface();
    if ( iface ) {
      QVariant prop = iface->property( p.qstring().latin1() );

      if ( prop.isValid() ) {
        switch( prop.type() ) {
        case QVariant::Int:
          return Number( prop.toInt() );
        case QVariant::String:
          return String( prop.toString() );
        default:
          break;
        }
      }
    }
  }

  return Function( new KonquerorFunc(this, p.qstring().latin1() ) );
}

Completion KonquerorFunc::tryExecute(const List &args)
{
  KParts::BrowserExtension *ext = konqueror->part->browserExtension();

  if(!ext)
    return Completion(Normal);

  KParts::BrowserInterface *iface = ext->browserInterface();

  if ( !iface )
    return Completion(Normal);

  QCString n = m_name.data();
  n += "()";
  iface->callMethod( n.data(), QVariant() );

  return Completion(Normal);
}

String Konqueror::toString() const
{
  return UString("[object Konqueror]");
}

#endif
/////////////////////////////////////////////////////////////////////////////

#include "kjs_window.moc"

