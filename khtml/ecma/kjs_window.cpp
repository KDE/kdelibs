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
#include <qptrdict.h>
#include <dom_string.h>
#include <kurl.h>
#include <kmessagebox.h>
#include <klocale.h>
#include <kparts/browserextension.h>

#include <kjs/operations.h>
#include "kjs_window.h"
#include "kjs_navigator.h"
#include "kjs_html.h"
#include "kjs_dom.h"

#include <qevent.h>
#include "khtmlview.h"
#include "htmlattrs.h"
#include <html_element.h>
#include "khtml_part.h"

using namespace KJS;

QPtrDict<Window> *window_dict = 0L;

Window *KJS::newWindow(KHTMLPart *p)
{
  Window *w;
  if (!window_dict)
    window_dict = new QPtrDict<Window>;
  else if ((w = window_dict->find(p)) != 0L)
    return w;
  
  w = new Window(p);
  window_dict->insert(p, w);
  
  return w;
}

class FrameArray : public HostImp {
public:
  FrameArray(KHTMLPart *p) : part(p) { }
  KJSO get(const UString &p) const;
private:
  KHTMLPart *part;
};

Window::Window(KHTMLPart *p)
  : part(p), winq(0L)
{
}

Window::~Window()
{
  window_dict->remove(part);
  if (window_dict->isEmpty()) {
    delete window_dict;
    window_dict = 0L;
  }
  delete winq;
}

bool Window::hasProperty(const UString &p, bool) const
{
  return !get(p).isA(UndefinedType);
}

KJSO Window::get(const UString &p) const
{
  if (p == "location")
    return KJSO(new Location(part));
  else if (p == "document")
    return getDOMNode(part->htmlDocument());
  else if (p == "navigator")
    return KJSO(new Navigator(part));
  else if (p == "self")
    return KJSO(newWindow(part));
  else if (p == "parent")
    return KJSO(newWindow(part->parentPart() ? part->parentPart() : part));
  else if (p == "top") {
    KHTMLPart *p = part;
    while (p->parentPart())
      p = p->parentPart();
    return KJSO(newWindow(p));
  } else if (p == "name")
    return String(part->name());
  else if (p == "Image")
    return KJSO(new ImageConstructor(Global::current()));
  else if (p == "alert")
    return Function(new WindowFunc(this, WindowFunc::Alert));
  else if (p == "confirm")
    return Function(new WindowFunc(this, WindowFunc::Confirm));
  else if (p == "prompt")
    return Function(new WindowFunc(this, WindowFunc::Prompt));
  else if (p == "open")
    return Function(new WindowFunc(this, WindowFunc::Open));
  else if (p == "setTimeout")
    return Function(new WindowFunc(this, WindowFunc::SetTimeout));
  else if (p == "clearTimeout")
    return Function(new WindowFunc(this, WindowFunc::ClearTimeout));
  else if (p == "frames")
    return new FrameArray(part);

  // allow shortcuts like 'Image1' instead of document.images.Image1
  DOM::HTMLCollection coll = part->htmlDocument().all();
  DOM::HTMLElement element = coll.namedItem(p.string());
  if (!element.isNull())
      return getDOMNode(element);
  
  // we are looking for a frame by name here.
  // essentially a hack relying on the assumption that the ordering of
  // frameNames() is identical to frames()
  // TODO: http://www.w3.org/TR/html4/appendix/notes.html#notes-frames
  QStringList list = part->frameNames();
  int i = list.findIndex(p.qstring());
  if (i >= 0) {
    QList<KParts::ReadOnlyPart> frames = part->frames();
    const KParts::ReadOnlyPart *frame = frames.at(i);
    assert(frame->inherits("KHTMLPart"));
    const KHTMLPart *khtml = static_cast<const KHTMLPart*>(frame);
    return KJSO(newWindow(const_cast<KHTMLPart*>(khtml)));
  }
  
  return Imp::get(p);
}

void Window::put(const UString &p, const KJSO &v)
{
  if (p == "status") {
    String s = v.toString();
    WindowFunc::setStatusBarText(part, s.value().qstring());
  } else if (p == "location") {
    QString str = v.toString().value().qstring();
    part->scheduleRedirection(0, str);
  } else if (p == "onload") {
    if (v.isA(ConstructorType)) {
      DOM::DOMString s = ((FunctionImp*)v.imp())->name().string() + "()";
      part->htmlDocument().body().setAttribute("onload", s);
    }
  } else
    Imp::put(p, v);
}

int Window::installTimeout(const UString &handler, int t)
{
  if (!winq)
    winq = new WindowQObject(this);

  return winq->installTimeout(handler, t);
}

void Window::clearTimeout(int timerId)
{
  if (winq)
    winq->clearTimeout(timerId);
}

Completion WindowFunc::tryExecute(const List &args)
{
  KJSO result;
  QString str, str2;
  int i;

  KHTMLView *widget = window->part->view();
  KJSO v = args[0];
  String s = v.toString();
  str = s.value().qstring();

  switch (id) {
  case Alert:
    KMessageBox::error(widget, str, "JavaScript");
    result = Undefined();
    break;
  case Confirm:
    i = KMessageBox::warningYesNo(widget, str, "JavaScript",
                                  i18n("OK"), i18n("Cancel"));
    result = Boolean((i == KMessageBox::Yes));
    break;
  case Prompt:
    if (args.size() >= 2)
      str2 = QInputDialog::getText("Konqueror: Prompt", str,
                                   args[1].toString().value().qstring());
    else
      str2 = QInputDialog::getText("Konqueror: Prompt", str);
    result = String(str2);
    break;
  case Open:
  {
    int w_left = 0, w_top = 0, w_width = 330, w_height = 400;
    KParts::WindowArgs winargs;

    // scan feature argument
    v = args[2];
    QString features;
    if (!v.isA(UndefinedType)) {
	features = v.toString().value().qstring();
	QStringList flist = QStringList::split(',', features);
	QStringList::ConstIterator it = flist.begin();
	while (it != flist.end()) {
	    int pos = (*it).find('=');
	    if (pos >= 0) {
		QString key = (*it).left(pos).stripWhiteSpace().lower();
		QString val = (*it).mid(pos + 1).stripWhiteSpace().lower();
		if (key == "left" || key == "screenx")
		  w_left = val.toInt();
		else if (key == "top" || key == "screeny")
		  w_top = val.toInt();
		else if (key == "height")
		  w_height = val.toInt();
		else if (key == "width")
		  w_width = val.toInt();
		else if (key == "menubar")
		  winargs.menuBarVisible = (val == "1" || val == "yes");
		else if (key == "toolbar")
		  winargs.toolBarsVisible = (val == "1" || val == "yes");
		else if (key == "status")
		  winargs.statusBarVisible = (val == "1" || val == "yes");
		else if (key == "resizable")
		  winargs.resizable = (val == "1" || val == "yes");
		else if (key == "fullscreen")
		  winargs.fullscreen = (val == "1" || val == "yes");
	    }
	    it++;
	}
    }

    // prepare arguments
    KURL url(str);
    KParts::URLArgs uargs;
    uargs.frameName = args[1].toString().value().qstring();
    uargs.serviceType = "text/html";
    // guarantee minimum size
    if (w_width < 100)
      w_width = 100;
    if (w_height < 100)
      w_height = 100;
    winargs.geometry = QRect(w_left, w_top, w_width, w_height);
    
    // request new window
    KParts::ReadOnlyPart *newPart = 0L;
    emit widget->part()->browserExtension()->createNewWindow(url, uargs,
							     winargs, newPart);
    if (newPart && newPart->inherits("KHTMLPart"))
	result = newWindow(static_cast<KHTMLPart*>(newPart));
    else
	result = Undefined();
    break;
  }
  case SetTimeout:
    result = Undefined();
    if (args.size() == 2 && v.isA(StringType)) {
      int i = args[1].toInt32();
      int r = (const_cast<Window*>(window))->installTimeout(s.value(), i);
      result = Number(r);
    } else
      result = Undefined();
    break;
  case ClearTimeout:
    result = Undefined();
    (const_cast<Window*>(window))->clearTimeout(v.toInt32());
    break;
  }
  return Completion(Normal, result);
}

void WindowFunc::setStatusBarText(KHTMLPart *p, const QString &s)
{
  p->setStatusBarText(s);
}

WindowQObject::WindowQObject(Window *w)
  : parent(w), timer(0L)
{
}

WindowQObject::~WindowQObject()
{
  delete timer;
}

int WindowQObject::installTimeout(const UString &handler, int t)
{
  /* TODO: multiple timers */
  if (!timer) {
    timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), SLOT(timeout()));
  }
  timeoutHandler = handler;
  timer->start(t, true);

  return 0;
}

void WindowQObject::clearTimeout(int /* timerId */)
{
  /* TODO: delete on id basis */
  if (timer) {
    timer->stop();
    timeoutHandler = "";
  }
}

void WindowQObject::timeout()
{
  parent->part->executeScript(timeoutHandler.qstring());
}

KJSO FrameArray::get(const UString &p) const
{
  QList<KParts::ReadOnlyPart> frames = part->frames();
  int len = frames.count();
  if (p == "length")
    return Number(len);
  
  const KParts::ReadOnlyPart *frame = 0L;

  // check for the name or number
  QStringList list = part->frameNames();
  int i = list.findIndex(p.qstring());
  if (i < 0)
    i = (int)p.toDouble();
  if (i >= 0 && i < len)
    frame = frames.at(i);

  if (frame && frame->inherits("KHTMLPart")) {
    const KHTMLPart *khtml = static_cast<const KHTMLPart*>(frame);
    return KJSO(newWindow(const_cast<KHTMLPart*>(khtml)));
  }
  
  return Undefined();
}

KJSO Location::get(const UString &p) const
{
  KURL url = part->url();
  QString str;

  if (p == "hash")
    str = url.ref();
  else if (p == "host") {
    str = url.host();
    if (url.port())
      str += ":" + QString::number(url.port());
  } else if (p == "hostname")
    str = url.host();
  else if (p == "href")
    str = url.prettyURL();
  else if (p == "pathname")
    str = url.path();
  else if (p == "port")
    str = url.port() ? QString::number(url.port()) : QString::fromLatin1("");
  else if (p == "protocol")
    str = url.protocol();
  else if (p == "search")
    str = url.query();
  else if (p == "[[==]]")
    return toString();
  else if (p == "replace")
    return Function(new LocationFunc(part, LocationFunc::Replace));
  else
    return Undefined();

  return String(str);
}

void Location::put(const UString &p, const KJSO &v)
{
  QString str = v.toString().value().qstring();

  if (p == "href")
    part->scheduleRedirection(0, str);
  /* TODO: remaining location properties */
}

KJSO Location::toPrimitive(Type) const
{
  return toString();
}

String Location::toString() const
{
  return String(part->url().prettyURL());
}

Completion LocationFunc::tryExecute(const List &args)
{
  QString str = args[0].toString().value().qstring();
  part->scheduleRedirection(0, str);
  return Completion(Normal, Undefined());
}

#include "kjs_window.moc"

