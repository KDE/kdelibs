// -*- c-basic-offset: 2 -*-
/*
 *  This file is part of the KDE libraries
 *  Copyright (C) 2000-2003 Harri Porten (porten@kde.org)
 *  Copyright (C) 2001-2003 David Faure (faure@kde.org)
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
#include "config.h"

#include <q3stylesheet.h>
#include <qtimer.h>
#include <q3paintdevicemetrics.h>
#include <qapplication.h>
#include <kdebug.h>
#include <kmessagebox.h>
#include <kinputdialog.h>
#include <klocale.h>
#include <kcodecs.h>
#include <kparts/browserinterface.h>
#include <kwin.h>

#if defined Q_WS_X11 && ! defined K_WS_QTONLY
#include <kwinmodule.h> // schroder
#endif

#include <kbookmarkmanager.h>
#include <kglobalsettings.h>
#include <assert.h>
#include <qstyle.h>
#include <qobject.h>
#include <kstringhandler.h>

#include "kjs_proxy.h"
#include "kjs_window.h"
#include "kjs_navigator.h"
#include "kjs_mozilla.h"
#include "kjs_html.h"
#include "kjs_range.h"
#include "kjs_traversal.h"
#include "kjs_css.h"
#include "kjs_events.h"
#include "xmlhttprequest.h"
#include "xmlserializer.h"
#include "domparser.h"

#include "khtmlview.h"
#include "khtml_part.h"
#include "khtmlpart_p.h"
#include "khtml_settings.h"
#include "xml/dom2_eventsimpl.h"
#include "xml/dom_docimpl.h"
#include "misc/htmltags.h"
#include "html/html_documentimpl.h"
#include "rendering/render_frames.h"

using namespace KJS;

namespace KJS {

  class History : public ObjectImp {
    friend class HistoryFunc;
  public:
    History(ExecState *exec, KHTMLPart *p)
      : ObjectImp(exec->interpreter()->builtinObjectPrototype()), part(p) { }
    virtual Value get(ExecState *exec, const Identifier &propertyName) const;
    Value getValueProperty(ExecState *exec, int token) const;
    virtual const ClassInfo* classInfo() const { return &info; }
    static const ClassInfo info;
    enum { Back, Forward, Go, Length };
  private:
    QPointer<KHTMLPart> part;
  };

  class External : public ObjectImp {
    friend class ExternalFunc;
  public:
    External(ExecState *exec, KHTMLPart *p)
      : ObjectImp(exec->interpreter()->builtinObjectPrototype()), part(p) { }
    virtual Value get(ExecState *exec, const Identifier &propertyName) const;
    virtual const ClassInfo* classInfo() const { return &info; }
    static const ClassInfo info;
    enum { AddFavorite };
  private:
    QPointer<KHTMLPart> part;
  };

  class FrameArray : public ObjectImp {
  public:
    FrameArray(ExecState *exec, KHTMLPart *p)
      : ObjectImp(exec->interpreter()->builtinObjectPrototype()), part(p) { }
    virtual Value get(ExecState *exec, const Identifier &propertyName) const;
  private:
    QPointer<KHTMLPart> part;
  };

#ifdef Q_WS_QWS
  class KonquerorFunc : public DOMFunction {
  public:
    KonquerorFunc(const Konqueror* k, const char* name)
      : DOMFunction(), konqueror(k), m_name(name) { }
    virtual Value tryCall(ExecState *exec, Object &thisObj, const List &args);

  private:
    const Konqueror* konqueror;
    QByteArray m_name;
  };
#endif
} // namespace KJS

#include "kjs_window.lut.h"
#include "rendering/render_replaced.h"

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

// We set the object prototype so that toString is implemented
Screen::Screen(ExecState *exec)
  : ObjectImp(exec->interpreter()->builtinObjectPrototype()) {}

Value Screen::get(ExecState *exec, const Identifier &p) const
{
#ifdef KJS_VERBOSE
  kdDebug(6070) << "Screen::get " << p.qstring() << endl;
#endif
  return lookupGetValue<Screen,ObjectImp>(exec,p,&ScreenTable,this);
}

Value Screen::getValueProperty(ExecState *exec, int token) const
{
#if defined Q_WS_X11 && ! defined K_WS_QTONLY
  KWinModule info(0, KWinModule::INFO_DESKTOP);
#endif
  QWidget *thisWidget = Window::retrieveActive(exec)->part()->widget();
  QRect sg = KGlobalSettings::desktopGeometry(thisWidget);

  switch( token ) {
  case Height:
    return Number(sg.height());
  case Width:
    return Number(sg.width());
  case ColorDepth:
  case PixelDepth: {
    Q3PaintDeviceMetrics m(thisWidget);
    return Number(m.depth());
  }
  case AvailLeft: {
#if defined Q_WS_X11 && ! defined K_WS_QTONLY
    QRect clipped = info.workArea().intersect(sg);
    return Number(clipped.x()-sg.x());
#else
    return Number(10);
#endif
  }
  case AvailTop: {
#if defined Q_WS_X11 && ! defined K_WS_QTONLY
    QRect clipped = info.workArea().intersect(sg);
    return Number(clipped.y()-sg.y());
#else
    return Number(10);
#endif
  }
  case AvailHeight: {
#if defined Q_WS_X11 && ! defined K_WS_QTONLY
    QRect clipped = info.workArea().intersect(sg);
    return Number(clipped.height());
#else
    return Number(100);
#endif
  }
  case AvailWidth: {
#if defined Q_WS_X11 && ! defined K_WS_QTONLY
    QRect clipped = info.workArea().intersect(sg);
    return Number(clipped.width());
#else
    return Number(100);
#endif
  }
  default:
    kdDebug(6070) << "WARNING: Screen::getValueProperty unhandled token " << token << endl;
    return Undefined();
  }
}

////////////////////// Window Object ////////////////////////

const ClassInfo Window::info = { "Window", 0, &WindowTable, 0 };

/*
@begin WindowTable 89
  atob		Window::AToB		DontDelete|Function 1
  btoa		Window::BToA		DontDelete|Function 1
  closed	Window::Closed		DontDelete|ReadOnly
  crypto	Window::Crypto		DontDelete|ReadOnly
  defaultStatus	Window::DefaultStatus	DontDelete
  defaultstatus	Window::DefaultStatus	DontDelete
  status	Window::Status		DontDelete
  document	Window::Document	DontDelete|ReadOnly
  Node		Window::Node		DontDelete
  frameElement		Window::FrameElement		DontDelete|ReadOnly
  Event		Window::EventCtor	DontDelete
  Range		Window::Range		DontDelete
  NodeFilter	Window::NodeFilter	DontDelete
  DOMException	Window::DOMException	DontDelete
  CSSRule	Window::CSSRule		DontDelete
  frames	Window::Frames		DontDelete|ReadOnly
  history	Window::_History	DontDelete|ReadOnly
  external	Window::_External	DontDelete|ReadOnly
  event		Window::Event		DontDelete|ReadOnly
  innerHeight	Window::InnerHeight	DontDelete|ReadOnly
  innerWidth	Window::InnerWidth	DontDelete|ReadOnly
  length	Window::Length		DontDelete|ReadOnly
  location	Window::_Location	DontDelete
  name		Window::Name		DontDelete
  navigator	Window::_Navigator	DontDelete|ReadOnly
  clientInformation	Window::ClientInformation	DontDelete|ReadOnly
  konqueror	Window::_Konqueror	DontDelete|ReadOnly
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
  XMLHttpRequest Window::XMLHttpRequest DontDelete|ReadOnly
  XMLSerializer	Window::XMLSerializer	DontDelete|ReadOnly
  DOMParser	Window::DOMParser	DontDelete|ReadOnly
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
  captureEvents	Window::CaptureEvents	DontDelete|Function 0
  releaseEvents	Window::ReleaseEvents	DontDelete|Function 0
  print		Window::Print		DontDelete|Function 0
  addEventListener	Window::AddEventListener	DontDelete|Function 3
  removeEventListener	Window::RemoveEventListener	DontDelete|Function 3
# IE extension
  navigate	Window::Navigate	DontDelete|Function 1
# Mozilla extension
  sidebar	Window::SideBar		DontDelete|ReadOnly

# Warning, when adding a function to this object you need to add a case in Window::get

# Event handlers
# IE also has: onactivate, onbefore/afterprint, onbeforedeactivate/unload, oncontrolselect,
# ondeactivate, onhelp, onmovestart/end, onresizestart/end, onscroll.
# It doesn't have onabort, onchange, ondragdrop (but NS has that last one).
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
IMPLEMENT_PROTOFUNC_DOM(WindowFunc)

Window::Window(khtml::ChildFrame *p)
  : ObjectImp(/*no proto*/), m_frame(p), screen(0), history(0), external(0), m_frames(0), loc(0), m_evt(0)
{
  winq = new WindowQObject(this);
  //kdDebug(6070) << "Window::Window this=" << this << " part=" << m_part << " " << m_part->name() << endl;
}

Window::~Window()
{
  delete winq;
}

Window *Window::retrieveWindow(KParts::ReadOnlyPart *p)
{
  Object obj = Object::dynamicCast( retrieve( p ) );
#ifndef NDEBUG
  // obj should never be null, except when javascript has been disabled in that part.
  KHTMLPart *part = qobject_cast<KHTMLPart*>(p);
  if ( part && part->jScriptEnabled() )
  {
    assert( obj.isValid() );
#ifndef QWS
    assert( dynamic_cast<KJS::Window*>(obj.imp()) ); // type checking
#endif
  }
#endif
  if ( !obj.isValid() ) // JS disabled
    return 0;
  return static_cast<KJS::Window*>(obj.imp());
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

Value Window::retrieve(KParts::ReadOnlyPart *p)
{
  assert(p);
  KHTMLPart * part = qobject_cast<KHTMLPart*>(p);
  KJSProxy *proxy = 0L;
  if (!part) {
    part = qobject_cast<KHTMLPart*>(p->parent());
    if (part)
      proxy = part->framejScript(p);
  } else
    proxy = part->jScript();
  if (proxy) {
#ifdef KJS_VERBOSE
    kdDebug(6070) << "Window::retrieve part=" << part << " '" << part->name() << "' interpreter=" << proxy->interpreter() << " window=" << proxy->interpreter()->globalObject().imp() << endl;
#endif
    return proxy->interpreter()->globalObject(); // the Global object is the "window"
  } else {
#ifdef KJS_VERBOSE
    kdDebug(6070) << "Window::retrieve part=" << p << " '" << p->name() << "' no jsproxy." << endl;
#endif
    return Undefined(); // This can happen with JS disabled on the domain of that window
  }
}

Location *Window::location() const
{
  if (!loc)
    const_cast<Window*>(this)->loc = new Location(m_frame);
  return loc;
}

ObjectImp* Window::frames( ExecState* exec ) const
{
  KHTMLPart *part = qobject_cast<KHTMLPart*>(m_frame->m_part);
  if (part)
    return m_frames ? m_frames :
      (const_cast<Window*>(this)->m_frames = new FrameArray(exec, part));
  return 0L;
}

// reference our special objects during garbage collection
void Window::mark()
{
  ObjectImp::mark();
  if (screen && !screen->marked())
    screen->mark();
  if (history && !history->marked())
    history->mark();
  if (external && !external->marked())
    external->mark();
  if (m_frames && !m_frames->marked())
    m_frames->mark();
  //kdDebug(6070) << "Window::mark " << this << " marking loc=" << loc << endl;
  if (loc && !loc->marked())
    loc->mark();
  if (winq)
    winq->mark();
}

bool Window::hasProperty(ExecState *exec, const Identifier &p) const
{
  // we don't want any operations on a closed window
  if (m_frame.isNull() || m_frame->m_part.isNull())
    return ( p == "closed" );

  if (ObjectImp::hasProperty(exec, p))
    return true;

  if (Lookup::findEntry(&WindowTable, p))
    return true;

  KHTMLPart *part = qobject_cast<KHTMLPart*>(m_frame->m_part);
  if (!part)
      return false;

  QString q = p.qstring();
  if (part->findFramePart(p.qstring()))
    return true;
  // allow window[1] or parent[1] etc. (#56983)
  bool ok;
  unsigned int i = p.toArrayIndex(&ok);
  if (ok) {
    Q3PtrList<KParts::ReadOnlyPart> frames = part->frames();
    unsigned int len = frames.count();
    if (i < len)
      return true;
  }

  // allow shortcuts like 'Image1' instead of document.images.Image1
  if (part->document().isHTMLDocument()) { // might be XML
    DOM::HTMLDocument doc = part->htmlDocument();
    // Keep in sync with tryGet

    if (static_cast<DOM::DocumentImpl*>(doc.handle())->underDocNamedCache().get(p.qstring()))
      return true;

    return !doc.getElementById(p.string()).isNull();
  }

  return false;
}

UString Window::toString(ExecState *) const
{
  return "[object Window]";
}

Value Window::get(ExecState *exec, const Identifier &p) const
{
#ifdef KJS_VERBOSE
  kdDebug(6070) << "Window("<<this<<")::get " << p.qstring() << endl;
#endif
  // we don't want any operations on a closed window
  if (m_frame.isNull() || m_frame->m_part.isNull()) {
    if ( p == "closed" )
      return Boolean( true );
    return Undefined();
  }

  // Look for overrides first
  ValueImp *val = getDirect(p);
  if (val) {
    //kdDebug(6070) << "Window::get found dynamic property '" << p.ascii() << "'" << endl;
    return isSafeScript(exec) ? Value(val) : Undefined();
  }

  const HashEntry* entry = Lookup::findEntry(&WindowTable, p);
  KHTMLPart *part = qobject_cast<KHTMLPart*>(m_frame->m_part);

  // properties that work on all windows
  if (entry) {
    // ReadOnlyPart first
    switch(entry->value) {
    case Closed:
      return Boolean( false );
    case _Location:
        // No isSafeScript test here, we must be able to _set_ location.href (#49819)
      return Value(location());
    case _Window:
    case Self:
      return retrieve(m_frame->m_part);
    default:
        break;
    }
    if (!part)
        return Undefined();
    // KHTMLPart next
    switch(entry->value) {
    case Frames:
      return Value(frames(exec));
    case Opener:
      if (!part->opener())
        return Null();    // ### a null Window might be better, but == null
      else                // doesn't work yet
        return retrieve(part->opener());
    case Parent:
      return retrieve(part->parentPart() ? part->parentPart() : (KHTMLPart*)part);
    case Top: {
      KHTMLPart *p = part;
      while (p->parentPart())
        p = p->parentPart();
      return retrieve(p);
    }
    case Alert:
    case Confirm:
    case Prompt:
    case Open:
    case Close:
    case Focus:
    case Blur:
    case AToB:
    case BToA:
      return lookupOrCreateFunction<WindowFunc>(exec,p,this,entry->value,entry->params,entry->attr);
    default:
      break;
    }
  } else if (!part) {
    // not a  KHTMLPart
    QString rvalue;
    KParts::LiveConnectExtension::Type rtype;
    unsigned long robjid;
    if (m_frame->m_liveconnect &&
        isSafeScript(exec) &&
        m_frame->m_liveconnect->get(0, p.qstring(), rtype, robjid, rvalue))
      return getLiveConnectValue(m_frame->m_liveconnect, p.qstring(), rtype, rvalue, robjid);
    return Undefined();
  }
  // properties that only work on safe windows
  if (isSafeScript(exec) &&  entry)
  {
    //kdDebug(6070) << "token: " << entry->value << endl;
    switch( entry->value ) {
    case Crypto:
      return Undefined(); // ###
    case DefaultStatus:
      return String(UString(part->jsDefaultStatusBarText()));
    case Status:
      return String(UString(part->jsStatusBarText()));
    case Document:
      if (part->document().isNull()) {
        kdDebug(6070) << "Document.write: adding <HTML><BODY> to create document" << endl;
        part->begin();
        part->write("<HTML><BODY>");
        part->end();
      }
      return getDOMNode(exec,part->document());
    case FrameElement:
      if (m_frame->m_frame)
        return getDOMNode(exec,m_frame->m_frame->element());
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
    case CSSRule:
      return getCSSRuleConstructor(exec);
    case EventCtor:
      return getEventConstructor(exec);
    case _History:
      return Value(history ? history :
                   (const_cast<Window*>(this)->history = new History(exec,part)));

    case _External:
      return Value(external ? external :
                   (const_cast<Window*>(this)->external = new External(exec,part)));

    case Event:
      if (m_evt)
        return getDOMEvent(exec,*m_evt);
      else {
#ifdef KJS_VERBOSE
        kdDebug(6070) << "WARNING: window(" << this << "," << part->name() << ").event, no event!" << endl;
#endif
        return Undefined();
      }
    case InnerHeight:
      if (!part->view())
        return Undefined();
      khtml::RenderWidget::flushWidgetResizes(); // make sure frames have their final size
      return Number(part->view()->visibleHeight());
    case InnerWidth:
      if (!part->view())
        return Undefined();
      khtml::RenderWidget::flushWidgetResizes(); // make sure frames have their final size
      return Number(part->view()->visibleWidth());
    case Length:
      return Number(part->frames().count());
    case Name:
      return String(part->name());
    case SideBar:
      return Value(new MozillaSidebarExtension(exec, part));
    case _Navigator:
    case ClientInformation: {
      // Store the navigator in the object so we get the same one each time.
      Value nav( new Navigator(exec, part) );
      const_cast<Window *>(this)->put(exec, "navigator", nav, DontDelete|ReadOnly|Internal);
      const_cast<Window *>(this)->put(exec, "clientInformation", nav, DontDelete|ReadOnly|Internal);
      return nav;
    }
#ifdef Q_WS_QWS
    case _Konqueror: {
      Value k( new Konqueror(exec, part) );
      const_cast<Window *>(this)->put(exec, "konqueror", k, DontDelete|ReadOnly|Internal);
      return k;
    }
#endif
    case OffscreenBuffering:
      return Boolean(true);
    case OuterHeight:
    case OuterWidth:
    {
      if (!part->widget())
        return Number(0);
      KWin::WindowInfo inf = KWin::windowInfo(part->widget()->topLevelWidget()->winId());
      return Number(entry->value == OuterHeight ?
                    inf.geometry().height() : inf.geometry().width());
    }
    case PageXOffset:
      return Number(part->view()->contentsX());
    case PageYOffset:
      return Number(part->view()->contentsY());
    case Personalbar:
      return Undefined(); // ###
    case ScreenLeft:
    case ScreenX: {
      if (!part->view())
        return Undefined();
      QRect sg = KGlobalSettings::desktopGeometry(part->view());
      return Number(part->view()->mapToGlobal(QPoint(0,0)).x() + sg.x());
    }
    case ScreenTop:
    case ScreenY: {
      if (!part->view())
        return Undefined();
      QRect sg = KGlobalSettings::desktopGeometry(part->view());
      return Number(part->view()->mapToGlobal(QPoint(0,0)).y() + sg.y());
    }
    case ScrollX: {
      if (!part->view())
        return Undefined();
      return Number(part->view()->contentsX());
    }
    case ScrollY: {
      if (!part->view())
        return Undefined();
      return Number(part->view()->contentsY());
    }
    case Scrollbars:
      return Undefined(); // ###
    case _Screen:
      return Value(screen ? screen :
                   (const_cast<Window*>(this)->screen = new Screen(exec)));
    case Image:
      return Value(new ImageConstructorImp(exec, part->document()));
    case Option:
      return Value(new OptionConstructorImp(exec, part->document()));
    case XMLHttpRequest:
      return Value(new XMLHttpRequestConstructorImp(exec, part->document()));
    case XMLSerializer:
      return Value(new XMLSerializerConstructorImp(exec));
    case DOMParser:
      return Value(new DOMParserConstructorImp(exec, part->xmlDocImpl()));
    case Scroll: // compatibility
    case ScrollBy:
    case ScrollTo:
    case MoveBy:
    case MoveTo:
    case ResizeBy:
    case ResizeTo:
    case CaptureEvents:
    case ReleaseEvents:
    case AddEventListener:
    case RemoveEventListener:
    case SetTimeout:
    case ClearTimeout:
    case SetInterval:
    case ClearInterval:
    case Print:
      return lookupOrCreateFunction<WindowFunc>(exec,p,this,entry->value,entry->params,entry->attr);
    // IE extension
    case Navigate:
      // Disabled in NS-compat mode. Supported by default - can't hurt, unless someone uses
      // if (navigate) to test for IE (unlikely).
      if ( exec->interpreter()->compatMode() == Interpreter::NetscapeCompat )
        return Undefined();
      return lookupOrCreateFunction<WindowFunc>(exec,p,this,entry->value,entry->params,entry->attr);
    case Onabort:
      return getListener(exec,DOM::EventImpl::ABORT_EVENT);
    case Onblur:
      return getListener(exec,DOM::EventImpl::BLUR_EVENT);
    case Onchange:
      return getListener(exec,DOM::EventImpl::CHANGE_EVENT);
    case Onclick:
      return getListener(exec,DOM::EventImpl::KHTML_ECMA_CLICK_EVENT);
    case Ondblclick:
      return getListener(exec,DOM::EventImpl::KHTML_ECMA_DBLCLICK_EVENT);
    case Ondragdrop:
      return getListener(exec,DOM::EventImpl::KHTML_DRAGDROP_EVENT);
    case Onerror:
      return getListener(exec,DOM::EventImpl::KHTML_ERROR_EVENT);
    case Onfocus:
      return getListener(exec,DOM::EventImpl::FOCUS_EVENT);
    case Onkeydown:
      return getListener(exec,DOM::EventImpl::KEYDOWN_EVENT);
    case Onkeypress:
      return getListener(exec,DOM::EventImpl::KEYPRESS_EVENT);
    case Onkeyup:
      return getListener(exec,DOM::EventImpl::KEYUP_EVENT);
    case Onload:
      return getListener(exec,DOM::EventImpl::LOAD_EVENT);
    case Onmousedown:
      return getListener(exec,DOM::EventImpl::MOUSEDOWN_EVENT);
    case Onmousemove:
      return getListener(exec,DOM::EventImpl::MOUSEMOVE_EVENT);
    case Onmouseout:
      return getListener(exec,DOM::EventImpl::MOUSEOUT_EVENT);
    case Onmouseover:
      return getListener(exec,DOM::EventImpl::MOUSEOVER_EVENT);
    case Onmouseup:
      return getListener(exec,DOM::EventImpl::MOUSEUP_EVENT);
    case Onmove:
      return getListener(exec,DOM::EventImpl::KHTML_MOVE_EVENT);
    case Onreset:
      return getListener(exec,DOM::EventImpl::RESET_EVENT);
    case Onresize:
      return getListener(exec,DOM::EventImpl::RESIZE_EVENT);
    case Onselect:
      return getListener(exec,DOM::EventImpl::SELECT_EVENT);
    case Onsubmit:
      return getListener(exec,DOM::EventImpl::SUBMIT_EVENT);
    case Onunload:
      return getListener(exec,DOM::EventImpl::UNLOAD_EVENT);
    }
  }

  // doing the remainder of ObjectImp::get() that is not covered by
  // the getDirect() call above.
  // #### guessed position. move further up or down?
  Object proto = Object::dynamicCast(prototype());
  assert(proto.isValid());
  if (p == specialPrototypePropertyName)
    return isSafeScript(exec) ? Value(proto) : Undefined();
  Value val2 = proto.get(exec, p);
  if (!val2.isA(UndefinedType)) {
    return isSafeScript(exec) ? val2 : Undefined();
  }

  KParts::ReadOnlyPart *rop = part->findFramePart( p.qstring() );
  if (rop)
    return retrieve(rop);

  // allow window[1] or parent[1] etc. (#56983)
  bool ok;
  unsigned int i = p.toArrayIndex(&ok);
  if (ok) {
    Q3PtrList<KParts::ReadOnlyPart> frames = part->frames();
    unsigned int len = frames.count();
    if (i < len) {
      KParts::ReadOnlyPart* frame = frames.at(i);
      if (frame)
        return Window::retrieve(frame);
    }
  }

  // allow shortcuts like 'Image1' instead of document.images.Image1
  if (isSafeScript(exec) &&
      part->document().isHTMLDocument()) { // might be XML
    // This is only for images, forms, layers and applets, see KJS::HTMLDocument::tryGet
    DOM::HTMLDocument doc = part->htmlDocument();

    DOM::ElementMappingCache::ItemInfo* info =
        static_cast<DOM::DocumentImpl*>(doc.handle())->underDocNamedCache().get(p.qstring());
    if (info) {
      if (info->nd)
        return getDOMNode(exec, info->nd);
      else {
        //No cached mapping, do it by hand
        NamedTagLengthDeterminer::TagLength tags[4] = {
          {ID_IMG, 0, 0L}, {ID_FORM, 0, 0L}, {ID_APPLET, 0, 0L}, {ID_LAYER, 0, 0L}
        };
        NamedTagLengthDeterminer(p.string(), tags, 4)(doc.handle());
        for (int i = 0; i < 4; i++)
          if (tags[i].length > 0) {
            if (tags[i].length == 1) {
              //Have a single answer -> cache
              info->nd = tags[i].last;
              return getDOMNode(exec, tags[i].last);
            }
            // Get all the items with the same name
            return getDOMNodeList(exec, DOM::NodeList(new DOM::NamedTagNodeListImpl(doc.handle(), tags[i].id, p.string())));
        }
      }
    }

    DOM::Element element = doc.getElementById(p.string() );
    if ( !element.isNull() )
      return getDOMNode(exec, element );
  }

  // This isn't necessarily a bug. Some code uses if(!window.blah) window.blah=1
  // But it can also mean something isn't loaded or implemented, hence the WARNING to help grepping.
#ifdef KJS_VERBOSE
  kdDebug(6070) << "WARNING: Window::get property not found: " << p.qstring() << endl;
#endif
  return Undefined();
}

void Window::put(ExecState* exec, const Identifier &propertyName, const Value &value, int attr)
{
  // we don't want any operations on a closed window
  if (m_frame.isNull() || m_frame->m_part.isNull()) {
    // ### throw exception? allow setting of some props like location?
    return;
  }

  // Called by an internal KJS call (e.g. InterpreterImp's constructor) ?
  // If yes, save time and jump directly to ObjectImp.
  if ( (attr != None && attr != DontDelete) ||
       // Same thing if we have a local override (e.g. "var location")
       ( isSafeScript( exec ) && ObjectImp::getDirect(propertyName) ) )
  {
    ObjectImp::put( exec, propertyName, value, attr );
    return;
  }

  const HashEntry* entry = Lookup::findEntry(&WindowTable, propertyName);
  if (entry && !m_frame.isNull() && !m_frame->m_part.isNull())
  {
#ifdef KJS_VERBOSE
    kdDebug(6070) << "Window("<<this<<")::put " << propertyName.qstring() << endl;
#endif
    switch( entry->value) {
    case _Location:
      goURL(exec, value.toString(exec).qstring(), false /*don't lock history*/);
      return;
    default:
      break;
    }
    KHTMLPart *part = qobject_cast<KHTMLPart*>(m_frame->m_part);
    if (part) {
    switch( entry->value ) {
    case Status: {
      if  (isSafeScript(exec) && part->settings()->windowStatusPolicy(part->url().host())
		== KHTMLSettings::KJSWindowStatusAllow) {
      String s = value.toString(exec);
      part->setJSStatusBarText(s.value().qstring());
      }
      return;
    }
    case DefaultStatus: {
      if (isSafeScript(exec) && part->settings()->windowStatusPolicy(part->url().host())
		== KHTMLSettings::KJSWindowStatusAllow) {
      String s = value.toString(exec);
      part->setJSDefaultStatusBarText(s.value().qstring());
      }
      return;
    }
    case Onabort:
      if (isSafeScript(exec))
        setListener(exec, DOM::EventImpl::ABORT_EVENT,value);
      return;
    case Onblur:
      if (isSafeScript(exec))
        setListener(exec, DOM::EventImpl::BLUR_EVENT,value);
      return;
    case Onchange:
      if (isSafeScript(exec))
        setListener(exec, DOM::EventImpl::CHANGE_EVENT,value);
      return;
    case Onclick:
      if (isSafeScript(exec))
        setListener(exec,DOM::EventImpl::KHTML_ECMA_CLICK_EVENT,value);
      return;
    case Ondblclick:
      if (isSafeScript(exec))
        setListener(exec,DOM::EventImpl::KHTML_ECMA_DBLCLICK_EVENT,value);
      return;
    case Ondragdrop:
      if (isSafeScript(exec))
        setListener(exec,DOM::EventImpl::KHTML_DRAGDROP_EVENT,value);
      return;
    case Onerror:
      if (isSafeScript(exec))
        setListener(exec,DOM::EventImpl::KHTML_ERROR_EVENT,value);
      return;
    case Onfocus:
      if (isSafeScript(exec))
        setListener(exec,DOM::EventImpl::FOCUS_EVENT,value);
      return;
    case Onkeydown:
      if (isSafeScript(exec))
        setListener(exec,DOM::EventImpl::KEYDOWN_EVENT,value);
      return;
    case Onkeypress:
      if (isSafeScript(exec))
        setListener(exec,DOM::EventImpl::KEYPRESS_EVENT,value);
      return;
    case Onkeyup:
      if (isSafeScript(exec))
        setListener(exec,DOM::EventImpl::KEYUP_EVENT,value);
      return;
    case Onload:
      if (isSafeScript(exec))
        setListener(exec,DOM::EventImpl::LOAD_EVENT,value);
      return;
    case Onmousedown:
      if (isSafeScript(exec))
        setListener(exec,DOM::EventImpl::MOUSEDOWN_EVENT,value);
      return;
    case Onmousemove:
      if (isSafeScript(exec))
        setListener(exec,DOM::EventImpl::MOUSEMOVE_EVENT,value);
      return;
    case Onmouseout:
      if (isSafeScript(exec))
        setListener(exec,DOM::EventImpl::MOUSEOUT_EVENT,value);
      return;
    case Onmouseover:
      if (isSafeScript(exec))
        setListener(exec,DOM::EventImpl::MOUSEOVER_EVENT,value);
      return;
    case Onmouseup:
      if (isSafeScript(exec))
        setListener(exec,DOM::EventImpl::MOUSEUP_EVENT,value);
      return;
    case Onmove:
      if (isSafeScript(exec))
        setListener(exec,DOM::EventImpl::KHTML_MOVE_EVENT,value);
      return;
    case Onreset:
      if (isSafeScript(exec))
        setListener(exec,DOM::EventImpl::RESET_EVENT,value);
      return;
    case Onresize:
      if (isSafeScript(exec))
        setListener(exec,DOM::EventImpl::RESIZE_EVENT,value);
      return;
    case Onselect:
      if (isSafeScript(exec))
        setListener(exec,DOM::EventImpl::SELECT_EVENT,value);
      return;
    case Onsubmit:
      if (isSafeScript(exec))
        setListener(exec,DOM::EventImpl::SUBMIT_EVENT,value);
      return;
    case Onunload:
      if (isSafeScript(exec))
        setListener(exec,DOM::EventImpl::UNLOAD_EVENT,value);
      return;
    case Name:
      if (isSafeScript(exec))
        part->setName( value.toString(exec).qstring().toLocal8Bit().data() );
      return;
    default:
      break;
    }
    }
  }
  if (m_frame->m_liveconnect &&
      isSafeScript(exec) &&
      m_frame->m_liveconnect->put(0, propertyName.qstring(), value.toString(exec).qstring()))
    return;
  if (isSafeScript(exec)) {
    //kdDebug(6070) << "Window("<<this<<")::put storing " << propertyName.qstring() << endl;
    ObjectImp::put(exec, propertyName, value, attr);
  }
}

bool Window::toBoolean(ExecState *) const
{
  return !m_frame.isNull() && !m_frame->m_part.isNull();
}

void Window::scheduleClose()
{
  kdDebug(6070) << "Window::scheduleClose window.close() " << m_frame << endl;
  Q_ASSERT(winq);
  QTimer::singleShot( 0, winq, SLOT( timeoutClose() ) );
}

void Window::closeNow()
{
  if (m_frame.isNull() || m_frame->m_part.isNull()) {
    kdDebug(6070) << k_funcinfo << "part is deleted already" << endl;
  } else {
    KHTMLPart *part = qobject_cast<KHTMLPart*>(m_frame->m_part);
    if (!part) {
      kdDebug(6070) << "closeNow on non KHTML part" << endl;
    } else {
      //kdDebug(6070) << k_funcinfo << " -> closing window" << endl;
      // We want to make sure that window.open won't find this part by name.
      part->setName( 0 );
      part->deleteLater();
      part = 0;
    }
  }
}

void Window::afterScriptExecution()
{
  DOM::DocumentImpl::updateDocumentsRendering();
  Q3ValueList<DelayedAction> delayedActions = m_delayed;
  m_delayed.clear();
  Q3ValueList<DelayedAction>::Iterator it = delayedActions.begin();
  for ( ; it != delayedActions.end() ; ++it )
  {
    switch ((*it).actionId) {
    case DelayedClose:
      scheduleClose();
      return; // stop here, in case of multiple actions
    case DelayedGoHistory:
      goHistory( (*it).param.toInt() );
      break;
    case NullAction:
      // FIXME: anything needs to be done here?  This is warning anyways.
      break;
    };
  }
}

bool Window::checkIsSafeScript(KParts::ReadOnlyPart *activePart) const
{
  if (m_frame.isNull() || m_frame->m_part.isNull()) { // part deleted ? can't grant access
    kdDebug(6070) << "Window::isSafeScript: accessing deleted part !" << endl;
    return false;
  }
  if (!activePart) {
    kdDebug(6070) << "Window::isSafeScript: current interpreter's part is 0L!" << endl;
    return false;
  }
   if ( activePart == m_frame->m_part ) // Not calling from another frame, no problem.
     return true;

  KHTMLPart *part = qobject_cast<KHTMLPart*>(m_frame->m_part);
  if (!part)
    return true; // not a KHTMLPart

  if ( part->document().isNull() )
    return true; // allow to access a window that was just created (e.g. with window.open("about:blank"))

  DOM::HTMLDocument thisDocument = part->htmlDocument();
  if ( thisDocument.isNull() ) {
    kdDebug(6070) << "Window::isSafeScript: trying to access an XML document !?" << endl;
    return false;
  }

  KHTMLPart *activeKHTMLPart = qobject_cast<KHTMLPart*>(activePart);
  if (!activeKHTMLPart)
    return true; // not a KHTMLPart

  DOM::HTMLDocument actDocument = activeKHTMLPart->htmlDocument();
  if ( actDocument.isNull() ) {
    kdDebug(6070) << "Window::isSafeScript: active part has no document!" << endl;
    return false;
  }
  DOM::DOMString actDomain = actDocument.domain();
  DOM::DOMString thisDomain = thisDocument.domain();

  if ( actDomain == thisDomain ) {
#ifdef KJS_VERBOSE
    //kdDebug(6070) << "JavaScript: access granted, domain is '" << actDomain.string() << "'" << endl;
#endif
    return true;
  }

  kdDebug(6070) << "WARNING: JavaScript: access denied for current frame '" << actDomain.string() << "' to frame '" << thisDomain.string() << "'" << endl;
  // TODO after 3.1: throw security exception (exec->setException())
  return false;
}

void Window::setListener(ExecState *exec, int eventId, Value func)
{
  KHTMLPart *part = qobject_cast<KHTMLPart*>(m_frame->m_part);
  if (!part || !isSafeScript(exec))
    return;
  DOM::DocumentImpl *doc = static_cast<DOM::DocumentImpl*>(part->htmlDocument().handle());
  if (!doc)
    return;

  doc->setHTMLWindowEventListener(eventId,getJSEventListener(func,true));
}

Value Window::getListener(ExecState *exec, int eventId) const
{
  KHTMLPart *part = qobject_cast<KHTMLPart*>(m_frame->m_part);
  if (!part || !isSafeScript(exec))
    return Undefined();
  DOM::DocumentImpl *doc = static_cast<DOM::DocumentImpl*>(part->htmlDocument().handle());
  if (!doc)
    return Undefined();

  DOM::EventListener *listener = doc->getHTMLWindowEventListener(eventId);
  if (listener && static_cast<JSEventListener*>(listener)->listenerObjImp())
    return static_cast<JSEventListener*>(listener)->listenerObj();
  else
    return Null();
}


JSEventListener *Window::getJSEventListener(const Value& val, bool html)
{
  // This function is so hot that it's worth coding it directly with imps.
  KHTMLPart *part = qobject_cast<KHTMLPart*>(m_frame->m_part);
  if (!part || val.type() != ObjectType)
    return 0;

  // It's ObjectType, so it must be valid.
  Object listenerObject = Object::dynamicCast(val);
  ObjectImp *listenerObjectImp = listenerObject.imp();

  // 'listener' is not a simple ecma function. (Always use sanity checks: Better safe than sorry!)
  if (!listenerObject.implementsCall() && part && part->jScript() && part->jScript()->interpreter())
  {
    Interpreter *interpreter = part->jScript()->interpreter();

    // 'listener' probably is an EventListener object containing a 'handleEvent' function.
    Value handleEventValue = listenerObject.get(interpreter->globalExec(), Identifier("handleEvent"));
    Object handleEventObject = Object::dynamicCast(handleEventValue);

    if(handleEventObject.isValid() && handleEventObject.implementsCall())
    {
      listenerObject = handleEventObject;
      listenerObjectImp = handleEventObject.imp();
    }
  }

  JSEventListener *existingListener = jsEventListeners[listenerObjectImp];
  if (existingListener) {
     if ( existingListener->isHTMLEventListener() != html )
        // The existingListener will have the wrong type, so onclick= will behave like addEventListener or vice versa.
        kdWarning() << "getJSEventListener: event listener already found but with html=" << !html << " - please report this, we thought it would never happen" << endl;
    return existingListener;
  }

  // Note that the JSEventListener constructor adds it to our jsEventListeners list
  return new JSEventListener(listenerObject, listenerObjectImp, Object(this), html);
}

JSLazyEventListener *Window::getJSLazyEventListener(const QString& code, const QString& name, DOM::NodeImpl *node)
{
  return new JSLazyEventListener(code, name, Object(this), node);
}

void Window::clear( ExecState *exec )
{
  delete winq;
  winq = 0L;
  // Get rid of everything, those user vars could hold references to DOM nodes
  deleteAllProperties( exec );

  // Break the dependency between the listeners and their object
  Q3PtrDictIterator<JSEventListener> it(jsEventListeners);
  for (; it.current(); ++it)
    it.current()->clear();
  // Forget about the listeners (the DOM::NodeImpls will delete them)
  jsEventListeners.clear();

  if (m_frame) {
    KJSProxy* proxy = m_frame->m_jscript;
    if (proxy) // i.e. JS not disabled
    {
      winq = new WindowQObject(this);
      // Now recreate a working global object for the next URL that will use us
      KJS::Interpreter *interpreter = proxy->interpreter();
      interpreter->initGlobalObject();
    }
  }
}

void Window::setCurrentEvent( DOM::Event *evt )
{
  m_evt = evt;
  //kdDebug(6070) << "Window " << this << " (part=" << m_part << ")::setCurrentEvent m_evt=" << evt << endl;
}

void Window::goURL(ExecState* exec, const QString& url, bool lockHistory)
{
  Window* active = Window::retrieveActive(exec);
  KHTMLPart *part = qobject_cast<KHTMLPart*>(m_frame->m_part);
  KHTMLPart *active_part = qobject_cast<KHTMLPart*>(active->part());
  // Complete the URL using the "active part" (running interpreter)
  if (active_part && part) {
    if (url[0] == QChar('#')) {
      part->gotoAnchor(url.mid(1));
    } else {
      QString dstUrl = active_part->htmlDocument().completeURL(url).string();
      kdDebug(6070) << "Window::goURL dstUrl=" << dstUrl << endl;

      // check if we're allowed to inject javascript
      // SYNC check with khtml_part.cpp::slotRedirect!
      if ( isSafeScript(exec) ||
            dstUrl.find(QLatin1String("javascript:"), 0, false) != 0 )
        part->scheduleRedirection(-1,
                                  dstUrl,
                                  lockHistory);
    }
  } else if (!part && !m_frame->m_part.isNull()) {
    KParts::BrowserExtension *b = KParts::BrowserExtension::childObject(m_frame->m_part);
    if (b)
      b->emit openURLRequest(m_frame->m_frame->element()->getDocument()->completeURL(url));
    kdDebug() << "goURL for ROPart" << endl;
  }
}

KParts::ReadOnlyPart *Window::part() const {
    return m_frame.isNull() ? 0L : static_cast<KParts::ReadOnlyPart *>(m_frame->m_part);
}

void Window::delayedGoHistory( int steps )
{
    m_delayed.append( DelayedAction( DelayedGoHistory, steps ) );
}

void Window::goHistory( int steps )
{
  KHTMLPart *part = qobject_cast<KHTMLPart*>(m_frame->m_part);
  if(!part)
      // TODO history readonlypart
    return;
  KParts::BrowserExtension *ext = part->browserExtension();
  if(!ext)
    return;
  KParts::BrowserInterface *iface = ext->browserInterface();

  if ( !iface )
    return;

  iface->callMethod( "goHistory(int)", steps );
  //emit ext->goHistory(steps);
}

void KJS::Window::resizeTo(QWidget* tl, int width, int height)
{
  KHTMLPart *part = qobject_cast<KHTMLPart*>(m_frame->m_part);
  if(!part)
      // TODO resizeTo readonlypart
    return;
  KParts::BrowserExtension *ext = part->browserExtension();
  if (!ext) {
    kdDebug(6070) << "Window::resizeTo found no browserExtension" << endl;
    return;
  }

  // Security check: within desktop limits and bigger than 100x100 (per spec)
  if ( width < 100 || height < 100 ) {
    kdDebug(6070) << "Window::resizeTo refused, window would be too small ("<<width<<","<<height<<")" << endl;
    return;
  }

  QRect sg = KGlobalSettings::desktopGeometry(tl);

  if ( width > sg.width() || height > sg.height() ) {
    kdDebug(6070) << "Window::resizeTo refused, window would be too big ("<<width<<","<<height<<")" << endl;
    return;
  }

  kdDebug(6070) << "resizing to " << width << "x" << height << endl;

  emit ext->resizeTopLevelWidget( width, height );

  // If the window is out of the desktop, move it up/left
  // (maybe we should use workarea instead of sg, otherwise the window ends up below kicker)
  int right = tl->x() + tl->frameGeometry().width();
  int bottom = tl->y() + tl->frameGeometry().height();
  int moveByX = 0;
  int moveByY = 0;
  if ( right > sg.right() )
    moveByX = - right + sg.right(); // always <0
  if ( bottom > sg.bottom() )
    moveByY = - bottom + sg.bottom(); // always <0
  if ( moveByX || moveByY )
    emit ext->moveTopLevelWidget( tl->x() + moveByX , tl->y() + moveByY );
}

Value Window::openWindow(ExecState *exec, const List& args)
{
  KHTMLPart *part = qobject_cast<KHTMLPart*>(m_frame->m_part);
  if (!part)
    return Undefined();
  KHTMLView *widget = part->view();
  Value v = args[0];
  QString str = v.toString(exec).qstring();

  // prepare arguments
  KURL url;
  if (!str.isEmpty())
  {
    KHTMLPart* p = qobject_cast<KHTMLPart*>(Window::retrieveActive(exec)->m_frame->m_part);
    if ( p )
      url = p->htmlDocument().completeURL(str).string();
    if ( !p ||
         !static_cast<DOM::DocumentImpl*>(p->htmlDocument().handle())->isURLAllowed(url.url()) )
      return Undefined();
  }

  KHTMLSettings::KJSWindowOpenPolicy policy =
		part->settings()->windowOpenPolicy(part->url().host());
  if ( policy == KHTMLSettings::KJSWindowOpenAsk ) {
    emit part->browserExtension()->requestFocus(part);
    QString caption;
    if (!part->url().host().isEmpty())
      caption = part->url().host() + " - ";
    caption += i18n( "Confirmation: JavaScript Popup" );
    if ( KMessageBox::questionYesNo(widget,
                                    str.isEmpty() ?
                                    i18n( "This site is requesting to open up a new browser "
                                          "window via JavaScript.\n"
                                          "Do you want to allow this?" ) :
                                    i18n( "<qt>This site is requesting to open<p>%1</p>in a new browser window via JavaScript.<br />"
                                          "Do you want to allow this?</qt>").arg(KStringHandler::csqueeze(url.htmlURL(),  100)),
                                    caption, i18n("Allow"), i18n("Do Not Allow") ) == KMessageBox::Yes )
      policy = KHTMLSettings::KJSWindowOpenAllow;
  } else if ( policy == KHTMLSettings::KJSWindowOpenSmart )
  {
    // window.open disabled unless from a key/mouse event
    if (static_cast<ScriptInterpreter *>(exec->interpreter())->isWindowOpenAllowed())
      policy = KHTMLSettings::KJSWindowOpenAllow;
  }

  QString frameName = args.size() > 1 ? args[1].toString(exec).qstring() : QString("_blank");

  v = args[2];
  QString features;
  if (!v.isNull() && v.type() != UndefinedType && v.toString(exec).size() > 0) {
    features = v.toString(exec).qstring();
    // Buggy scripts have ' at beginning and end, cut those
    if (features.startsWith("\'") && features.endsWith("\'"))
      features = features.mid(1, features.length()-2);
  }

  if ( policy != KHTMLSettings::KJSWindowOpenAllow ) {
    if ( url.isEmpty() )
      part->setSuppressedPopupIndicator(true, 0);
    else {
      part->setSuppressedPopupIndicator(true, part);
      m_suppressedWindowInfo.append( SuppressedWindowInfo( exec, url, frameName, features ) );
    }
    return Undefined();
  } else {
    return executeOpenWindow(exec, url, frameName, features);
  }
}

Value Window::executeOpenWindow(ExecState *exec, const KURL& url, const QString& frameName, const QString& features)
{
    KHTMLPart *p = qobject_cast<KHTMLPart *>(m_frame->m_part);
    KHTMLView *widget = p->view();
    KParts::WindowArgs winargs;

    // scan feature argument
    if (!features.isEmpty()) {
      // specifying window params means false defaults
      winargs.menuBarVisible = false;
      winargs.toolBarsVisible = false;
      winargs.statusBarVisible = false;
      winargs.scrollBarsVisible = false;
      QStringList flist = QStringList::split(',', features);
      QStringList::ConstIterator it = flist.begin();
      while (it != flist.end()) {
        QString s = *it++;
        QString key, val;
        int pos = s.find('=');
        if (pos >= 0) {
          key = s.left(pos).trimmed().lower();
          val = s.mid(pos + 1).trimmed().lower();
          QRect screen = KGlobalSettings::desktopGeometry(widget->topLevelWidget());

          if (key == "left" || key == "screenx") {
            winargs.x = (int)val.toFloat() + screen.x();
            if (winargs.x < screen.x() || winargs.x > screen.right())
              winargs.x = screen.x(); // only safe choice until size is determined
          } else if (key == "top" || key == "screeny") {
            winargs.y = (int)val.toFloat() + screen.y();
            if (winargs.y < screen.y() || winargs.y > screen.bottom())
              winargs.y = screen.y(); // only safe choice until size is determined
          } else if (key == "height") {
            winargs.height = (int)val.toFloat() + 2*qApp->style()->pixelMetric( QStyle::PM_DefaultFrameWidth ) + 2;
            if (winargs.height > screen.height())  // should actually check workspace
              winargs.height = screen.height();
            if (winargs.height < 100)
              winargs.height = 100;
          } else if (key == "width") {
            winargs.width = (int)val.toFloat() + 2*qApp->style()->pixelMetric( QStyle::PM_DefaultFrameWidth ) + 2;
            if (winargs.width > screen.width())    // should actually check workspace
              winargs.width = screen.width();
            if (winargs.width < 100)
              winargs.width = 100;
          } else {
            goto boolargs;
          }
          continue;
        } else {
          // leaving away the value gives true
          key = s.trimmed().lower();
          val = "1";
        }
      boolargs:
        if (key == "menubar")
          winargs.menuBarVisible = (val == "1" || val == "yes");
        else if (key == "toolbar")
          winargs.toolBarsVisible = (val == "1" || val == "yes");
        else if (key == "location")  // ### missing in WindowArgs
          winargs.toolBarsVisible = (val == "1" || val == "yes");
        else if (key == "status" || key == "statusbar")
          winargs.statusBarVisible = (val == "1" || val == "yes");
        else if (key == "scrollbars")
          winargs.scrollBarsVisible = (val == "1" || val == "yes");
        else if (key == "resizable")
          winargs.resizable = (val == "1" || val == "yes");
        else if (key == "fullscreen")
          winargs.fullscreen = (val == "1" || val == "yes");
      }
    }

    KParts::URLArgs uargs;
    uargs.frameName = frameName;

    if ( uargs.frameName.lower() == "_top" )
    {
      while ( p->parentPart() )
        p = p->parentPart();
      Window::retrieveWindow(p)->goURL(exec, url.url(), false /*don't lock history*/);
      return Window::retrieve(p);
    }
    if ( uargs.frameName.lower() == "_parent" )
    {
      if ( p->parentPart() )
        p = p->parentPart();
      Window::retrieveWindow(p)->goURL(exec, url.url(), false /*don't lock history*/);
      return Window::retrieve(p);
    }
    if ( uargs.frameName.lower() == "_self")
    {
      Window::retrieveWindow(p)->goURL(exec, url.url(), false /*don't lock history*/);
      return Window::retrieve(p);
    }
    if ( uargs.frameName.lower() == "replace" )
    {
      Window::retrieveWindow(p)->goURL(exec, url.url(), true /*lock history*/);
      return Window::retrieve(p);
    }
    uargs.serviceType = "text/html";

    // request window (new or existing if framename is set)
    KParts::ReadOnlyPart *newPart = 0L;
    emit p->browserExtension()->createNewWindow(KURL(), uargs,winargs,newPart);
    if (newPart && qobject_cast<KHTMLPart*>(newPart)) {
      KHTMLPart *khtmlpart = static_cast<KHTMLPart*>(newPart);
      //qDebug("opener set to %p (this Window's part) in new Window %p  (this Window=%p)",part,win,window);
      khtmlpart->setOpener(p);
      khtmlpart->setOpenedByJS(true);
      if (khtmlpart->document().isNull()) {
        khtmlpart->begin();
        khtmlpart->write("<HTML><BODY>");
        khtmlpart->end();
        if ( p->docImpl() ) {
          //kdDebug(6070) << "Setting domain to " << p->docImpl()->domain().string() << endl;
          khtmlpart->docImpl()->setDomain( p->docImpl()->domain());
          khtmlpart->docImpl()->setBaseURL( p->docImpl()->baseURL() );
        }
      }
      uargs.serviceType = QString::null;
      if (uargs.frameName.lower() == "_blank")
        uargs.frameName = QString::null;
      if (!url.isEmpty())
        emit khtmlpart->browserExtension()->openURLRequest(url,uargs);
      return Window::retrieve(khtmlpart); // global object
    } else
      return Undefined();
}

void Window::forgetSuppressedWindows()
{
  m_suppressedWindowInfo.clear();
}

void Window::showSuppressedWindows()
{
  QList<SuppressedWindowInfo> suppressedWindowInfo = m_suppressedWindowInfo;
  m_suppressedWindowInfo.clear();
  foreach ( SuppressedWindowInfo info, suppressedWindowInfo ) {
    executeOpenWindow(info.exec, info.url, info.frameName, info.features);
  }
}

Value WindowFunc::tryCall(ExecState *exec, Object &thisObj, const List &args)
{
  KJS_CHECK_THIS( Window, thisObj );
  Window *window = static_cast<Window *>(thisObj.imp());
  QString str, str2;

  KHTMLPart *part = qobject_cast<KHTMLPart*>(window->m_frame->m_part);
  if (!part)
    return Undefined();

  KHTMLView *widget = part->view();
  Value v = args[0];
  UString s = v.toString(exec);
  str = s.qstring();

  QString caption;
  if (part && !part->url().host().isEmpty())
    caption = part->url().host() + " - ";
  caption += "JavaScript"; // TODO: i18n
  // functions that work everywhere
  switch(id) {
  case Window::Alert:
    if (!widget->dialogsAllowed())
      return Undefined();
    if ( part && part->xmlDocImpl() )
      part->xmlDocImpl()->updateRendering();
    if ( part )
      emit part->browserExtension()->requestFocus(part);
    KMessageBox::error(widget, Q3StyleSheet::convertFromPlainText(str), caption);
    return Undefined();
  case Window::Confirm:
    if (!widget->dialogsAllowed())
      return Undefined();
    if ( part && part->xmlDocImpl() )
      part->xmlDocImpl()->updateRendering();
    if ( part )
      emit part->browserExtension()->requestFocus(part);
    return Boolean((KMessageBox::warningYesNo(widget, Q3StyleSheet::convertFromPlainText(str), caption,
                                                KStdGuiItem::ok(), KStdGuiItem::cancel()) == KMessageBox::Yes));
  case Window::Prompt:
    if (!widget->dialogsAllowed())
      return Undefined();
    if ( part && part->xmlDocImpl() )
      part->xmlDocImpl()->updateRendering();
    if ( part )
      emit part->browserExtension()->requestFocus(part);
    bool ok;
    if (args.size() >= 2)
      str2 = KInputDialog::getText(caption,
                                   Q3StyleSheet::convertFromPlainText(str),
                                   args[1].toString(exec).qstring(), &ok, widget);
    else
      str2 = KInputDialog::getText(caption,
                                   Q3StyleSheet::convertFromPlainText(str),
                                   QString::null, &ok, widget);
    if ( ok )
        return String(str2);
    else
        return Null();
  case Window::Open:
    return window->openWindow(exec, args);
  case Window::Close: {
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
    bool doClose = false;
    if (!part->openedByJS())
    {
      // To conform to the SPEC, we only ask if the window
      // has more than one entry in the history (NS does that too).
      History history(exec,part);

      if ( history.get( exec, "length" ).toInt32(exec) <= 1 )
      {
        doClose = true;
      }
      else
      {
        // Can we get this dialog with tabs??? Does it close the window or the tab in that case?
        emit part->browserExtension()->requestFocus(part);
        if ( KMessageBox::questionYesNo( window->part()->widget(),
                                         i18n("Close window?"), i18n("Confirmation Required"),
                                         KStdGuiItem::close(), KStdGuiItem::cancel() )
             == KMessageBox::Yes )
          doClose = true;
      }
    }
    else
      doClose = true;

    if (doClose)
    {
      // If this is the current window (the one the interpreter runs in),
      // then schedule a delayed close (so that the script terminates first).
      // But otherwise, close immediately. This fixes w=window.open("","name");w.close();window.open("name");
      if ( Window::retrieveActive(exec) == window ) {
        if (widget) {
          // quit all dialogs of this view
          // this fixes 'setTimeout('self.close()',1000); alert("Hi");' crash
          widget->closeChildDialogs();
        }
        //kdDebug() << "scheduling delayed close"  << endl;
        // We'll close the window at the end of the script execution
        Window* w = const_cast<Window*>(window);
        w->m_delayed.append( Window::DelayedAction( Window::DelayedClose ) );
      } else {
        //kdDebug() << "closing NOW"  << endl;
        (const_cast<Window*>(window))->closeNow();
      }
    }
    return Undefined();
  }
  case Window::Navigate:
    window->goURL(exec, args[0].toString(exec).qstring(), false /*don't lock history*/);
    return Undefined();
  case Window::Focus: {
    KHTMLSettings::KJSWindowFocusPolicy policy =
		part->settings()->windowFocusPolicy(part->url().host());
    if(policy == KHTMLSettings::KJSWindowFocusAllow && widget) {
      widget->topLevelWidget()->raise();
      KWin::deIconifyWindow( widget->topLevelWidget()->winId() );
      widget->setActiveWindow();
      emit part->browserExtension()->requestFocus(part);
    }
    return Undefined();
  }
  case Window::Blur:
    // TODO
    return Undefined();
  case Window::BToA:
  case Window::AToB: {
      if (!s.is8Bit())
          return Undefined();
       QByteArray  in, out;
       char *binData = s.ascii();
       in.setRawData( binData, s.size() );
       if (id == Window::AToB)
           KCodecs::base64Decode( in, out );
       else
           KCodecs::base64Encode( in, out );
       in.resetRawData( binData, s.size() );
       UChar *d = new UChar[out.size()];
       for (int i = 0; i < out.size(); i++)
           d[i].uc = (uchar) out[i];
       UString ret(d, out.size(), false /*no copy*/);
       return String(ret);
  }

  };


  // now unsafe functions..
  if (!window->isSafeScript(exec))
    return Undefined();

  switch (id) {
  case Window::ScrollBy:
    if(args.size() == 2 && widget)
      widget->scrollBy(args[0].toInt32(exec), args[1].toInt32(exec));
    return Undefined();
  case Window::Scroll:
  case Window::ScrollTo:
    if(args.size() == 2 && widget)
      widget->setContentsPos(args[0].toInt32(exec), args[1].toInt32(exec));
    return Undefined();
  case Window::MoveBy: {
    KHTMLSettings::KJSWindowMovePolicy policy =
		part->settings()->windowMovePolicy(part->url().host());
    if(policy == KHTMLSettings::KJSWindowMoveAllow && args.size() == 2 && widget)
    {
      KParts::BrowserExtension *ext = part->browserExtension();
      if (ext) {
        QWidget * tl = widget->topLevelWidget();
        QRect sg = KGlobalSettings::desktopGeometry(tl);

        QPoint dest = tl->pos() + QPoint( args[0].toInt32(exec), args[1].toInt32(exec) );
        // Security check (the spec talks about UniversalBrowserWrite to disable this check...)
        if ( dest.x() >= sg.x() && dest.y() >= sg.x() &&
             dest.x()+tl->width() <= sg.width()+sg.x() &&
             dest.y()+tl->height() <= sg.height()+sg.y() )
          emit ext->moveTopLevelWidget( dest.x(), dest.y() );
      }
    }
    return Undefined();
  }
  case Window::MoveTo: {
    KHTMLSettings::KJSWindowMovePolicy policy =
		part->settings()->windowMovePolicy(part->url().host());
    if(policy == KHTMLSettings::KJSWindowMoveAllow && args.size() == 2 && widget)
    {
      KParts::BrowserExtension *ext = part->browserExtension();
      if (ext) {
        QWidget * tl = widget->topLevelWidget();
        QRect sg = KGlobalSettings::desktopGeometry(tl);

        QPoint dest( args[0].toInt32(exec)+sg.x(), args[1].toInt32(exec)+sg.y() );
        // Security check (the spec talks about UniversalBrowserWrite to disable this check...)
        if ( dest.x() >= sg.x() && dest.y() >= sg.y() &&
             dest.x()+tl->width() <= sg.width()+sg.x() &&
             dest.y()+tl->height() <= sg.height()+sg.y() )
		emit ext->moveTopLevelWidget( dest.x(), dest.y() );
      }
    }
    return Undefined();
  }
  case Window::ResizeBy: {
    KHTMLSettings::KJSWindowResizePolicy policy =
		part->settings()->windowResizePolicy(part->url().host());
    if(policy == KHTMLSettings::KJSWindowResizeAllow
    		&& args.size() == 2 && widget)
    {
      QWidget * tl = widget->topLevelWidget();
      QRect geom = tl->frameGeometry();
      window->resizeTo( tl,
                        geom.width() + args[0].toInt32(exec),
                        geom.height() + args[1].toInt32(exec) );
    }
    return Undefined();
  }
  case Window::ResizeTo: {
    KHTMLSettings::KJSWindowResizePolicy policy =
               part->settings()->windowResizePolicy(part->url().host());
    if(policy == KHTMLSettings::KJSWindowResizeAllow
               && args.size() == 2 && widget)
    {
      QWidget * tl = widget->topLevelWidget();
      window->resizeTo( tl, args[0].toInt32(exec), args[1].toInt32(exec) );
    }
    return Undefined();
  }
  case Window::SetTimeout:
  case Window::SetInterval: {
    bool singleShot;
    int i; // timeout interval
    if (args.size() == 0)
      return Undefined();
    if (args.size() > 1) {
      singleShot = (id == Window::SetTimeout);
      i = args[1].toInt32(exec);
    } else {
      // second parameter is missing. Emulate Mozilla behavior.
      singleShot = true;
      i = 4;
    }
    if (v.isA(StringType)) {
      int r = (const_cast<Window*>(window))->winq->installTimeout(Identifier(s), i, singleShot );
      return Number(r);
    }
    else if (v.isA(ObjectType) && Object::dynamicCast(v).implementsCall()) {
      Object func = Object::dynamicCast(v);
      List funcArgs;
      ListIterator it = args.begin();
      int argno = 0;
      while (it != args.end()) {
	Value arg = it++;
	if (argno++ >= 2)
	    funcArgs.append(arg);
      }
      if (args.size() < 2)
	funcArgs.append(Number(i));
      int r = (const_cast<Window*>(window))->winq->installTimeout(func, funcArgs, i, singleShot );
      return Number(r);
    }
    else
      return Undefined();
  }
  case Window::ClearTimeout:
  case Window::ClearInterval:
    (const_cast<Window*>(window))->winq->clearTimeout(v.toInt32(exec));
    return Undefined();
  case Window::Print:
    if ( widget ) {
      // ### TODO emit onbeforeprint event
      widget->print();
      // ### TODO emit onafterprint event
    }
  case Window::CaptureEvents:
  case Window::ReleaseEvents:
    // Do nothing for now. These are NS-specific legacy calls.
    break;
  case Window::AddEventListener: {
        JSEventListener *listener = Window::retrieveActive(exec)->getJSEventListener(args[1]);
        if (listener) {
	    DOM::DocumentImpl* docimpl = static_cast<DOM::DocumentImpl *>(part->document().handle());
            docimpl->addWindowEventListener(DOM::EventImpl::typeToId(args[0].toString(exec).string()),listener,args[2].toBoolean(exec));
        }
        return Undefined();
    }
  case Window::RemoveEventListener: {
        JSEventListener *listener = Window::retrieveActive(exec)->getJSEventListener(args[1]);
        if (listener) {
	    DOM::DocumentImpl* docimpl = static_cast<DOM::DocumentImpl *>(part->document().handle());
            docimpl->removeWindowEventListener(DOM::EventImpl::typeToId(args[0].toString(exec).string()),listener,args[2].toBoolean(exec));
        }
        return Undefined();
    }

  }
  return Undefined();
}

////////////////////// ScheduledAction ////////////////////////

// KDE 4: Make those parameters const ... &
ScheduledAction::ScheduledAction(Object _func, List _args, QTime _nextTime, int _interval, bool _singleShot,
				  int _timerId)
{
  //kdDebug(6070) << "ScheduledAction::ScheduledAction(isFunction) " << this << endl;
  func = static_cast<ObjectImp*>(_func.imp());
  args = _args;
  isFunction = true;
  singleShot = _singleShot;
  nextTime = _nextTime;
  interval = _interval;
  executing = false;
  timerId = _timerId;
}

// KDE 4: Make it const QString &
ScheduledAction::ScheduledAction(QString _code, QTime _nextTime, int _interval, bool _singleShot, int _timerId)
{
  //kdDebug(6070) << "ScheduledAction::ScheduledAction(!isFunction) " << this << endl;
  //func = 0;
  //args = 0;
  func = 0;
  code = _code;
  isFunction = false;
  singleShot = _singleShot;
  nextTime = _nextTime;
  interval = _interval;
  executing = false;
  timerId = _timerId;
}

bool ScheduledAction::execute(Window *window)
{
  KHTMLPart *part = qobject_cast<KHTMLPart*>(window->m_frame->m_part);
  if (!part || !part->jScriptEnabled())
    return false;
  ScriptInterpreter *interpreter = static_cast<ScriptInterpreter *>(part->jScript()->interpreter());

  interpreter->setProcessingTimerCallback(true);

  //kdDebug(6070) << "ScheduledAction::execute " << this << endl;
  if (isFunction) {
    if (func->implementsCall()) {
      // #### check this
      Q_ASSERT( part );
      if ( part )
      {
        KJS::Interpreter *interpreter = part->jScript()->interpreter();
        ExecState *exec = interpreter->globalExec();
        Q_ASSERT( window == interpreter->globalObject().imp() );
        Object obj( window );
        func->call(exec,obj,args); // note that call() creates its own execution state for the func call
        if (exec->hadException())
          exec->clearException();

        // Update our document's rendering following the execution of the timeout callback.
        part->document().updateRendering();
      }
    }
  }
  else {
    part->executeScript(DOM::Node(), code);
  }

  interpreter->setProcessingTimerCallback(false);
  return true;
}

void ScheduledAction::mark()
{
  if (func && !func->marked())
    func->mark();
  args.mark();
}

ScheduledAction::~ScheduledAction()
{
  //kdDebug(6070) << "ScheduledAction::~ScheduledAction " << this << endl;
}

////////////////////// WindowQObject ////////////////////////

WindowQObject::WindowQObject(Window *w)
  : parent(w)
{
  //kdDebug(6070) << "WindowQObject::WindowQObject " << this << endl;
  if ( !parent->m_frame )
      kdDebug(6070) << "WARNING: null part in " << k_funcinfo << endl;
  else
      connect( parent->m_frame, SIGNAL( destroyed() ),
               this, SLOT( parentDestroyed() ) );
  pausedTime = 0;
  lastTimerId = 0;
}

WindowQObject::~WindowQObject()
{
  //kdDebug(6070) << "WindowQObject::~WindowQObject " << this << endl;
  parentDestroyed(); // reuse same code
}

void WindowQObject::parentDestroyed()
{
  killTimers();

  Q3PtrListIterator<ScheduledAction> it(scheduledActions);
  for (; it.current(); ++it)
    delete it.current();
  scheduledActions.clear();
}

int WindowQObject::installTimeout(const Identifier &handler, int t, bool singleShot)
{
  int id = ++lastTimerId;
  if (t < 10) t = 10;
  QTime nextTime = QTime::currentTime().addMSecs(-pausedTime).addMSecs(t);
  ScheduledAction *action = new ScheduledAction(handler.qstring(),nextTime,t,singleShot,id);
  scheduledActions.append(action);
  setNextTimer();
  return id;
}

int WindowQObject::installTimeout(const Value &func, List args, int t, bool singleShot)
{
  Object objFunc = Object::dynamicCast( func );
  if (!objFunc.isValid())
    return 0;
  int id = ++lastTimerId;
  if (t < 10) t = 10;
  QTime nextTime = QTime::currentTime().addMSecs(-pausedTime).addMSecs(t);
  ScheduledAction *action = new ScheduledAction(objFunc,args,nextTime,t,singleShot,id);
  scheduledActions.append(action);
  setNextTimer();
  return id;
}

void WindowQObject::clearTimeout(int timerId)
{
  Q3PtrListIterator<ScheduledAction> it(scheduledActions);
  for (; it.current(); ++it) {
    ScheduledAction *action = it.current();
    if (action->timerId == timerId) {
      scheduledActions.removeRef(action);
      if (!action->executing)
	delete action;
      return;
    }
  }
}

bool WindowQObject::hasTimers() const
{
  return scheduledActions.count();
}

void WindowQObject::mark()
{
  Q3PtrListIterator<ScheduledAction> it(scheduledActions);
  for (; it.current(); ++it)
    it.current()->mark();
}

void WindowQObject::timerEvent(QTimerEvent *)
{
  killTimers();

  if (scheduledActions.isEmpty())
    return;

  QTime currentActual = QTime::currentTime();
  QTime currentAdjusted = currentActual.addMSecs(-pausedTime);

  // Work out which actions are to be executed. We take a separate copy of
  // this list since the main one may be modified during action execution
  Q3PtrList<ScheduledAction> toExecute;
  Q3PtrListIterator<ScheduledAction> it(scheduledActions);
  for (; it.current(); ++it)
    if (currentAdjusted >= it.current()->nextTime)
      toExecute.append(it.current());

  // ### verify that the window can't be closed (and action deleted) during execution
  it = Q3PtrListIterator<ScheduledAction>(toExecute);
  for (; it.current(); ++it) {
    ScheduledAction *action = it.current();
    if (!scheduledActions.containsRef(action)) // removed by clearTimeout()
      continue;

    action->executing = true; // prevent deletion in clearTimeout()

    if (action->singleShot)
      scheduledActions.removeRef(action);
    if (parent->part()) {
      bool ok = action->execute(parent);
      if ( !ok ) // e.g. JS disabled
        scheduledActions.removeRef( action );
    }

    action->executing = false;

    if (!scheduledActions.containsRef(action))
      delete action;
    else
      action->nextTime = action->nextTime.addMSecs(action->interval);
  }

  pausedTime += currentActual.msecsTo(QTime::currentTime());

  // Work out when next event is to occur
  setNextTimer();
}

void WindowQObject::setNextTimer()
{
  if (scheduledActions.isEmpty())
    return;

  Q3PtrListIterator<ScheduledAction> it(scheduledActions);
  QTime nextTime = it.current()->nextTime;
  for (++it; it.current(); ++it)
    if (nextTime > it.current()->nextTime)
      nextTime = it.current()->nextTime;

  QTime nextTimeActual = nextTime.addMSecs(pausedTime);
  int nextInterval = QTime::currentTime().msecsTo(nextTimeActual);
  if (nextInterval < 0)
    nextInterval = 0;
  timerIds.append(startTimer(nextInterval));
}

void WindowQObject::killTimers()
{
 for (int i = 0; i < timerIds.size(); ++i) 
 {
    killTimer(timerIds.at(i));
 } 
 timerIds.clear();
}

void WindowQObject::timeoutClose()
{
  parent->closeNow();
}

Value FrameArray::get(ExecState *exec, const Identifier &p) const
{
#ifdef KJS_VERBOSE
  kdDebug(6070) << "FrameArray::get " << p.qstring() << " part=" << (void*)part << endl;
#endif
  if (part.isNull())
    return Undefined();

  Q3PtrList<KParts::ReadOnlyPart> frames = part->frames();
  unsigned int len = frames.count();
  if (p == lengthPropertyName)
    return Number(len);
  else if (p== "location") // non-standard property, but works in NS and IE
  {
    Object obj = Object::dynamicCast( Window::retrieve( part ) );
    if ( obj.isValid() )
      return obj.get( exec, "location" );
    return Undefined();
  }

  // check for the name or number
  KParts::ReadOnlyPart *frame = part->findFramePart(p.qstring());
  if (!frame) {
    bool ok;
    unsigned int i = p.toArrayIndex(&ok);
    if (ok && i < len)
      frame = frames.at(i);
  }

  // we are potentially fetching a reference to a another Window object here.
  // i.e. we may be accessing objects from another interpreter instance.
  // Therefore we have to be a bit careful with memory management.
  if (frame) {
    return Window::retrieve(frame);
  }

  return ObjectImp::get(exec, p);
}

////////////////////// Location Object ////////////////////////

const ClassInfo Location::info = { "Location", 0, &LocationTable, 0 };
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
  assign	Location::Assign	DontDelete|Function 1
  toString	Location::ToString	DontDelete|Function 0
  replace	Location::Replace	DontDelete|Function 1
  reload	Location::Reload	DontDelete|Function 0
@end
*/
IMPLEMENT_PROTOFUNC_DOM(LocationFunc)
Location::Location(khtml::ChildFrame *f) : m_frame(f)
{
  //kdDebug(6070) << "Location::Location " << this << " m_part=" << (void*)m_part << endl;
}

Location::~Location()
{
  //kdDebug(6070) << "Location::~Location " << this << " m_part=" << (void*)m_part << endl;
}

KParts::ReadOnlyPart *Location::part() const {
  return m_frame ? static_cast<KParts::ReadOnlyPart *>(m_frame->m_part) : 0L;
}

Value Location::get(ExecState *exec, const Identifier &p) const
{
#ifdef KJS_VERBOSE
  kdDebug(6070) << "Location::get " << p.qstring() << " m_part=" << (void*)m_frame->m_part << endl;
#endif

  if (m_frame.isNull() || m_frame->m_part.isNull())
    return Undefined();

  const HashEntry *entry = Lookup::findEntry(&LocationTable, p);

  // properties that work on all Location objects
  if ( entry && entry->value == Replace )
      return lookupOrCreateFunction<LocationFunc>(exec,p,this,entry->value,entry->params,entry->attr);

  // XSS check
  const Window* window = Window::retrieveWindow( m_frame->m_part );
  if ( !window || !window->isSafeScript(exec) )
    return Undefined();

  KURL url = m_frame->m_part->url();
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
      return String( url.port() ? QString::number((int)url.port()) : QLatin1String("") );
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
  if (entry && (entry->attr & Function))
    return lookupOrCreateFunction<LocationFunc>(exec,p,this,entry->value,entry->params,entry->attr);

  return Undefined();
}

void Location::put(ExecState *exec, const Identifier &p, const Value &v, int attr)
{
#ifdef KJS_VERBOSE
  kdDebug(6070) << "Location::put " << p.qstring() << " m_part=" << (void*)m_frame->m_part << endl;
#endif
  if (m_frame.isNull() || m_frame->m_part.isNull())
    return;

  const Window* window = Window::retrieveWindow( m_frame->m_part );
  if ( !window )
    return;

  KURL url = m_frame->m_part->url();

  const HashEntry *entry = Lookup::findEntry(&LocationTable, p);

  if (entry) {

    // XSS check. Only new hrefs can be set from other sites
    if (entry->value != Href && !window->isSafeScript(exec))
      return;

    QString str = v.toString(exec).qstring();
    switch (entry->value) {
    case Href: {
      KHTMLPart* p =qobject_cast<KHTMLPart*>(Window::retrieveActive(exec)->part());
      if ( p )
        url = p->htmlDocument().completeURL( str ).string();
      else
        url = str;
      break;
    }
    case Hash:
      // when the hash is already the same ignore it
      if (str == url.ref()) return;
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
  } else {
    ObjectImp::put(exec, p, v, attr);
    return;
  }

  Window::retrieveWindow(m_frame->m_part)->goURL(exec, url.url(), false /* don't lock history*/ );
}

Value Location::toPrimitive(ExecState *exec, Type) const
{
  if (m_frame) {
    Window* window = Window::retrieveWindow( m_frame->m_part );
    if ( window && window->isSafeScript(exec) )
      return String(toString(exec));
  }
  return Undefined();
}

UString Location::toString(ExecState *exec) const
{
  if (m_frame) {
    Window* window = Window::retrieveWindow( m_frame->m_part );
    if ( window && window->isSafeScript(exec) )
    {
      if (!m_frame->m_part->url().hasPath())
        return m_frame->m_part->url().prettyURL()+"/";
      else
        return m_frame->m_part->url().prettyURL();
    }
  }
  return "";
}

Value LocationFunc::tryCall(ExecState *exec, Object &thisObj, const List &args)
{
  KJS_CHECK_THIS( Location, thisObj );
  Location *location = static_cast<Location *>(thisObj.imp());
  KParts::ReadOnlyPart *part = location->part();

  if (!part) return Undefined();

  Window* window = Window::retrieveWindow(part);

  if ( !window->isSafeScript(exec) && id != Location::Replace)
      return Undefined();

  switch (id) {
  case Location::Assign:
  case Location::Replace:
    Window::retrieveWindow(part)->goURL(exec, args[0].toString(exec).qstring(),
            id == Location::Replace);
    break;
  case Location::Reload: {
    KHTMLPart *khtmlpart = qobject_cast<KHTMLPart*>(part);
    if (part)
      khtmlpart->scheduleRedirection(-1, part->url().url(), true/*lock history*/);
    break;
  }
  case Location::ToString:
    return String(location->toString(exec));
  }
  return Undefined();
}

////////////////////// External Object ////////////////////////

const ClassInfo External::info = { "External", 0, 0, 0 };
/*
@begin ExternalTable 4
  addFavorite	External::AddFavorite	DontDelete|Function 1
@end
*/
IMPLEMENT_PROTOFUNC_DOM(ExternalFunc)

Value External::get(ExecState *exec, const Identifier &p) const
{
  return lookupGetFunction<ExternalFunc,ObjectImp>(exec,p,&ExternalTable,this);
}

Value ExternalFunc::tryCall(ExecState *exec, Object &thisObj, const List &args)
{
  KJS_CHECK_THIS( External, thisObj );
  External *external = static_cast<External *>(thisObj.imp());

  KHTMLPart *part = external->part;
  if (!part)
    return Undefined();

  KHTMLView *widget = part->view();

  switch (id) {
  case External::AddFavorite:
  {
    if (!widget->dialogsAllowed())
      return Undefined();
    part->xmlDocImpl()->updateRendering();
    if (args.size() != 1 && args.size() != 2)
      return Undefined();

    QString url = args[0].toString(exec).qstring();
    QString title;
    if (args.size() == 2)
      title = args[1].toString(exec).qstring();

    // AK - don't do anything yet, for the moment i
    // just wanted the base js handling code in cvs
    return Undefined();

    QString question;
    if ( title.isEmpty() )
      question = i18n("Do you want a bookmark pointing to the location \"%1\" to be added to your collection?")
                 .arg(url);
    else
      question = i18n("Do you want a bookmark pointing to the location \"%1\" titled \"%2\" to be added to your collection?")
                 .arg(url).arg(title);

    emit part->browserExtension()->requestFocus(part);

    QString caption;
    if (!part->url().host().isEmpty())
       caption = part->url().host() + " - ";
    caption += i18n("JavaScript Attempted Bookmark Insert");

    if (KMessageBox::warningYesNo(
          widget, question, caption,
          i18n("Insert"), i18n("Disallow")) == KMessageBox::Yes)
    {
      KBookmarkManager *mgr = KBookmarkManager::userBookmarksManager();
      mgr->addBookmarkDialog(url,title);
    }
    break;
  }
  default:
    return Undefined();
  }

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
IMPLEMENT_PROTOFUNC_DOM(HistoryFunc)

Value History::get(ExecState *exec, const Identifier &p) const
{
  return lookupGet<HistoryFunc,History,ObjectImp>(exec,p,&HistoryTable,this);
}

Value History::getValueProperty(ExecState *, int token) const
{
  // if previous or next is implemented, make sure its not a major
  // privacy leak (see i.e. http://security.greymagic.com/adv/gm005-op/)
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
    kdDebug(6070) << "WARNING: Unhandled token in History::getValueProperty : " << token << endl;
    return Undefined();
  }
}

Value HistoryFunc::tryCall(ExecState *exec, Object &thisObj, const List &args)
{
  KJS_CHECK_THIS( History, thisObj );
  History *history = static_cast<History *>(thisObj.imp());

  Value v = args[0];
  Number n;
  if(v.isValid())
    n = v.toInteger(exec);

  int steps;
  switch (id) {
  case History::Back:
    steps = -1;
    break;
  case History::Forward:
    steps = 1;
    break;
  case History::Go:
    steps = n.intValue();
    break;
  default:
    return Undefined();
  }

  // Special case for go(0) from a frame -> reload only the frame
  // go(i!=0) from a frame navigates into the history of the frame only,
  // in both IE and NS (but not in Mozilla).... we can't easily do that
  // in Konqueror...
  if (!steps) // add && history->part->parentPart() to get only frames, but doesn't matter
  {
    history->part->openURL( history->part->url() ); /// ## need args.reload=true?
  } else
  {
    // Delay it.
    // Testcase: history.back(); alert("hello");
    Window* window = Window::retrieveWindow( history->part );
    window->delayedGoHistory( steps );
  }
  return Undefined();
}

/////////////////////////////////////////////////////////////////////////////

#ifdef Q_WS_QWS

const ClassInfo Konqueror::info = { "Konqueror", 0, 0, 0 };

bool Konqueror::hasProperty(ExecState *exec, const Identifier &p) const
{
  if ( p.qstring().startsWith( "goHistory" ) ) return false;

  return true;
}

Value Konqueror::get(ExecState *exec, const Identifier &p) const
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

  return Value( new KonquerorFunc(this, p.qstring().latin1() ) );
}

Value KonquerorFunc::tryCall(ExecState *exec, Object &, const List &args)
{
  KParts::BrowserExtension *ext = konqueror->part->browserExtension();

  if(!ext)
    return Undefined();

  KParts::BrowserInterface *iface = ext->browserInterface();

  if ( !iface )
    return Undefined();

  QByteArray n = m_name.data();
  n += "()";
  iface->callMethod( n.data(), QVariant() );

  return Undefined();
}

UString Konqueror::toString(ExecState *) const
{
  return UString("[object Konqueror]");
}

#endif
/////////////////////////////////////////////////////////////////////////////

#include "kjs_window.moc"
