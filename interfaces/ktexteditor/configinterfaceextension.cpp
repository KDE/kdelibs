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

// $Id: configinterfaceextension.cpp 465272 2005-09-29 09:47:40Z mueller $

#include "configinterfaceextension.h"
#include "configinterfaceextension.moc"

#include "document.h"
#include "plugin.h"

namespace KTextEditor
{

class PrivateConfigInterfaceExtension
{
  public:
    PrivateConfigInterfaceExtension() {}
     ~PrivateConfigInterfaceExtension() {}
};

}

using namespace KTextEditor;

ConfigPage::ConfigPage ( QWidget *parent, const char *name ) : QWidget (parent, name) { ; }
 
ConfigPage::~ConfigPage () { ; }

unsigned int ConfigInterfaceExtension::globalConfigInterfaceExtensionNumber = 0;

ConfigInterfaceExtension::ConfigInterfaceExtension()
{
  globalConfigInterfaceExtensionNumber++;
  myConfigInterfaceExtensionNumber = globalConfigInterfaceExtensionNumber++;

  d = new PrivateConfigInterfaceExtension();
}

ConfigInterfaceExtension::~ConfigInterfaceExtension()
{
  delete d;
}

unsigned int ConfigInterfaceExtension::configInterfaceExtensionNumber () const
{
  return myConfigInterfaceExtensionNumber;
}

void ConfigInterfaceExtension::setConfigInterfaceExtensionDCOPSuffix (const QCString &/*suffix*/)
{
  //d->interface->setObjId ("ConfigInterfaceExtension#"+suffix);
}

ConfigInterfaceExtension *KTextEditor::configInterfaceExtension (Document *doc)
{           
  if (!doc)
    return 0;

  return static_cast<ConfigInterfaceExtension*>(doc->qt_cast("KTextEditor::ConfigInterfaceExtension"));
}
                      
ConfigInterfaceExtension *KTextEditor::configInterfaceExtension (Plugin *plugin)
{                       
  if (!plugin)
    return 0;

  return static_cast<ConfigInterfaceExtension*>(plugin->qt_cast("KTextEditor::ConfigInterfaceExtension"));
}
