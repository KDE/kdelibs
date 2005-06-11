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

#include "configpage.h"
#include "configpage.moc"

#include "factory.h"
#include "factory.moc"

#include "editor.h"
#include "editor.moc"

#include "document.h"
#include "document.moc"

#include "view.h"
#include "view.moc"

#include "plugin.h"
#include "plugin.moc"

#include "codecompletioninterface.h"
#include "commandinterface.h"
#include "configinterface.h"
#include "highlightinginterface.h"
#include "markinterface.h"
#include "modificationinterface.h"
#include "searchinterface.h"
#include "sessionconfiginterface.h"
#include "templateinterface.h"
#include "texthintinterface.h"
#include "variableinterface.h"

#include <kaction.h>
#include <kparts/factory.h>
#include <kparts/componentfactory.h>
#include <kdebug.h>

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
static int globalPluginNumber = 0;

Factory::Factory( QObject *parent )
 : KParts::Factory( parent )
{
}

Factory::~Factory()
{
}

Editor::Editor( QObject *parent )
 : QObject ( parent )
{
}

Editor::~Editor()
{
}

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

bool Document::cursorInText(const Cursor& cursor)
{
  if ( (cursor.line()<0) || (cursor.line()>=lines())) return false;
  return (cursor.column()>=0) && (cursor.column()<=lineLength(cursor.line())); // = because new line isn't usually contained in line length
}

bool View::setSelection(const Cursor& position, int length,bool wrap)
{
  KTextEditor::Document *doc=document();
  if (!document()) return false;
  if (length==0) return false;
  if (!doc->cursorInText(position)) return false;
  Cursor end=Cursor(position.line(),position.column());
  if (!wrap) {
    int col=end.column()+length;
    if (col<0) col=0;
    if (col>doc->lineLength(end.line())) col=doc->lineLength(end.line());
    end.setColumn(col);
  } else {
    kdDebug()<<"KTextEditor::View::setSelection(pos,len,true) not implemented yet"<<endl;
  }
  return setSelection(position,end);
}

bool View::insertText (const QString &text )
{
  KTextEditor::Document *doc=document();
  if (!doc) return false;
  return doc->insertText(cursorPosition(),text);
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

Plugin *KTextEditor::createPlugin ( const char* libname, Document *document )
{
  return KParts::ComponentFactory::createInstanceFromLibrary<Plugin>( libname, document, "" );
}

Editor *KTextEditor::editor(const char *libname)
{
  KLibFactory *fact=KLibLoader::self()->factory(libname);

  KTextEditor::Factory *ef=qobject_cast<KTextEditor::Factory*>(fact);

  if (!ef) return 0;

  return ef->editor();
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

// kate: space-indent on; indent-width 2; replace-tabs on;
