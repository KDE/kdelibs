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

#include <qtimer.h>
#include <qinputdialog.h>
#include <qpaintdevicemetrics.h>
#include <qapplication.h>
#include <kdebug.h>
#include <kmessagebox.h>
#include <klocale.h>
#include <kparts/browserinterface.h>
#include <kwin.h>
#include <kwinmodule.h>
#include <kconfig.h>
#include <assert.h>

#include <kjs/lookup.h>
#include "kjs_proxy.h"
#include "kjs_window.h"
#include "kjs_navigator.h"
#include "kjs_html.h"
#include "kjs_range.h"
#include "kjs_traversal.h"
#include "kjs_events.h"

#include "khtmlview.h"
#include "khtml_part.h"
#include "xml/dom2_eventsimpl.h"

using namespace KJS;

namespace KJS {

////////////////////// History Object ////////////////////////

  class History : public ObjectImp {
    friend class HistoryFunc;
  public:
    History(KHTMLPart *p) : part(p) { }
    virtual Value get(ExecState *exec, const UString &propertyName) const;
    Value getValue(ExecState *exec, int token) const;
    virtual const ClassInfo* classInfo() const { return &info; }
    static const ClassInfo info;
    enum { Back, Forward, Go, Length };
  private:
    QGuardedPtr<KHTMLPart> part;
  };

  class FrameArray : public ObjectImp {
  public:
    FrameArray(KHTMLPart *p) : part(p) { }
    virtual Value get(ExecState *exec, const UString &propertyName) const;
  private:
    QGuardedPtr<KHTMLPart> part;
  };

#ifdef Q_WS_QWS
  class KonquerorFunc : public DOMFunction {
  public:
    KonquerorFunc(const Konqueror* k, const char* name)
      : DOMFunction(), konqueror(k), m_name(name) { }
    virtual Value tryCall(ExecState *exec, Object &thisObj, const List &args);

  private:
    const Konqueror* konqueror;
    QCString m_name;
  };
#endif
}; // namespace KJS

#include "kjs_window.lut.h"

////////////////////// Screen Object ////////////////////////

// table for screen object
/*
@begin ScreenTable 7
  height        Screen::Height		DontEnum|ReadOnly
  width         Screen::Width		DontEnum|ReadOnly
  colorDepth    Screen::ColorDepth	DontEnum|ReadOnly
  pixelDepth    Screen::PixelDepth	DontEnum|ReadOnly
  availLeft     Screen::AvailLeft	DontEnum|ReadOnly
  availTop      Screen::AvailTop	DontEnum|ReadOnly
  availHeight   Screen::AvailHeight	DontEnum|ReadOnly
  availWidth    Screen::AvailWidth	DontEnum|ReadOnly
@end
*/

const ClassInfo Screen::info = { "Screen", 0, &ScreenTable, 0 };

Value Screen::get(ExecState *exec, const UString &p) const
{
  return lookupGetValue<Screen,ObjectImp>(exec,p,&ScreenTable,this);
}

Value Screen::getValue(ExecState *, int token) const
{
  KWinModule info;

  switch( token ) {
  case Height:
    return Number(QApplication::desktop()->height());
  case Width:
    return Number(QApplication::desktop()->width());
  case ColorDepth:
  case PixelDepth: {
    QPaintDeviceMetrics m(QApplication::desktop());
    return Number(m.depth());
  }
  case AvailLeft:
    return Number(info.workArea().left());
  case AvailTop:
    return Number(info.workArea().top());
  case AvailHeight:
    return Number(info.workArea().height());
  case AvailWidth:
    return Number(info.workArea().width());
  default:
    kdWarning() << "Screen::getValue unhandled token " << token << endl;
    return Undefined();
  }
}

////////////////////// Window Object ////////////////////////

const ClassInfo Window::info = { "Window", 0, &WindowTable, 0 };

/*
@begin WindowTable 77
  closed	Window::Closed		DontDelete|ReadOnly
  crypto	Window::Crypto		DontDelete|ReadOnly
  defaultStatus	Window::DefaultStatus	DontDelete
  defaultstatus	Window::DefaultStatus	DontDelete
  status	Window::Status		DontDelete
  document	Window::Document	DontDelete|ReadOnly
  Node		Window::Node		DontDelete|ReadOnly
  Range		Window::Range		DontDelete|ReadOnly
  NodeFilter	Window::NodeFilter	DontDelete|ReadOnly
  DOMException	Window::DOMException	DontDelete|ReadOnly
  frames	Window::Frames		DontDelete|ReadOnly
  history	Window::_History	DontDelete|ReadOnly
  event		Window::Event		DontDelete|ReadOnly
  innerHeight	Window::InnerHeight	DontDelete|ReadOnly
  innerWidth	Window::InnerWidth	DontDelete|ReadOnly
  length	Window::Length		DontDelete|ReadOnly
  location	Window::_Location	DontDelete
  name		Window::Name		DontDelete
  navigator	Window::_Navigator	DontDelete|ReadOnly
  konqueror	Window::Konqueror	DontDelete|ReadOnly
  offscreenBuffering	Window::OffscreenBuffering	DontDelete|ReadOnly
  opener	Window::Opener		DontDelete|ReadOnly
  outerHeight	Window::OuterHeight	DontDelete|ReadOnly
  outerWidth	Window::OuterWidth	DontDelete|ReadOnly
  pageXOffset	Window::PageXOffset	DontDelete|ReadOnly
  pageYOffset	Window::PageYOffset	DontDelete|ReadOnly
  parent	Window::Parent		DontDelete|ReadOnly
  personalbar	Window::Personalbar	DontDelete|ReadOnly
  screenX	Window::ScreenX		DontDelete|ReadOnly
  screenY	Window::ScreenY		DontDelete|ReadOnly
  scrollbars	Window::Scrollbars	DontDelete|ReadOnly
  scroll	Window::Scroll		DontDelete|Function 2
  scrollBy	Window::ScrollBy	DontDelete|Function 2
  scrollTo	Window::ScrollTo	DontDelete|Function 2
  moveBy	Window::MoveBy		DontDelete|Function 2
  moveTo	Window::MoveTo		DontDelete|Function 2
  resizeBy	Window::ResizeBy	DontDelete|Function 2
  resizeTo	Window::ResizeTo	DontDelete|Function 2
  self		Window::Self		DontDelete|ReadOnly
  window	Window::_Window		DontDelete|ReadOnly
  top		Window::Top		DontDelete|ReadOnly
  screen	Window::_Screen		DontDelete|ReadOnly
  Image		Window::Image		DontDelete|ReadOnly
  Option	Window::Option		DontDelete|ReadOnly
  alert		Window::Alert		DontDelete|Function 1
  confirm	Window::Confirm		DontDelete|Function 1
  prompt	Window::Prompt		DontDelete|Function 2
  open		Window::Open		DontDelete|Function 3
  setTimeout	Window::SetTimeout	DontDelete|Function 2
  clearTimeout	Window::ClearTimeout	DontDelete|Function 1
  focus		Window::Focus		DontDelete|Function 0
  blur		Window::Blur		DontDelete|Function 0
  close		Window::Close		DontDelete|Function 0
  setInterval	Window::SetInterval	DontDelete|Function 2
  clearInterval	Window::ClearInterval	DontDelete|Function 1
  onabort	Window::Onabort		DontDelete
  onblur	Window::Onblur		DontDelete
  onchange	Window::Onchange	DontDelete
  onclick	Window::Onclick		DontDelete
  ondblclick	Window::Ondblclick	DontDelete
  ondragdrop	Window::Ondragdrop	DontDelete
  onerror	Window::Onerror		DontDelete
  onfocus	Window::Onfocus		DontDelete
  onkeydown	Window::Onkeydown	DontDelete
  onkeypress	Window::Onkeypress	DontDelete
  onkeyup	Window::Onkeyup		DontDelete
  onload	Window::Onload		DontDelete
  onmousedown	Window::Onmousedown	DontDelete
  onmousemove	Window::Onmousemove	DontDelete
  onmouseout	Window::Onmouseout	DontDelete
  onmouseover	Window::Onmouseover	DontDelete
  onmouseup	Window::Onmouseup	DontDelete
  onmove	Window::Onmove		DontDelete
  onreset	Window::Onreset		DontDelete
  onresize	Window::Onresize	DontDelete
  onselect	Window::Onselect	DontDelete
  onsubmit	Window::Onsubmit	DontDelete
  onunload	Window::Onunload	DontDelete
@end
*/
IMPLEMENT_PROTOFUNC(WindowFunc)

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
  ValueImp *imp = retrieve( p ).imp();
  assert( imp );
#ifndef QWS
  assert( dynamic_cast<KJS::Window*>(imp) );
#endif
  return static_cast<KJS::Window*>(imp);
}

Window *Window::retrieveActive(ExecState *exec)
{
  ValueImp *imp = exec->interpreter()->globalObject().imp();
  assert( imp );
#ifndef QWS
  assert( dynamic_cast<KJS::Window*>(imp) );
#endif
  return static_cast<KJS::Window*>(imp);
}

Object Window::retrieve(KHTMLPart *p)
{
  assert(p);
  KJSProxy *proxy = KJSProxy::proxy( p );
  if (proxy)
    return proxy->interpreter()->globalObject(); // the Global object is the "window"
  else
    return Object();
}

Location *Window::location() const
{
  if (!loc)
    const_cast<Window*>(this)->loc = new Location(m_part);
  return loc;
}

// reference our special objects during garbage collection
void Window::mark()
{
  ObjectImp::mark();
  if (screen && !screen->marked())
    screen->mark();
  if (history && !history->marked())
    history->mark();
  if (frames && !frames->marked())
    frames->mark();
  //kdDebug() << "Window::mark " << this << " marking loc=" << loc << endl;
  if (loc && !loc->marked())
    loc->mark();
}

bool Window::hasProperty(ExecState * /*exec*/, const UString &/*p*/, bool /*recursive*/) const
{
  //fprintf( stderr, "Window::hasProperty: always saying true\n" );

  // emulate IE behaviour: it doesn't throw exceptions when undeclared
  // variables are used. Returning true here will lead to get() returning
  // 'undefined' in those cases.
  return true;
}

UString Window::toString(ExecState *) const
{
  return "[object Window]";
}

Value Window::get(ExecState *exec, const UString &p) const
{
#ifdef KJS_VERBOSE
  kdDebug(6070) << "Window::get " << p.qstring() << endl;
#endif
  if ( p == "closed" )
    return Boolean(m_part.isNull());

  // we don't want any operations on a closed window
  if (m_part.isNull())
    return Undefined();

  // Look for overrides first
  ValueImp * val = ObjectImp::getDirect(p);
  if (val) {
    kdDebug() << "Window::get found dynamic property '" << p.ascii() << "'" << endl;
    if (isSafeScript(exec))
      return Value(val);
  }

  const HashEntry* entry = Lookup::findEntry(&WindowTable, p);
  if (entry)
  {
    kdDebug() << "token: " << entry->value << endl;
    switch( entry->value ) {
    case Crypto:
      return Undefined(); // ###
    case DefaultStatus:
      return String(UString(m_part->jsDefaultStatusBarText()));
    case Status:
      return String(UString(m_part->jsStatusBarText()));
    case Document:
      if (isSafeScript(exec))
      {
        Value val = getDOMNode(exec,m_part->document());
        // Cache the value. This also prevents the GC from deleting the document
        // while the window exists (important if the users sets properties on it).
        const_cast<Window*>(this)->ObjectImp::put( exec, UString("document"), val, Internal );
        // ## This is a hack. The whole cache should persist.
        return val;
      }
      else
        return Undefined();
    case Node:
      return getNodeConstructor(exec);
    case Range:
      return getRangeConstructor(exec);
    case NodeFilter:
      return getNodeFilterConstructor(exec);
    case DOMException:
      return getDOMExceptionConstructor(exec);
    case Frames:
      return Value(frames ? frames :
                   (const_cast<Window*>(this)->frames = new FrameArray(m_part)));
    case _History:
      return Value(history ? history :
                   (const_cast<Window*>(this)->history = new History(m_part)));

    case Event:
      if (m_evt)
        return getDOMEvent(exec,*m_evt);
      else
        return Undefined();
    case InnerHeight:
      return Number(m_part->view()->visibleHeight());
    case InnerWidth:
      return Number(m_part->view()->visibleWidth());
    case Length:
      return Number(m_part->frames().count());
    case _Location:
      if (isSafeScript(exec))
        return Value(location());
      else
        return Undefined();
    case Name:
      return String(m_part->name());
    case _Navigator:
      return new Navigator(m_part);
#ifdef Q_WS_QWS
    case Konqueror:
      return new Konqueror(m_part);
#endif
    case OffscreenBuffering:
      return Boolean(true);
    case Opener:
      if (!m_part->opener())
        return Null(); 	// ### a null Window might be better, but == null
      else                // doesn't work yet
        return retrieve(m_part->opener());
    case OuterHeight:
    case OuterWidth:
    {
      if (!m_part->widget())
        return Number(0);
      KWin::Info inf = KWin::info(m_part->widget()->topLevelWidget()->winId());
      return Number(entry->value == OuterHeight ?
                    inf.geometry.height() : inf.geometry.width());
    }
    case PageXOffset:
      return Number(m_part->view()->contentsX());
    case PageYOffset:
      return Number(m_part->view()->contentsY());
    case Parent:
      return Value(retrieve(m_part->parentPart() ? m_part->parentPart() : (KHTMLPart*)m_part));
    case Personalbar:
      return Undefined(); // ###
    case ScreenX:
      return Number(m_part->view() ? m_part->view()->mapToGlobal(QPoint(0,0)).x() : 0);
    case ScreenY:
      return Number(m_part->view() ? m_part->view()->mapToGlobal(QPoint(0,0)).y() : 0);
    case Scrollbars:
      return Undefined(); // ###
    case Self:
    case _Window:
      return Value(retrieve(m_part));
    case Top: {
      KHTMLPart *p = m_part;
      while (p->parentPart())
        p = p->parentPart();
      return Value(retrieve(p));
    }
    case _Screen:
      return Value(screen ? screen :
                   (const_cast<Window*>(this)->screen = new Screen()));
    case Image:
      return Value(new ImageConstructorImp(exec, m_part->document()));
    case Option:
      return Value(new OptionConstructorImp(exec, m_part->document()));
    case Alert:
    case Confirm:
    case Prompt:
    case Open:
    case Focus:
    case Blur:
    case Close:
    case Scroll: // compatibility
    case ScrollBy:
    case ScrollTo:
    case MoveBy:
    case MoveTo:
    case ResizeBy:
    case ResizeTo:
      return lookupOrCreateFunction<WindowFunc>(exec,p,this,entry->value,entry->params,entry->attr);
    case SetTimeout:
    case ClearTimeout:
    case SetInterval:
    case ClearInterval:
      if (isSafeScript(exec))
        return lookupOrCreateFunction<WindowFunc>(exec,p,this,entry->value,entry->params,entry->attr);
      else
        return Undefined();
    case Onabort:
      if (isSafeScript(exec))
        return getListener(exec,DOM::EventImpl::ABORT_EVENT);
      else
        return Undefined();
    case Onblur:
      if (isSafeScript(exec))
        return getListener(exec,DOM::EventImpl::BLUR_EVENT);
      else
        return Undefined();
    case Onchange:
      if (isSafeScript(exec))
        return getListener(exec,DOM::EventImpl::CHANGE_EVENT);
      else
        return Undefined();
    case Onclick:
      if (isSafeScript(exec))
        return getListener(exec,DOM::EventImpl::KHTML_CLICK_EVENT);
      else
        return Undefined();
    case Ondblclick:
      if (isSafeScript(exec))
        return getListener(exec,DOM::EventImpl::KHTML_DBLCLICK_EVENT);
      else
        return Undefined();
    case Ondragdrop:
      if (isSafeScript(exec))
        return getListener(exec,DOM::EventImpl::KHTML_DRAGDROP_EVENT);
      else
        return Undefined();
    case Onerror:
      if (isSafeScript(exec))
        return getListener(exec,DOM::EventImpl::KHTML_ERROR_EVENT);
      else
        return Undefined();
    case Onfocus:
      if (isSafeScript(exec))
        return getListener(exec,DOM::EventImpl::FOCUS_EVENT);
      else
        return Undefined();
    case Onkeydown:
      if (isSafeScript(exec))
        return getListener(exec,DOM::EventImpl::KHTML_KEYDOWN_EVENT);
      else
        return Undefined();
    case Onkeypress:
      if (isSafeScript(exec))
        return getListener(exec,DOM::EventImpl::KHTML_KEYPRESS_EVENT);
      else
        return Undefined();
    case Onkeyup:
      if (isSafeScript(exec))
        return getListener(exec,DOM::EventImpl::KHTML_KEYUP_EVENT);
      else
        return Undefined();
    case Onload:
      if (isSafeScript(exec))
        return getListener(exec,DOM::EventImpl::LOAD_EVENT);
      else
        return Undefined();
    case Onmousedown:
      if (isSafeScript(exec))
        return getListener(exec,DOM::EventImpl::MOUSEDOWN_EVENT);
      else
        return Undefined();
    case Onmousemove:
      if (isSafeScript(exec))
        return getListener(exec,DOM::EventImpl::MOUSEMOVE_EVENT);
      else
        return Undefined();
    case Onmouseout:
      if (isSafeScript(exec))
        return getListener(exec,DOM::EventImpl::MOUSEOUT_EVENT);
      else
        return Undefined();
    case Onmouseover:
      if (isSafeScript(exec))
        return getListener(exec,DOM::EventImpl::MOUSEOVER_EVENT);
      else
        return Undefined();
    case Onmouseup:
      if (isSafeScript(exec))
        return getListener(exec,DOM::EventImpl::MOUSEUP_EVENT);
      else
        return Undefined();
    case Onmove:
      if (isSafeScript(exec))
        return getListener(exec,DOM::EventImpl::KHTML_MOVE_EVENT);
      else
        return Undefined();
    case Onreset:
      if (isSafeScript(exec))
        return getListener(exec,DOM::EventImpl::RESET_EVENT);
      else
        return Undefined();
    case Onresize:
      if (isSafeScript(exec))
        return getListener(exec,DOM::EventImpl::RESIZE_EVENT);
      else
        return Undefined();
    case Onselect:
      if (isSafeScript(exec))
        return getListener(exec,DOM::EventImpl::SELECT_EVENT);
      else
        return Undefined();
    case Onsubmit:
      if (isSafeScript(exec))
        return getListener(exec,DOM::EventImpl::SUBMIT_EVENT);
      else
        return Undefined();
    case Onunload:
      if (isSafeScript(exec))
        return getListener(exec,DOM::EventImpl::UNLOAD_EVENT);
      else
        return Undefined();
    }
  }
  KHTMLPart *kp = m_part->findFrame( p.qstring() );
  if (kp)
    return Value(retrieve(kp));

  // allow shortcuts like 'Image1' instead of document.images.Image1
  if (isSafeScript(exec) &&
      m_part->document().isHTMLDocument()) { // might be XML
    DOM::HTMLCollection coll = m_part->htmlDocument().all();
    DOM::HTMLElement element = coll.namedItem(p.string());
    if (!element.isNull()) {
      return getDOMNode(exec,element);
    }
  }

  kdDebug() << "Window::get property not found: " << p.qstring() << endl;
  return Undefined();
}

void Window::put(ExecState* exec, const UString &p, const Value &v, int attr)
{
  // Called by an internal KJS call (e.g. InterpreterImp's constructor) ?
  // If yes, save time and jump directly to ObjectImp.
  if ( attr != None )
  {
    ObjectImp::put( exec, p, v, attr );
    return;
  }

#ifdef KJS_VERBOSE
  kdDebug(6070) << "Window::put " << p.qstring() << endl;
#endif
  const HashEntry* entry = Lookup::findEntry(&WindowTable, p);
  if (entry)
  {
    switch( entry->value ) {
    case Status: {
      String s = v.toString(exec);
      m_part->setJSStatusBarText(s.value().qstring());
      return;
    }
    case DefaultStatus: {
      String s = v.toString(exec);
      m_part->setJSDefaultStatusBarText(s.value().qstring());
      return;
    }
    case _Location: {
      QString str = v.toString(exec).qstring();
      KHTMLPart* p = Window::retrieveActive(exec)->m_part;
      if ( p )
        m_part->scheduleRedirection(0, p->htmlDocument().
                                    completeURL(str).string().prepend( "target://_self/?#" ));
      return;
    }
    case Onabort:
      if (isSafeScript(exec))
        setListener(exec, DOM::EventImpl::ABORT_EVENT,v);
      return;
    case Onblur:
      if (isSafeScript(exec))
        setListener(exec, DOM::EventImpl::BLUR_EVENT,v);
      return;
    case Onchange:
      if (isSafeScript(exec))
        setListener(exec, DOM::EventImpl::CHANGE_EVENT,v);
      return;
    case Onclick:
      if (isSafeScript(exec))
        setListener(exec,DOM::EventImpl::KHTML_CLICK_EVENT,v);
      return;
    case Ondblclick:
      if (isSafeScript(exec))
        setListener(exec,DOM::EventImpl::KHTML_DBLCLICK_EVENT,v);
      return;
    case Ondragdrop:
      if (isSafeScript(exec))
        setListener(exec,DOM::EventImpl::KHTML_DRAGDROP_EVENT,v);
      return;
    case Onerror:
      if (isSafeScript(exec))
        setListener(exec,DOM::EventImpl::KHTML_ERROR_EVENT,v);
      return;
    case Onfocus:
      if (isSafeScript(exec))
        setListener(exec,DOM::EventImpl::FOCUS_EVENT,v);
      return;
    case Onkeydown:
      if (isSafeScript(exec))
        setListener(exec,DOM::EventImpl::KHTML_KEYDOWN_EVENT,v);
      return;
    case Onkeypress:
      if (isSafeScript(exec))
        setListener(exec,DOM::EventImpl::KHTML_KEYPRESS_EVENT,v);
      return;
    case Onkeyup:
      if (isSafeScript(exec))
        setListener(exec,DOM::EventImpl::KHTML_KEYUP_EVENT,v);
      return;
    case Onload:
      if (isSafeScript(exec))
        setListener(exec,DOM::EventImpl::LOAD_EVENT,v);
      return;
    case Onmousedown:
      if (isSafeScript(exec))
        setListener(exec,DOM::EventImpl::MOUSEDOWN_EVENT,v);
      return;
    case Onmousemove:
      if (isSafeScript(exec))
        setListener(exec,DOM::EventImpl::MOUSEMOVE_EVENT,v);
      return;
    case Onmouseout:
      if (isSafeScript(exec))
        setListener(exec,DOM::EventImpl::MOUSEOUT_EVENT,v);
      return;
    case Onmouseover:
      if (isSafeScript(exec))
        setListener(exec,DOM::EventImpl::MOUSEOVER_EVENT,v);
      return;
    case Onmouseup:
      if (isSafeScript(exec))
        setListener(exec,DOM::EventImpl::MOUSEUP_EVENT,v);
      return;
    case Onmove:
      if (isSafeScript(exec))
        setListener(exec,DOM::EventImpl::KHTML_MOVE_EVENT,v);
      return;
    case Onreset:
      if (isSafeScript(exec))
        setListener(exec,DOM::EventImpl::RESET_EVENT,v);
      return;
    case Onresize:
      if (isSafeScript(exec))
        setListener(exec,DOM::EventImpl::RESIZE_EVENT,v);
      return;
    case Onselect:
      if (isSafeScript(exec))
        setListener(exec,DOM::EventImpl::SELECT_EVENT,v);
      return;
    case Onsubmit:
      if (isSafeScript(exec))
        setListener(exec,DOM::EventImpl::SUBMIT_EVENT,v);
      return;
    case Onunload:
      if (isSafeScript(exec))
        setListener(exec,DOM::EventImpl::UNLOAD_EVENT,v);
      return;
    case Name:
      if (isSafeScript(exec))
        m_part->setName( v.toString(exec).qstring().local8Bit().data() );
      return;
    default:
      break;
    }
  }
  if (isSafeScript(exec))
    ObjectImp::put(exec, p, v, attr);
}

bool Window::toBoolean(ExecState *) const
{
  return !m_part.isNull();
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
  kdDebug(6070) << "Window::scheduleClose window.close() " << m_part << endl;
  QTimer::singleShot( 0, winq, SLOT( timeoutClose() ) );
}

bool Window::isSafeScript(ExecState *exec) const
{
  KHTMLPart *act = static_cast<KJS::ScriptInterpreter *>( exec->interpreter() )->part();
  if (!act)
    kdDebug(6070) << "Window::isSafeScript: KJS::Global::current().extra() is 0L!" << endl;
  return act && originCheck(m_part->url(), act->url());
}

void Window::setListener(ExecState *exec, int eventId, Value func)
{
  if (!isSafeScript(exec))
    return;
  DOM::DocumentImpl *doc = static_cast<DOM::DocumentImpl*>(m_part->htmlDocument().handle());
  if (!doc)
    return;

  doc->setWindowEventListener(eventId,getJSEventListener(func,true));
}

Value Window::getListener(ExecState *exec, int eventId) const
{
  if (!isSafeScript(exec))
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


JSEventListener *Window::getJSEventListener(const Value& val, bool html)
{
  Object obj = Object::dynamicCast(val);
  if (obj.isNull())
    return 0;

  QListIterator<JSEventListener> it(jsEventListeners);

  for (; it.current(); ++it)
    if (it.current()->listenerObj().imp() == obj.imp())
      return it.current();

  JSEventListener *listener = new JSEventListener(obj,Object(this),html);
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
  //kdDebug() << "Window(part=" << m_part << ")::setCurrentEvent m_evt=" << evt << endl;
}

Value WindowFunc::tryCall(ExecState *exec, Object &thisObj, const List &args)
{
  Window *window = static_cast<Window *>(thisObj.imp());
  Value result;
  QString str, str2;

  if (!window->m_part)
    return Undefined();

  KHTMLPart *part = window->m_part;
  if (!part)
    return Undefined();

  KHTMLView *widget = part->view();
  Value v = args[0];
  UString s = v.toString(exec);
  str = s.qstring();

  switch (id) {
  case Window::Alert:
    part->xmlDocImpl()->updateRendering();
    KMessageBox::error(widget, str, "JavaScript");
    result = Undefined();
    break;
  case Window::Confirm:
    part->xmlDocImpl()->updateRendering();
    result = Boolean((KMessageBox::warningYesNo(widget, str, "JavaScript",
                                                i18n("OK"), i18n("Cancel")) == KMessageBox::Yes));
    break;
  case Window::Prompt:
    part->xmlDocImpl()->updateRendering();
    if (args.size() >= 2)
      str2 = QInputDialog::getText("Konqueror: Prompt", str,
#if QT_VERSION >= 300
                                   QLineEdit::Normal,
#endif
                                   args[1].toString(exec).qstring());
    else
      str2 = QInputDialog::getText("Konqueror: Prompt", str);
    result = String(str2);
    break;
  case Window::Open:
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
      if (!v.isNull()) {
        features = v.toString(exec).qstring();
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
      uargs.frameName = !args[1].isNull() ?
                        args[1].toString(exec).qstring()
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
  case Window::ScrollBy:
    if(args.size() == 2 && widget)
      widget->scrollBy(args[0].toInt32(exec), args[1].toInt32(exec));
    result = Undefined();
    break;
  case Window::ScrollTo:
    if(args.size() == 2 && widget)
      widget->setContentsPos(args[0].toInt32(exec), args[1].toInt32(exec));
    result = Undefined();
    break;
  case Window::MoveBy:
    if(args.size() == 2 && widget)
    {
      QWidget * tl = widget->topLevelWidget();
      QPoint dest = tl->pos() + QPoint( args[0].toInt32(exec), args[1].toInt32(exec) );
      // Security check (the spec talks about UniversalBrowserWrite to disable this check...)
      if ( dest.x() >= 0 && dest.y() >= 0 &&
           dest.x()+tl->width() <= QApplication::desktop()->width() &&
           dest.y()+tl->height() <= QApplication::desktop()->height() )
        tl->move( dest );
    }
    result = Undefined();
    break;
  case Window::MoveTo:
    if(args.size() == 2 && widget)
    {
      QWidget * tl = widget->topLevelWidget();
      QPoint dest = QPoint( args[0].toInt32(exec), args[1].toInt32(exec) );
      // Security check (the spec talks about UniversalBrowserWrite to disable this check...)
      if ( dest.x() >= 0 && dest.y() >= 0 &&
           dest.x()+tl->width() <= QApplication::desktop()->width() &&
           dest.y()+tl->height() <= QApplication::desktop()->height() )
        tl->move( dest );
    }
    result = Undefined();
    break;
  case Window::ResizeBy:
    if(args.size() == 2 && widget)
    {
      QWidget * tl = widget->topLevelWidget();
      QSize dest = tl->size() + QSize( args[0].toInt32(exec), args[1].toInt32(exec) );
      // Security check: within desktop limits and bigger than 100x100 (per spec)
      if ( tl->x()+dest.width() <= QApplication::desktop()->width() &&
           tl->y()+dest.height() <= QApplication::desktop()->height() &&
           dest.width() >= 100 && dest.height() >= 100 )
        tl->resize( dest );
    }
    result = Undefined();
    break;
  case Window::ResizeTo:
    if(args.size() == 2 && widget)
    {
      QWidget * tl = widget->topLevelWidget();
      QSize dest = QSize( args[0].toInt32(exec), args[1].toInt32(exec) );
      // Security check: within desktop limits and bigger than 100x100 (per spec)
      if ( tl->x()+dest.width() <= QApplication::desktop()->width() &&
           tl->y()+dest.height() <= QApplication::desktop()->height() &&
           dest.width() >= 100 && dest.height() >= 100 )
        tl->resize( dest );
    }
    result = Undefined();
    break;
  case Window::SetTimeout:
    if (args.size() == 2 && v.isA(StringType)) {
      int i = args[1].toInt32(exec);
      int r = (const_cast<Window*>(window))->installTimeout(s, i, true /*single shot*/);
      result = Number(r);
    }
    else if (args.size() >= 2 && Object::dynamicCast(v).implementsCall()) {
      Value func = args[0];
      int i = args[1].toInt32(exec);
#if 0
//  ### TODO
      List *funcArgs = args.copy();
      funcArgs->removeFirst(); // all args after 2 go to the function
      funcArgs->removeFirst();
#endif
      int r = (const_cast<Window*>(window))->installTimeout(s, i, true /*single shot*/);
      result = Number(r);
    }
    else
      result = Undefined();
    break;
  case Window::SetInterval:
    if (args.size() == 2 && v.isA(StringType)) {
      int i = args[1].toInt32(exec);
      int r = (const_cast<Window*>(window))->installTimeout(s, i, false);
      result = Number(r);
    }
    else if (args.size() >= 2 && Object::dynamicCast(v).implementsCall()) {
      Value func = args[0];
      int i = args[1].toInt32(exec);
#if 0
// ### TODO
      List *funcArgs = args.copy();
      funcArgs->removeFirst(); // all args after 2 go to the function
      funcArgs->removeFirst();
#endif
      int r = (const_cast<Window*>(window))->installTimeout(s, i, false);
      result = Number(r);
    }
    else
      result = Undefined();
    break;
  case Window::ClearTimeout:
  case Window::ClearInterval:
    result = Undefined();
    (const_cast<Window*>(window))->clearTimeout(v.toInt32(exec));
    break;
  case Window::Focus:
    if (widget)
      widget->setActiveWindow();
    result = Undefined();
    break;
  case Window::Blur:
    result = Undefined();
    // TODO
    break;
  case Window::Close:
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
      if ( history.get( exec, "length" ).toInt32(exec) <= 1 ||
           KMessageBox::questionYesNo( window->part()->widget(), i18n("Close window ?"), i18n("Confirmation required") ) == KMessageBox::Yes )
        (const_cast<Window*>(window))->scheduleClose();
    }
    else
    {
      (const_cast<Window*>(window))->scheduleClose();
    }

    result = Undefined();
    break;
  }
  return result;
}

////////////////////// ScheduledAction ////////////////////////

ScheduledAction::ScheduledAction(Object _func, List _args, bool _singleShot)
{
  func = _func;
  args = _args;
  isFunction = true;
  singleShot = _singleShot;
}

ScheduledAction::ScheduledAction(QString _code, bool _singleShot)
{
  //func = 0;
  //args = 0;
  code = _code;
  isFunction = false;
  singleShot = _singleShot;
}

void ScheduledAction::execute(Window *window)
{
  if (isFunction) {
    if (func.implementsCall()) {
      // #### check this
      Q_ASSERT( window->m_part );
      if ( window->m_part )
      {
        KJS::Interpreter *interpreter = KJSProxy::proxy( window->m_part )->interpreter();
        ExecState *exec = interpreter->globalExec();
        Q_ASSERT( window == interpreter->globalObject().imp() );
        Object obj( window );
        func.call(exec,obj,args); // note that call() creates its own execution state for the func call
      }
    }
  }
  else {
    window->m_part->executeScript(code);
  }
}

ScheduledAction::~ScheduledAction()
{
  // args is a value now
  //if (isFunction)
  //  delete args;
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

int WindowQObject::installTimeout(const Value &func, List args, int t, bool singleShot)
{
  Object objFunc = Object::dynamicCast( func );
  int id = startTimer(t);
  scheduledActions.insert(id, new ScheduledAction(objFunc,args,singleShot));
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

Value FrameArray::get(ExecState *exec, const UString &p) const
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
    return Window::retrieve( part ).get( exec, "location" );

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
    return Window::retrieve(const_cast<KHTMLPart*>(khtml));
  }

  return ObjectImp::get(exec, p);
}

////////////////////// Location Object ////////////////////////

const ClassInfo Location::info = { "Location", 0, 0, 0 };
/*
@begin LocationTable 11
  hash		Location::Hash		DontDelete
  host		Location::Host		DontDelete
  hostname	Location::Hostname	DontDelete
  href		Location::Href		DontDelete
  pathname	Location::Pathname	DontDelete
  port		Location::Port		DontDelete
  protocol	Location::Protocol	DontDelete
  search	Location::Search	DontDelete
  [[==]]	Location::EqualEqual	DontDelete|ReadOnly
  toString	Location::ToString	DontDelete|Function 0
  replace	Location::Replace	DontDelete|Function 1
  reload	Location::Reload	DontDelete|Function 0
@end
*/
IMPLEMENT_PROTOFUNC(LocationFunc)
Location::Location(KHTMLPart *p) : m_part(p)
{
  //kdDebug(6070) << "Location::Location " << this << " m_part=" << (void*)m_part << endl;
}

Location::~Location()
{
  //kdDebug(6070) << "Location::~Location " << this << " m_part=" << (void*)m_part << endl;
}

Value Location::get(ExecState *exec, const UString &p) const
{
#ifdef KJS_VERBOSE
  kdDebug(6070) << "Location::get " << p.qstring() << " m_part=" << (void*)m_part << endl;
#endif

  if (m_part.isNull())
    return Undefined();

  KURL url = m_part->url();
  const HashEntry *entry = Lookup::findEntry(&LocationTable, p);
  if (entry)
    switch (entry->value) {
    case Hash:
      return String( url.ref().isNull() ? QString("") : "#" + url.ref() );
    case Host: {
      UString str = url.host();
      if (url.port())
        str += ":" + QString::number((int)url.port());
      return String(str);
      // Note: this is the IE spec. The NS spec swaps the two, it says
      // "The hostname property is the concatenation of the host and port properties, separated by a colon."
      // Bleh.
    }
    case Hostname:
      return String( url.host() );
    case Href:
      if (!url.hasPath())
        return String( url.prettyURL()+"/" );
      else
        return String( url.prettyURL() );
    case Pathname:
      return String( url.path().isEmpty() ? QString("/") : url.path() );
    case Port:
      return String( url.port() ? QString::number((int)url.port()) : QString::fromLatin1("") );
    case Protocol:
      return String( url.protocol()+":" );
    case Search:
      return String( url.query() );
    case EqualEqual: // [[==]]
      return String(toString(exec));
    case ToString:
      return lookupOrCreateFunction<LocationFunc>(exec,p,this,entry->value,entry->params,entry->attr);
    }
  // Look for overrides
  ValueImp * val = ObjectImp::getDirect(p);
  if (val)
    return Value(val);
  if (entry)
    switch (entry->value) {
    case Replace:
      return lookupOrCreateFunction<LocationFunc>(exec,p,this,entry->value,entry->params,entry->attr);
    case Reload:
      return lookupOrCreateFunction<LocationFunc>(exec,p,this,entry->value,entry->params,entry->attr);
    }

  return Undefined();
}

void Location::put(ExecState *exec, const UString &p, const Value &v, int attr)
{
#ifdef KJS_VERBOSE
  kdDebug(6070) << "Location::put " << p.qstring() << " m_part=" << (void*)m_part << endl;
#endif
  if (m_part.isNull())
    return;

  QString str = v.toString(exec).qstring();
  KURL url = m_part->url();
  const HashEntry *entry = Lookup::findEntry(&LocationTable, p);
  if (entry)
    switch (entry->value) {
    case Href: {
      KHTMLPart* p = Window::retrieveActive(exec)->part();
      if ( p )
        url = p->htmlDocument().completeURL( str ).string();
      else
        url = str;
      break;
    }
    case Hash:
      url.setRef(str);
      break;
    case Host: {
      QString host = str.left(str.find(":"));
      QString port = str.mid(str.find(":")+1);
      url.setHost(host);
      url.setPort(port.toUInt());
      break;
    }
    case Hostname:
      url.setHost(str);
      break;
    case Pathname:
      url.setPath(str);
      break;
    case Port:
      url.setPort(str.toUInt());
      break;
    case Protocol:
      url.setProtocol(str);
      break;
    case Search:
      url.setQuery(str);
      break;
    }
  else {
    ObjectImp::put(exec, p, v, attr);
    return;
  }

  m_part->scheduleRedirection(0, url.url().prepend( "target://_self/?#" ) );
}

Value Location::toPrimitive(ExecState *exec, Type) const
{
  return String(toString(exec));
}

UString Location::toString(ExecState *) const
{
  if (!m_part->url().hasPath())
    return m_part->url().prettyURL()+"/";
  else
    return m_part->url().prettyURL();
}

Value LocationFunc::tryCall(ExecState *exec, Object &thisObj, const List &args)
{
  Location *location = static_cast<Location *>(thisObj.imp());
  KHTMLPart *part = location->part();
  if (part) {
    switch (id) {
    case Location::Replace:
    {
      QString str = args[0].toString(exec).qstring();
      KHTMLPart* p = Window::retrieveActive(exec)->part();
      if ( p )
        part->scheduleRedirection(0, p->htmlDocument().
                                  completeURL(str).string().prepend( "target://_self/?#" ));
      break;
    }
    case Location::Reload:
      part->scheduleRedirection(0, part->url().url().prepend( "target://_self/?#" ) );
      break;
    case Location::ToString:
      return String(location->toString(exec));
    }
  } else
    kdDebug(6070) << "LocationFunc::tryExecute - no part!" << endl;
  return Undefined();
}

////////////////////// History Object ////////////////////////

const ClassInfo History::info = { "History", 0, 0, 0 };
/*
@begin HistoryTable 4
  length	History::Length		DontDelete|ReadOnly
  back		History::Back		DontDelete|Function 0
  forward	History::Forward	DontDelete|Function 0
  go		History::Go		DontDelete|Function 1
@end
*/
IMPLEMENT_PROTOFUNC(HistoryFunc)

Value History::get(ExecState *exec, const UString &p) const
{
  return lookupGet<HistoryFunc,History,ObjectImp>(exec,p,&HistoryTable,this);
}

Value History::getValue(ExecState *, int token) const
{
  switch (token) {
  case Length:
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
  default:
    kdWarning() << "Unhandled token in History::getValue : " << token << endl;
    return Value();
  }
}

Value HistoryFunc::tryCall(ExecState *exec, Object &thisObj, const List &args)
{
  History *history = static_cast<History *>(thisObj.imp());
  KParts::BrowserExtension *ext = history->part->browserExtension();

  Value v = args[0];
  Number n;
  if(!v.isNull())
    n = v.toInteger(exec);

  if(!ext)
    return Undefined();

  KParts::BrowserInterface *iface = ext->browserInterface();

  if ( !iface )
    return Undefined();

  switch (id) {
  case History::Back:
    iface->callMethod( "goHistory(int)", -1 );
//      emit ext->goHistory(-1);
    break;
  case History::Forward:
    iface->callMethod( "goHistory(int)", (int)1 );
//      emit ext->goHistory(1);
    break;
  case History::Go:
    iface->callMethod( "goHistory(int)", n.intValue() );
//      emit ext->goHistory(n.intValue());
    break;
  default:
    break;
  }

  return Undefined();
}

/////////////////////////////////////////////////////////////////////////////

#ifdef Q_WS_QWS

const ClassInfo Konqueror::info = { "Konqueror", 0, 0, 0 };

bool Konqueror::hasProperty(ExecState *exec, const UString &p, bool recursive) const
{
  if ( p.qstring().startsWith( "goHistory" ) ) return false;

  return true;
}

Value Konqueror::get(ExecState *exec, const UString &p) const
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

  return /*Function*/( new KonquerorFunc(this, p.qstring().latin1() ) );
}

Value KonquerorFunc::tryCall(ExecState *exec, Object &thisObj, const List &args)
{
  KParts::BrowserExtension *ext = konqueror->part->browserExtension();

  if(!ext)
    return Undefined();

  KParts::BrowserInterface *iface = ext->browserInterface();

  if ( !iface )
    return Undefined();

  QCString n = m_name.data();
  n += "()";
  iface->callMethod( n.data(), QVariant() );

  return Undefined();
}

String Konqueror::toString(ExecState *) const
{
  return UString("[object Konqueror]");
}

#endif
/////////////////////////////////////////////////////////////////////////////

#include "kjs_window.moc"

