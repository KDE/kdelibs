/* This file is part of the KDE project
   Copyright (C) 2001 Christoph Cullmann (cullmann@kde.org)

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#include <config.h>

#include "cursor.h"

#include "document.h"
#include "document.moc"

#include "view.h"
#include "view.moc"

#include "plugin.h"
#include "plugin.moc"

#include "codecompletioninterface.h"
#include "commandinterface.h"
#include "configinterface.h"
#include "encodinginterface.h"
#include "highlightinginterface.h"
#include "markinterface.h"
#include "menuinterface.h"
#include "modificationinterface.h"
#include "searchinterface.h"
#include "selectioninterface.h"
#include "sessionconfiginterface.h"
#include "templateinterface.h"
#include "texthintinterface.h"
#include "variableinterface.h"

#include "configinterface.moc"

#include <kaction.h>
#include <kparts/factory.h>
#include <kparts/componentfactory.h>

using namespace KTextEditor;

namespace KTextEditor
{
  class PrivateDocument
  {
  public:
    PrivateDocument ()
    {
    }

    ~PrivateDocument()
    {
    }
  };

  class PrivateView
  {
  public:
    PrivateView ()
    {
    }

    ~PrivateView()
    {
    }
  };

  class PrivatePlugin
  {
  public:
    PrivatePlugin ()
    {
    }

    ~PrivatePlugin ()
    {
    }
  };
}

static int globalDocumentNumber = 0;
static int globalViewNumber = 0;
static int globalPluginNumber = 0;

Document::Document( QObject *parent, const char *name )
 : KParts::ReadWritePart( parent, name )
 , m_d (0)
 , m_documentNumber (++globalDocumentNumber)
{
}

Document::~Document()
{
}

int Document::documentNumber () const
{
  return m_documentNumber;
}

View::View( QWidget *parent )
 : QWidget( parent )
 , m_d (0)
 , m_viewNumber (++globalViewNumber)
{
}

View::~View()
{
}

int View::viewNumber () const
{
  return m_viewNumber;
}

Plugin::Plugin( Document *document, const char *name )
 : QObject (document, name )
 , m_d (0)
 , m_pluginNumber (++globalPluginNumber)
 , m_doc (document)
{
}

Plugin::~Plugin()
{
}

int Plugin::pluginNumber () const
{
  return m_pluginNumber;
}

Document *Plugin::document ()
{
  return m_doc;
}

Document *KTextEditor::createDocument ( const char* libname, QObject *parent )
{
  return KParts::ComponentFactory::createPartInstanceFromLibrary<Document>( libname, 0, 0, parent, "" );
}

Plugin *KTextEditor::createPlugin ( const char* libname, Document *document )
{
  return KParts::ComponentFactory::createInstanceFromLibrary<Plugin>( libname, document, "" );
}

KTextEditor::CommandInterface *KTextEditor::commandInterface (Document *doc)
{
  if (!doc)
    return 0;

  return dynamic_cast<KTextEditor::CommandInterface*>( doc );
}

KTextEditor::ModificationInterface *KTextEditor::modificationInterface (Document *doc)
{
  if (!doc)
    return 0;

  return dynamic_cast<KTextEditor::ModificationInterface*>( doc );
}

SelectionInterface *KTextEditor::selectionInterface (View *view)
{
  if (!view)
    return 0;

  return dynamic_cast<KTextEditor::SelectionInterface*>( view );
}


CodeCompletionInterface *KTextEditor::codeCompletionInterface (View *view)
{
  if (!view)
    return 0;

  return dynamic_cast<KTextEditor::CodeCompletionInterface*>( view );
}

ConfigInterface *KTextEditor::configInterface (Document *doc)
{
  if (!doc)
    return 0;

  return dynamic_cast<KTextEditor::ConfigInterface*>( doc );
}

ConfigInterface *KTextEditor::configInterface (Plugin *plugin)
{
  if (!plugin)
    return 0;

  return dynamic_cast<KTextEditor::ConfigInterface*>( plugin );
}

HighlightingInterface *KTextEditor::highlightingInterface (Document *doc)
{
  if (!doc)
    return 0;

  return dynamic_cast<KTextEditor::HighlightingInterface*>( doc );
}


EncodingInterface *KTextEditor::encodingInterface (Document *doc)
{
  if (!doc)
    return 0;

  return dynamic_cast<EncodingInterface*>( doc );
}


SearchInterface *KTextEditor::searchInterface (Document *doc)
{
  if (!doc)
    return 0;

  return dynamic_cast<KTextEditor::SearchInterface*>( doc );
}


MarkInterface *KTextEditor::markInterface (Document *doc)
{
  if (!doc)
    return 0;

  return dynamic_cast<KTextEditor::MarkInterface*>(doc);
}

MenuInterface *KTextEditor::menuInterface (View *view)
{
  if (!view)
    return 0;

  return dynamic_cast<KTextEditor::MenuInterface*>( view );
}

TextHintInterface *textHintInterface (View *view)
{
  if (!view)
    return 0;

  return dynamic_cast<KTextEditor::TextHintInterface*>( view );
}

VariableInterface *KTextEditor::variableInterface( Document *doc )
{
  if ( ! doc )
    return 0;

  return dynamic_cast<KTextEditor::VariableInterface*>( doc );
}


SessionConfigInterface *KTextEditor::sessionConfigInterface (Document *doc)
{
  if (!doc)
    return 0;

  return dynamic_cast<SessionConfigInterface*>( doc );
}

SessionConfigInterface *KTextEditor::sessionConfigInterface (View *view)
{
  if (!view)
    return 0;

  return dynamic_cast<SessionConfigInterface*>( view );
}

SessionConfigInterface *KTextEditor::sessionConfigInterface (Plugin *plugin)
{
  if (!plugin)
    return 0;

  return dynamic_cast<SessionConfigInterface*>( plugin );
}
