/* This file is part of the KDE libraries
   Copyright (C) 2001-2004 Christoph Cullmann <cullmann@kde.org>
   Copyright (C) 2001 Joseph Wenninger <jowenn@kde.org>
   Copyright (C) 1999 Jochen Wilhelmy <digisnap@cs.tu-berlin.de>

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

//BEGIN includes
#include "katedocument.h"
#include "katedocument.moc"

#include "katefactory.h"
#include "katedialogs.h"
#include "katehighlight.h"
#include "kateview.h"
#include "kateviewinternal.h"
#include "katesearch.h"
#include "kateautoindent.h"
#include "katetextline.h"
#include "katedocumenthelpers.h"
#include "katebuffer.h"
#include "katecodefoldinghelpers.h"
#include "kateprinter.h"
#include "katelinerange.h"
#include "katesupercursor.h"
#include "katearbitraryhighlight.h"
#include "katerenderer.h"
#include "kateattribute.h"
#include "kateconfig.h"
#include "katefiletype.h"
#include "kateschema.h"

#include <ktexteditor/plugin.h>

#include <kio/job.h>
#include <kio/netaccess.h>

#include <kparts/event.h>

#include <klocale.h>
#include <kglobal.h>
#include <kapplication.h>
#include <kpopupmenu.h>
#include <kconfig.h>
#include <kfiledialog.h>
#include <kmessagebox.h>
#include <kspell.h>
#include <kstdaction.h>
#include <kiconloader.h>
#include <kxmlguifactory.h>
#include <kdialogbase.h>
#include <kdebug.h>
#include <kglobalsettings.h>
#include <ksavefile.h>
#include <klibloader.h>
#include <kdirwatch.h>
#include <kwin.h>
#include <kencodingfiledialog.h>
#include <ktempfile.h>
#include <kmdcodec.h>

#include <qtimer.h>
#include <qfile.h>
#include <qclipboard.h>
#include <qtextstream.h>
#include <qtextcodec.h>
#include <qmap.h>
//END  includes

//BEGIN PRIVATE CLASSES
class KatePartPluginItem
{
  public:
    KTextEditor::Plugin *plugin;
};
//END PRIVATE CLASSES

// BEGIN d'tor, c'tor
//
// KateDocument Constructor
//
KateDocument::KateDocument ( bool bSingleViewMode, bool bBrowserView,
                             bool bReadOnly, QWidget *parentWidget,
                             const char *widgetName, QObject *parent, const char *name)
: Kate::Document(parent, name),
  m_plugins (KateFactory::self()->plugins().count()),
  selectStart(this, true),
  selectEnd(this, true),
  m_undoDontMerge(false),
  m_undoIgnoreCancel(false),
  lastUndoGroupWhenSaved( 0 ),
  docWasSavedWhenUndoWasEmpty( true ),
  m_modOnHd (false),
  m_modOnHdReason (0),
  m_job (0),
  m_tempFile (0),
  m_imStartLine( 0 ),
  m_imStart( 0 ),
  m_imEnd( 0 ),
  m_imSelStart( 0 ),
  m_imSelEnd( 0 ),
  m_imComposeEvent( false )
{
  // my dcop object
  setObjId ("KateDocument#"+documentDCOPSuffix());

  // ktexteditor interfaces
  setBlockSelectionInterfaceDCOPSuffix (documentDCOPSuffix());
  setConfigInterfaceDCOPSuffix (documentDCOPSuffix());
  setConfigInterfaceExtensionDCOPSuffix (documentDCOPSuffix());
  setCursorInterfaceDCOPSuffix (documentDCOPSuffix());
  setEditInterfaceDCOPSuffix (documentDCOPSuffix());
  setEncodingInterfaceDCOPSuffix (documentDCOPSuffix());
  setHighlightingInterfaceDCOPSuffix (documentDCOPSuffix());
  setMarkInterfaceDCOPSuffix (documentDCOPSuffix());
  setMarkInterfaceExtensionDCOPSuffix (documentDCOPSuffix());
  setPrintInterfaceDCOPSuffix (documentDCOPSuffix());
  setSearchInterfaceDCOPSuffix (documentDCOPSuffix());
  setSelectionInterfaceDCOPSuffix (documentDCOPSuffix());
  setSelectionInterfaceExtDCOPSuffix (documentDCOPSuffix());
  setSessionConfigInterfaceDCOPSuffix (documentDCOPSuffix());
  setUndoInterfaceDCOPSuffix (documentDCOPSuffix());
  setWordWrapInterfaceDCOPSuffix (documentDCOPSuffix());

  // init local plugin array
  m_plugins.fill (0);

  // register doc at factory
  KateFactory::self()->registerDocument (this);

  m_reloading = false;

  m_buffer = new KateBuffer (this);

  // init the config object, be careful not to use it
  // until the initial readConfig() call is done
  m_config = new KateDocumentConfig (this);

  // init some more vars !
  m_activeView = 0L;

  hlSetByUser = false;
  m_fileType = -1;
  m_fileTypeSetByUser = false;
  setInstance( KateFactory::self()->instance() );

  editSessionNumber = 0;
  editIsRunning = false;
  noViewUpdates = false;
  m_editCurrentUndo = 0L;
  editWithUndo = false;
  editTagFrom = false;

  m_docNameNumber = 0;

  m_kspell = 0;

  blockSelect = false;

  m_bSingleViewMode = bSingleViewMode;
  m_bBrowserView = bBrowserView;
  m_bReadOnly = bReadOnly;

  m_marks.setAutoDelete( true );
  m_markPixmaps.setAutoDelete( true );
  m_markDescriptions.setAutoDelete( true );
  setMarksUserChangable( markType01 );

  m_highlight = 0L;

  m_undoMergeTimer = new QTimer(this);
  connect(m_undoMergeTimer, SIGNAL(timeout()), SLOT(undoCancel()));

  clearMarks ();
  clearUndo ();
  clearRedo ();
  setModified (false);
  internalSetHlMode (0);
  docWasSavedWhenUndoWasEmpty = true;

  m_extension = new KateBrowserExtension( this );
  m_arbitraryHL = new KateArbitraryHighlight();
  m_indenter = KateAutoIndent::createIndenter ( this, 0 );

  m_indenter->updateConfig ();

  // some nice signals from the buffer
  connect(m_buffer, SIGNAL(tagLines(int,int)), this, SLOT(tagLines(int,int)));
  connect(m_buffer, SIGNAL(codeFoldingUpdated()),this,SIGNAL(codeFoldingUpdated()));

  // if the user changes the highlight with the dialog, notify the doc
  connect(KateHlManager::self(),SIGNAL(changed()),SLOT(internalHlChanged()));

  // signal for the arbitrary HL
  connect(m_arbitraryHL, SIGNAL(tagLines(KateView*, KateSuperRange*)), SLOT(tagArbitraryLines(KateView*, KateSuperRange*)));

  // signals for mod on hd
  connect( KateFactory::self()->dirWatch(), SIGNAL(dirty (const QString &)),
           this, SLOT(slotModOnHdDirty (const QString &)) );

  connect( KateFactory::self()->dirWatch(), SIGNAL(created (const QString &)),
           this, SLOT(slotModOnHdCreated (const QString &)) );

  connect( KateFactory::self()->dirWatch(), SIGNAL(deleted (const QString &)),
           this, SLOT(slotModOnHdDeleted (const QString &)) );

  // update doc name
  setDocName ("");

  // if single view mode, like in the konqui embedding, create a default view ;)
  if ( m_bSingleViewMode )
  {
    KTextEditor::View *view = createView( parentWidget, widgetName );
    insertChildClient( view );
    view->show();
    setWidget( view );
  }

  connect(this,SIGNAL(sigQueryClose(bool *, bool*)),this,SLOT(slotQueryClose_save(bool *, bool*)));

  // ask what to do with modified files on focus!
  if ( s_fileChangedDialogsActivated )
    for (uint z = 0; z < m_views.count(); z++)
      connect( m_views.at(z), SIGNAL(gotFocus( Kate::View * )), this, SLOT(slotModifiedOnDisk()) );

  m_isasking = 0;
}

//
// KateDocument Destructor
//
KateDocument::~KateDocument()
{
  // remove file from dirwatch
  deactivateDirWatch ();

  if (!singleViewMode())
  {
    // clean up remaining views
    m_views.setAutoDelete( true );
    m_views.clear();
  }

  m_highlight->release();

  delete m_editCurrentUndo;

  delete m_arbitraryHL;

  // cleanup the undo items, very important, truee :/
  undoItems.setAutoDelete(true);
  undoItems.clear();

  // clean up plugins
  unloadAllPlugins ();

  // kspell stuff
  if( m_kspell )
  {
    m_kspell->setAutoDelete(true);
    m_kspell->cleanUp(); // need a way to wait for this to complete
    delete m_kspell;
  }

  delete m_config;
  delete m_indenter;
  KateFactory::self()->deregisterDocument (this);
}
//END

//BEGIN Plugins
void KateDocument::unloadAllPlugins ()
{
  for (uint i=0; i<m_plugins.count(); i++)
    unloadPlugin (i);
}

void KateDocument::enableAllPluginsGUI (KateView *view)
{
  for (uint i=0; i<m_plugins.count(); i++)
    enablePluginGUI (m_plugins[i], view);
}

void KateDocument::disableAllPluginsGUI (KateView *view)
{
  for (uint i=0; i<m_plugins.count(); i++)
    disablePluginGUI (m_plugins[i], view);
}

void KateDocument::loadPlugin (uint pluginIndex)
{
  if (m_plugins[pluginIndex]) return;

  m_plugins[pluginIndex] = KTextEditor::createPlugin (QFile::encodeName((KateFactory::self()->plugins())[pluginIndex]->library()), this);

  enablePluginGUI (m_plugins[pluginIndex]);
}

void KateDocument::unloadPlugin (uint pluginIndex)
{
  if (!m_plugins[pluginIndex]) return;

  disablePluginGUI (m_plugins[pluginIndex]);

  delete m_plugins[pluginIndex];
  m_plugins[pluginIndex] = 0L;
}

void KateDocument::enablePluginGUI (KTextEditor::Plugin *plugin, KateView *view)
{
  if (!plugin) return;
  if (!KTextEditor::pluginViewInterface(plugin)) return;

  KXMLGUIFactory *factory = view->factory();
  if ( factory )
    factory->removeClient( view );

  KTextEditor::pluginViewInterface(plugin)->addView(view);

  if ( factory )
    factory->addClient( view );
}

void KateDocument::enablePluginGUI (KTextEditor::Plugin *plugin)
{
  if (!plugin) return;
  if (!KTextEditor::pluginViewInterface(plugin)) return;

  for (uint i=0; i< m_views.count(); i++)
    enablePluginGUI (plugin, m_views.at(i));
}

void KateDocument::disablePluginGUI (KTextEditor::Plugin *plugin, KateView *view)
{
  if (!plugin) return;
  if (!KTextEditor::pluginViewInterface(plugin)) return;

  KXMLGUIFactory *factory = view->factory();
  if ( factory )
    factory->removeClient( view );

  KTextEditor::pluginViewInterface( plugin )->removeView( view );

  if ( factory )
    factory->addClient( view );
}

void KateDocument::disablePluginGUI (KTextEditor::Plugin *plugin)
{
  if (!plugin) return;
  if (!KTextEditor::pluginViewInterface(plugin)) return;

  for (uint i=0; i< m_views.count(); i++)
    disablePluginGUI (plugin, m_views.at(i));
}
//END

//BEGIN KTextEditor::Document stuff

KTextEditor::View *KateDocument::createView( QWidget *parent, const char *name )
{
  KateView* newView = new KateView( this, parent, name);
  connect(newView, SIGNAL(cursorPositionChanged()), SLOT(undoCancel()));
  if ( s_fileChangedDialogsActivated )
    connect( newView, SIGNAL(gotFocus( Kate::View * )), this, SLOT(slotModifiedOnDisk()) );
  return newView;
}

QPtrList<KTextEditor::View> KateDocument::views () const
{
  return m_textEditViews;
}
//END

//BEGIN KTextEditor::ConfigInterfaceExtension stuff

uint KateDocument::configPages () const
{
  return 11;
}

KTextEditor::ConfigPage *KateDocument::configPage (uint number, QWidget *parent, const char * )
{
  switch( number )
  {
    case 0:
      return colorConfigPage (parent);

    case 1:
      return editConfigPage (parent);

    case 2:
      return keysConfigPage (parent);

    case 3:
      return indentConfigPage(parent);

    case 4:
      return selectConfigPage(parent);

    case 5:
      return saveConfigPage( parent );

    case 6:
      return viewDefaultsConfigPage(parent);

    case 7:
      return hlConfigPage (parent);

    case 9:
      return new KateSpellConfigPage (parent);

    case 10:
      return new KatePartPluginConfigPage (parent);

    case 8:
      return new KateFileTypeConfigTab (parent);

    default:
      return 0;
  }
}

QString KateDocument::configPageName (uint number) const
{
  switch( number )
  {
    case 0:
      return i18n ("Fonts & Colors");

    case 3:
      return i18n ("Indentation");

    case 4:
      return i18n ("Selection");

    case 1:
      return i18n ("Editing");

    case 2:
      return i18n ("Shortcuts");

    case 7:
      return i18n ("Highlighting");

    case 6:
      return i18n ("View Defaults");

    case 10:
      return i18n ("Plugins");

    case 5:
      return i18n("Open/Save");

    case 9:
      return i18n("Spelling");

    case 8:
      return i18n("Filetypes");

    default:
      return 0;
  }
}

QString KateDocument::configPageFullName (uint number) const
{
  switch( number )
  {
    case 0:
      return i18n ("Font & Color Schemas");

    case 3:
      return i18n ("Indentation Rules");

    case 4:
      return i18n ("Selection Behavior");

    case 1:
      return i18n ("Editing Options");

    case 2:
      return i18n ("Shortcuts Configuration");

    case 7:
      return i18n ("Highlighting Rules");

    case 6:
      return i18n("View Defaults");

    case 10:
      return i18n ("Plugin Manager");

    case 5:
      return i18n("File Opening & Saving");

    case 9:
      return i18n("Spell Checker Behavior");

    case 8:
      return i18n("Filetype Specific Settings");

    default:
      return 0;
  }
}

QPixmap KateDocument::configPagePixmap (uint number, int size) const
{
  switch( number )
  {
    case 0:
      return BarIcon("colorize", size);

    case 3:
      return BarIcon("rightjust", size);

    case 4:
      return BarIcon("frame_edit", size);

    case 1:
      return BarIcon("edit", size);

    case 2:
      return BarIcon("key_enter", size);

    case 7:
      return BarIcon("source", size);

    case 6:
      return BarIcon("view_text",size);

    case 10:
      return BarIcon("connect_established", size);

    case 5:
      return BarIcon("filesave", size);

    case 9:
      return BarIcon("spellcheck", size);

    case 8:
      return BarIcon("edit", size);

    default:
      return 0;
  }
}
//END

//BEGIN KTextEditor::EditInterface stuff

QString KateDocument::text() const
{
  QString s;

  for (uint i = 0; i < m_buffer->count(); i++)
  {
    KateTextLine::Ptr textLine = m_buffer->plainLine(i);

    if (textLine)
    {
      s.append (textLine->string());

      if ((i+1) < m_buffer->count())
        s.append('\n');
    }
  }

  return s;
}

QString KateDocument::text ( uint startLine, uint startCol, uint endLine, uint endCol ) const
{
  return text(startLine, startCol, endLine, endCol, false);
}

QString KateDocument::text ( uint startLine, uint startCol, uint endLine, uint endCol, bool blockwise) const
{
  if ( blockwise && (startCol > endCol) )
    return QString ();

  QString s;

  if (startLine == endLine)
  {
    if (startCol > endCol)
      return QString ();

    KateTextLine::Ptr textLine = m_buffer->plainLine(startLine);

    if ( !textLine )
      return QString ();

    return textLine->string(startCol, endCol-startCol);
  }
  else
  {
    for (uint i = startLine; (i <= endLine) && (i < m_buffer->count()); i++)
    {
      KateTextLine::Ptr textLine = m_buffer->plainLine(i);

      if ( !blockwise )
      {
        if (i == startLine)
          s.append (textLine->string(startCol, textLine->length()-startCol));
        else if (i == endLine)
          s.append (textLine->string(0, endCol));
        else
          s.append (textLine->string());
      }
      else
      {
        s.append (textLine->string (startCol, endCol - startCol));
      }

      if ( i < endLine )
        s.append('\n');
    }
  }

  return s;
}

QString KateDocument::textLine( uint line ) const
{
  KateTextLine::Ptr l = m_buffer->plainLine(line);

  if (!l)
    return QString();

  return l->string();
}

bool KateDocument::setText(const QString &s)
{
  if (!isReadWrite())
    return false;

  QPtrList<KTextEditor::Mark> m = marks ();
  QValueList<KTextEditor::Mark> msave;

  for (uint i=0; i < m.count(); i++)
    msave.append (*m.at(i));

  editStart ();

  // delete the text
  clear();

  // insert the new text
  insertText (0, 0, s);

  editEnd ();

  for (uint i=0; i < msave.count(); i++)
    setMark (msave[i].line, msave[i].type);

  return true;
}

bool KateDocument::clear()
{
  if (!isReadWrite())
    return false;

  for (KateView * view = m_views.first(); view != 0L; view = m_views.next() ) {
    view->clear();
    view->tagAll();
    view->update();
  }

  clearMarks ();

  return removeText (0,0,lastLine()+1, 0);
}

bool KateDocument::insertText( uint line, uint col, const QString &s)
{
  return insertText (line, col, s, false);
}

bool KateDocument::insertText( uint line, uint col, const QString &s, bool blockwise )
{
  if (!isReadWrite())
    return false;

  if (s.isEmpty())
    return true;

  if (line == numLines())
    editInsertLine(line,"");
  else if (line > lastLine())
    return false;

  editStart ();

  uint insertPos = col;
  uint len = s.length();

  QString buf;

  bool replacetabs = ( config()->configFlags() & KateDocumentConfig::cfReplaceTabsDyn );
  uint tw = config()->tabWidth();

  for (uint pos = 0; pos < len; pos++)
  {
    QChar ch = s[pos];

    if (ch == '\n')
    {
      if ( !blockwise )
      {
        editInsertText (line, insertPos, buf);
        editWrapLine (line, insertPos + buf.length());
      }
      else
      {
        editInsertText (line, col, buf);

        if ( line == lastLine() )
          editWrapLine (line, col + buf.length());
      }

      line++;
      insertPos = 0;
      buf.truncate(0);
    }
    else
    {
      if ( replacetabs && ch == '\t' )
      {
        uint tr = tw - ( ((blockwise?col:insertPos)+buf.length())%tw ); //###
        for ( uint i=0; i < tr; i++ )
          buf += ' ';
      }
      else
        buf += ch; // append char to buffer
    }
  }

  if ( !blockwise )
    editInsertText (line, insertPos, buf);
  else
    editInsertText (line, col, buf);

  editEnd ();

  return true;
}

bool KateDocument::removeText ( uint startLine, uint startCol, uint endLine, uint endCol )
{
  return removeText (startLine, startCol, endLine, endCol, false);
}

bool KateDocument::removeText ( uint startLine, uint startCol, uint endLine, uint endCol, bool blockwise )
{
  if (!isReadWrite())
    return false;

  if ( blockwise && (startCol > endCol) )
    return false;

  if ( startLine > endLine )
    return false;

  if ( startLine > lastLine() )
    return false;

  editStart ();

  if ( !blockwise )
  {
    if ( endLine > lastLine() )
    {
      endLine = lastLine()+1;
      endCol = 0;
    }

    if (startLine == endLine)
    {
      editRemoveText (startLine, startCol, endCol-startCol);
    }
    else if ((startLine+1) == endLine)
    {
      if ( (m_buffer->plainLine(startLine)->length()-startCol) > 0 )
        editRemoveText (startLine, startCol, m_buffer->plainLine(startLine)->length()-startCol);

      editRemoveText (startLine+1, 0, endCol);
      editUnWrapLine (startLine);
    }
    else
    {
      for (uint line = endLine; line >= startLine; line--)
      {
        if ((line > startLine) && (line < endLine))
        {
          editRemoveLine (line);
        }
        else
        {
          if (line == endLine)
          {
            if ( endLine <= lastLine() )
              editRemoveText (line, 0, endCol);
          }
          else
          {
            if ( (m_buffer->plainLine(line)->length()-startCol) > 0 )
              editRemoveText (line, startCol, m_buffer->plainLine(line)->length()-startCol);

            editUnWrapLine (startLine);
          }
        }

        if ( line == 0 )
          break;
      }
    }
  }
  else
  {
    if ( endLine > lastLine() )
      endLine = lastLine ();

    for (uint line = endLine; line >= startLine; line--)
    {
      editRemoveText (line, startCol, endCol-startCol);

      if ( line == 0 )
        break;
    }
  }

  editEnd ();

  return true;
}

bool KateDocument::insertLine( uint l, const QString &str )
{
  if (!isReadWrite())
    return false;

  if (l > numLines())
    return false;

  return editInsertLine (l, str);
}

bool KateDocument::removeLine( uint line )
{
  if (!isReadWrite())
    return false;

  if (line > lastLine())
    return false;

  return editRemoveLine (line);
}

uint KateDocument::length() const
{
  uint l = 0;

  for (uint i = 0; i < m_buffer->count(); i++)
  {
    KateTextLine::Ptr line = m_buffer->plainLine(i);

    if (line)
      l += line->length();
  }

  return l;
}

uint KateDocument::numLines() const
{
  return m_buffer->count();
}

uint KateDocument::numVisLines() const
{
  return m_buffer->countVisible ();
}

int KateDocument::lineLength ( uint line ) const
{
  KateTextLine::Ptr l = m_buffer->plainLine(line);

  if (!l)
    return -1;

  return l->length();
}
//END

//BEGIN KTextEditor::EditInterface internal stuff
//
// Starts an edit session with (or without) undo, update of view disabled during session
//
void KateDocument::editStart (bool withUndo)
{
  editSessionNumber++;

  if (editSessionNumber > 1)
    return;

  editIsRunning = true;
  noViewUpdates = true;
  editWithUndo = withUndo;

  editTagLineStart = 0xffffffff;
  editTagLineEnd = 0;
  editTagFrom = false;

  if (editWithUndo)
    undoStart();
  else
    undoCancel();

  for (uint z = 0; z < m_views.count(); z++)
  {
    m_views.at(z)->editStart ();
  }

  m_buffer->editStart ();
}

void KateDocument::undoStart()
{
  if (m_editCurrentUndo || m_imComposeEvent) return;

  // Make sure the buffer doesn't get bigger than requested
  if ((config()->undoSteps() > 0) && (undoItems.count() > config()->undoSteps()))
  {
    undoItems.setAutoDelete(true);
    undoItems.removeFirst();
    undoItems.setAutoDelete(false);
    docWasSavedWhenUndoWasEmpty = false;
  }

  // new current undo item
  m_editCurrentUndo = new KateUndoGroup(this);
}

void KateDocument::undoEnd()
{
  if (m_imComposeEvent)
    return;

  if (m_editCurrentUndo)
  {
    if (!m_undoDontMerge && undoItems.last() && undoItems.last()->merge(m_editCurrentUndo))
      delete m_editCurrentUndo;
    else
      undoItems.append(m_editCurrentUndo);

    m_undoDontMerge = false;
    m_undoIgnoreCancel = true;

    m_editCurrentUndo = 0L;

    // (Re)Start the single-shot timer to cancel the undo merge
    // the user has 5 seconds to input more data, or undo merging gets canceled for the current undo item.
    m_undoMergeTimer->start(5000, true);

    emit undoChanged();
  }
}

void KateDocument::undoCancel()
{
  if (m_undoIgnoreCancel) {
    m_undoIgnoreCancel = false;
    return;
  }

  m_undoDontMerge = true;

  Q_ASSERT(!m_editCurrentUndo);

  // As you can see by the above assert, neither of these should really be required
  delete m_editCurrentUndo;
  m_editCurrentUndo = 0L;
}

//
// End edit session and update Views
//
void KateDocument::editEnd ()
{
  if (editSessionNumber == 0)
    return;

  // wrap the new/changed text
  if (editSessionNumber == 1)
    if (editWithUndo && config()->wordWrap())
      wrapText (editTagLineStart, editTagLineEnd);

  editSessionNumber--;

  if (editSessionNumber > 0)
    return;

  // end buffer edit, will trigger hl update
  m_buffer->editEnd ();

  if (editWithUndo)
    undoEnd();

  for (uint z = 0; z < m_views.count(); z++)
  {
    m_views.at(z)->editEnd (editTagLineStart, editTagLineEnd, editTagFrom);
  }

  setModified(true);
  emit textChanged ();

  noViewUpdates = false;
  editIsRunning = false;
}

bool KateDocument::wrapText (uint startLine, uint endLine)
{
  uint col = config()->wordWrapAt();

  if (col == 0)
    return false;

  editStart ();

  for (uint line = startLine; (line <= endLine) && (line < numLines()); line++)
  {
    KateTextLine::Ptr l = m_buffer->line(line);

    if (!l)
      return false;

    if (l->length() > col)
    {
      KateTextLine::Ptr nextl = m_buffer->line(line+1);

      const QChar *text = l->text();
      uint eolPosition = l->length()-1;
      uint searchStart = col;

      //If where we are wrapping is an end of line and is a space we don't
      //want to wrap there
      if (col == eolPosition && text[col].isSpace())
        searchStart--;

      // Scan backwards looking for a place to break the line
      // We are not interested in breaking at the first char
      // of the line (if it is a space), but we are at the second
      // anders: if we can't find a space, try breaking on a word
      // boundry, using KateHighlight::canBreakAt().
      // This could be a priority (setting) in the hl/filetype/document
      int z = 0;
      uint nw = 0; // alternative position, a non word character
      for (z=searchStart; z > 0; z--)
      {
        if (text[z].isSpace()) break;
        if ( ! nw && m_highlight->canBreakAt( text[z] , l->attribute(z) ) )
        nw = z;
      }

      if (z > 0)
      {
        // cu space
        editRemoveText (line, z, 1);
      }
      else
      {
        // There was no space to break at so break at a nonword character if
        // found, or at the wrapcolumn ( that needs be configurable )
        // Don't try and add any white space for the break
        if ( nw && nw < col ) nw++; // break on the right side of the character
        z = nw ? nw : col;
      }

      if (nextl && !nextl->isAutoWrapped())
      {
        editWrapLine (line, z, true);
        editMarkLineAutoWrapped (line+1, true);

        endLine++;
      }
      else
      {
        if (nextl && (nextl->length() > 0) && !nextl->getChar(0).isSpace() && ((l->length() < 1) || !l->getChar(l->length()-1).isSpace()))
          editInsertText (line+1, 0, QString (" "));

        bool newLineAdded = false;
        editWrapLine (line, z, false, &newLineAdded);

        editMarkLineAutoWrapped (line+1, true);

        if (newLineAdded)
          endLine++;
      }
    }
  }

  editEnd ();

  return true;
}

void KateDocument::editAddUndo (KateUndoGroup::UndoType type, uint line, uint col, uint len, const QString &text)
{
  if (editIsRunning && editWithUndo && m_editCurrentUndo) {
    m_editCurrentUndo->addItem(type, line, col, len, text);

    // Clear redo buffer
    if (redoItems.count()) {
      redoItems.setAutoDelete(true);
      redoItems.clear();
      redoItems.setAutoDelete(false);
    }
  }
}

void KateDocument::editTagLine (uint line)
{
  if (line < editTagLineStart)
    editTagLineStart = line;

  if (line > editTagLineEnd)
    editTagLineEnd = line;
}

void KateDocument::editInsertTagLine (uint line)
{
  if (line < editTagLineStart)
    editTagLineStart = line;

  if (line <= editTagLineEnd)
    editTagLineEnd++;

  if (line > editTagLineEnd)
    editTagLineEnd = line;

  editTagFrom = true;
}

void KateDocument::editRemoveTagLine (uint line)
{
  if (line < editTagLineStart)
    editTagLineStart = line;

  if (line < editTagLineEnd)
    editTagLineEnd--;

  if (line > editTagLineEnd)
    editTagLineEnd = line;

  editTagFrom = true;
}

bool KateDocument::editInsertText ( uint line, uint col, const QString &str )
{
  if (!isReadWrite())
    return false;

  QString s = str;

  KateTextLine::Ptr l = m_buffer->line(line);

  if (!l)
    return false;

    if ( config()->configFlags() & KateDocumentConfig::cfReplaceTabsDyn )
    {
      uint tw = config()->tabWidth();
      int pos = 0;
      uint l = 0;
      while ( (pos = s.find('\t')) > -1 )
      {
        l = tw - ( (col + pos)%tw );
        s.replace( pos, 1, QString().fill( ' ', l ) );
      }
    }

  editStart ();

  editAddUndo (KateUndoGroup::editInsertText, line, col, s.length(), s);

  l->insertText (col, s.length(), s.unicode());
//   removeTrailingSpace(line); // ### nessecary?

  m_buffer->changeLine(line);
  editTagLine (line);

  for( QPtrListIterator<KateSuperCursor> it (m_superCursors); it.current(); ++it )
    it.current()->editTextInserted (line, col, s.length());

  editEnd ();

  return true;
}

bool KateDocument::editRemoveText ( uint line, uint col, uint len )
{
  if (!isReadWrite())
    return false;

  KateTextLine::Ptr l = m_buffer->line(line);

  if (!l)
    return false;

  editStart ();

  editAddUndo (KateUndoGroup::editRemoveText, line, col, len, l->string().mid(col, len));

  l->removeText (col, len);
  removeTrailingSpace( line ); // ### nessecary?

  m_buffer->changeLine(line);

  editTagLine(line);

  for( QPtrListIterator<KateSuperCursor> it (m_superCursors); it.current(); ++it )
    it.current()->editTextRemoved (line, col, len);

  editEnd ();

  return true;
}

bool KateDocument::editMarkLineAutoWrapped ( uint line, bool autowrapped )
{
  if (!isReadWrite())
    return false;

  KateTextLine::Ptr l = m_buffer->line(line);

  if (!l)
    return false;

  editStart ();

  editAddUndo (KateUndoGroup::editMarkLineAutoWrapped, line, autowrapped ? 1 : 0, 0, QString::null);

  l->setAutoWrapped (autowrapped);

  m_buffer->changeLine(line);

  editEnd ();

  return true;
}

bool KateDocument::editWrapLine ( uint line, uint col, bool newLine, bool *newLineAdded)
{
  if (!isReadWrite())
    return false;

  KateTextLine::Ptr l = m_buffer->line(line);

  if (!l)
    return false;

  editStart ();

  KateTextLine::Ptr nl = m_buffer->line(line+1);

  int pos = l->length() - col;

  if (pos < 0)
    pos = 0;

  editAddUndo (KateUndoGroup::editWrapLine, line, col, pos, (!nl || newLine) ? "1" : "0");

  if (!nl || newLine)
  {
    KateTextLine::Ptr tl = new KateTextLine();

    tl->insertText (0, pos, l->text()+col, l->attributes()+col);
    l->truncate(col);

    m_buffer->insertLine (line+1, tl);
    m_buffer->changeLine(line);

    QPtrList<KTextEditor::Mark> list;
    for( QIntDictIterator<KTextEditor::Mark> it( m_marks ); it.current(); ++it )
    {
      if( it.current()->line >= line )
      {
        if ((col == 0) || (it.current()->line > line))
          list.append( it.current() );
      }
    }

    for( QPtrListIterator<KTextEditor::Mark> it( list ); it.current(); ++it )
    {
      KTextEditor::Mark* mark = m_marks.take( it.current()->line );
      mark->line++;
      m_marks.insert( mark->line, mark );
    }

    if( !list.isEmpty() )
      emit marksChanged();

    editInsertTagLine (line);

    // yes, we added a new line !
    if (newLineAdded)
      (*newLineAdded) = true;
  }
  else
  {
    nl->insertText (0, pos, l->text()+col, l->attributes()+col);
    l->truncate(col);

    m_buffer->changeLine(line);
    m_buffer->changeLine(line+1);

    // no, no new line added !
    if (newLineAdded)
      (*newLineAdded) = false;
  }

  editTagLine(line);
  editTagLine(line+1);

  for( QPtrListIterator<KateSuperCursor> it (m_superCursors); it.current(); ++it )
    it.current()->editLineWrapped (line, col, !nl || newLine);

  editEnd ();

  return true;
}

bool KateDocument::editUnWrapLine ( uint line, bool removeLine, uint length )
{
  if (!isReadWrite())
    return false;

  KateTextLine::Ptr l = m_buffer->line(line);
  KateTextLine::Ptr tl = m_buffer->line(line+1);

  if (!l || !tl)
    return false;

  editStart ();

  uint col = l->length ();

  editAddUndo (KateUndoGroup::editUnWrapLine, line, col, length, removeLine ? "1" : "0");

  if (removeLine)
  {
    l->insertText (col, tl->length(), tl->text(), tl->attributes());

    m_buffer->changeLine(line);
    m_buffer->removeLine(line+1);
  }
  else
  {
    l->insertText (col, (tl->length() < length) ? tl->length() : length, tl->text(), tl->attributes());
    tl->removeText (0, (tl->length() < length) ? tl->length() : length);

    m_buffer->changeLine(line);
    m_buffer->changeLine(line+1);
  }

  QPtrList<KTextEditor::Mark> list;
  for( QIntDictIterator<KTextEditor::Mark> it( m_marks ); it.current(); ++it )
  {
    if( it.current()->line >= line+1 )
      list.append( it.current() );

    if ( it.current()->line == line+1 )
    {
      KTextEditor::Mark* mark = m_marks.take( line );

      if (mark)
      {
        it.current()->type |= mark->type;
      }
    }
  }

  for( QPtrListIterator<KTextEditor::Mark> it( list ); it.current(); ++it )
  {
    KTextEditor::Mark* mark = m_marks.take( it.current()->line );
    mark->line--;
    m_marks.insert( mark->line, mark );
  }

  if( !list.isEmpty() )
    emit marksChanged();

  if (removeLine)
    editRemoveTagLine(line);

  editTagLine(line);
  editTagLine(line+1);

  for( QPtrListIterator<KateSuperCursor> it (m_superCursors); it.current(); ++it )
    it.current()->editLineUnWrapped (line, col, removeLine, length);

  editEnd ();

  return true;
}

bool KateDocument::editInsertLine ( uint line, const QString &s )
{
  if (!isReadWrite())
    return false;

  if ( line > numLines() )
    return false;

  editStart ();

  editAddUndo (KateUndoGroup::editInsertLine, line, 0, s.length(), s);

  removeTrailingSpace( line ); // old line

  KateTextLine::Ptr tl = new KateTextLine();
  tl->insertText (0, s.length(), s.unicode(), 0);
  m_buffer->insertLine(line, tl);
  m_buffer->changeLine(line);

  editInsertTagLine (line);
  editTagLine(line);

  removeTrailingSpace( line ); // new line

  QPtrList<KTextEditor::Mark> list;
  for( QIntDictIterator<KTextEditor::Mark> it( m_marks ); it.current(); ++it )
  {
    if( it.current()->line >= line )
      list.append( it.current() );
  }

  for( QPtrListIterator<KTextEditor::Mark> it( list ); it.current(); ++it )
  {
    KTextEditor::Mark* mark = m_marks.take( it.current()->line );
    mark->line++;
    m_marks.insert( mark->line, mark );
  }

  if( !list.isEmpty() )
    emit marksChanged();

  for( QPtrListIterator<KateSuperCursor> it (m_superCursors); it.current(); ++it )
    it.current()->editLineInserted (line);

  editEnd ();

  return true;
}

bool KateDocument::editRemoveLine ( uint line )
{
  if (!isReadWrite())
    return false;

  if ( line > lastLine() )
    return false;

  if ( numLines() == 1 )
    return editRemoveText (0, 0, m_buffer->line(0)->length());

  editStart ();

  editAddUndo (KateUndoGroup::editRemoveLine, line, 0, lineLength(line), textLine(line));

  m_buffer->removeLine(line);

  editRemoveTagLine (line);

  QPtrList<KTextEditor::Mark> list;
  KTextEditor::Mark* rmark = 0;
  for( QIntDictIterator<KTextEditor::Mark> it( m_marks ); it.current(); ++it )
  {
    if ( (it.current()->line > line) )
      list.append( it.current() );
    else if ( (it.current()->line == line) )
      rmark = it.current();
  }

  if (rmark)
    delete (m_marks.take (rmark->line));

  for( QPtrListIterator<KTextEditor::Mark> it( list ); it.current(); ++it )
  {
    KTextEditor::Mark* mark = m_marks.take( it.current()->line );
    mark->line--;
    m_marks.insert( mark->line, mark );
  }

  if( !list.isEmpty() )
    emit marksChanged();

  for( QPtrListIterator<KateSuperCursor> it (m_superCursors); it.current(); ++it )
    it.current()->editLineRemoved (line);

  editEnd();

  return true;
}
//END

//BEGIN KTextEditor::SelectionInterface stuff

bool KateDocument::setSelection( const KateTextCursor& start, const KateTextCursor& end )
{
  KateTextCursor oldSelectStart = selectStart;
  KateTextCursor oldSelectEnd = selectEnd;

  if (start <= end) {
    selectStart.setPos(start);
    selectEnd.setPos(end);
  } else {
    selectStart.setPos(end);
    selectEnd.setPos(start);
  }

  tagSelection(oldSelectStart, oldSelectEnd);

  repaintViews();

  emit selectionChanged ();

  return true;
}

bool KateDocument::setSelection( uint startLine, uint startCol, uint endLine, uint endCol )
{
  if (hasSelection())
    clearSelection(false, false);

  return setSelection( KateTextCursor(startLine, startCol), KateTextCursor(endLine, endCol) );
}

bool KateDocument::clearSelection()
{
  return clearSelection(true);
}

bool KateDocument::clearSelection(bool redraw, bool finishedChangingSelection)
{
  if( !hasSelection() )
    return false;

  KateTextCursor oldSelectStart = selectStart;
  KateTextCursor oldSelectEnd = selectEnd;

  selectStart.setPos(-1, -1);
  selectEnd.setPos(-1, -1);

  tagSelection(oldSelectStart, oldSelectEnd);

  oldSelectStart = selectStart;
  oldSelectEnd = selectEnd;

  if (redraw)
    repaintViews();

  if (finishedChangingSelection)
    emit selectionChanged();

  return true;
}

bool KateDocument::hasSelection() const
{
  return selectStart != selectEnd;
}

QString KateDocument::selection() const
{
  int sc = selectStart.col();
  int ec = selectEnd.col();

  if ( blockSelect )
  {
    if (sc > ec)
    {
      uint tmp = sc;
      sc = ec;
      ec = tmp;
    }
  }

  return text (selectStart.line(), sc, selectEnd.line(), ec, blockSelect);
}

bool KateDocument::removeSelectedText ()
{
  if (!hasSelection())
    return false;

  editStart ();

  int sc = selectStart.col();
  int ec = selectEnd.col();

  if ( blockSelect )
  {
    if (sc > ec)
    {
      uint tmp = sc;
      sc = ec;
      ec = tmp;
    }
  }

  removeText (selectStart.line(), sc, selectEnd.line(), ec, blockSelect);

  // don't redraw the cleared selection - that's done in editEnd().
  clearSelection(false);

  editEnd ();

  return true;
}

bool KateDocument::selectAll()
{
  setBlockSelectionMode (false);

  return setSelection (0, 0, lastLine(), lineLength(lastLine()));
}
//END

//BEGIN KTextEditor::BlockSelectionInterface stuff

bool KateDocument::blockSelectionMode ()
{
  return blockSelect;
}

bool KateDocument::setBlockSelectionMode (bool on)
{
  if (on != blockSelect)
  {
    blockSelect = on;

    KateTextCursor oldSelectStart = selectStart;
    KateTextCursor oldSelectEnd = selectEnd;

    clearSelection(false, false);

    setSelection(oldSelectStart, oldSelectEnd);

    for (KateView * view = m_views.first(); view; view = m_views.next())
    {
      view->slotSelectionTypeChanged();
    }
  }

  return true;
}

bool KateDocument::toggleBlockSelectionMode ()
{
  return setBlockSelectionMode (!blockSelect);
}
//END

//BEGIN KTextEditor::UndoInterface stuff

uint KateDocument::undoCount () const
{
  return undoItems.count ();
}

uint KateDocument::redoCount () const
{
  return redoItems.count ();
}

uint KateDocument::undoSteps () const
{
  return m_config->undoSteps();
}

void KateDocument::setUndoSteps(uint steps)
{
  m_config->setUndoSteps (steps);
}

void KateDocument::undo()
{
  if ((undoItems.count() > 0) && undoItems.last())
  {
    clearSelection ();

    undoItems.last()->undo();
    redoItems.append (undoItems.last());
    undoItems.removeLast ();
    updateModified();

    emit undoChanged ();
  }
}

void KateDocument::redo()
{
  if ((redoItems.count() > 0) && redoItems.last())
  {
    clearSelection ();

    redoItems.last()->redo();
    undoItems.append (redoItems.last());
    redoItems.removeLast ();
    updateModified();

    emit undoChanged ();
  }
}

void KateDocument::updateModified()
{
  if ( ( lastUndoGroupWhenSaved &&
         !undoItems.isEmpty() &&
         undoItems.last() == lastUndoGroupWhenSaved )
       || ( undoItems.isEmpty() && docWasSavedWhenUndoWasEmpty ) )
  {
    setModified( false );
    kdDebug(13020) << k_funcinfo << "setting modified to false!" << endl;
  };
}

void KateDocument::clearUndo()
{
  undoItems.setAutoDelete (true);
  undoItems.clear ();
  undoItems.setAutoDelete (false);

  lastUndoGroupWhenSaved = 0;
  docWasSavedWhenUndoWasEmpty = false;

  emit undoChanged ();
}

void KateDocument::clearRedo()
{
  redoItems.setAutoDelete (true);
  redoItems.clear ();
  redoItems.setAutoDelete (false);

  emit undoChanged ();
}

QPtrList<KTextEditor::Cursor> KateDocument::cursors () const
{
  return myCursors;
}
//END

//BEGIN KTextEditor::SearchInterface stuff

bool KateDocument::searchText (unsigned int startLine, unsigned int startCol, const QString &text, unsigned int *foundAtLine, unsigned int *foundAtCol, unsigned int *matchLen, bool casesensitive, bool backwards)
{
  if (text.isEmpty())
    return false;

  int line = startLine;
  int col = startCol;

  if (!backwards)
  {
    int searchEnd = lastLine();

    while (line <= searchEnd)
    {
      KateTextLine::Ptr textLine = m_buffer->plainLine(line);

      if (!textLine)
        return false;

      uint foundAt, myMatchLen;
      bool found = textLine->searchText (col, text, &foundAt, &myMatchLen, casesensitive, false);

      if (found)
      {
        (*foundAtLine) = line;
        (*foundAtCol) = foundAt;
        (*matchLen) = myMatchLen;
        return true;
      }

      col = 0;
      line++;
    }
  }
  else
  {
    // backward search
    int searchEnd = 0;

    while (line >= searchEnd)
    {
      KateTextLine::Ptr textLine = m_buffer->plainLine(line);

      if (!textLine)
        return false;

      uint foundAt, myMatchLen;
      bool found = textLine->searchText (col, text, &foundAt, &myMatchLen, casesensitive, true);

      if (found)
      {
        if ((uint) line == startLine && foundAt + myMatchLen >= (uint) col
            && line == selectStart.line() && foundAt == (uint) selectStart.col()
            && line == selectEnd.line() && foundAt + myMatchLen == (uint) selectEnd.col())
        {
          // To avoid getting stuck at one match we skip a match if it is already
          // selected (most likely because it has just been found).
          if (foundAt > 0)
            col = foundAt - 1;
          else {
            if (--line >= 0)
              col = lineLength(line);
          }
          continue;
        }

        (*foundAtLine) = line;
        (*foundAtCol) = foundAt;
        (*matchLen) = myMatchLen;
        return true;
      }

      if (line >= 1)
        col = lineLength(line-1);

      line--;
    }
  }

  return false;
}

bool KateDocument::searchText (unsigned int startLine, unsigned int startCol, const QRegExp &regexp, unsigned int *foundAtLine, unsigned int *foundAtCol, unsigned int *matchLen, bool backwards)
{
  if (regexp.isEmpty() || !regexp.isValid())
    return false;

  int line = startLine;
  int col = startCol;

  if (!backwards)
  {
    int searchEnd = lastLine();

    while (line <= searchEnd)
    {
      KateTextLine::Ptr textLine = m_buffer->plainLine(line);

      if (!textLine)
        return false;

      uint foundAt, myMatchLen;
      bool found = textLine->searchText (col, regexp, &foundAt, &myMatchLen, false);

      if (found)
      {
        // A special case which can only occur when searching with a regular expression consisting
        // only of a lookahead (e.g. ^(?=\{) for a function beginning without selecting '{').
        if (myMatchLen == 0 && (uint) line == startLine && foundAt == (uint) col)
        {
          if (col < lineLength(line))
            col++;
          else {
            line++;
            col = 0;
          }
          continue;
        }

        (*foundAtLine) = line;
        (*foundAtCol) = foundAt;
        (*matchLen) = myMatchLen;
        return true;
      }

      col = 0;
      line++;
    }
  }
  else
  {
    // backward search
    int searchEnd = 0;

    while (line >= searchEnd)
    {
      KateTextLine::Ptr textLine = m_buffer->plainLine(line);

      if (!textLine)
        return false;

      uint foundAt, myMatchLen;
      bool found = textLine->searchText (col, regexp, &foundAt, &myMatchLen, true);

      if (found)
      {
        if ((uint) line == startLine && foundAt + myMatchLen >= (uint) col
            && line == selectStart.line() && foundAt == (uint) selectStart.col()
            && line == selectEnd.line() && foundAt + myMatchLen == (uint) selectEnd.col())
        {
          // To avoid getting stuck at one match we skip a match if it is already
          // selected (most likely because it has just been found).
          if (foundAt > 0)
            col = foundAt - 1;
          else {
            if (--line >= 0)
              col = lineLength(line);
          }
          continue;
        }

        (*foundAtLine) = line;
        (*foundAtCol) = foundAt;
        (*matchLen) = myMatchLen;
        return true;
      }

      if (line >= 1)
        col = lineLength(line-1);

      line--;
    }
  }

  return false;
}
//END

//BEGIN KTextEditor::HighlightingInterface stuff

uint KateDocument::hlMode ()
{
  return KateHlManager::self()->findHl(m_highlight);
}

bool KateDocument::setHlMode (uint mode)
{
  if (internalSetHlMode (mode))
  {
    setDontChangeHlOnSave();
    return true;
  }

  return false;
}

bool KateDocument::internalSetHlMode (uint mode)
{
   KateHighlighting *h = KateHlManager::self()->getHl(mode);

   // aha, hl will change
   if (h != m_highlight)
   {
     if (m_highlight != 0L)
       m_highlight->release();

      h->use();

      m_highlight = h;

     // invalidate hl
      m_buffer->setHighlight(m_highlight);

     // invalidate the hl again (but that is neary a noop) + update all views
      makeAttribs();

     emit hlChanged();
    }

    return true;
}

uint KateDocument::hlModeCount ()
{
  return KateHlManager::self()->highlights();
}

QString KateDocument::hlModeName (uint mode)
{
  return KateHlManager::self()->hlName (mode);
}

QString KateDocument::hlModeSectionName (uint mode)
{
  return KateHlManager::self()->hlSection (mode);
}

void KateDocument::setDontChangeHlOnSave()
{
  hlSetByUser = true;
}
//END

//BEGIN KTextEditor::ConfigInterface stuff
void KateDocument::readConfig(KConfig *config)
{
  config->setGroup("Kate Document Defaults");

  // read max loadable blocks, more blocks will be swapped out
  KateBuffer::setMaxLoadedBlocks (config->readNumEntry("Maximal Loaded Blocks", KateBuffer::maxLoadedBlocks()));

  KateDocumentConfig::global()->readConfig (config);

  config->setGroup("Kate View Defaults");
  KateViewConfig::global()->readConfig (config);

  config->setGroup("Kate Renderer Defaults");
  KateRendererConfig::global()->readConfig (config);
}

void KateDocument::writeConfig(KConfig *config)
{
  config->setGroup("Kate Document Defaults");

  // write max loadable blocks, more blocks will be swapped out
  config->writeEntry("Maximal Loaded Blocks", KateBuffer::maxLoadedBlocks());

  KateDocumentConfig::global()->writeConfig (config);

  config->setGroup("Kate View Defaults");
  KateViewConfig::global()->writeConfig (config);

  config->setGroup("Kate Renderer Defaults");
  KateRendererConfig::global()->writeConfig (config);
}

void KateDocument::readConfig()
{
  KConfig *config = kapp->config();
  readConfig (config);
}

void KateDocument::writeConfig()
{
  KConfig *config = kapp->config();
  writeConfig (config);
  config->sync();
}

void KateDocument::readSessionConfig(KConfig *config)
{
  // restore the url
  KURL url (config->readEntry("URL"));

  // get the encoding
  QString tmpenc=config->readEntry("Encoding");
  if (!tmpenc.isEmpty() && (tmpenc != encoding()))
    setEncoding(tmpenc);

  // open the file if url valid
  if (!url.isEmpty() && url.isValid())
    openURL (url);

  // restore the hl stuff
  internalSetHlMode(KateHlManager::self()->nameFind(config->readEntry("Highlighting")));

  if (hlMode() > 0)
    hlSetByUser = true;

  // Restore Bookmarks
  QValueList<int> marks = config->readIntListEntry("Bookmarks");
  for( uint i = 0; i < marks.count(); i++ )
    addMark( marks[i], KateDocument::markType01 );
}

void KateDocument::writeSessionConfig(KConfig *config)
{
  // save url
  config->writeEntry("URL", m_url.prettyURL() );

  // save encoding
  config->writeEntry("Encoding",encoding());

  // save hl
  config->writeEntry("Highlighting", m_highlight->name());

  // Save Bookmarks
  QValueList<int> marks;
  for( QIntDictIterator<KTextEditor::Mark> it( m_marks );
       it.current() && it.current()->type & KTextEditor::MarkInterface::markType01;
       ++it )
     marks << it.current()->line;

  config->writeEntry( "Bookmarks", marks );
}

void KateDocument::configDialog()
{
  KDialogBase *kd = new KDialogBase ( KDialogBase::IconList,
                                      i18n("Configure"),
                                      KDialogBase::Ok | KDialogBase::Cancel | KDialogBase::Help,
                                      KDialogBase::Ok,
                                      kapp->mainWidget() );

  KWin::setIcons( kd->winId(), kapp->icon(), kapp->miniIcon() );

  QPtrList<KTextEditor::ConfigPage> editorPages;

  for (uint i = 0; i < KTextEditor::configInterfaceExtension (this)->configPages (); i++)
  {
    QStringList path;
    path.clear();
    path << KTextEditor::configInterfaceExtension (this)->configPageName (i);
    QVBox *page = kd->addVBoxPage(path, KTextEditor::configInterfaceExtension (this)->configPageFullName (i),
                              KTextEditor::configInterfaceExtension (this)->configPagePixmap(i, KIcon::SizeMedium) );

    editorPages.append (KTextEditor::configInterfaceExtension (this)->configPage(i, page));
  }

  if (kd->exec())
  {
    KateDocumentConfig::global()->configStart ();
    KateViewConfig::global()->configStart ();
    KateRendererConfig::global()->configStart ();

    for (uint i=0; i<editorPages.count(); i++)
    {
      editorPages.at(i)->apply();
    }

    KateDocumentConfig::global()->configEnd ();
    KateViewConfig::global()->configEnd ();
    KateRendererConfig::global()->configEnd ();

    writeConfig ();
  }

  delete kd;
}

uint KateDocument::mark( uint line )
{
  if( !m_marks[line] )
    return 0;
  return m_marks[line]->type;
}

void KateDocument::setMark( uint line, uint markType )
{
  clearMark( line );
  addMark( line, markType );
}

void KateDocument::clearMark( uint line )
{
  if( line > lastLine() )
    return;

  if( !m_marks[line] )
    return;

  KTextEditor::Mark* mark = m_marks.take( line );
  emit markChanged( *mark, MarkRemoved );
  emit marksChanged();
  delete mark;
  tagLines( line, line );
  repaintViews(true);
}

void KateDocument::addMark( uint line, uint markType )
{
  if( line > lastLine())
    return;

  if( markType == 0 )
    return;

  if( m_marks[line] ) {
    KTextEditor::Mark* mark = m_marks[line];

    // Remove bits already set
    markType &= ~mark->type;

    if( markType == 0 )
      return;

    // Add bits
    mark->type |= markType;
  } else {
    KTextEditor::Mark *mark = new KTextEditor::Mark;
    mark->line = line;
    mark->type = markType;
    m_marks.insert( line, mark );
  }

  // Emit with a mark having only the types added.
  KTextEditor::Mark temp;
  temp.line = line;
  temp.type = markType;
  emit markChanged( temp, MarkAdded );

  emit marksChanged();
  tagLines( line, line );
  repaintViews(true);
}

void KateDocument::removeMark( uint line, uint markType )
{
  if( line > lastLine() )
    return;
  if( !m_marks[line] )
    return;

  KTextEditor::Mark* mark = m_marks[line];

  // Remove bits not set
  markType &= mark->type;

  if( markType == 0 )
    return;

  // Subtract bits
  mark->type &= ~markType;

  // Emit with a mark having only the types removed.
  KTextEditor::Mark temp;
  temp.line = line;
  temp.type = markType;
  emit markChanged( temp, MarkRemoved );

  if( mark->type == 0 )
    m_marks.remove( line );

  emit marksChanged();
  tagLines( line, line );
  repaintViews(true);
}

QPtrList<KTextEditor::Mark> KateDocument::marks()
{
  QPtrList<KTextEditor::Mark> list;

  for( QIntDictIterator<KTextEditor::Mark> it( m_marks );
       it.current(); ++it ) {
    list.append( it.current() );
  }

  return list;
}

void KateDocument::clearMarks()
{
  for( QIntDictIterator<KTextEditor::Mark> it( m_marks );
       it.current(); ++it ) {
    KTextEditor::Mark* mark = it.current();
    emit markChanged( *mark, MarkRemoved );
    tagLines( mark->line, mark->line );
  }

  m_marks.clear();

  emit marksChanged();
  repaintViews(true);
}

void KateDocument::setPixmap( MarkInterface::MarkTypes type, const QPixmap& pixmap )
{
  m_markPixmaps.replace( type, new QPixmap( pixmap ) );
}

void KateDocument::setDescription( MarkInterface::MarkTypes type, const QString& description )
{
  m_markDescriptions.replace( type, new QString( description ) );
}

QPixmap *KateDocument::markPixmap( MarkInterface::MarkTypes type )
{
  return m_markPixmaps[type];
}

QColor KateDocument::markColor( MarkInterface::MarkTypes type )
{
  uint reserved = 0x1 << KTextEditor::MarkInterface::reservedMarkersCount() - 1;
  if ((uint)type >= (uint)markType01 && (uint)type <= reserved) {
    return KateRendererConfig::global()->lineMarkerColor(type);
  } else {
    return QColor();
  }
}

QString KateDocument::markDescription( MarkInterface::MarkTypes type )
{
  if( m_markDescriptions[type] )
    return *m_markDescriptions[type];
  return QString::null;
}

void KateDocument::setMarksUserChangable( uint markMask )
{
  m_editableMarks = markMask;
}

uint KateDocument::editableMarks()
{
  return m_editableMarks;
}
//END

//BEGIN KTextEditor::PrintInterface stuff
bool KateDocument::printDialog ()
{
  return KatePrinter::print (this);
}

bool KateDocument::print ()
{
  return KatePrinter::print (this);
}
//END

//BEGIN KTextEditor::DocumentInfoInterface (### unfinished)
QString KateDocument::mimeType()
{
  KMimeType::Ptr result = KMimeType::defaultMimeTypePtr();

  // if the document has a URL, try KMimeType::findByURL
  if ( ! m_url.isEmpty() )
    result = KMimeType::findByURL( m_url );

  else if ( m_url.isEmpty() || ! m_url.isLocalFile() )
    result = mimeTypeForContent();

  return result->name();
}

// TODO implement this -- how to calculate?
long KateDocument::fileSize()
{
  return 0;
}

// TODO implement this
QString KateDocument::niceFileSize()
{
  return "UNKNOWN";
}

KMimeType::Ptr KateDocument::mimeTypeForContent()
{
  QByteArray buf (1024);
  uint bufpos = 0;

  for (uint i=0; i < numLines(); i++)
  {
    QString line = textLine( i );
    uint len = line.length() + 1;

    if (bufpos + len > 1024)
      len = 1024 - bufpos;

    memcpy(&buf[bufpos], (line + "\n").latin1(), len);

    bufpos += len;

    if (bufpos >= 1024)
      break;
  }
  buf.resize( bufpos );

  int accuracy = 0;
  return KMimeType::findByContent( buf, &accuracy );
}
//END KTextEditor::DocumentInfoInterface


//BEGIN KParts::ReadWrite stuff

bool KateDocument::openURL( const KURL &url )
{
  // no valid URL
  if ( !url.isValid() )
    return false;

  // could not close old one
  if ( !closeURL() )
    return false;

  // set my url
  m_url = url;

  if ( m_url.isLocalFile() )
  {
    // local mode, just like in kpart

    m_file = m_url.path();

    emit started( 0 );

    if (openFile())
    {
      emit completed();
      emit setWindowCaption( m_url.prettyURL() );

      return true;
    }

    return false;
  }
  else
  {
    // remote mode

    m_bTemp = true;

    m_tempFile = new KTempFile ();
    m_file = m_tempFile->name();

    m_job = KIO::get ( url, false, isProgressInfoEnabled() );

    // connect to slots
    connect( m_job, SIGNAL( data( KIO::Job*, const QByteArray& ) ),
           SLOT( slotDataKate( KIO::Job*, const QByteArray& ) ) );

    connect( m_job, SIGNAL( result( KIO::Job* ) ),
           SLOT( slotFinishedKate( KIO::Job* ) ) );

    // set text mode
    m_job->addMetaData ("textmode", "true");

    QWidget *w = widget ();
    if (!w && !m_views.isEmpty ())
      w = m_views.first();

    if (w)
      m_job->setWindow (w->topLevelWidget());

    emit started( m_job );

    return true;
  }
}

void KateDocument::slotDataKate ( KIO::Job *, const QByteArray &data )
{
  kdDebug(13020) << "KateDocument::slotData" << endl;

  if (!m_tempFile || !m_tempFile->file())
    return;

  m_tempFile->file()->writeBlock (data);
}

void KateDocument::slotFinishedKate ( KIO::Job * job )
{
  kdDebug(13020) << "KateDocument::slotJobFinished" << endl;

  if (!m_tempFile)
    return;

  delete m_tempFile;
  m_tempFile = 0;
  m_job = 0;

  if (job->error())
    emit canceled( job->errorString() );
  else
  {
    if ( openFile(job) )
      emit setWindowCaption( m_url.prettyURL() );

    emit completed();
  }
}

void KateDocument::abortLoadKate()
{
  if ( m_job )
  {
    kdDebug(13020) << "Aborting job " << m_job << endl;
    m_job->kill();
    m_job = 0;
  }

  delete m_tempFile;
  m_tempFile = 0;
}

bool KateDocument::openFile()
{
  return openFile (0);
}

bool KateDocument::openFile(KIO::Job * job)
{
  // add new m_file to dirwatch
  activateDirWatch ();

  //
  // use metadata
  //
  if (job)
  {
    QString metaDataCharset = job->queryMetaData("charset");

    if (!metaDataCharset.isEmpty ())
      setEncoding (metaDataCharset);
  }

  //
  // service type magic to get encoding right
  //
  QString serviceType = m_extension->urlArgs().serviceType.simplifyWhiteSpace();
  int pos = serviceType.find(';');
  if (pos != -1)
    setEncoding (serviceType.mid(pos+1));

  // do we have success ?
  bool success = m_buffer->openFile (m_file);

  //
  // yeah, success
  //
  if (success)
  {
    if (m_highlight && !m_url.isLocalFile()) {
      // The buffer's highlighting gets nuked by KateBuffer::clear()
      m_buffer->setHighlight(m_highlight);
    }

    // update our hl type if needed
    if (!hlSetByUser)
    {
      int hl (KateHlManager::self()->detectHighlighting (this));

      if (hl >= 0)
        internalSetHlMode(hl);
    }
    // update file type
    updateFileType (KateFactory::self()->fileTypeManager()->fileType (this));

    // read vars
    readVariables();

    // update the md5 digest
    createDigest( m_digest );
  }

  //
  // update views
  //
  updateViews();

  //
  // emit the signal we need for example for kate app
  //
  emit fileNameChanged ();

  //
  // set doc name, dummy value as arg, don't need it
  //
  setDocName  (QString::null);

  //
  // to houston, we are not modified
  //
  if (m_modOnHd)
  {
    m_modOnHd = false;
    m_modOnHdReason = 0;
    emit modifiedOnDisc (this, m_modOnHd, 0);
  }

  //
  // display errors
  //
  if (s_openErrorDialogsActivated)
  {
    if (!success && m_buffer->loadingBorked())
      KMessageBox::error (widget(), i18n ("The file %1 could not be loaded completely, as there is not enough temporary disk storage for it.").arg(m_url.url()));
    else if (!success)
      KMessageBox::error (widget(), i18n ("The file %1 could not be loaded, as it was not possible to read from it.\n\nCheck if you have read access to this file.").arg(m_url.url()));
  }

  //
  // return the success
  //
  return success;
}

bool KateDocument::save()
{
  // FIXME reorder for efficiency, prompt user in case of failure
  bool l ( url().isLocalFile() );
  if ( ( ( l && config()->backupFlags() & KateDocumentConfig::LocalFiles ) ||
         ( ! l && config()->backupFlags() & KateDocumentConfig::RemoteFiles ) )
       && isModified() ) {
    KURL u( url().directory(false) + config()->backupPrefix() + url().fileName() + config()->backupSuffix() );
    if ( ! KIO::NetAccess::upload( url().path(), u, kapp->mainWidget() ) )
      kdDebug(13020)<<"backing up failed ("<<url().prettyURL()<<" -> "<<u.prettyURL()<<")"<<endl;
  }

  return KParts::ReadWritePart::save();
}

bool KateDocument::saveFile()
{
  //
  // we really want to save this file ?
  //
  bool reallySaveIt = !m_buffer->loadingBorked() || (KMessageBox::warningYesNo(widget(),
      i18n("This file could not be loaded correctly due to lack of temporary disk space. Saving it could cause data loss.\n\nDo you really want to save it?")) == KMessageBox::Yes);

  if ( !url().isEmpty() )
  {
    if (s_fileChangedDialogsActivated && m_modOnHd)
    {
      QString str = reasonedMOHString() + "\n\n";

      if (!isModified())
      {
        if (!(KMessageBox::warningYesNo(0,
               str + i18n("Do you really want to save this unmodified file? You could overwrite changed data in the file on disk.")) == KMessageBox::Yes))
          reallySaveIt = false;
      }
      else
      {
        if (!(KMessageBox::warningYesNo(0,
               str + i18n("Do you really want to save this file? Both your open file and the file on disk were changed. There could be some data lost.")) == KMessageBox::Yes))
          reallySaveIt = false;
      }
    }
  }

  //
  // can we encode it if we want to save it ?
  //
  bool canEncode = true;

  if (reallySaveIt)
    canEncode = m_buffer->canEncode ();

  //
  // start with worst case, we had no success
  //
  bool success = false;

  // remove file from dirwatch
  deactivateDirWatch ();

  //
  // try to save
  //
  if (reallySaveIt && canEncode)
    success = m_buffer->saveFile (m_file);

  // update the md5 digest
  createDigest( m_digest );

  // add m_file again to dirwatch
  activateDirWatch ();

  //
  // hurray, we had success, do stuff we need
  //
  if (success)
  {
    // update our hl type if needed
    if (!hlSetByUser)
    {
      int hl (KateHlManager::self()->detectHighlighting (this));

      if (hl >= 0)
        internalSetHlMode(hl);
    }

    // update our file type
    updateFileType (KateFactory::self()->fileTypeManager()->fileType (this));

    // read our vars
    readVariables();
  }

  //
  // emit the signal we need for example for kate app
  //
  emit fileNameChanged ();

  //
  // set doc name, dummy value as arg, don't need it
  //
  setDocName  (QString::null);

  //
  // we are not modified
  //
  if (success && m_modOnHd)
  {
    m_modOnHd = false;
    m_modOnHdReason = 0;
    emit modifiedOnDisc (this, m_modOnHd, 0);
  }

  //
  // display errors
  //
  if (reallySaveIt && !canEncode)
    KMessageBox::error (widget(), i18n ("The document could not be saved, as the selected encoding cannot encode every unicode character in it. If you are unsure of which encoding to use, try UTF-8 or UTF-16."));
  else if (reallySaveIt && !success)
    KMessageBox::error (widget(), i18n ("The document could not be saved, as it was not possible to write to %1.\n\nCheck that you have write access to this file or that enough disk space is available.").arg(m_url.url()));

  //
  // return success
  //
  return success;
}

void KateDocument::activateDirWatch ()
{
  // same file as we are monitoring, return
  if (m_file == m_dirWatchFile)
    return;

  // remove the old watched file
  deactivateDirWatch ();

  // add new file if needed
  if (m_url.isLocalFile() && !m_file.isEmpty())
  {
    KateFactory::self()->dirWatch ()->addFile (m_file);
    m_dirWatchFile = m_file;
  }
}

void KateDocument::deactivateDirWatch ()
{
  if (!m_dirWatchFile.isEmpty())
    KateFactory::self()->dirWatch ()->removeFile (m_dirWatchFile);

  m_dirWatchFile = QString::null;
}

bool KateDocument::closeURL()
{
  abortLoadKate();

  //
  // file mod on hd
  //
  if ( !m_reloading && !url().isEmpty() )
  {
    if (s_fileChangedDialogsActivated && m_modOnHd)
    {
      if (!(KMessageBox::warningYesNo(0,
               reasonedMOHString() + "\n\n" + i18n("Do you really want to continue to close this file? Data loss may occur.")) == KMessageBox::Yes))
        return false;
    }
  }

  //
  // first call the normal kparts implementation
  //
  if (!KParts::ReadWritePart::closeURL ())
    return false;

  // remove file from dirwatch
  deactivateDirWatch ();

  //
  // empty url + filename
  //
  m_url = KURL ();
  m_file = QString::null;

  // we are not modified
  if (m_modOnHd)
  {
    m_modOnHd = false;
    m_modOnHdReason = 0;
    emit modifiedOnDisc (this, m_modOnHd, 0);
  }

  // clear the buffer
  m_buffer->clear();

  // remove all marks
  clearMarks ();

  // clear undo/redo history
  clearUndo();
  clearRedo();

  // no, we are no longer modified
  setModified(false);

  // we have no longer any hl
  internalSetHlMode(0);

  // update all our views
  for (KateView * view = m_views.first(); view != 0L; view = m_views.next() )
  {
    // Explicitly call the internal version because we don't want this to look like
    // an external request (and thus have the view not QWidget::scroll()ed.
    view->setCursorPositionInternal(0, 0, 1, false);
    view->updateView(true);
  }

  // uh, filename changed
  emit fileNameChanged ();

  // update doc name
  setDocName (QString::null);

  // success
  return true;
}

void KateDocument::setReadWrite( bool rw )
{
  if (isReadWrite() != rw)
  {
    KParts::ReadWritePart::setReadWrite (rw);

    for( KateView* view = m_views.first(); view != 0L; view = m_views.next() )
    {
      view->slotUpdate();
      view->slotReadWriteChanged ();
    }
  }
}

void KateDocument::setModified(bool m) {

  if (isModified() != m) {
    KParts::ReadWritePart::setModified (m);

    for( KateView* view = m_views.first(); view != 0L; view = m_views.next() )
    {
      view->slotUpdate();
    }

    emit modifiedChanged ();
    emit modStateChanged ((Kate::Document *)this);
  }
  if ( m == false && ! undoItems.isEmpty() )
  {
    lastUndoGroupWhenSaved = undoItems.last();
  }

  if ( m == false ) docWasSavedWhenUndoWasEmpty = undoItems.isEmpty();
}
//END

//BEGIN Kate specific stuff ;)

void KateDocument::makeAttribs()
{
  m_highlight->clearAttributeArrays ();

  for (uint z = 0; z < m_views.count(); z++)
    m_views.at(z)->renderer()->updateAttributes ();

  m_buffer->invalidateHighlighting();

  tagAll ();
}

// the attributes of a hl have changed, update
void KateDocument::internalHlChanged()
{
  makeAttribs();
}

void KateDocument::addView(KTextEditor::View *view) {
  if (!view)
    return;

  m_views.append( (KateView *) view  );
  m_textEditViews.append( view );

  // apply the view & renderer vars from the file type
  const KateFileType *t = 0;
  if ((m_fileType > -1) && (t = KateFactory::self()->fileTypeManager()->fileType(m_fileType)))
    readVariableLine (t->varLine, true);

  // apply the view & renderer vars from the file
  readVariables (true);

  m_activeView = (KateView *) view;
}

void KateDocument::removeView(KTextEditor::View *view) {
  if (!view)
    return;

  if (m_activeView == view)
    m_activeView = 0L;

  m_views.removeRef( (KateView *) view );
  m_textEditViews.removeRef( view  );
}

void KateDocument::addSuperCursor(KateSuperCursor *cursor, bool privateC) {
  if (!cursor)
    return;

  m_superCursors.append( cursor );

  if (!privateC)
    myCursors.append( cursor );
}

void KateDocument::removeSuperCursor(KateSuperCursor *cursor, bool privateC) {
  if (!cursor)
    return;

  if (!privateC)
    myCursors.removeRef( cursor  );

  m_superCursors.removeRef( cursor  );
}

bool KateDocument::ownedView(KateView *view) {
  // do we own the given view?
  return (m_views.containsRef(view) > 0);
}

bool KateDocument::isLastView(int numViews) {
  return ((int) m_views.count() == numViews);
}

uint KateDocument::currentColumn( const KateTextCursor& cursor )
{
  KateTextLine::Ptr textLine = m_buffer->plainLine(cursor.line());

  if (textLine)
    return textLine->cursorX(cursor.col(), config()->tabWidth());
  else
    return 0;
}

bool KateDocument::typeChars ( KateView *view, const QString &chars )
{
  KateTextLine::Ptr textLine = m_buffer->plainLine(view->cursorLine ());

  if (!textLine)
    return false;

  int oldLine = view->cursorLine ();
  int oldCol = view->cursorColumnReal ();

  bool bracketInserted = false;
  QString buf;
  QChar c;
  for( uint z = 0; z < chars.length(); z++ )
  {
    QChar ch = c = chars[z];

    if (ch.isPrint() || ch == '\t')
    {
      buf.append (ch);

      if (!bracketInserted && (config()->configFlags() & KateDocument::cfAutoBrackets))
      {
        if (ch == '(') { bracketInserted = true; buf.append (')'); }
        if (ch == '[') { bracketInserted = true; buf.append (']'); }
        if (ch == '{') { bracketInserted = true; buf.append ('}'); }
      }
    }
  }

  if (buf.isEmpty())
    return false;

  editStart ();

  if (!(config()->configFlags() & KateDocument::cfPersistent) && hasSelection() )
    removeSelectedText();

  if (config()->configFlags()  & KateDocument::cfOvr)
    removeText (view->cursorLine(), view->cursorColumnReal(), view->cursorLine(), QMIN( view->cursorColumnReal()+buf.length(), textLine->length() ) );

  insertText (view->cursorLine(), view->cursorColumnReal(), buf);
  m_indenter->processChar(c);

  editEnd ();

  if (bracketInserted)
    view->setCursorPositionInternal (view->cursorLine(), view->cursorColumnReal()-1);

  emit charactersInteractivelyInserted (oldLine, oldCol, chars);

  return true;
}

void KateDocument::newLine( KateTextCursor& c, KateViewInternal *v )
{
  editStart();

  if( !(config()->configFlags()  & cfPersistent) && hasSelection() )
    removeSelectedText();

  // temporary hack to get the cursor pos right !!!!!!!!!
  c = v->getCursor ();

  if (c.line() > (int)lastLine())
   c.setLine(lastLine());

  uint ln = c.line();

  KateTextLine::Ptr textLine = kateTextLine(c.line());
  if (c.col() > (int)textLine->length())
    c.setCol(textLine->length());

  if (!(config()->configFlags() & KateDocument::cfAutoIndent))
  {
    insertText( c.line(), c.col(), "\n" );
    c.setPos(c.line() + 1, 0);
  }
  else
  {
    int pos = textLine->firstChar();
    if (c.col() < pos)
      c.setCol(pos); // place cursor on first char if before

    insertText (c.line(), c.col(), "\n");

    KateDocCursor cursor (c.line() + 1, pos, this);
    m_indenter->processNewline(cursor, true);
    c.setPos(cursor);
  }

  removeTrailingSpace( ln );

  editEnd();
}

void KateDocument::transpose( const KateTextCursor& cursor)
{
  KateTextLine::Ptr textLine = m_buffer->plainLine(cursor.line());

  if (!textLine || (textLine->length() < 2))
    return;

  uint col = cursor.col();

  if (col > 0)
    col--;

  if ((textLine->length() - col) < 2)
    return;

  uint line = cursor.line();
  QString s;

  //clever swap code if first character on the line swap right&left
  //otherwise left & right
  s.append (textLine->getChar(col+1));
  s.append (textLine->getChar(col));
  //do the swap

  // do it right, never ever manipulate a textline
  editStart ();
  editRemoveText (line, col, 2);
  editInsertText (line, col, s);
  editEnd ();
}

void KateDocument::backspace( const KateTextCursor& c )
{
  if( !(config()->configFlags() & cfPersistent) && hasSelection() ) {
    removeSelectedText();
    return;
  }

  uint col = QMAX( c.col(), 0 );
  uint line = QMAX( c.line(), 0 );

  if ((col == 0) && (line == 0))
    return;

  if (col > 0)
  {
    if (!(config()->configFlags() & KateDocument::cfBackspaceIndents))
    {
      // ordinary backspace
      //c.cursor.col--;
      removeText(line, col-1, line, col);
    }
    else
    {
      // backspace indents: erase to next indent position

      KateTextLine::Ptr textLine = m_buffer->plainLine(line);
      int colX = textLine->cursorX(col, config()->tabWidth());
      int pos = textLine->firstChar();
      if (pos > 0)
        pos = textLine->cursorX(pos, config()->tabWidth());

      if (pos < 0 || pos >= (int)colX)
      {
        // only spaces on left side of cursor
        // search a line with less spaces
        int y = line;
        while (--y >= 0)
        {
          textLine = m_buffer->plainLine(y);
          pos = textLine->firstChar();

          if (pos >= 0)
          {
            pos = textLine->cursorX(pos, config()->tabWidth());
            if (pos < (int)colX)
            {
              replaceWithOptimizedSpace(line, col, pos, config()->configFlags());
              break;
            }
          }
        }
        if (y < 0) {
          // FIXME: what shoud we do in this case?
          removeText(line, 0, line, col);
        }
      }
      else
        removeText(line, col-1, line, col);
    }
  }
  else
  {
    // col == 0: wrap to previous line
    if (line >= 1)
    {
      KateTextLine::Ptr textLine = m_buffer->plainLine(line-1);
      if (config()->wordWrap() && textLine->endingWith(QString::fromLatin1(" ")))
      {
        // gg: in hard wordwrap mode, backspace must also eat the trailing space
        removeText (line-1, textLine->length()-1, line, 0);
      }
      else
        removeText (line-1, textLine->length(), line, 0);
    }
  }

  emit backspacePressed();
}

void KateDocument::del( const KateTextCursor& c )
{
  if ( !(config()->configFlags() & cfPersistent) && hasSelection() ) {
    removeSelectedText();
    return;
  }

  if( c.col() < (int) m_buffer->plainLine(c.line())->length())
  {
    removeText(c.line(), c.col(), c.line(), c.col()+1);
  }
  else
  {
    removeText(c.line(), c.col(), c.line()+1, 0);
  }
}

void KateDocument::cut()
{
  if (!hasSelection())
    return;

  copy();
  removeSelectedText();
}

void KateDocument::copy()
{
  if (!hasSelection())
    return;

  QApplication::clipboard()->setText(selection ());
}

void KateDocument::paste ( KateView* view )
{
  QString s = QApplication::clipboard()->text();

  if (s.isEmpty())
    return;

  m_undoDontMerge = true;

  editStart ();

  if (!(config()->configFlags() & KateDocument::cfPersistent) && hasSelection() )
    removeSelectedText();

  uint line = view->cursorLine ();
  uint column = view->cursorColumnReal ();

  insertText ( line, column, s, blockSelect );

  KateDocCursor begin((int)editTagLineStart, 0, this);
  KateDocCursor end((int)editTagLineEnd, 0, this);

  editEnd();

  // move cursor right for block select, as the user is moved right internal
  // even in that case, but user expects other behavior in block selection
  // mode !
  if (blockSelect)
  {
    uint lines = s.contains (QChar ('\n'));
    view->setCursorPositionInternal (line+lines, column);
  }

  if (m_indenter->canProcessLine())
  {
    editStart();
    m_indenter->processSection (begin, end);
    editEnd();
  }

  m_undoDontMerge = true;
}

void KateDocument::selectWord( const KateTextCursor& cursor )
{
  int start, end, len;

  KateTextLine::Ptr textLine = m_buffer->plainLine(cursor.line());
  len = textLine->length();
  start = end = cursor.col();
  while (start > 0 && m_highlight->isInWord(textLine->getChar(start - 1), textLine->attribute(start - 1))) start--;
  while (end < len && m_highlight->isInWord(textLine->getChar(end), textLine->attribute(start - 1))) end++;
  if (end <= start) return;

  if (!(config()->configFlags() & KateDocument::cfKeepSelection))
    clearSelection ();

  setSelection (cursor.line(), start, cursor.line(), end);
}

void KateDocument::selectLine( const KateTextCursor& cursor )
{
  if (!(config()->configFlags() & KateDocument::cfKeepSelection))
    clearSelection ();

  setSelection (cursor.line(), 0, cursor.line()/*+1, 0*/, m_buffer->plainLine(cursor.line())->length() );
}

void KateDocument::selectLength( const KateTextCursor& cursor, int length )
{
  int start, end;

  KateTextLine::Ptr textLine = m_buffer->plainLine(cursor.line());
  start = cursor.col();
  end = start + length;
  if (end <= start) return;

  if (!(config()->configFlags() & KateDocument::cfKeepSelection))
    clearSelection ();
  setSelection (cursor.line(), start, cursor.line(), end);
}

void KateDocument::insertIndentChars ( KateView *view )
{
  editStart ();

  QString s;
  if (config()->configFlags() & KateDocument::cfSpaceIndent)
  {
    int width = config()->indentationWidth();
    s.fill (' ', width - (view->cursorColumnReal() % width));
  }
  else
    s.append ('\t');

  insertText (view->cursorLine(), view->cursorColumnReal(), s);

  editEnd ();
}

void KateDocument::indent ( KateView *, uint line, int change)
{
  editStart ();

  if (!hasSelection())
  {
    // single line
    optimizeLeadingSpace(line, config()->configFlags(), change);
  }
  else
  {
    int sl = selectStart.line();
    int el = selectEnd.line();
    int ec = selectEnd.col();

    if ((ec == 0) && ((el-1) >= 0))
    {
      el--; /* */
    }

    if (config()->configFlags() & KateDocument::cfKeepIndentProfile && change < 0) {
      // unindent so that the existing indent profile doesn't get screwed
      // if any line we may unindent is already full left, don't do anything
      int adjustedChange = -change;

      for (line = sl; (int) line <= el && adjustedChange > 0; line++) {
        KateTextLine::Ptr textLine = m_buffer->plainLine(line);
        int firstChar = textLine->firstChar();
        if (firstChar >= 0 && (lineSelected(line) || lineHasSelected(line))) {
          int maxUnindent = textLine->cursorX(firstChar, config()->tabWidth()) / config()->indentationWidth();
          if (maxUnindent < adjustedChange)
            adjustedChange = maxUnindent;
        }
      }

      change = -adjustedChange;
    }

    for (line = sl; (int) line <= el; line++) {
      if (lineSelected(line) || lineHasSelected(line)) {
        optimizeLeadingSpace(line, config()->configFlags(), change);
      }
    }
  }

  editEnd ();
}

void KateDocument::align(uint line)
{
  if (m_indenter->canProcessLine())
  {
    editStart ();

    if (!hasSelection())
    {
      KateDocCursor curLine(line, 0, this);
      m_indenter->processLine (curLine);
      editEnd ();
      activeView()->setCursorPosition (line, curLine.col());
    }
    else
    {
      m_indenter->processSection(selectStart, selectEnd);
      editEnd ();
    }
  }
}

/*
  Optimize the leading whitespace for a single line.
  If change is > 0, it adds indentation units (indentationChars)
  if change is == 0, it only optimizes
  If change is < 0, it removes indentation units
  This will be used to indent, unindent, and optimal-fill a line.
  If excess space is removed depends on the flag cfKeepExtraSpaces
  which has to be set by the user
*/
void KateDocument::optimizeLeadingSpace(uint line, int flags, int change)
{
  KateTextLine::Ptr textline = m_buffer->plainLine(line);

  int first_char = textline->firstChar();

  int w = 0;
  if (flags & KateDocument::cfSpaceIndent)
    w = config()->indentationWidth();
  else
    w = config()->tabWidth();

  if (first_char < 0)
    first_char = textline->length();

  int space =  textline->cursorX(first_char, config()->tabWidth()) + change * w;
  if (space < 0)
    space = 0;

  if (!(flags & KateDocument::cfKeepExtraSpaces))
  {
    uint extra = space % w;

    space -= extra;
    if (extra && change < 0) {
      // otherwise it unindents too much (e.g. 12 chars when indentation is 8 chars wide)
      space += w;
    }
  }

  //kdDebug(13020)  << "replace With Op: " << line << " " << first_char << " " << space << endl;
  replaceWithOptimizedSpace(line, first_char, space, flags);
}

void KateDocument::replaceWithOptimizedSpace(uint line, uint upto_column, uint space, int flags)
{
  uint length;
  QString new_space;

  if (flags & KateDocument::cfSpaceIndent) {
    length = space;
    new_space.fill(' ', length);
  }
  else {
    length = space / config()->tabWidth();
    new_space.fill('\t', length);

    QString extra_space;
    extra_space.fill(' ', space % config()->tabWidth());
    length += space % config()->tabWidth();
    new_space += extra_space;
  }

  KateTextLine::Ptr textline = m_buffer->plainLine(line);
  uint change_from;
  for (change_from = 0; change_from < upto_column && change_from < length; change_from++) {
    if (textline->getChar(change_from) != new_space[change_from])
      break;
  }

  editStart();

  if (change_from < upto_column)
    removeText(line, change_from, line, upto_column);

  if (change_from < length)
    insertText(line, change_from, new_space.right(length - change_from));

  editEnd();
}

/*
  Remove a given string at the begining
  of the current line.
*/
bool KateDocument::removeStringFromBegining(int line, QString &str)
{
  KateTextLine::Ptr textline = m_buffer->plainLine(line);

  int index = 0;
  bool there = false;

  if (textline->startingWith(str))
    there = true;
  else
  {
    index = textline->firstChar ();

    if ((index >= 0) && (textline->length() >= (index + str.length())) && (textline->string(index, str.length()) == str))
      there = true;
  }

  if (there)
  {
    // Remove some chars
    removeText (line, index, line, index+str.length());
  }

  return there;
}

/*
  Remove a given string at the end
  of the current line.
*/
bool KateDocument::removeStringFromEnd(int line, QString &str)
{
  KateTextLine::Ptr textline = m_buffer->plainLine(line);

  int index = 0;
  bool there = false;

  if(textline->endingWith(str))
  {
    index = textline->length() - str.length();
    there = true;
  }
  else
  {
    index = textline->lastChar ()-str.length()+1;

    if ((index >= 0) && (textline->length() >= (index + str.length())) && (textline->string(index, str.length()) == str))
      there = true;
  }

  if (there)
  {
    // Remove some chars
    removeText (line, index, line, index+str.length());
  }

  return there;
}

/*
  Add to the current line a comment line mark at
  the begining.
*/
void KateDocument::addStartLineCommentToSingleLine( int line, int attrib )
{
  QString commentLineMark = m_highlight->getCommentSingleLineStart( attrib ) + " ";
  insertText (line, 0, commentLineMark);
}

/*
  Remove from the current line a comment line mark at
  the begining if there is one.
*/
bool KateDocument::removeStartLineCommentFromSingleLine( int line, int attrib )
{
  QString shortCommentMark = m_highlight->getCommentSingleLineStart( attrib );
  QString longCommentMark = shortCommentMark + " ";

  editStart();

  // Try to remove the long comment mark first
  bool removed = (removeStringFromBegining(line, longCommentMark)
                  || removeStringFromBegining(line, shortCommentMark));

  editEnd();

  return removed;
}

/*
  Add to the current line a start comment mark at the
 begining and a stop comment mark at the end.
*/
void KateDocument::addStartStopCommentToSingleLine( int line, int attrib )
{
  QString startCommentMark = m_highlight->getCommentStart( attrib ) + " ";
  QString stopCommentMark = " " + m_highlight->getCommentEnd( attrib );

  editStart();

  // Add the start comment mark
  insertText (line, 0, startCommentMark);

  // Go to the end of the line
  int col = m_buffer->plainLine(line)->length();

  // Add the stop comment mark
  insertText (line, col, stopCommentMark);

  editEnd();
}

/*
  Remove from the current line a start comment mark at
  the begining and a stop comment mark at the end.
*/
bool KateDocument::removeStartStopCommentFromSingleLine( int line, int attrib )
{
  QString shortStartCommentMark = m_highlight->getCommentStart( attrib );
  QString longStartCommentMark = shortStartCommentMark + " ";
  QString shortStopCommentMark = m_highlight->getCommentEnd( attrib );
  QString longStopCommentMark = " " + shortStopCommentMark;

  editStart();

  // Try to remove the long start comment mark first
  bool removedStart = (removeStringFromBegining(line, longStartCommentMark)
                       || removeStringFromBegining(line, shortStartCommentMark));

  bool removedStop = false;
  if (removedStart)
  {
    // Try to remove the long stop comment mark first
    removedStop = (removeStringFromEnd(line, longStopCommentMark)
                      || removeStringFromEnd(line, shortStopCommentMark));
  }

  editEnd();

  return (removedStart || removedStop);
}

/*
  Add to the current selection a start comment
 mark at the begining and a stop comment mark
 at the end.
*/
void KateDocument::addStartStopCommentToSelection( int attrib )
{
  QString startComment = m_highlight->getCommentStart( attrib );
  QString endComment = m_highlight->getCommentEnd( attrib );

  int sl = selectStart.line();
  int el = selectEnd.line();
  int sc = selectStart.col();
  int ec = selectEnd.col();

  if ((ec == 0) && ((el-1) >= 0))
  {
    el--;
    ec = m_buffer->plainLine (el)->length();
  }

  editStart();

  insertText (el, ec, endComment);
  insertText (sl, sc, startComment);

  editEnd ();

  // Set the new selection
  ec += endComment.length() + ( (el == sl) ? startComment.length() : 0 );
  setSelection(sl, sc, el, ec);
}

/*
  Add to the current selection a comment line
 mark at the begining of each line.
*/
void KateDocument::addStartLineCommentToSelection( int attrib )
{
  QString commentLineMark = m_highlight->getCommentSingleLineStart( attrib ) + " ";

  int sl = selectStart.line();
  int el = selectEnd.line();

  if ((selectEnd.col() == 0) && ((el-1) >= 0))
  {
    el--;
  }

  editStart();

  // For each line of the selection
  for (int z = el; z >= sl; z--) {
    insertText (z, 0, commentLineMark);
  }

  editEnd ();

  // Set the new selection
  selectEnd.setCol(selectEnd.col() + ((el == selectEnd.line()) ? commentLineMark.length() : 0) );
  setSelection(selectStart.line(), 0, selectEnd.line(), selectEnd.col());
}

bool KateDocument::nextNonSpaceCharPos(int &line, int &col)
{
  for(; line < (int)m_buffer->count(); line++) {
    col = m_buffer->plainLine(line)->nextNonSpaceChar(col);
    if(col != -1)
      return true; // Next non-space char found
    col = 0;
  }
  // No non-space char found
  line = -1;
  col = -1;
  return false;
}

bool KateDocument::previousNonSpaceCharPos(int &line, int &col)
{
  while(true)
  {
    col = m_buffer->plainLine(line)->previousNonSpaceChar(col);
    if(col != -1) return true;
    if(line == 0) return false;
    --line;
    col = m_buffer->plainLine(line)->length();
}
  // No non-space char found
  line = -1;
  col = -1;
  return false;
}

/*
  Remove from the selection a start comment mark at
  the begining and a stop comment mark at the end.
*/
bool KateDocument::removeStartStopCommentFromSelection( int attrib )
{
  QString startComment = m_highlight->getCommentStart( attrib );
  QString endComment = m_highlight->getCommentEnd( attrib );

  int sl = selectStart.line();
  int el = selectEnd.line();
  int sc = selectStart.col();
  int ec = selectEnd.col();

  // The selection ends on the char before selectEnd
  if (ec != 0) {
    ec--;
  } else {
    if (el > 0) {
      el--;
      ec = m_buffer->plainLine(el)->length() - 1;
    }
  }

  int startCommentLen = startComment.length();
  int endCommentLen = endComment.length();

  // had this been perl or sed: s/^\s*$startComment(.+?)$endComment\s*/$1/

  bool remove = nextNonSpaceCharPos(sl, sc)
      && m_buffer->plainLine(sl)->stringAtPos(sc, startComment)
      && previousNonSpaceCharPos(el, ec)
      && ( (ec - endCommentLen + 1) >= 0 )
      && m_buffer->plainLine(el)->stringAtPos(ec - endCommentLen + 1, endComment);

  if (remove) {
    editStart();

    removeText (el, ec - endCommentLen + 1, el, ec + 1);
    removeText (sl, sc, sl, sc + startCommentLen);

    editEnd ();

    // Set the new selection
    ec -= endCommentLen + ( (el == sl) ? startCommentLen : 0 );
    setSelection(sl, sc, el, ec + 1);
  }

  return remove;
}

/*
  Remove from the begining of each line of the
  selection a start comment line mark.
*/
bool KateDocument::removeStartLineCommentFromSelection( int attrib )
{
  QString shortCommentMark = m_highlight->getCommentSingleLineStart( attrib );
  QString longCommentMark = shortCommentMark + " ";

  int sl = selectStart.line();
  int el = selectEnd.line();

  if ((selectEnd.col() == 0) && ((el-1) >= 0))
  {
    el--;
  }

  // Find out how many char will be removed from the last line
  int removeLength = 0;
  if (m_buffer->plainLine(el)->startingWith(longCommentMark))
    removeLength = longCommentMark.length();
  else if (m_buffer->plainLine(el)->startingWith(shortCommentMark))
    removeLength = shortCommentMark.length();

  bool removed = false;

  editStart();

  // For each line of the selection
  for (int z = el; z >= sl; z--)
  {
    // Try to remove the long comment mark first
    removed = (removeStringFromBegining(z, longCommentMark)
                 || removeStringFromBegining(z, shortCommentMark)
                 || removed);
  }

  editEnd();

  if(removed) {
    // Set the new selection
    selectEnd.setCol(selectEnd.col() - ((el == selectEnd.line()) ? removeLength : 0) );
    setSelection(selectStart.line(), selectStart.col(), selectEnd.line(), selectEnd.col());
  }

  return removed;
}

/*
  Comment or uncomment the selection or the current
  line if there is no selection.
*/
void KateDocument::comment( KateView *, uint line, int change)
{
  // We need to check that we can sanely comment the selectino or region.
  // It is if the attribute of the first and last character of the range to
  // comment belongs to the same language definition.
  // for lines with no text, we need the attribute for the lines context.
  bool hassel = hasSelection();
  int startAttrib, endAttrib;
  if ( hassel )
  {
    KateTextLine::Ptr ln = kateTextLine( selectStart.line() );
    int l = selectStart.line(), c = selectStart.col();
    startAttrib = nextNonSpaceCharPos( l, c ) ? kateTextLine( l )->attribute( c ) : 0;

    ln = kateTextLine( selectEnd.line() );
    l = selectEnd.line(), c = selectEnd.col();
    endAttrib = previousNonSpaceCharPos( l, c ) ? kateTextLine( l )->attribute( c ) : 0;
  }
  else
  {
    KateTextLine::Ptr ln = kateTextLine( line );
    if ( ln->length() )
    {
      startAttrib = ln->attribute( ln->firstChar() );
      endAttrib = ln->attribute( ln->lastChar() );
    }
    else
    {
      int l = line, c = 0;
      if ( nextNonSpaceCharPos( l, c )  || previousNonSpaceCharPos( l, c ) )
        startAttrib = endAttrib = kateTextLine( l )->attribute( c );
      else
        startAttrib = endAttrib = 0;
    }
  }

  if ( ! m_highlight->canComment( startAttrib, endAttrib ) )
  {
    kdDebug(13020)<<"canComment( "<<startAttrib<<", "<<endAttrib<<" ) returned false!"<<endl;
    return;
  }

  bool hasStartLineCommentMark = !(m_highlight->getCommentSingleLineStart( startAttrib ).isEmpty());
  bool hasStartStopCommentMark = ( !(m_highlight->getCommentStart( startAttrib ).isEmpty())
      && !(m_highlight->getCommentEnd( endAttrib ).isEmpty()) );

  bool removed = false;

  if (change > 0)
  {
    if ( !hassel )
    {
      if ( hasStartLineCommentMark )
        addStartLineCommentToSingleLine( line, startAttrib );
      else if ( hasStartStopCommentMark )
        addStartStopCommentToSingleLine( line, startAttrib );
    }
    else
    {
      // anders: prefer single line comment to avoid nesting probs
      // If the selection starts after first char in the first line
      // or ends before the last char of the last line, we may use
      // multiline comment markers.
      // TODO We should try to detect nesting.
      //    - if selection ends at col 0, most likely she wanted that
      // line ignored
      if ( hasStartStopCommentMark &&
           ( !hasStartLineCommentMark || (
             ( selectStart.col() > m_buffer->plainLine( selectStart.line() )->firstChar() ) ||
               ( selectEnd.col() < ((int)m_buffer->plainLine( selectEnd.line() )->length()) )
         ) ) )
        addStartStopCommentToSelection( startAttrib );
      else if ( hasStartLineCommentMark )
        addStartLineCommentToSelection( startAttrib );
    }
  }
  else
  {
    if ( !hassel )
    {
      removed = ( hasStartLineCommentMark
                  && removeStartLineCommentFromSingleLine( line, startAttrib ) )
        || ( hasStartStopCommentMark
             && removeStartStopCommentFromSingleLine( line, startAttrib ) );
    }
    else
    {
      // anders: this seems like it will work with above changes :)
      removed = ( hasStartLineCommentMark
                  && removeStartLineCommentFromSelection( startAttrib ) )
        || ( hasStartStopCommentMark
             && removeStartStopCommentFromSelection( startAttrib ) );
    }
  }
}

void KateDocument::transform( KateView *, const KateTextCursor &c,
                            KateDocument::TextTransform t )
{
  editStart();
  uint cl( c.line() ), cc( c.col() );

  if ( hasSelection() )
  {
    // cache the selection and cursor, so we can be sure to restore.
    KateTextCursor s = selectStart;
    KateTextCursor e = selectEnd;

    int ln = selStartLine();
    while ( ln <= selEndLine() )
    {
      uint start, end;
      start = (ln == selStartLine() || blockSelectionMode()) ?
          selStartCol() : 0;
      end = (ln == selEndLine() || blockSelectionMode()) ?
          selEndCol() : lineLength( ln );
      QString s = text( ln, start, ln, end );

      if ( t == Uppercase )
        s = s.upper();
      else if ( t == Lowercase )
        s = s.lower();
      else // Capitalize
      {
        KateTextLine::Ptr l = m_buffer->plainLine( ln );
        uint p ( 0 );
        while( p < s.length() )
        {
          // If bol or the character before is not in a word, up this one:
          // 1. if both start and p is 0, upper char.
          // 2. if blockselect or first line, and p == 0 and start-1 is not in a word, upper
          // 3. if p-1 is not in a word, upper.
          if ( ( ! start && ! p ) ||
               ( ( ln == selStartLine() || blockSelectionMode() ) &&
                 ! p && ! m_highlight->isInWord( l->getChar( start - 1 )) ) ||
               ( p && ! m_highlight->isInWord( s.at( p-1 ) ) )
             )
            s[p] = s.at(p).upper();
          p++;
        }
      }

      removeText( ln, start, ln, end );
      insertText( ln, start, s );

      ln++;
    }

    // restore selection
    setSelection( s, e );

  } else {  // no selection
    QString s;
    int n ( cc );
    switch ( t ) {
      case Uppercase:
      s = text( cl, cc, cl, cc + 1 ).upper();
      break;
      case Lowercase:
      s = text( cl, cc, cl, cc + 1 ).lower();
      break;
      case Capitalize:
      {
        KateTextLine::Ptr l = m_buffer->plainLine( cl );
        while ( n > 0 && m_highlight->isInWord( l->getChar( n-1 ), l->attribute( n-1 ) ) )
          n--;
        s = text( cl, n, cl, n + 1 ).upper();
      }
      break;
      default:
      break;
    }
    removeText( cl, n, cl, n+1 );
    insertText( cl, n, s );
  }

  editEnd();

  if ( activeView() )
    activeView()->setCursorPosition( cl, cc );
}

void KateDocument::joinLines( uint first, uint last )
{
//   if ( first == last ) last += 1;
  editStart();
  int l( first );
  while ( first < last )
  {
    editUnWrapLine( l );
    first++;
  }
  editEnd();
}

QString KateDocument::getWord( const KateTextCursor& cursor ) {
  int start, end, len;

  KateTextLine::Ptr textLine = m_buffer->plainLine(cursor.line());
  len = textLine->length();
  start = end = cursor.col();
  if (start > len)        // Probably because of non-wrapping cursor mode.
    return QString("");

  while (start > 0 && m_highlight->isInWord(textLine->getChar(start - 1), textLine->attribute(start - 1))) start--;
  while (end < len && m_highlight->isInWord(textLine->getChar(end), textLine->attribute(end))) end++;
  len = end - start;
  return QString(&textLine->text()[start], len);
}

void KateDocument::tagLines(int start, int end)
{
  for (uint z = 0; z < m_views.count(); z++)
    m_views.at(z)->tagLines (start, end, true);
}

void KateDocument::tagLines(KateTextCursor start, KateTextCursor end)
{
  // May need to switch start/end cols if in block selection mode
  if (blockSelectionMode() && start.col() > end.col()) {
    int sc = start.col();
    start.setCol(end.col());
    end.setCol(sc);
  }

  for (uint z = 0; z < m_views.count(); z++)
    m_views.at(z)->tagLines(start, end, true);
}

void KateDocument::tagSelection(const KateTextCursor &oldSelectStart, const KateTextCursor &oldSelectEnd)
{
  if (hasSelection()) {
    if (oldSelectStart.line() == -1) {
      // We have to tag the whole lot if
      // 1) we have a selection, and:
      //  a) it's new; or
      tagLines(selectStart, selectEnd);

    } else if (blockSelectionMode() && (oldSelectStart.col() != selectStart.col() || oldSelectEnd.col() != selectEnd.col())) {
      //  b) we're in block selection mode and the columns have changed
      tagLines(selectStart, selectEnd);
      tagLines(oldSelectStart, oldSelectEnd);

    } else {
      if (oldSelectStart != selectStart) {
        if (oldSelectStart < selectStart)
          tagLines(oldSelectStart, selectStart);
        else
          tagLines(selectStart, oldSelectStart);
      }

      if (oldSelectEnd != selectEnd) {
        if (oldSelectEnd < selectEnd)
          tagLines(oldSelectEnd, selectEnd);
        else
          tagLines(selectEnd, oldSelectEnd);
      }
    }

  } else {
    // No more selection, clean up
    tagLines(oldSelectStart, oldSelectEnd);
  }
}

void KateDocument::repaintViews(bool paintOnlyDirty)
{
  for (uint z = 0; z < m_views.count(); z++)
    m_views.at(z)->repaintText(paintOnlyDirty);
}

void KateDocument::tagAll()
{
  for (uint z = 0; z < m_views.count(); z++)
  {
    m_views.at(z)->tagAll();
    m_views.at(z)->updateView (true);
  }
}

void KateDocument::updateViews()
{
  if (noViewUpdates)
    return;

  for (KateView * view = m_views.first(); view != 0L; view = m_views.next() )
  {
    view->updateView(true);
  }
}

uint KateDocument::configFlags ()
{
  return config()->configFlags();
}

void KateDocument::setConfigFlags (uint flags)
{
  config()->setConfigFlags(flags);
}

bool KateDocument::lineColSelected (int line, int col)
{
  if ( (!blockSelect) && (col < 0) )
    col = 0;

  KateTextCursor cursor(line, col);

  if (blockSelect)
    return cursor.line() >= selectStart.line() && cursor.line() <= selectEnd.line() && cursor.col() >= selectStart.col() && cursor.col() < selectEnd.col();
  else
    return (cursor >= selectStart) && (cursor < selectEnd);
}

bool KateDocument::lineSelected (int line)
{
  return (!blockSelect)
    && (selectStart <= KateTextCursor(line, 0))
    && (line < selectEnd.line());
}

bool KateDocument::lineEndSelected (int line, int endCol)
{
  return (!blockSelect)
    && (line > selectStart.line() || (line == selectStart.line() && (selectStart.col() < endCol || endCol == -1)))
    && (line < selectEnd.line() || (line == selectEnd.line() && (endCol <= selectEnd.col() && endCol != -1)));
}

bool KateDocument::lineHasSelected (int line)
{
  return (selectStart < selectEnd)
    && (line >= selectStart.line())
    && (line <= selectEnd.line());
}

bool KateDocument::lineIsSelection (int line)
{
  return (line == selectStart.line() && line == selectEnd.line());
}

inline bool isStartBracket( const QChar& c ) { return c == '{' || c == '[' || c == '('; }
inline bool isEndBracket  ( const QChar& c ) { return c == '}' || c == ']' || c == ')'; }
inline bool isBracket     ( const QChar& c ) { return isStartBracket( c ) || isEndBracket( c ); }

/*
   Bracket matching uses the following algorithm:
   If in overwrite mode, match the bracket currently underneath the cursor.
   Otherwise, if the character to the left of the cursor is an ending bracket,
   match it. Otherwise if the character to the right of the cursor is a
   starting bracket, match it. Otherwise, if the the character to the left
   of the cursor is an starting bracket, match it. Otherwise, if the character
   to the right of the cursor is an ending bracket, match it. Otherwise, don't
   match anything.
*/
void KateDocument::newBracketMark( const KateTextCursor& cursor, KateTextRange& bm )
{
  bm.setValid(false);

  bm.start() = cursor;

  if( !findMatchingBracket( bm.start(), bm.end() ) )
    return;

  bm.setValid(true);
}

bool KateDocument::findMatchingBracket( KateTextCursor& start, KateTextCursor& end )
{
  KateTextLine::Ptr textLine = m_buffer->plainLine( start.line() );
  if( !textLine )
    return false;

  QChar right = textLine->getChar( start.col() );
  QChar left  = textLine->getChar( start.col() - 1 );
  QChar bracket;

  if ( config()->configFlags() & cfOvr ) {
    if( isBracket( right ) ) {
      bracket = right;
    } else {
      return false;
    }
  } else if ( isEndBracket( left ) ) {
    start.setCol(start.col() - 1);
    bracket = left;
  } else if ( isStartBracket( right ) ) {
    bracket = right;
  } else if ( isBracket( left ) ) {
    start.setCol(start.col() - 1);
    bracket = left;
  } else if ( isBracket( right ) ) {
    bracket = right;
  } else {
    return false;
  }

  QChar opposite;

  switch( bracket ) {
  case '{': opposite = '}'; break;
  case '}': opposite = '{'; break;
  case '[': opposite = ']'; break;
  case ']': opposite = '['; break;
  case '(': opposite = ')'; break;
  case ')': opposite = '('; break;
  default: return false;
  }

  bool forward = isStartBracket( bracket );
  int startAttr = textLine->attribute( start.col() );
  uint count = 0;
  end = start;

  while( true ) {
    /* Increment or decrement, check base cases */
    if( forward ) {
      end.setCol(end.col() + 1);
      if( end.col() >= lineLength( end.line() ) ) {
        if( end.line() >= (int)lastLine() )
          return false;
        end.setPos(end.line() + 1, 0);
        textLine = m_buffer->plainLine( end.line() );
      }
    } else {
      end.setCol(end.col() - 1);
      if( end.col() < 0 ) {
        if( end.line() <= 0 )
          return false;
        end.setLine(end.line() - 1);
        end.setCol(lineLength( end.line() ) - 1);
        textLine = m_buffer->plainLine( end.line() );
      }
    }

    /* Easy way to skip comments */
    if( textLine->attribute( end.col() ) != startAttr )
      continue;

    /* Check for match */
    QChar c = textLine->getChar( end.col() );
    if( c == bracket ) {
      count++;
    } else if( c == opposite ) {
      if( count == 0 )
        return true;
      count--;
    }

  }
}

void KateDocument::guiActivateEvent( KParts::GUIActivateEvent *ev )
{
  KParts::ReadWritePart::guiActivateEvent( ev );
  if ( ev->activated() )
    emit selectionChanged();
}

void KateDocument::setDocName (QString name )
{
  if ( !name.isEmpty() )
  {
    // TODO check for similarly named documents
    m_docName = name;
    emit nameChanged((Kate::Document *) this);
    return;
  }

  // if the name is set, and starts with FILENAME, it should not be changed!
  if ( ! url().isEmpty() && m_docName.startsWith( url().filename() ) ) return;

  int count = -1;

  for (uint z=0; z < KateFactory::self()->documents()->count(); z++)
  {
    if ( (KateFactory::self()->documents()->at(z) != this) && (KateFactory::self()->documents()->at(z)->url().filename() == url().filename()) )
      if ( KateFactory::self()->documents()->at(z)->m_docNameNumber > count )
        count = KateFactory::self()->documents()->at(z)->m_docNameNumber;
  }

  m_docNameNumber = count + 1;

  m_docName = url().filename();

  if (m_docName.isEmpty())
    m_docName = i18n ("Untitled");

  if (m_docNameNumber > 0)
    m_docName = QString(m_docName + " (%1)").arg(m_docNameNumber+1);

  emit nameChanged ((Kate::Document *) this);
}

void KateDocument::slotModifiedOnDisk( Kate::View *v )
{
  if ( !s_fileChangedDialogsActivated || m_isasking )
    return;

  if (m_modOnHd && !url().isEmpty())
  {
    m_isasking = 1;

    int exitval = ( v && v->hasFocus() ? 0 : -1 );

    switch ( KMessageBox::warningYesNoCancel( widget(),
                reasonedMOHString() + "\n\n" + i18n("What do you want to do?"),
                i18n("File Was Modified on Disk"),
                i18n("&Reload File"), i18n("&Ignore Changes")) )
    {
      case KMessageBox::Yes: // "reload file"
        m_modOnHd = false; // trick reloadFile() to not ask again
        emit modifiedOnDisc( this, false, 0 );
        reloadFile();
        m_isasking = 0;
        break;

      case KMessageBox::No:  // "ignore changes"
        m_modOnHd = false;
        emit modifiedOnDisc( this, false, 0 );
        m_isasking = 0;
        break;

      default:               // cancel: ignore next focus event
        m_isasking = -1;
    }
  }
}

void KateDocument::setModifiedOnDisk( int reason )
{
  m_modOnHdReason = reason;
  emit modifiedOnDisc( this, (reason > 0), reason );
}

class KateDocumentTmpMark
{
  public:
    QString line;
    KTextEditor::Mark mark;
};

void KateDocument::reloadFile()
{
  if ( !url().isEmpty() )
  {
    if (m_modOnHd && s_fileChangedDialogsActivated)
    {
      int i = KMessageBox::warningYesNoCancel
                (0, reasonedMOHString() + "\n\n" + i18n("What do you want to do?"),
                i18n("File Was Changed on Disk"), i18n("&Reload File"), i18n("&Ignore Changes"));

      if ( i != KMessageBox::Yes)
      {
        if (i == KMessageBox::No)
        {
          m_modOnHd = false;
          m_modOnHdReason = 0;
          emit modifiedOnDisc (this, m_modOnHd, 0);
        }

        return;
      }
    }

    QValueList<KateDocumentTmpMark> tmp;

    for( QIntDictIterator<KTextEditor::Mark> it( m_marks ); it.current(); ++it )
    {
      KateDocumentTmpMark m;

      m.line = textLine (it.current()->line);
      m.mark = *it.current();

      tmp.append (m);
    }

    uint mode = hlMode ();
    bool byUser = hlSetByUser;

    m_storedVariables.clear();

    m_reloading = true;
    KateDocument::openURL( url() );
    m_reloading = false;

    for (uint z=0; z < tmp.size(); z++)
    {
      if (z < numLines())
      {
        if (textLine(tmp[z].mark.line) == tmp[z].line)
          setMark (tmp[z].mark.line, tmp[z].mark.type);
      }
    }

    if (byUser)
      setHlMode (mode);
  }
}

void KateDocument::flush ()
{
  closeURL ();
}

void KateDocument::setWordWrap (bool on)
{
  config()->setWordWrap (on);
}

bool KateDocument::wordWrap ()
{
  return config()->wordWrap ();
}

void KateDocument::setWordWrapAt (uint col)
{
  config()->setWordWrapAt (col);
}

unsigned int KateDocument::wordWrapAt ()
{
  return config()->wordWrapAt ();
}

void KateDocument::applyWordWrap ()
{
  if (hasSelection())
    wrapText (selectStart.line(), selectEnd.line());
  else
    wrapText (0, lastLine());
}

void KateDocument::setPageUpDownMovesCursor (bool on)
{
  config()->setPageUpDownMovesCursor (on);
}

bool KateDocument::pageUpDownMovesCursor ()
{
  return config()->pageUpDownMovesCursor ();
}

void KateDocument::exportAs(const QString& filter)
{
  if (filter=="kate_html_export")
  {
    KURL url = KFileDialog::getSaveURL(QString::null,"text/html",0,i18n("Export File As"));
    if ( url.isEmpty() )
      return;

    QString filename;
    KTempFile tmp; // ### only used for network export

    if ( url.isLocalFile() )
      filename = url.path();
    else
      filename = tmp.name();

    KSaveFile *savefile=new KSaveFile(filename);
    if (!savefile->status())
    {
      if (exportDocumentToHTML(savefile->textStream(),filename))
        savefile->close();
      else savefile->abort();
      //if (!savefile->status()) --> Error
    }
//     else
//       {/*ERROR*/}
    delete savefile;

    if ( url.isLocalFile() )
        return;

    KIO::NetAccess::upload( filename, url, 0 );
  }
}

/* For now, this should become an plugin */
bool KateDocument::exportDocumentToHTML(QTextStream *outputStream,const QString &name)
{
  outputStream->setEncoding(QTextStream::UnicodeUTF8);
  // let's write the HTML header :
  (*outputStream) << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" << endl;
  (*outputStream) << "<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Strict//EN\" \"DTD/xhtml1-strict.dtd\">" << endl;
  (*outputStream) << "<html xmlns=\"http://www.w3.org/1999/xhtml\">" << endl;
  (*outputStream) << "<head>" << endl;
  (*outputStream) << "<meta http-equiv=\"Content-Type\" content=\"text/html; charset=UTF-8\" />" << endl;
  (*outputStream) << "<meta name=\"Generator\" content=\"Kate, the KDE Advanced Text Editor\" />" << endl;
  // for the title, we write the name of the file (/usr/local/emmanuel/myfile.cpp -> myfile.cpp)
  (*outputStream) << "<title>" << name.right(name.length() - name.findRev('/') -1) << "</title>" << endl;
  (*outputStream) << "</head>" << endl;

  (*outputStream) << "<body><pre>" << endl;
  // for each line :

  // some variables :
  bool previousCharacterWasBold = false;
  bool previousCharacterWasItalic = false;
  // when entering a new color, we'll close all the <b> & <i> tags,
  // for HTML compliancy. that means right after that font tag, we'll
  // need to reinitialize the <b> and <i> tags.
  bool needToReinitializeTags = false;
  QColor previousCharacterColor(0,0,0); // default color of HTML characters is black
  (*outputStream) << "<span style='color: #000000'>";

  for (uint curLine=0;curLine<numLines();curLine++)
  { // html-export that line :
    KateTextLine::Ptr textLine = m_buffer->plainLine(curLine);
    //ASSERT(textLine != NULL);
    // for each character of the line : (curPos is the position in the line)
    for (uint curPos=0;curPos<textLine->length();curPos++)
    {
      // atm hardcode default schema, later add selector to the exportAs methode :)
      QMemArray<KateAttribute> *attributes = m_highlight->attributes (0);
      KateAttribute* charAttributes = 0;

      if (textLine->attribute(curPos) < attributes->size())
        charAttributes = &attributes->at(textLine->attribute(curPos));
      else
        charAttributes = &attributes->at(0);

      //ASSERT(charAttributes != NULL);
      // let's give the color for that character :
      if ( (charAttributes->textColor() != previousCharacterColor))
      {  // the new character has a different color :
        // if we were in a bold or italic section, close it
        if (previousCharacterWasBold)
          (*outputStream) << "</b>";
        if (previousCharacterWasItalic)
          (*outputStream) << "</i>";

        // close the previous font tag :
        (*outputStream) << "</span>";
        // let's read that color :
        int red, green, blue;
        // getting the red, green, blue values of the color :
        charAttributes->textColor().rgb(&red, &green, &blue);
        (*outputStream) << "<span style='color: #"
              << ( (red < 0x10)?"0":"")  // need to put 0f, NOT f for instance. don't touch 1f.
              << QString::number(red, 16) // html wants the hex value here (hence the 16)
              << ( (green < 0x10)?"0":"")
              << QString::number(green, 16)
              << ( (blue < 0x10)?"0":"")
              << QString::number(blue, 16)
              << "'>";
        // we need to reinitialize the bold/italic status, since we closed all the tags
        needToReinitializeTags = true;
      }
      // bold status :
      if ( (needToReinitializeTags && charAttributes->bold()) ||
          (!previousCharacterWasBold && charAttributes->bold()) )
        // we enter a bold section
        (*outputStream) << "<b>";
      if ( !needToReinitializeTags && (previousCharacterWasBold && !charAttributes->bold()) )
        // we leave a bold section
        (*outputStream) << "</b>";

      // italic status :
      if ( (needToReinitializeTags && charAttributes->italic()) ||
           (!previousCharacterWasItalic && charAttributes->italic()) )
        // we enter an italic section
        (*outputStream) << "<i>";
      if ( !needToReinitializeTags && (previousCharacterWasItalic && !charAttributes->italic()) )
        // we leave an italic section
        (*outputStream) << "</i>";

      // write the actual character :
      (*outputStream) << HTMLEncode(textLine->getChar(curPos));

      // save status for the next character :
      previousCharacterWasItalic = charAttributes->italic();
      previousCharacterWasBold = charAttributes->bold();
      previousCharacterColor = charAttributes->textColor();
      needToReinitializeTags = false;
    }
    // finish the line :
    (*outputStream) << endl;
  }

  // Be good citizens and close our tags
  if (previousCharacterWasBold)
    (*outputStream) << "</b>";
  if (previousCharacterWasItalic)
    (*outputStream) << "</i>";

  // HTML document end :
  (*outputStream) << "</span>";  // i'm guaranteed a span is started (i started one at the beginning of the output).
  (*outputStream) << "</pre></body>";
  (*outputStream) << "</html>";
  // close the file :
  return true;
}

QString KateDocument::HTMLEncode(QChar theChar)
{
  switch (theChar.latin1())
  {
  case '>':
    return QString("&gt;");
  case '<':
    return QString("&lt;");
  case '&':
    return QString("&amp;");
  };
  return theChar;
}

Kate::ConfigPage *KateDocument::colorConfigPage (QWidget *p)
{
  return (Kate::ConfigPage*) new KateSchemaConfigPage (p, this);
}

Kate::ConfigPage *KateDocument::viewDefaultsConfigPage (QWidget *p)
{
  return (Kate::ConfigPage*) new KateViewDefaultsConfig(p);
}

Kate::ConfigPage *KateDocument::fontConfigPage (QWidget *p)
{
  return (Kate::ConfigPage*) new KateSchemaConfigPage ( p );
}

Kate::ConfigPage *KateDocument::indentConfigPage (QWidget *p)
{
  return (Kate::ConfigPage*) new KateIndentConfigTab(p);
}

Kate::ConfigPage *KateDocument::selectConfigPage (QWidget *p)
{
  return (Kate::ConfigPage*) new KateSelectConfigTab(p);
}

Kate::ConfigPage *KateDocument::editConfigPage (QWidget *p)
{
  return (Kate::ConfigPage*) new KateEditConfigTab(p);
}

Kate::ConfigPage *KateDocument::keysConfigPage (QWidget *p)
{
  return (Kate::ConfigPage*) new KateEditKeyConfiguration(p, this);
}

Kate::ConfigPage *KateDocument::hlConfigPage (QWidget *p)
{
  return (Kate::ConfigPage*) new KateHlConfigPage (p);
}

Kate::ConfigPage *KateDocument::saveConfigPage(QWidget *p)
{
  return (Kate::ConfigPage*) new KateSaveConfigTab(p);
}

Kate::ActionMenu *KateDocument::hlActionMenu (const QString& text, QObject* parent, const char* name)
{
  KateViewHighlightAction *menu = new KateViewHighlightAction (text, parent, name);
  menu->setWhatsThis(i18n("Here you can choose how the current document should be highlighted."));
  menu->updateMenu (this);

  return (Kate::ActionMenu *)menu;
}

Kate::ActionMenu *KateDocument::exportActionMenu (const QString& text, QObject* parent, const char* name)
{
  KateExportAction *menu = new KateExportAction (text, parent, name);
  menu->updateMenu (this);
  menu->setWhatsThis(i18n("This command allows you to export the current document"
    " with all highlighting information into a markup document, e.g. HTML."));
  return (Kate::ActionMenu *)menu;
}

void KateDocument::dumpRegionTree()
{
  m_buffer->foldingTree()->debugDump();
}

unsigned int KateDocument::getRealLine(unsigned int virtualLine)
{
  return m_buffer->lineNumber (virtualLine);
}

unsigned int KateDocument::getVirtualLine(unsigned int realLine)
{
  return m_buffer->lineVisibleNumber (realLine);
}

unsigned int KateDocument::visibleLines ()
{
  return m_buffer->countVisible ();
}

KateTextLine::Ptr KateDocument::kateTextLine(uint i)
{
  return m_buffer->line (i);
}

KateTextLine::Ptr KateDocument::plainKateTextLine(uint i)
{
  return m_buffer->plainLine (i);
}
//END

//BEGIN KTextEditor::CursorInterface stuff

KTextEditor::Cursor *KateDocument::createCursor ( )
{
  return new KateSuperCursor (this, false, 0, 0, this);
}

void KateDocument::tagArbitraryLines(KateView* view, KateSuperRange* range)
{
  if (view)
    view->tagLines(range->start(), range->end());
  else
    tagLines(range->start(), range->end());
}

//
// Spellchecking IN again
//
void KateDocument::spellcheck()
{
  if( !isReadWrite() || text().isEmpty())
    return;

  QString mt = mimeType()/*->name()*/;

  KSpell::SpellerType type = KSpell::Text;
  if ( mt == "text/x-tex" || mt == "text/x-latex" )
    type = KSpell::TeX;
  else if ( mt == "text/html" || mt == "text/xml" )
    type = KSpell::HTML;

  m_kspell = new KSpell( 0, i18n("Spellcheck"),
                         this, SLOT(ready(KSpell *)), 0, true, false, type );

  connect( m_kspell, SIGNAL(death()),
           this, SLOT(spellCleanDone()) );

  connect( m_kspell, SIGNAL(misspelling(const QString&, const QStringList&, unsigned int)),
           this, SLOT(misspelling(const QString&, const QStringList&, unsigned int)) );
  connect( m_kspell, SIGNAL(corrected(const QString&, const QString&, unsigned int)),
           this, SLOT(corrected(const QString&, const QString&, unsigned int)) );
  connect( m_kspell, SIGNAL(done(const QString&)),
           this, SLOT(spellResult(const QString&)) );
}

void KateDocument::ready(KSpell *)
{
  m_kspell->setProgressResolution( 1 );

  m_kspell->check( text() );

  kdDebug () << "SPELLING READY STATUS: " << m_kspell->status () << endl;
}

void KateDocument::locatePosition( uint pos, uint& line, uint& col )
{
  uint cnt = 0;

  line = col = 0;

  // Find pos  -- CHANGEME: store the last found pos's cursor
  //   and do these searched relative to that to
  //   (significantly) increase the speed of the spellcheck
  for( ; line < numLines() && cnt <= pos; line++ )
    cnt += lineLength(line) + 1;

  line--;
  col = pos - (cnt - lineLength(line)) + 1;
}

void KateDocument::misspelling( const QString& origword, const QStringList&, unsigned int pos )
{
  uint line, col;

  locatePosition( pos, line, col );

  if (activeView())
    activeView()->setCursorPositionInternal (line, col, 1);

  setSelection( line, col, line, col + origword.length() );
}

void KateDocument::corrected( const QString& originalword, const QString& newword, unsigned int pos )
{
  uint line, col;

  locatePosition( pos, line, col );

  removeText( line, col, line, col + originalword.length() );
  insertText( line, col, newword );
}

void KateDocument::spellResult( const QString& )
{
  clearSelection();
  m_kspell->cleanUp();
}

void KateDocument::spellCleanDone()
{
  KSpell::spellStatus status = m_kspell->status();

  if( status == KSpell::Error ) {
    KMessageBox::sorry( 0,
      i18n("ISpell could not be started. "
           "Please make sure you have ISpell "
           "properly configured and in your PATH."));
  } else if( status == KSpell::Crashed ) {
    KMessageBox::sorry( 0,
      i18n("ISpell seems to have crashed."));
  }

  delete m_kspell;
  m_kspell = 0;

  kdDebug () << "SPELLING END" << endl;
}
//END

void KateDocument::lineInfo (KateLineInfo *info, unsigned int line)
{
  m_buffer->lineInfo(info,line);
}

KateCodeFoldingTree *KateDocument::foldingTree ()
{
  return m_buffer->foldingTree();
}

void KateDocument::setEncoding (const QString &e)
{
  m_config->setEncoding(e);
}

QString KateDocument::encoding() const
{
  return m_config->encoding();
}

void KateDocument::updateConfig ()
{
  emit undoChanged ();
  tagAll();

  for (KateView * view = m_views.first(); view != 0L; view = m_views.next() )
  {
    view->updateDocumentConfig ();
  }

  // switch indenter if needed
  if (m_indenter->modeNumber() != m_config->indentationMode())
  {
    delete m_indenter;
    m_indenter = KateAutoIndent::createIndenter ( this, m_config->indentationMode() );
  }

  m_indenter->updateConfig();

  m_buffer->setTabWidth (config()->tabWidth());

  // plugins
  for (uint i=0; i<KateFactory::self()->plugins().count(); i++)
  {
    if (config()->plugin (i))
      loadPlugin (i);
    else
      unloadPlugin (i);
  }
}

//BEGIN Variable reader
// "local variable" feature by anders, 2003
/* TODO
      add config options (how many lines to read, on/off)
      add interface for plugins/apps to set/get variables
      add view stuff
*/
QRegExp KateDocument::kvLine = QRegExp("kate:(.*)");
QRegExp KateDocument::kvVar = QRegExp("([\\w\\-]+)\\s+([^;]+)");

void KateDocument::readVariables(bool onlyViewAndRenderer)
{
  if (!onlyViewAndRenderer)
    m_config->configStart();

  // views!
  KateView *v;
  for (v = m_views.first(); v != 0L; v= m_views.next() )
  {
    v->config()->configStart();
    v->renderer()->config()->configStart();
  }
  // read a number of lines in the top/bottom of the document
  for (uint i=0; i < QMIN( 9, numLines() ); ++i )
  {
    readVariableLine( textLine( i ), onlyViewAndRenderer );
  }
  if ( numLines() > 10 )
  {
    for ( uint i = QMAX(10, numLines() - 10); i < numLines(); ++i )
    {
      readVariableLine( textLine( i ), onlyViewAndRenderer );
    }
  }

  if (!onlyViewAndRenderer)
    m_config->configEnd();

  for (v = m_views.first(); v != 0L; v= m_views.next() )
  {
    v->config()->configEnd();
    v->renderer()->config()->configEnd();
  }
}

void KateDocument::readVariableLine( QString t, bool onlyViewAndRenderer )
{
  if ( kvLine.search( t ) > -1 )
  {
    QStringList vvl; // view variable names
    vvl << "dynamic-word-wrap" << "dynamic-word-wrap-indicators"
        << "line-numbers" << "icon-border" << "folding-markers"
        << "bookmark-sorting" << "auto-center-lines"
        << "icon-bar-color"
        // renderer
        << "background-color" << "selection-color"
        << "current-line-color" << "bracket-highlight-color"
        << "word-wrap-marker-color"
        << "font" << "font-size" << "scheme";
    int p( 0 );
    QString s = kvLine.cap(1);
    QString  var, val;
    while ( (p = kvVar.search( s, p )) > -1 )
    {
      p += kvVar.matchedLength();
      var = kvVar.cap( 1 );
      val = kvVar.cap( 2 ).stripWhiteSpace();
      bool state; // store booleans here
      int n; // store ints here

      // only apply view & renderer config stuff
      if (onlyViewAndRenderer)
      {
        if ( vvl.contains( var ) ) // FIXME define above
          setViewVariable( var, val );
      }
      else
      {
        // BOOL  SETTINGS
        if ( var == "word-wrap" && checkBoolValue( val, &state ) )
          setWordWrap( state ); // ??? FIXME CHECK
        else if ( var == "block-selection"  && checkBoolValue( val, &state ) )
          setBlockSelectionMode( state );
        // KateConfig::configFlags
        // FIXME should this be optimized to only a few calls? how?
        else if ( var == "auto-indent" && checkBoolValue( val, &state ) )
          m_config->setConfigFlags( KateDocumentConfig::cfAutoIndent, state );
        else if ( var == "backspace-indents" && checkBoolValue( val, &state ) )
          m_config->setConfigFlags( KateDocumentConfig::cfBackspaceIndents, state );
        else if ( var == "replace-tabs" && checkBoolValue( val, &state ) )
          m_config->setConfigFlags( KateDocumentConfig::cfReplaceTabsDyn, state );
        else if ( var == "remove-trailing-space" && checkBoolValue( val, &state ) )
          m_config->setConfigFlags( KateDocumentConfig::cfRemoveTrailingDyn, state );
        else if ( var == "wrap-cursor" && checkBoolValue( val, &state ) )
          m_config->setConfigFlags( KateDocumentConfig::cfWrapCursor, state );
        else if ( var == "auto-brackets" && checkBoolValue( val, &state ) )
          m_config->setConfigFlags( KateDocumentConfig::cfAutoBrackets, state );
        else if ( var == "persistent-selection" && checkBoolValue( val, &state ) )
          m_config->setConfigFlags( KateDocumentConfig::cfPersistent, state );
        else if ( var == "keep-selection" && checkBoolValue( val, &state ) )
          m_config->setConfigFlags( KateDocumentConfig::cfBackspaceIndents, state );
        else if ( var == "overwrite-mode" && checkBoolValue( val, &state ) )
          m_config->setConfigFlags( KateDocumentConfig::cfOvr, state );
        else if ( var == "keep-indent-profile" && checkBoolValue( val, &state ) )
          m_config->setConfigFlags( KateDocumentConfig::cfKeepIndentProfile, state );
        else if ( var == "keep-extra-spaces" && checkBoolValue( val, &state ) )
          m_config->setConfigFlags( KateDocumentConfig::cfKeepExtraSpaces, state );
        else if ( var == "tab-indents" && checkBoolValue( val, &state ) )
          m_config->setConfigFlags( KateDocumentConfig::cfTabIndents, state );
        else if ( var == "show-tabs" && checkBoolValue( val, &state ) )
          m_config->setConfigFlags( KateDocumentConfig::cfShowTabs, state );
        else if ( var == "space-indent" && checkBoolValue( val, &state ) )
          m_config->setConfigFlags( KateDocumentConfig::cfSpaceIndent, state );
        else if ( var == "smart-home" && checkBoolValue( val, &state ) )
          m_config->setConfigFlags( KateDocumentConfig::cfSmartHome, state );
        else if ( var == "replace-tabs-save" && checkBoolValue( val, &state ) )
          m_config->setConfigFlags( KateDocumentConfig::cfReplaceTabs, state );
        else if ( var == "replace-trailing-space-save" && checkBoolValue( val, &state ) )
          m_config->setConfigFlags( KateDocumentConfig::cfRemoveSpaces, state );
        else if ( var == "auto-insert-doxygen" && checkBoolValue( val, &state) )
          m_config->setConfigFlags( KateDocumentConfig::cfDoxygenAutoTyping, state);

        // INTEGER SETTINGS
        else if ( var == "tab-width" && checkIntValue( val, &n ) )
          m_config->setTabWidth( n );
        else if ( var == "indent-width"  && checkIntValue( val, &n ) )
          m_config->setIndentationWidth( n );
        else if ( var == "indent-mode" )
        {
          if ( checkIntValue( val, &n ) )
            m_config->setIndentationMode( n );
          else
            m_config->setIndentationMode( KateAutoIndent::modeNumber( val) );
        }
        else if ( var == "word-wrap-column" && n > 0  && checkIntValue( val, &n ) ) // uint, but hard word wrap at 0 will be no fun ;)
          m_config->setWordWrapAt( n );
        else if ( var == "undo-steps"  && n >= 0  && checkIntValue( val, &n ) )
          setUndoSteps( n );

        // STRING SETTINGS
        else if ( var == "eol" || var == "end-of-line" )
        {
          QStringList l;
          l << "unix" << "dos" << "mac";
          if ( (n = l.findIndex( val.lower() )) != -1 )
            m_config->setEol( n );
        }
        else if ( var == "encoding" )
          m_config->setEncoding( val );
        else if ( var == "syntax" || var == "hl" )
        {
          for ( uint i=0; i < hlModeCount(); i++ )
          {
            if ( hlModeName( i ) == val )
            {
              setHlMode( i );
              break;
            }
          }
        }

        // VIEW SETTINGS
        else if ( vvl.contains( var ) )
          setViewVariable( var, val );
        else
        {
          m_storedVariables.insert( var, val );
          emit variableChanged( var, val );
        }
      }
    }
  }
}

void KateDocument::setViewVariable( QString var, QString val )
{
  KateView *v;
  bool state;
  int n;
  QColor c;
  for (v = m_views.first(); v != 0L; v= m_views.next() )
  {
    if ( var == "dynamic-word-wrap" && checkBoolValue( val, &state ) )
      v->config()->setDynWordWrap( state );
    //else if ( var = "dynamic-word-wrap-indicators" )
    else if ( var == "line-numbers" && checkBoolValue( val, &state ) )
      v->config()->setLineNumbers( state );
    else if (var == "icon-border" && checkBoolValue( val, &state ) )
      v->config()->setIconBar( state );
    else if (var == "folding-markers" && checkBoolValue( val, &state ) )
      v->config()->setFoldingBar( state );
    else if ( var == "auto-center-lines" && checkIntValue( val, &n ) )
      v->config()->setAutoCenterLines( n ); // FIXME uint, > N ??
    else if ( var == "icon-bar-color" && checkColorValue( val, c ) )
      v->renderer()->config()->setIconBarColor( c );
    // RENDERER
    else if ( var == "background-color" && checkColorValue( val, c ) )
      v->renderer()->config()->setBackgroundColor( c );
    else if ( var == "selection-color" && checkColorValue( val, c ) )
      v->renderer()->config()->setSelectionColor( c );
    else if ( var == "current-line-color" && checkColorValue( val, c ) )
      v->renderer()->config()->setHighlightedLineColor( c );
    else if ( var == "bracket-highlight-color" && checkColorValue( val, c ) )
      v->renderer()->config()->setHighlightedBracketColor( c );
    else if ( var == "word-wrap-marker-color" && checkColorValue( val, c ) )
      v->renderer()->config()->setWordWrapMarkerColor( c );
    else if ( var == "font" || ( var == "font-size" && checkIntValue( val, &n ) ) )
    {
      QFont _f( *v->renderer()->config()->font(  ) );

      if ( var == "font" )
      {
        _f.setFamily( val );
        _f.setFixedPitch( QFont( val ).fixedPitch() );
      }
      else
        _f.setPointSize( n );

      v->renderer()->config()->setFont( _f );
    }
    else if ( var == "scheme" )
    {
      v->renderer()->config()->setSchema( KateFactory::self()->schemaManager()->number( val ) );
    }
  }
}

bool KateDocument::checkBoolValue( QString val, bool *result )
{
  val = val.stripWhiteSpace().lower();
  QStringList l;
  l << "1" << "on" << "true";
  if ( l.contains( val ) )
  {
    *result = true;
    return true;
  }
  l.clear();
  l << "0" << "off" << "false";
  if ( l.contains( val ) )
  {
    *result = false;
    return true;
  }
  return false;
}

bool KateDocument::checkIntValue( QString val, int *result )
{
  bool ret( false );
  *result = val.toInt( &ret );
  return ret;
}

bool KateDocument::checkColorValue( QString val, QColor &c )
{
  c.setNamedColor( val );
  return c.isValid();
}

// KTextEditor::variable
QString KateDocument::variable( const QString &name ) const
{
  if ( m_storedVariables.contains( name ) )
    return m_storedVariables[ name ];

  return "";
}

//END

void KateDocument::slotModOnHdDirty (const QString &path)
{
  if ((path == m_dirWatchFile) && (!m_modOnHd || m_modOnHdReason != 1))
  {
    // compare md5 with the one we have (if we have one)
    if ( ! m_digest.isEmpty() )
    {
      QCString tmp;
      if ( createDigest( tmp ) && tmp == m_digest )
        return;
    }

    m_modOnHd = true;
    m_modOnHdReason = 1;

    // reenable dialog if not running atm
    if (m_isasking == -1)
      m_isasking = false;

    emit modifiedOnDisc (this, m_modOnHd, m_modOnHdReason);
  }
}

void KateDocument::slotModOnHdCreated (const QString &path)
{
  if ((path == m_dirWatchFile) && (!m_modOnHd || m_modOnHdReason != 2))
  {
    m_modOnHd = true;
    m_modOnHdReason = 2;

    // reenable dialog if not running atm
    if (m_isasking == -1)
      m_isasking = false;

    emit modifiedOnDisc (this, m_modOnHd, m_modOnHdReason);
  }
}

void KateDocument::slotModOnHdDeleted (const QString &path)
{
  if ((path == m_dirWatchFile) && (!m_modOnHd || m_modOnHdReason != 3))
  {
    m_modOnHd = true;
    m_modOnHdReason = 3;

    // reenable dialog if not running atm
    if (m_isasking == -1)
      m_isasking = false;

    emit modifiedOnDisc (this, m_modOnHd, m_modOnHdReason);
  }
}

bool KateDocument::createDigest( QCString &result )
{
  bool ret = false;
  result = "";
  if ( url().isLocalFile() )
  {
    QFile f ( url().path() );
    if ( f.open( IO_ReadOnly) )
    {
      KMD5 md5;
      ret = md5.update( f );
      md5.hexDigest( result );
      f.close();
    }
  }
  return ret;
}

void KateDocument::removeTrailingSpace( uint line )
{
  // remove trailing spaces from left line if required
  if ( config()->configFlags() & KateDocumentConfig::cfRemoveTrailingDyn )
  {
    KateTextLine::Ptr ln = kateTextLine( line );

    if ( ! ln ) return;

    if ( line == activeView()->cursorLine()
         && activeView()->cursorColumnReal() >= (uint)QMAX(0,ln->lastChar()) )
      return;

    if ( ln->length() )
    {
      uint p = ln->lastChar() + 1;
      uint l = ln->length() - p;
      if ( l )
        editRemoveText( line, p, l);
    }
  }
}

bool KateDocument::wrapCursor ()
{
  return !blockSelect && (configFlags() & KateDocument::cfWrapCursor);
}

void KateDocument::updateFileType (int newType, bool user)
{
  if (user || !m_fileTypeSetByUser)
  {
    const KateFileType *t = 0;
    if ((newType == -1) || (t = KateFactory::self()->fileTypeManager()->fileType (newType)))
    {
      m_fileType = newType;

      if (t)
      {
        m_config->configStart();
        // views!
        KateView *v;
        for (v = m_views.first(); v != 0L; v= m_views.next() )
        {
          v->config()->configStart();
          v->renderer()->config()->configStart();
        }

        readVariableLine( t->varLine );

        m_config->configEnd();
        for (v = m_views.first(); v != 0L; v= m_views.next() )
        {
          v->config()->configEnd();
          v->renderer()->config()->configEnd();
        }
      }
    }
  }
}

uint KateDocument::documentNumber () const
{
  return KTextEditor::Document::documentNumber ();
}




void KateDocument::slotQueryClose_save(bool *handled, bool* abortClosing) {
      *handled=true;
      *abortClosing=true;
      if (m_url.isEmpty())
      {
        KEncodingFileDialog::Result res=KEncodingFileDialog::getSaveURLAndEncoding(config()->encoding(),
                QString::null,QString::null,0,i18n("Save File"));

        if( res.URLs.isEmpty() || !checkOverwrite( res.URLs.first() ) ) {
                *abortClosing=true;
                return;
        }
        setEncoding( res.encoding );
          saveAs( res.URLs.first() );
        *abortClosing=false;
      }
      else
      {
          save();
          *abortClosing=false;
      }

}

bool KateDocument::checkOverwrite( KURL u )
{
  if( !u.isLocalFile() )
    return true;

  QFileInfo info( u.path() );
  if( !info.exists() )
    return true;

  return KMessageBox::Cancel != KMessageBox::warningContinueCancel( 0,
    i18n( "A file named \"%1\" already exists. "
          "Are you sure you want to overwrite it?" ).arg( info.fileName() ),
    i18n( "Overwrite File?" ),
    i18n( "&Overwrite" ) );
}

void KateDocument::setDefaultEncoding (const QString &encoding)
{
  s_defaultEncoding = encoding;
}

void KateDocument::setIMSelectionValue( uint imStartLine, uint imStart, uint imEnd,
                                        uint imSelStart, uint imSelEnd, bool imComposeEvent )
{
  m_imStartLine = imStartLine;
  m_imStart = imStart;
  m_imEnd = imEnd;
  m_imSelStart = imSelStart;
  m_imSelEnd = imSelEnd;
  m_imComposeEvent = imComposeEvent;
}

void KateDocument::getIMSelectionValue( uint *imStartLine, uint *imStart, uint *imEnd,
                                        uint *imSelStart, uint *imSelEnd )
{
  *imStartLine = m_imStartLine;
  *imStart = m_imStart;
  *imEnd = m_imEnd;
  *imSelStart = m_imSelStart;
  *imSelEnd = m_imSelEnd;
}

// kate: space-indent on; indent-width 2; replace-tabs on;
