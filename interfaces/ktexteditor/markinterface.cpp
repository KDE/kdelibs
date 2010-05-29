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

// $Id: markinterface.cpp 465272 2005-09-29 09:47:40Z mueller $

#include "markinterface.h"
#include "document.h"

namespace KTextEditor
{

class PrivateMarkInterface
{
  public:
    PrivateMarkInterface() {}
    ~PrivateMarkInterface() {}
};

}

using namespace KTextEditor;

unsigned int MarkInterface::globalMarkInterfaceNumber = 0;

MarkInterface::MarkInterface()
{
  globalMarkInterfaceNumber++;
  myMarkInterfaceNumber = globalMarkInterfaceNumber++;

  d = new PrivateMarkInterface();
}

MarkInterface::~MarkInterface()
{
  delete d;
}

unsigned int MarkInterface::markInterfaceNumber () const
{
  return myMarkInterfaceNumber;
}

void MarkInterface::setMarkInterfaceDCOPSuffix (const QCString &/*suffix*/)
{
  //d->interface->setObjId ("MarkInterface#"+suffix);
}

MarkInterface *KTextEditor::markInterface (Document *doc)
{                                 
  if (!doc)
    return 0;

  return static_cast<MarkInterface*>(doc->qt_cast("KTextEditor::MarkInterface"));
}

int MarkInterface::reservedMarkersCount()
{
    return 7;
}
