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
#include <kwinmodule.h>
#include <kconfig.h>

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

#include <qevent.h>
#include "khtmlview.h"
#include <html_element.h>
#include <html_documentimpl.h>
#include "khtml_part.h"
#include "xml/dom_docimpl.h"

using namespace KJS;

namespace KJS {

ScriptMap *script_map = 0L;

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

Window::Window(KHTMLPart *p)
  : m_part(p), screen(0), history(0), frames(0), loc(0),
    openedByJS(false)
{
  winq = new WindowQObject(this);
  //kdDebug() << "Window::Window this=" << this << " part=" << part << endl;
}

Window::~Window()
{
  kdDebug() << "Window::~Window this=" << this << " part=" << m_part << endl;
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
  assert(p && script_map);
  KJScript *script;
  ScriptMap::Iterator it = script_map->find(p);
  if (it == script_map->end()) {
    KJScript *old = KJScript::current();
    script = kjs_create(p);
    old->init();
  } else {
    script = it.data();
  }
  // the Global object is the "window"
  return script->globalObject();
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
  if (screen && !screen->refcount)
    screen->mark();
  if (history && !history->refcount)
    history->mark();
  if (frames && !frames->refcount)
    frames->mark();
  if (loc && !loc->refcount)
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

String Window::toString() const
{
  return UString( "object Window" );
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

KJSO Window::get(const UString &p) const
{
  if (p == "closed")
    return Boolean(m_part.isNull());

  // we don't want any operations on a closed window
  if (m_part.isNull())
    return Undefined();

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
 // else if (p == "event")
//    return getDOMEvent(static_cast<DOM::Event>(m_part->view()->lastDOMMouseEvent()));
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
  else if (p == "offscreenBuffering")
    return Boolean(true);
  else if (p == "opener")
    if (opener.isNull())
      return Null(); 	// ### a null Window might be better, but == null
    else                // doesn't work yet
      return retrieve(opener);
  else if (p == "outerHeight")
    return Number(m_part->view() ? m_part->view()->height() : 0); // ###
  else if (p == "outerWidth")
    return Number(m_part->view() ? m_part->view()->width() : 0); // ###
  else if (p == "pageXOffset")
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
  if (p == "status") {
    String s = v.toString();
    m_part->setJSStatusBarText(s.value().qstring());
  } else if (p == "defaultStatus" || p == "defaultstatus") {
    String s = v.toString();
    m_part->setJSDefaultStatusBarText(s.value().qstring());
  } else if (p == "location") {
    QString str = v.toString().value().qstring();
    m_part->scheduleRedirection(0, Window::retrieveActive()->m_part->
                              completeURL(str).url().prepend( "target://_self/#" ));
  } else if (p == "onload") {
    if (isSafeScript() && v.isA(ConstructorType)) {
      // ### other attributes like this?
      DOM::Element body;
      if (!m_part->htmlDocument().isNull() &&
          !(body = m_part->htmlDocument().body()).isNull())
        body.setAttribute("onload",((FunctionImp*)v.imp())->name().string() + "()");
    }
  } else {
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
  return act && originCheck(m_part->url(), act->url());
}

Completion WindowFunc::tryExecute(const List &args)
{
  KJSO result;
  QString str, str2;
  int i;

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
    i = KMessageBox::warningYesNo(widget, str, "JavaScript",
                                  i18n("OK"), i18n("Cancel"));
    result = Boolean((i == KMessageBox::Yes));
    break;
  case Prompt:
    part->xmlDocImpl()->updateRendering();
    if (args.size() >= 2)
      str2 = QInputDialog::getText("Konqueror: Prompt", str,
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
	      winargs.height = val.toInt();
	    else if (key == "width")
	      winargs.width = val.toInt();
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
        if (!str.isEmpty()) {
	    if (part->baseURL().isEmpty())
    	    url = KURL(part->url(), str);
	    else
	        url = KURL(part->baseURL(), str);
        }
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
	  Window *win = Window::retrieveWindow(khtmlpart);
	    //qDebug("opener set to %p (this Window's part) in new Window %p  (this Window=%p)",part,win,window);
	    win->opener = part;
	    win->openedByJS = true;
	    uargs.serviceType = QString::null;
	    if (uargs.frameName == "_blank")
              uargs.frameName = QString::null;
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
    break;
  case SetTimeout:
    if (args.size() == 2 && v.isA(StringType)) {
      int i = args[1].toInt32();
      int r = (const_cast<Window*>(window))->installTimeout(s.value(), i, true /*single shot*/);
      result = Number(r);
    } else
      result = Undefined();
    break;
  case SetInterval:
    if (args.size() == 2 && v.isA(StringType)) {
      int i = args[1].toInt32();
      int r = (const_cast<Window*>(window))->installTimeout(s.value(), i, false);
      result = Number(r);
    } else
      result = Undefined();
    break;
  case ClearTimeout:
  case ClearInterval:
    result = Undefined();
    (const_cast<Window*>(window))->clearTimeout(v.toInt32());
    break;
  case Focus:
    result = Undefined();
    // TODO
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
    if (!window->openedByJS)
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
  }
  return Completion(ReturnValue, result);

}

void WindowFunc::initJScript(KHTMLPart *p)
{
  (void)p->jScript(); // dummy call to create an interpreter
}

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
  map.clear();
  ScriptMap::Iterator it = script_map->find(part);
  if (it == script_map->end())
    return;
  //  KJScript *scr = it.data();
  script_map->remove(part);
}

int WindowQObject::installTimeout(const UString &handler, int t, bool singleShot)
{
  int id = startTimer(t);
  QString hnd = handler.qstring();
  // prepend 0 or 1 to differentiate between single timeouts and intervals
  hnd.prepend(singleShot ? "0" : "1");
  map.insert(id, hnd);

  return id;
}

void WindowQObject::clearTimeout(int timerId)
{
  killTimer(timerId);
  map.remove(timerId);
}

void WindowQObject::timerEvent(QTimerEvent *e)
{
  if (!parent->part().isNull()) {
    QString hnd = map[e->timerId()];
    parent->part()->executeScript(hnd.mid(1));
    // remove single shots installed by setTimeout()
    if (hnd.startsWith("0"))
      clearTimeout(e->timerId());
  }
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
  if (part.isNull())
    return Undefined();

  QList<KParts::ReadOnlyPart> frames = part->frames();
  int len = frames.count();
  if (p == "length")
    return Number(len);

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

Location::Location(KHTMLPart *p) : part(p) { }

KJSO Location::get(const UString &p) const
{
  if (part.isNull())
    return Undefined();

  KURL url = part->url();
  QString str;

  if (p == "hash")
    str = url.ref();
  else if (p == "host") {
    str = url.host();
    if (url.port())
      str += ":" + QString::number((int)url.port());
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
  else if (HostImp::hasProperty(p))
    return HostImp::get(p);
  else if (p == "replace")
    return Function(new LocationFunc(part, LocationFunc::Replace));
  else if (p == "reload")
    return Function(new LocationFunc(part, LocationFunc::Reload));
  else
    return Undefined();

  return String(str);
}

void Location::put(const UString &p, const KJSO &v)
{
  if (part.isNull())
    return;

  QString str = v.toString().value().qstring();
  KURL url;

  if (p == "href")
       url = Window::retrieveActive()->part()->completeURL(str);
  else {
    url = part->url();
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
  part->scheduleRedirection(0, url.url().prepend( "target://_self/#" ) );
}

KJSO Location::toPrimitive(Type) const
{
    return toString();
}

String Location::toString() const
{

 if (!part->url().hasPath())
        return String(part->url().prettyURL()+"/");
    else
        return String(part->url().prettyURL());
}

LocationFunc::LocationFunc(KHTMLPart *p, int i) : part(p), id(i) { };

Completion LocationFunc::tryExecute(const List &args)
{
  if (!part.isNull()) {
    switch (id) {
    case Replace:
      part->scheduleRedirection(0, args[0].toString().value().qstring().prepend( "target://_self/#" ) );
      break;
    case Reload:
      part->scheduleRedirection(0, part->url().url().prepend( "target://_self/#" ) );
      break;
    }
  }
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

#include "kjs_window.moc"

