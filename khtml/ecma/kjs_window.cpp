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
#include <dom_string.h>
#include <kurl.h>
#include <kmessagebox.h>
#include <klocale.h>

#include <kjs/operations.h>
#include "kjs_window.h"

#include <qevent.h>
#include "khtmlview.h"
#include "htmlattrs.h"
#include <html_element.h>
#include "khtml_part.h"

using namespace KJS;

Window::Window(KHTMLView *w)
  : widget(w), winq(0L)
{
}

Window::~Window()
{
  delete winq;
}

KJSO Window::tryGet(const UString &p) const
{
  if (p == "location")
    return KJSO(new Location(widget->part()));
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

  return Imp::get(p);
}

void Window::tryPut(const UString &p, const KJSO &v)
{
  if (p == "status") {
    String s = v.toString();
    WindowFunc::setStatusBarText(widget->part(), s.value().qstring());
  } else if (p == "location") {
    QString str = v.toString().value().qstring();
    widget->part()->scheduleRedirection(0, str);
  } else if (p == "onload") {
    if (v.isA(ConstructorType)) {
      DOM::DOMString s = ((FunctionImp*)v.imp())->name().string() + "()";
      // doesn't work yet
      //      widget->part()->htmlDocument().body().setAttribute(ATTR_ONLOAD, s);
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

  KHTMLView *widget = window->widget;
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
    v = args[1];
    s = v.toString();
    QString target = s.value().qstring();
    widget->part()->urlSelected( str, 0, 0, target );
      // ### add size and other parameters defined in the third argument.
      // see http://msdn.microsoft.com/workshop/author/dhtml/reference/methods/open_0.asp
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
  parent->widget->part()->executeScript(timeoutHandler.qstring());
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

#include "kjs_window.moc"

