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

#include <stdio.h>

#include <dom_string.h>
#include <kmessagebox.h>
#include <klocale.h>

#include <operations.h>
#include "kjs_window.h"

using namespace KJS;

KJSO *Window::get(const CString &p) const
{
  if (p == "alert")
    return new WindowFunc(widget, WindowFunc::Alert);
  else if (p == "confirm")
    return new WindowFunc(widget, WindowFunc::Confirm);

  return new KJSUndefined();
}

void Window::put(const CString &, KJSO *, int)
{
}

KJSO *WindowFunc::execute(KJSContext *context)
{
  Ptr result;
  Ptr v, s;
  QString str;
  int i;

  v = context->activation->get("0");
  s = toString(v);
  str = s->sVal().qstring();

  switch (id) {
  case Alert:
    KMessageBox::error((QWidget*)widget, str, "JavaScript");
    result = new KJSUndefined();
    break;
  case Confirm:
    i = KMessageBox::warningYesNo((QWidget*)widget, str, "JavaScript",
				  i18n("OK"), i18n("Cancel"));
    result = new KJSBoolean((i == KMessageBox::Yes));
    break;
  }

  return new KJSCompletion(Normal, result);
}
