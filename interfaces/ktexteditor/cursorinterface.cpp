/* This file is part of the KDE libraries
   Copyright (C) 2001 Christoph Cullmann <cullmann@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

// $Id: cursorinterface.cpp 465272 2005-09-29 09:47:40Z mueller $

#include "cursorinterface.h"
#include "document.h"

namespace KTextEditor
{

class PrivateCursorInterface
{
  public:
    PrivateCursorInterface() {}
    ~PrivateCursorInterface() {}
};

}

using namespace KTextEditor;

unsigned int CursorInterface::globalCursorInterfaceNumber = 0;

CursorInterface::CursorInterface()
{
  globalCursorInterfaceNumber++;
  myCursorInterfaceNumber = globalCursorInterfaceNumber++;

  d = new PrivateCursorInterface();
}

CursorInterface::~CursorInterface()
{
  delete d;
}

unsigned int CursorInterface::cursorInterfaceNumber () const
{
  return myCursorInterfaceNumber;
}

void CursorInterface::setCursorInterfaceDCOPSuffix (const QCString &/*suffix*/)
{
  //d->interface->setObjId ("CursorInterface#"+suffix);
}

CursorInterface *KTextEditor::cursorInterface (Document *doc)
{               
  if (!doc)
    return 0;

  return static_cast<CursorInterface*>(doc->qt_cast("KTextEditor::CursorInterface"));
}
