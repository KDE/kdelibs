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
#include <dom_string.h>
#include <kmessagebox.h>
#include <klocale.h>

#include <kjs/operations.h>
#include "kjs_window.h"

#include <qevent.h>
#include "khtmlview.h"
#include "khtml_part.h"

using namespace KJS;

Window::~Window()
{
  delete timer;
}

KJSO Window::tryGet(const UString &p) const
{
  if (p == "alert")
    return Function(new WindowFunc(this, WindowFunc::Alert));
  else if (p == "confirm")
    return Function(new WindowFunc(this, WindowFunc::Confirm));
  else if (p == "open")
    return Function(new WindowFunc(this, WindowFunc::Open));
  else if (p == "setTimeout")
    return Function(new WindowFunc(this, WindowFunc::SetTimeout));

  return Undefined();
}

void Window::tryPut(const UString &p, const KJSO &v)
{
  if (p == "status") {
    String s = v.toString();
    WindowFunc::setStatusBarText(widget->part(), s.value().qstring());
  }
}

void Window::timeout()
{
  widget->part()->executeScript(timeoutHandler.qstring());
}

void Window::installTimeout(const UString &handler, int t)
{
  if (!timer) {
    timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), SLOT(timeout()));
  }
  timeoutHandler = handler;
  timer->start(t, true);
}

Completion WindowFunc::tryExecute(const List &args)
{
  KJSO result;
  QString str;
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
      (const_cast<Window*>(window))->installTimeout(s.value(), i);
      break;
    }
  }
  return Completion(Normal, result);
}

void WindowFunc::setStatusBarText(KHTMLPart *p, const QString &s)
{
  p->setStatusBarText(s);
}

#include "kjs_window.moc"
