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

#include "wordwrapinterface.h"  
#include "document.h"


namespace KTextEditor
{

class PrivateWordWrapInterface
{
  public:
    PrivateWordWrapInterface() {}
    ~PrivateWordWrapInterface() {}
};

}

using namespace KTextEditor;

unsigned int WordWrapInterface::globalWordWrapInterfaceNumber = 0;

WordWrapInterface::WordWrapInterface()
{
  globalWordWrapInterfaceNumber++;
  myWordWrapInterfaceNumber = globalWordWrapInterfaceNumber++;

  d = new PrivateWordWrapInterface();
}

WordWrapInterface::~WordWrapInterface()
{
  delete d;
}

unsigned int WordWrapInterface::wordWrapInterfaceNumber () const
{
  return myWordWrapInterfaceNumber;
}

void WordWrapInterface::setWordWrapInterfaceDCOPSuffix (const QCString &/*suffix*/)
{
  //d->interface->setObjId ("WordWrapInterface#"+suffix);
}
  
WordWrapInterface *KTextEditor::wordWrapInterface (Document *doc)
{          
  if (!doc)
    return 0;

  return static_cast<WordWrapInterface*>(doc->qt_cast("KTextEditor::WordWrapInterface"));
}
