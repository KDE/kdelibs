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
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

// $Id$

#include "undointerface.h"
#include "undodcopinterface.h"
#include "document.h"

namespace KTextEditor
{

class PrivateUndoInterface
{
  public:
    PrivateUndoInterface() {interface = 0;}
    ~PrivateUndoInterface() {}
    UndoDCOPInterface *interface;
};

}

using namespace KTextEditor;

unsigned int UndoInterface::globalUndoInterfaceNumber = 0;

UndoInterface::UndoInterface()
{
  globalUndoInterfaceNumber++;
  myUndoInterfaceNumber = globalUndoInterfaceNumber++;

  d = new PrivateUndoInterface();   
  QString name = "UndoInterface#" + QString::number(myUndoInterfaceNumber);
  d->interface = new UndoDCOPInterface(this, name.latin1());
}

UndoInterface::~UndoInterface()
{
  delete d->interface;
  delete d;
}

unsigned int UndoInterface::undoInterfaceNumber () const
{
  return myUndoInterfaceNumber;
}

void UndoInterface::setUndoInterfaceDCOPSuffix (const QCString &suffix)
{
  d->interface->setObjId ("UndoInterface#"+suffix);
}

UndoInterface *KTextEditor::undoInterface (Document *doc)
{  
  if (!doc)
    return 0;

  return static_cast<UndoInterface*>(doc->qt_cast("KTextEditor::UndoInterface"));
}
