/* This file is part of the KDE libraries
   Copyright (C) 2001-2004 Christoph Cullmann <cullmann@kde.org>
   Copyright (C) 2001 Joseph Wenninger <jowenn@kde.org>
   Copyright (C) 1999 Jochen Wilhelmy <digisnap@cs.tu-berlin.de>
   Copyright (C) 2006 Hamish Rodda <rodda@kde.org>
   Copyright (C) 2007 Mirko Stocker <me@misto.ch>
   Copyright (C) 2009 Michel Ludwig <michel.ludwig@kdemail.net>

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
   Boston, MA 02111-13020, USA.
*/

//BEGIN includes
#include "katedocument.h"
#include "katedocument.moc"
#include "kateglobal.h"
#include "katedialogs.h"
#include "katehighlight.h"
#include "kateview.h"
#include "kateautoindent.h"
#include "katetextline.h"
#include "katedocumenthelpers.h"
#include "kateprinter.h"
#include "katesmartcursor.h"
#include "katerenderer.h"
#include "kateregexp.h"
#include "kateplaintextsearch.h"
#include "kateescapedtextsearch.h"
#include "kateregexpsearch.h"
#include <ktexteditor/attribute.h>
#include "kateconfig.h"
#include "katemodemanager.h"
#include "kateschema.h"
#include "katetemplatehandler.h"
#include "katesmartmanager.h"
#include <ktexteditor/plugin.h>
#include <ktexteditor/loadsavefiltercheckplugin.h>
#include <ktexteditor/rangefeedback.h>
#include "kateedit.h"
#include "katebuffer.h"
#include "kateundomanager.h"
#include "katepartpluginmanager.h"
#include "katevireplacemode.h"
#include "spellcheck/prefixstore.h"
#include "spellcheck/ontheflycheck.h"
#include "spellcheck/spellcheck.h"

#include <kio/job.h>
#include <kio/jobuidelegate.h>
#include <kio/netaccess.h>
#include <kfileitem.h>

#include <kparts/event.h>

#include <klocale.h>
#include <kglobal.h>
#include <kapplication.h>
#include <kmenu.h>
#include <kconfig.h>
#include <kfiledialog.h>
#include <kmessagebox.h>
#include <kstandardaction.h>
#include <kxmlguifactory.h>
#include <kdebug.h>
#include <kglobalsettings.h>
#include <klibloader.h>
#include <kdirwatch.h>
#include <kencodingfiledialog.h>
#include <ktemporaryfile.h>
#include <kcodecs.h>
#include <kstandarddirs.h>
#include <kstringhandler.h>

#include <kservicetypetrader.h>

#include <QtDBus/QtDBus>
#include <QtCore/QTimer>
#include <QtCore/QFile>
#include <QtGui/QClipboard>
#include <QtCore/QTextStream>
#include <QtCore/QTextCodec>
#include <QtCore/QMap>
#include <QtCore/QMutex>
//END  includes


static int dummy = 0;


class KateDocument::LoadSaveFilterCheckPlugins
{
  public:
    LoadSaveFilterCheckPlugins() {
      KService::List traderList = KServiceTypeTrader::self()->query("KTextEditor/LoadSaveFilterCheckPlugin");

      foreach(const KService::Ptr &ptr, traderList)
      {
        QString libname;
        libname=ptr->library();
        libname=libname.right(libname.length()-12); //ktexteditor_ == 12
        m_plugins[libname]=0;//new KatePythonEncodingCheck();
      }

    }
    ~LoadSaveFilterCheckPlugins() {
      if ( m_plugins.count()==0) return;
      QHashIterator<QString,KTextEditor::LoadSaveFilterCheckPlugin*>i(m_plugins);
        while (i.hasNext())
          i.next();
          delete i.value();
      m_plugins.clear();
    }
    bool preSavePostDialogFilterCheck(const QString& pluginName,KateDocument *document,QWidget *parentWidget) {
      KTextEditor::LoadSaveFilterCheckPlugin *plug=getPlugin(pluginName);
      if (!plug) {
        if (KMessageBox::warningContinueCancel (parentWidget
        , i18n ("The filter/check plugin '%1' could not be found, still continue saving of %2", pluginName,document->url().pathOrUrl())
        , i18n ("Saving problems")
        , KGuiItem(i18n("Save Nevertheless"))
        , KStandardGuiItem::cancel()) != KMessageBox::Continue)
          return false;
        else
          return true;
      }
      return plug->preSavePostDialogFilterCheck(document);
    }
    void postLoadFilter(const QString& pluginName,KateDocument *document) {
      KTextEditor::LoadSaveFilterCheckPlugin *plug=getPlugin(pluginName);
      if (!plug) return;
      plug->postLoadFilter(document);
    }
    bool postSaveFilterCheck(const QString& pluginName,KateDocument *document,bool saveas) {
      KTextEditor::LoadSaveFilterCheckPlugin *plug=getPlugin(pluginName);
      if (!plug) return false;
      return plug->postSaveFilterCheck(document,saveas);
    }
  private:
    KTextEditor::LoadSaveFilterCheckPlugin *getPlugin(const QString & pluginName)
    {
      if (!m_plugins.contains(pluginName)) return 0;
      if (!m_plugins[pluginName]) {
        m_plugins[pluginName]=KLibLoader::createInstance<KTextEditor::LoadSaveFilterCheckPlugin>( "ktexteditor_"+pluginName);
      }
      return m_plugins[pluginName];
    }
    QHash <QString,KTextEditor::LoadSaveFilterCheckPlugin*> m_plugins;
};

//BEGIN d'tor, c'tor
//
// KateDocument Constructor
//
KateDocument::KateDocument ( bool bSingleViewMode, bool bBrowserView,
                             bool bReadOnly, QWidget *parentWidget,
                             QObject *parent)
: KTextEditor::Document (parent),
  m_activeView(0L),
  m_undoManager(new KateUndoManager(this)),
  m_annotationModel( 0 ),
  m_saveAs(false),
  m_indenter (new KateAutoIndent(this)),
  m_modOnHd (false),
  m_modOnHdReason (OnDiskUnmodified),
  s_fileChangedDialogsActivated (false),
  m_savingToUrl(false),
  m_onTheFlyChecker(NULL),
  m_dictionaryRangeNotifier(NULL)
{
  setComponentData ( KateGlobal::self()->componentData () );

  QString pathName ("/Kate/Document/%1");
  pathName = pathName.arg (++dummy);

  // my dbus object
  QDBusConnection::sessionBus().registerObject (pathName, this);

  // register doc at factory
  KateGlobal::self()->registerDocument(this);

  m_reloading = false;

  m_editHistory = new KateEditHistory(this);
  m_smartManager = new KateSmartManager(this);
  m_buffer = new KateBuffer(this);

  // init the config object, be careful not to use it
  // until the initial readConfig() call is done
  m_config = new KateDocumentConfig(this);

  // init some more vars !
  setActiveView(0L);

  hlSetByUser = false;
  m_bomSetByUser=false;
  m_fileTypeSetByUser = false;

  editSessionNumber = 0;
  editIsRunning = false;

  m_docNameNumber = 0;
  m_docName = "need init";

  m_bSingleViewMode = bSingleViewMode;
  m_bBrowserView = bBrowserView;
  m_bReadOnly = bReadOnly;

  setEditableMarks( markType01 );

  clearMarks ();
  setModified (false);

  // normal hl
  m_buffer->setHighlight (0);

  m_blockRemoveTrailingSpaces = false;
  m_extension = new KateBrowserExtension( this );

  // important, fill in the config into the indenter we use...
  m_indenter->updateConfig ();

  // some nice signals from the buffer
  connect(m_buffer, SIGNAL(tagLines(int,int)), this, SLOT(tagLines(int,int)));
  connect(m_buffer, SIGNAL(respellCheckBlock(int, int)), this , SLOT(respellCheckBlock(int, int)));
  connect(m_buffer, SIGNAL(codeFoldingUpdated()),this,SIGNAL(codeFoldingUpdated()));

  // if the user changes the highlight with the dialog, notify the doc
  connect(KateHlManager::self(),SIGNAL(changed()),SLOT(internalHlChanged()));

  // signals for mod on hd
  connect( KateGlobal::self()->dirWatch(), SIGNAL(dirty (const QString &)),
           this, SLOT(slotModOnHdDirty (const QString &)) );

  connect( KateGlobal::self()->dirWatch(), SIGNAL(created (const QString &)),
           this, SLOT(slotModOnHdCreated (const QString &)) );

  connect( KateGlobal::self()->dirWatch(), SIGNAL(deleted (const QString &)),
           this, SLOT(slotModOnHdDeleted (const QString &)) );

  connect (this,SIGNAL(completed()),this,SLOT(slotCompleted()));
  connect (this,SIGNAL(canceled(const QString&)),this,SLOT(slotCanceled()));
  // update doc name
  setDocName (QString());

  // if single view mode, like in the konqui embedding, create a default view ;)
  // be lazy, only create it now, if any parentWidget is given, otherwise widget()
  // will create it on demand...
  if ( m_bSingleViewMode && parentWidget )
  {
    KTextEditor::View *view = (KTextEditor::View*)createView( parentWidget );
    insertChildClient( view );
    view->show();
    setWidget( view );
  }

  connect(this,SIGNAL(sigQueryClose(bool *, bool*)),this,SLOT(slotQueryClose_save(bool *, bool*)));

  m_isasking = 0;

  onTheFlySpellCheckingEnabled(config()->onTheFlySpellCheck());

  // register document in plugins
  KatePartPluginManager::self()->addDocument(this);
}

//
// KateDocument Destructor
//
KateDocument::~KateDocument()
{
  // delete it here because it has to be deleted before the SmartMutex is destroyed
  delete m_onTheFlyChecker;
  m_onTheFlyChecker = NULL;

  clearDictionaryRanges();
  deleteDiscardedSmartRanges();

  // Tell the world that we're about to close (== destruct)
  // Apps must receive this in a direct signal-slot connection, and prevent
  // any further use of interfaces once they return.
  emit aboutToClose(this);

  // remove file from dirwatch
  deactivateDirWatch ();

  // thanks for offering, KPart, but we're already self-destructing
  setAutoDeleteWidget(false);
  setAutoDeletePart(false);

  // clean up remaining views
  while (!m_views.isEmpty()) {
    delete m_views.takeFirst();
  }

  // de-register from plugin
  KatePartPluginManager::self()->removeDocument(this);

  // cu marks
  for (QHash<int, KTextEditor::Mark*>::const_iterator i = m_marks.constBegin(); i != m_marks.constEnd(); ++i)
    delete i.value();
  m_marks.clear();

  delete m_config;
  KateGlobal::self()->deregisterDocument (this);
}
//END

// on-demand view creation
QWidget *KateDocument::widget()
{
  // no singleViewMode -> no widget()...
  if (!singleViewMode())
    return 0;

  // does a widget exist already? use it!
  if (KTextEditor::Document::widget())
    return KTextEditor::Document::widget();

  // create and return one...
  KTextEditor::View *view = (KTextEditor::View*)createView(0);
  insertChildClient( view );
  setWidget( view );
  return view;
}

//BEGIN KTextEditor::Document stuff

KTextEditor::View *KateDocument::createView( QWidget *parent )
{
  KateView* newView = new KateView( this, parent);
  if ( s_fileChangedDialogsActivated )
    connect( newView, SIGNAL(focusIn( KTextEditor::View * )), this, SLOT(slotModifiedOnDisk()) );

  emit viewCreated (this, newView);

  return newView;
}

const QList<KTextEditor::View*> &KateDocument::views () const
{
  return m_textEditViews;
}

KTextEditor::Editor *KateDocument::editor ()
{
  return KateGlobal::self();
}

KTextEditor::Range KateDocument::rangeOnLine(KTextEditor::Range range, int line) const
{
  int col1 = const_cast<KateDocument*>(this)->toVirtualColumn(range.start());
  int col2 = const_cast<KateDocument*>(this)->toVirtualColumn(range.end());

  KateTextLine::Ptr tl = const_cast<KateDocument*>(this)->kateTextLine(line);
  col1 = tl->fromVirtualColumn(col1, config()->tabWidth());
  col2 = tl->fromVirtualColumn(col2, config()->tabWidth());
  
  return KTextEditor::Range(line, col1, line, col2);
}

//BEGIN KTextEditor::EditInterface stuff

QString KateDocument::text() const
{
  QString s;

  for (int i = 0; i < m_buffer->count(); i++)
  {
    KateTextLine::Ptr textLine = m_buffer->plainLine(i);

    if (textLine)
    {
      s.append (textLine->string());

      if ((i+1) < m_buffer->count())
        s.append(QChar::fromAscii('\n'));
    }
  }

  return s;
}

QString KateDocument::text( const KTextEditor::Range& range, bool blockwise ) const
{
  if (!range.isValid()) {
    kWarning() << k_funcinfo << "Text requested for invalid range" << range;
    return QString();
  }

  if ( blockwise && (range.start().column() > range.end().column()) )
    return QString ();

  QString s;

  if (range.start().line() == range.end().line())
  {
    if (range.start().column() > range.end().column())
      return QString ();

    KateTextLine::Ptr textLine = m_buffer->plainLine(range.start().line());

    if ( !textLine )
      return QString ();

    return textLine->string(range.start().column(), range.end().column()-range.start().column());
  }
  else
  {

    for (int i = range.start().line(); (i <= range.end().line()) && (i < m_buffer->count()); ++i)
    {
      KateTextLine::Ptr textLine = m_buffer->plainLine(i);

      if ( !blockwise )
      {
        if (i == range.start().line())
          s.append (textLine->string(range.start().column(), textLine->length()-range.start().column()));
        else if (i == range.end().line())
          s.append (textLine->string(0, range.end().column()));
        else
          s.append (textLine->string());
      }
      else
      {
        KTextEditor::Range subRange = rangeOnLine(range, i);
        s.append(textLine->string(subRange.start().column(), subRange.columnWidth()));
      }

      if ( i < range.end().line() )
        s.append(QChar::fromAscii('\n'));
    }
  }

  return s;
}

QChar KateDocument::character( const KTextEditor::Cursor & position ) const
{
  KateTextLine::Ptr textLine = m_buffer->plainLine(position.line());

  if ( !textLine )
    return QChar();

  if (position.column() >= 0 && position.column() < textLine->string().length())
    return textLine->string().at(position.column());

  return QChar();
}

QStringList KateDocument::textLines( const KTextEditor::Range & range, bool blockwise ) const
{
  QStringList ret;

  if (!range.isValid()) {
    kWarning() << k_funcinfo << "Text requested for invalid range" << range;
    return ret;
  }

  if ( blockwise && (range.start().column() > range.end().column()) )
    return ret;

  if (range.start().line() == range.end().line())
  {
    Q_ASSERT(range.start() <= range.end());

    KateTextLine::Ptr textLine = m_buffer->plainLine(range.start().line());

    if ( !textLine )
      return ret;

    ret << textLine->string(range.start().column(), range.end().column() - range.start().column());
  }
  else
  {
    for (int i = range.start().line(); (i <= range.end().line()) && (i < m_buffer->count()); ++i)
    {
      KateTextLine::Ptr textLine = m_buffer->plainLine(i);

      if ( !blockwise )
      {
        if (i == range.start().line())
          ret << textLine->string(range.start().column(), textLine->length() - range.start().column());
        else if (i == range.end().line())
          ret << textLine->string(0, range.end().column());
        else
          ret << textLine->string();
      }
      else
      {
        KTextEditor::Range subRange = rangeOnLine(range, i);
        ret << textLine->string(subRange.start().column(), subRange.columnWidth());
      }
    }
  }

  return ret;
}

QString KateDocument::line( int line ) const
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

  QList<KTextEditor::Mark> msave;

  foreach (KTextEditor::Mark* mark, m_marks)
    msave.append(*mark);

  editStart ();

  // delete the text
  clear();

  // insert the new text
  insertText (KTextEditor::Cursor(), s);

  editEnd ();

  foreach (const KTextEditor::Mark& mark, msave)
    setMark (mark.line, mark.type);

  return true;
}

bool KateDocument::setText( const QStringList & text )
{
  if (!isReadWrite())
    return false;

  QList<KTextEditor::Mark> msave;

  foreach (KTextEditor::Mark* mark, m_marks)
    msave.append(*mark);

  editStart ();

  // delete the text
  clear();

  // insert the new text
  insertText (KTextEditor::Cursor::start(), text);

  editEnd ();

  foreach (const KTextEditor::Mark& mark, msave)
    setMark (mark.line, mark.type);

  return true;
}

bool KateDocument::clear()
{
  if (!isReadWrite())
    return false;

  foreach (KateView *view, m_views) {
    view->clear();
    view->tagAll();
    view->update();
  }

  clearMarks ();

  return removeText (KTextEditor::Range(KTextEditor::Cursor(), KTextEditor::Cursor(lastLine()+1, 0)));
}

bool KateDocument::insertText( const KTextEditor::Cursor& position, const QString& text, bool block )
{
  if (!isReadWrite())
    return false;

  if (text.isEmpty())
    return true;

  editStart();

  int currentLine = position.line();
  int currentLineStart = 0;
  int totalLength = text.length();
  int insertColumn = position.column();

  if (position.line() > lines())
  {
    int line = lines();
    while (line != position.line() + totalLength + 1)
    {
      editInsertLine(line,QString());
      line++;
    }
  }

  bool replacetabs = ( config()->configFlags() & KateDocumentConfig::cfReplaceTabsDyn );
  int tabWidth = config()->tabWidth();

  static const QChar newLineChar('\n');
  static const QChar tabChar('\t');
  static const QChar spaceChar(' ');

  int insertColumnExpanded = insertColumn;
  KateTextLine::Ptr l = kateTextLine( currentLine );
  if (l)
    insertColumnExpanded = l->toVirtualColumn( insertColumn, tabWidth );

  int pos = 0;
  for (; pos < totalLength; pos++)
  {
    const QChar& ch = text.at(pos);

    if (ch == newLineChar)
    {
      // Only perform the text insert if there is text to insert
      if (currentLineStart < pos)
        editInsertText(currentLine, insertColumn, text.mid(currentLineStart, pos - currentLineStart));

      if ( !block )
      {
        editWrapLine(currentLine, insertColumn + pos - currentLineStart);
        insertColumn = 0;
      }
      else
      {
        if ( currentLine == lastLine() )
          editWrapLine(currentLine , insertColumn + pos - currentLineStart);
        insertColumn = position.column(); // tab expansion might change this
      }

      currentLine++;
      currentLineStart = pos + 1;
      l = kateTextLine( currentLine );
      if (l)
        insertColumnExpanded = l->toVirtualColumn( insertColumn, tabWidth );
    }
    else
    {
      if ( replacetabs && ch == tabChar )
      {
        int spacesRequired = tabWidth - ( (insertColumnExpanded + pos - currentLineStart) % tabWidth );
        editInsertText(currentLine, insertColumn, text.mid(currentLineStart, pos - currentLineStart) + QString(spacesRequired, spaceChar));

        insertColumn += pos - currentLineStart + spacesRequired;
        currentLineStart = pos + 1;
        l = kateTextLine( currentLine );
        if (l)
          insertColumnExpanded = l->toVirtualColumn( insertColumn, tabWidth );
      }
    }
  }

  // Only perform the text insert if there is text to insert
  if (currentLineStart < pos)
    editInsertText(currentLine, insertColumn, text.mid(currentLineStart, pos - currentLineStart));

  editEnd();
  return true;
}

bool KateDocument::insertText( const KTextEditor::Cursor & position, const QStringList & textLines, bool block )
{
  if (!isReadWrite())
    return false;

  if (textLines.isEmpty() || (textLines.count() == 1 && textLines.first().isEmpty()))
    return true;

  // FIXME - huh, contradicted below
  if (position.line() > lines())
    return false;

  editStart();

  if (position.line() > lines())
    editInsertLine(position.line(),QString());

  int currentLine = position.line();
  int currentLineStart = 0;
  int insertColumn = position.column();

  bool replacetabs = ( config()->configFlags() & KateDocumentConfig::cfReplaceTabsDyn );
  int tabWidth = config()->tabWidth();

  static const QChar newLineChar('\n');
  static const QChar tabChar('\t');
  static const QChar spaceChar(' ');

  int insertColumnExpanded = insertColumn;
  KateTextLine::Ptr l = kateTextLine( currentLine );
  if (l)
    insertColumnExpanded = l->toVirtualColumn( insertColumn, tabWidth );

  foreach (const QString &text, textLines)
  {
    int pos = 0;
    for (; pos < text.length(); pos++)
    {
      const QChar& ch = text.at(pos);

      if (ch == newLineChar)
      {
        // Only perform the text insert if there is text to insert
        if (currentLineStart < pos)
          editInsertText(currentLine, insertColumn, text.mid(currentLineStart, pos - currentLineStart));

        if ( !block )
        {
          editWrapLine(currentLine, pos + insertColumn);
          insertColumn = 0;
        }
        else
        {
          if ( currentLine == lastLine() )
            editWrapLine(currentLine , insertColumn + pos - currentLineStart);
          insertColumn = position.column(); // tab expansion might change this
        }

        currentLine++;
        currentLineStart = pos + 1;
        l = kateTextLine( currentLine );
        if (l)
          insertColumnExpanded = l->toVirtualColumn( insertColumn, tabWidth );
      }
      else
      {
        if ( replacetabs && ch == tabChar )
        {
          int spacesRequired = tabWidth - ( (insertColumnExpanded + pos - currentLineStart) % tabWidth );
          editInsertText(currentLine, insertColumn, text.mid(currentLineStart, pos - currentLineStart) + QString(spacesRequired, spaceChar));

          insertColumn += pos - currentLineStart + spacesRequired;
          l = kateTextLine( currentLine );
          if (l)
            insertColumnExpanded = l->toVirtualColumn( insertColumn, tabWidth );
          currentLineStart = pos + 1;
        }
      }
    }

    // Only perform the text insert if there is text to insert
    if (currentLineStart < pos - currentLineStart)
      editInsertText(currentLine, insertColumn, text.mid(currentLineStart, pos - currentLineStart));
  }

  editEnd();
  return true;
}


bool KateDocument::removeText ( const KTextEditor::Range &_range, bool block )
{
  KTextEditor::Range range = _range;

  if (!isReadWrite())
    return false;

  if ( block && (range.start().column() > range.end().column()) )
    return false;

  // Should now be impossible to trigger with the new Range class
  Q_ASSERT( range.start().line() <= range.end().line() );

  if ( range.start().line() > lastLine() )
    return false;

  if (!block)
    emit aboutToRemoveText(range);

  editStart();

  if ( !block )
  {
    if ( range.end().line() > lastLine() )
    {
      range.end().setPosition(lastLine()+1, 0);
    }

    if (range.onSingleLine())
    {
      editRemoveText(range.start().line(), range.start().column(), range.columnWidth());
    }
    else
    {
      int from = range.start().line();
      int to = range.end().line();

      if (to <= lastLine())
        editRemoveText(to, 0, range.end().column());

      if (range.start().column() == 0 && from > 0)
        --from;

      editRemoveLines(from+1, to-1);

      if (range.start().column() > 0 || range.start().line() == 0) {
        editRemoveText(from, range.start().column(), m_buffer->plainLine(from)->length() - range.start().column());
        editUnWrapLine(from);
      }
    }
  } // if ( ! block )
  else
  {
    int startLine = qMax(0, range.start().line());
    for (int line = qMin(range.end().line(), lastLine()); line >= startLine; --line)
      editRemoveText(line, range.start().column(), range.end().column() - range.start().column());
  }

  editEnd ();
  emit textRemoved();
  return true;
}

bool KateDocument::insertLine( int l, const QString &str )
{
  if (!isReadWrite())
    return false;

  if (l < 0 || l > lines())
    return false;

  return editInsertLine (l, str);
}

bool KateDocument::insertLines( int line, const QStringList & text )
{
  if (!isReadWrite())
    return false;

  if (line < 0 || line > lines())
    return false;

  bool success = true;
  foreach (const QString &string, text)
    // FIXME assumes no \n in each string
    success &= editInsertLine (line++, string);

  return success;
}

bool KateDocument::removeLine( int line )
{
  if (!isReadWrite())
    return false;

  if (line < 0 || line > lastLine())
    return false;

  return editRemoveLine (line);
}

int KateDocument::totalCharacters() const
{
  int l = 0;

  for (int i = 0; i < m_buffer->count(); ++i)
  {
    KateTextLine::Ptr line = m_buffer->plainLine(i);

    if (line)
      l += line->length();
  }

  return l;
}

int KateDocument::lines() const
{
  return m_buffer->count();
}

int KateDocument::numVisLines() const
{
  return m_buffer->countVisible ();
}

int KateDocument::lineLength ( int line ) const
{
  if (line < 0 || line > lastLine())
    return -1;

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
void KateDocument::editStart (Kate::EditSource editSource)
{
  editSessionNumber++;

  if (editSource == Kate::NoEditSource)
    m_editSources.push(m_editSources.isEmpty() ? Kate::UserInputEdit : m_editSources.top());
  else
    m_editSources.push(editSource);

  // Unlocked in editEnd
  smartMutex()->lock();

  if (editSessionNumber > 1)
    return;

  editIsRunning = true;

  m_undoManager->editStart();

  foreach(KateView *view,m_views)
  {
    view->editStart ();
  }

  m_buffer->editStart ();
}

KateUndoManager *KateDocument::undoManager() {
  return m_undoManager;
}

void KateDocument::undoSafePoint() {
  m_undoManager->undoSafePoint();
}

//
// End edit session and update Views
//
void KateDocument::editEnd ()
{
  if (editSessionNumber == 0) {
    //Dangerous, because bad editStart() editEnd() mismatches defeat the smart-lock protection,
    //which can lead to random crashes
    Q_ASSERT(0);
    return;
  }

  // wrap the new/changed text, if something really changed!
  if (m_buffer->editChanged() && (editSessionNumber == 1))
    if (m_undoManager->isUndoTrackingEnabled() && config()->wordWrap())
      wrapText (m_buffer->editTagStart(), m_buffer->editTagEnd());

  editSessionNumber--;

  m_editSources.pop();

  // Was locked in editStart
  smartMutex()->unlock();

  if (editSessionNumber > 0)
    return;

  // end buffer edit, will trigger hl update
  // this will cause some possible adjustment of tagline start/end
  m_buffer->editEnd ();

  m_undoManager->editEnd();

  // edit end for all views !!!!!!!!!
  foreach(KateView *view, m_views)
    view->editEnd (m_buffer->editTagStart(), m_buffer->editTagEnd(), m_buffer->editTagFrom());

  if (m_buffer->editChanged())
  {
    setModified(true);
    emit textChanged (this);
  }

  editIsRunning = false;
}

void KateDocument::pushEditState ()
{
  editStateStack.push(editSessionNumber);
}

void KateDocument::popEditState ()
{
  if (editStateStack.isEmpty()) return;

  int count = editStateStack.pop() - editSessionNumber;
  while (count < 0) { ++count; editEnd(); }
  while (count > 0) { --count; editStart(); }
}

void KateDocument::inputMethodStart()
{
  m_undoManager->inputMethodStart();
}

void KateDocument::inputMethodEnd()
{
  m_undoManager->inputMethodEnd();
}

bool KateDocument::wrapText(int startLine, int endLine)
{
  if (startLine < 0 || endLine < 0)
    return false;

  if (!isReadWrite())
    return false;

  int col = config()->wordWrapAt();

  if (col == 0)
    return false;

  editStart ();

  for (int line = startLine; (line <= endLine) && (line < lines()); line++)
  {
    KateTextLine::Ptr l = kateTextLine(line);

    if (!l)
      return false;

    kDebug (13020) << "try wrap line: " << line;

    if (l->virtualLength(m_buffer->tabWidth()) > col)
    {
      KateTextLine::Ptr nextl = kateTextLine(line+1);

      kDebug (13020) << "do wrap line: " << line;

      int eolPosition = l->length()-1;

      // take tabs into account here, too
      int x = 0;
      const QString & t = l->string();
      int z2 = 0;
      for ( ; z2 < l->length(); z2++)
      {
        static const QChar tabChar('\t');
        if (t.at(z2) == tabChar)
          x += m_buffer->tabWidth() - (x % m_buffer->tabWidth());
        else
          x++;

        if (x > col)
          break;
      }

      int searchStart = qMin (z2, l->length()-1);

      // If where we are wrapping is an end of line and is a space we don't
      // want to wrap there
      if (searchStart == eolPosition && t.at(searchStart).isSpace())
        searchStart--;

      // Scan backwards looking for a place to break the line
      // We are not interested in breaking at the first char
      // of the line (if it is a space), but we are at the second
      // anders: if we can't find a space, try breaking on a word
      // boundary, using KateHighlight::canBreakAt().
      // This could be a priority (setting) in the hl/filetype/document
      int z = 0;
      int nw = 0; // alternative position, a non word character
      for (z=searchStart; z > 0; z--)
      {
        if (t.at(z).isSpace()) break;
        if ( ! nw && highlight()->canBreakAt( t.at(z) , l->attribute(z) ) )
        nw = z;
      }

      bool removeTrailingSpace = false;
      if (z > 0)
      {
        // So why don't we just remove the trailing space right away?
        // Well, the (view's) cursor may be directly in front of that space
        // (user typing text before the last word on the line), and if that
        // happens, the cursor would be moved to the next line, which is not
        // what we want (bug #106261)
        z++;
        removeTrailingSpace = true;
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
        if (nextl && (nextl->length() > 0) && !nextl->at(0).isSpace() && ((l->length() < 1) || !l->at(l->length()-1).isSpace()))
          editInsertText (line+1, 0, QString (" "));

        bool newLineAdded = false;
        editWrapLine (line, z, false, &newLineAdded);

        editMarkLineAutoWrapped (line+1, true);

        endLine++;
      }

      if (removeTrailingSpace) {
        // cu space
        editRemoveText (line, z - 1, 1);
      }
    }
  }

  editEnd ();

  return true;
}

bool KateDocument::editInsertText ( int line, int col, const QString &s, Kate::EditSource editSource )
{
  if (line < 0 || col < 0)
    return false;

  if (!isReadWrite())
    return false;

  KateTextLine::Ptr l = kateTextLine(line);

  if (!l)
    return false;

  editStart (editSource);

  QString s2 = s;
  int col2 = col;
  if (col2 > l->length()) {
    s2 = QString(col2 - l->length(), QLatin1Char(' ')) + s;
    col2 = l->length();
  }

  m_undoManager->slotTextInserted(line, col2, s2);

  l->insertText (col2, s2);

  m_buffer->changeLine(line);

  history()->doEdit( new KateEditInfo(m_editSources.top(), KTextEditor::Range(line, col2, line, col),
              QStringList(), KTextEditor::Range(line, col2, line, col2 + s2.length()), QStringList(s2)) );
  emit KTextEditor::Document::textInserted(this, KTextEditor::Range(line, col2, line, col2 + s2.length()));

  editEnd();

  return true;
}

bool KateDocument::editRemoveText ( int line, int col, int len, Kate::EditSource editSource )
{
  if (line < 0 || col < 0 || len < 0)
    return false;

  if (!isReadWrite())
    return false;

  KateTextLine::Ptr l = kateTextLine(line);

  if (!l)
    return false;

  editStart (editSource);

  m_undoManager->slotTextRemoved(line, col, l->string().mid(col, len));

  l->removeText (col, len);
  removeTrailingSpace( line );

  m_buffer->changeLine(line);

  history()->doEdit( new KateEditInfo(m_editSources.top(), KTextEditor::Range(line, col, line, col + len), QStringList(l->string().mid(col, len)), KTextEditor::Range(line, col, line, col), QStringList()) );
  emit KTextEditor::Document::textRemoved(this, KTextEditor::Range(line, col, line, col + len));

  editEnd ();

  return true;
}

bool KateDocument::editMarkLineAutoWrapped ( int line, bool autowrapped )
{
  if (line < 0)
    return false;

  if (!isReadWrite())
    return false;

  KateTextLine::Ptr l = kateTextLine(line);

  if (!l)
    return false;

  editStart ();

  m_undoManager->slotMarkLineAutoWrapped(line, autowrapped);

  l->setAutoWrapped (autowrapped);

  m_buffer->changeLine(line);

  editEnd ();

  return true;
}

bool KateDocument::editWrapLine ( int line, int col, bool newLine, bool *newLineAdded)
{
  if (line < 0 || col < 0)
    return false;

  if (!isReadWrite())
    return false;

  KateTextLine::Ptr l = kateTextLine(line);

  if (!l)
    return false;

  editStart ();

  KateTextLine::Ptr nextLine = kateTextLine(line+1);

  int pos = l->length() - col;

  if (pos < 0)
    pos = 0;

  m_undoManager->slotLineWrapped(line, col, pos, (!nextLine || newLine));

  if (!nextLine || newLine)
  {
    KateTextLine::Ptr textLine(new KateTextLine());

    textLine->insertText (0, l->string().mid(col, pos));
    l->truncate(col);

    m_buffer->insertLine (line+1, textLine);
    m_buffer->changeLine(line);

    QList<KTextEditor::Mark*> list;
    for (QHash<int, KTextEditor::Mark*>::const_iterator i = m_marks.constBegin(); i != m_marks.constEnd(); ++i)
    {
      if( i.value()->line >= line )
      {
        if ((col == 0) || (i.value()->line > line))
          list.append( i.value() );
      }
    }

    for( int i=0; i < list.size(); ++i )
      m_marks.take( list[i]->line );

    for( int i=0; i < list.size(); ++i )
    {
      list[i]->line++;
      m_marks.insert( list[i]->line, list[i] );
    }

    if( !list.isEmpty() )
      emit marksChanged( this );

    // yes, we added a new line !
    if (newLineAdded)
      (*newLineAdded) = true;

    history()->doEdit( new KateEditInfo(m_editSources.top(), KTextEditor::Range(line, col, line, col), QStringList(), KTextEditor::Range(line, col, line+1, 0), QStringList()) );
  }
  else
  {
    nextLine->insertText (0, l->string().mid(col, pos));
    l->truncate(col);

    m_buffer->changeLine(line);
    m_buffer->changeLine(line+1);

    // no, no new line added !
    if (newLineAdded)
      (*newLineAdded) = false;

    history()->doEdit( new KateEditInfo(m_editSources.top(), KTextEditor::Range(line, col, line+1, 0), QStringList(), KTextEditor::Range(line, col, line+1, pos), QStringList()) );
  }

  emit KTextEditor::Document::textInserted(this, KTextEditor::Range(line, col, line+1, pos));

  editEnd ();

  return true;
}

bool KateDocument::editUnWrapLine ( int line, bool removeLine, int length )
{
  if (line < 0 || length < 0)
    return false;

  if (!isReadWrite())
    return false;

  KateTextLine::Ptr l = kateTextLine(line);
  KateTextLine::Ptr nextLine = kateTextLine(line+1);

  if (!l || !nextLine)
    return false;

  editStart ();

  int col = l->length ();

  m_undoManager->slotLineUnWrapped(line, col, length, removeLine);

  if (removeLine)
  {
    l->insertText (col, nextLine->string());

    m_buffer->changeLine(line);
    m_buffer->removeLine(line+1);
  }
  else
  {
    l->insertText (col, nextLine->string().left((nextLine->length() < length) ? nextLine->length() : length));
    nextLine->removeText (0, (nextLine->length() < length) ? nextLine->length() : length);

    m_buffer->changeLine(line);
    m_buffer->changeLine(line+1);
  }

  QList<KTextEditor::Mark*> list;
  for (QHash<int, KTextEditor::Mark*>::const_iterator i = m_marks.constBegin(); i != m_marks.constEnd(); ++i)
  {
    if( i.value()->line >= line+1 )
      list.append( i.value() );

    if ( i.value()->line == line+1 )
    {
      KTextEditor::Mark* mark = m_marks.take( line );

      if (mark)
      {
        i.value()->type |= mark->type;
      }
    }
  }

   for( int i=0; i < list.size(); ++i )
      m_marks.take( list[i]->line );

   for( int i=0; i < list.size(); ++i )
   {
      list[i]->line--;
      m_marks.insert( list[i]->line, list[i] );
    }

  if( !list.isEmpty() )
    emit marksChanged( this );

  history()->doEdit( new KateEditInfo(m_editSources.top(), KTextEditor::Range(line, col, line+1, 0), QStringList(QString()), KTextEditor::Range(line, col, line, col), QStringList()) );
  emit KTextEditor::Document::textRemoved(this, KTextEditor::Range(line, col, line+1, 0));

  editEnd ();

  return true;
}

bool KateDocument::editInsertLine ( int line, const QString &s, Kate::EditSource editSource )
{
  if (line < 0)
    return false;

  if (!isReadWrite())
    return false;

  if ( line > lines() )
    return false;

  editStart (editSource);

  m_undoManager->slotLineInserted(line, s);

  removeTrailingSpace( line ); // old line

  KateTextLine::Ptr tl(new KateTextLine());
  tl->insertText (0, s);
  m_buffer->insertLine(line, tl);
  m_buffer->changeLine(line);

  removeTrailingSpace( line ); // new line

  QList<KTextEditor::Mark*> list;
  for (QHash<int, KTextEditor::Mark*>::const_iterator i = m_marks.constBegin(); i != m_marks.constEnd(); ++i)
  {
    if( i.value()->line >= line )
      list.append( i.value() );
  }

   for( int i=0; i < list.size(); ++i )
      m_marks.take( list[i]->line );

   for( int i=0; i < list.size(); ++i )
   {
      list[i]->line++;
      m_marks.insert( list[i]->line, list[i] );
    }

  if( !list.isEmpty() )
    emit marksChanged( this );

  KTextEditor::Range rangeInserted(line, 0, line, tl->length());

  if (line) {
    KateTextLine::Ptr prevLine = plainKateTextLine(line - 1);
    rangeInserted.start().setPosition(line - 1, prevLine->length());
  } else {
    rangeInserted.end().setPosition(line + 1, 0);
  }

  history()->doEdit( new KateEditInfo(m_editSources.top(), KTextEditor::Range(rangeInserted.start(), rangeInserted.start()), QStringList(), rangeInserted, QStringList(s)) );
  emit KTextEditor::Document::textInserted(this, rangeInserted);

  editEnd ();

  return true;
}

bool KateDocument::editRemoveLine ( int line, Kate::EditSource editSource )
{
  return editRemoveLines(line, line, editSource);
}

bool KateDocument::editRemoveLines ( int from, int to, Kate::EditSource editSource )
{
  if (to < from || from < 0 || to > lastLine())
    return false;

  if (!isReadWrite())
    return false;

  if (lines() == 1)
    return editRemoveText(0, 0, kateTextLine(0)->length());

  editStart(editSource);
  QStringList oldText;

  for (int line = to; line >= from; line--) {
    KateLineInfo info;
    lineInfo(&info, line);
    if (info.startsInVisibleBlock)
      foldingTree()->toggleRegionVisibility(line);
  }

  for (int line = to; line >= from; line--) {
    oldText << this->line(line);
    m_undoManager->slotLineRemoved(line, this->line(line));

    m_buffer->removeLine(line);
  }

  QList<int> rmark;
  QList<int> list;

  foreach (KTextEditor::Mark* mark, m_marks) {
    int line = mark->line;
    if (line > to)
      list << line;
    else if (line >= from)
      rmark << line;
  }

  foreach (int line, rmark)
    delete m_marks.take(line);

  foreach (int line, list)
  {
    KTextEditor::Mark* mark = m_marks.take(line);
    mark->line -= to - from + 1;
    m_marks.insert(mark->line, mark);
  }

  if (!list.isEmpty())
    emit marksChanged(this);

  KTextEditor::Range rangeRemoved(from, 0, to + 1, 0);

  if (to == lastLine() + to - from + 1) {
    rangeRemoved.end().setPosition(to, oldText.first().length());
    if (from > 0) {
      KateTextLine::Ptr prevLine = plainKateTextLine(from - 1);
      rangeRemoved.start().setPosition(from - 1, prevLine->length());
    }
  }

  history()->doEdit(new KateEditInfo(m_editSources.top(), rangeRemoved, oldText, KTextEditor::Range(rangeRemoved.start(), rangeRemoved.start()), QStringList()));
  emit KTextEditor::Document::textRemoved(this, rangeRemoved);

  editEnd();

  return true;
}
//END

//BEGIN KTextEditor::UndoInterface stuff

uint KateDocument::undoCount () const
{
  return m_undoManager->undoCount ();
}

uint KateDocument::redoCount () const
{
  return m_undoManager->redoCount ();
}

void KateDocument::undo()
{
  m_undoManager->undo();
}

void KateDocument::redo()
{
  m_undoManager->redo();
}
//END

//BEGIN KTextEditor::SearchInterface stuff
QVector<KTextEditor::Range> KateDocument::searchText(
    const KTextEditor::Range & range,
    const QString & pattern,
    const KTextEditor::Search::SearchOptions options)
{
  // TODO
  // * support BlockInputRange
  // * support DotMatchesNewline

  const bool escapeSequences =  options.testFlag(KTextEditor::Search::EscapeSequences);
  const bool regexMode       =  options.testFlag(KTextEditor::Search::Regex);
  const bool caseSensitive   = !options.testFlag(KTextEditor::Search::CaseInsensitive);
  const bool backwards       =  options.testFlag(KTextEditor::Search::Backwards);
  const bool wholeWords      =  options.testFlag(KTextEditor::Search::WholeWords);

  if (regexMode)
  {
    // regexp search
    // escape sequences are supported by definition
    KateRegExpSearch searcher(this, caseSensitive);
    return searcher.search(range, pattern, backwards);
  }

  if (escapeSequences)
  {
    // escaped search
    KateEscapedTextSearch searcher(this, caseSensitive, wholeWords);
    return searcher.search(range, pattern, backwards);
  }

  // plaintext search
  KatePlainTextSearch searcher(this, caseSensitive, wholeWords);
  return searcher.search(range, pattern, backwards);
}



KTextEditor::Search::SearchOptions KateDocument::supportedSearchOptions() const
{
  KTextEditor::Search::SearchOptions supported(KTextEditor::Search::Default);
  supported |= KTextEditor::Search::Regex;
  supported |= KTextEditor::Search::CaseInsensitive;
  supported |= KTextEditor::Search::Backwards;
// supported |= KTextEditor::Search::BlockInputRange;
  supported |= KTextEditor::Search::EscapeSequences;
  supported |= KTextEditor::Search::WholeWords;
// supported |= KTextEditor::Search::DotMatchesNewline;
  return supported;
}
//END

QWidget * KateDocument::dialogParent()
{
    QWidget *w=widget();

    if(!w)
    {
        w=activeView();

        if(!w)
            w=QApplication::activeWindow();
    }

    return w;
}

//BEGIN KTextEditor::HighlightingInterface stuff
bool KateDocument::setMode (const QString &name)
{
  updateFileType (name);
  return true;
}

QString KateDocument::mode () const
{
  return m_fileType;
}

QStringList KateDocument::modes () const
{
  QStringList m;

  const QList<KateFileType *> &modeList = KateGlobal::self()->modeManager()->list();
  for (int i = 0; i < modeList.size(); ++i)
    m << modeList[i]->name;

  return m;
}

bool KateDocument::setHighlightingMode (const QString &name)
{
  m_buffer->setHighlight (KateHlManager::self()->nameFind(name));
  return true;
}

QString KateDocument::highlightingMode () const
{
  return highlight()->name ();
}

QStringList KateDocument::highlightingModes () const
{
  QStringList hls;

  for (int i = 0; i < KateHlManager::self()->highlights(); ++i)
    hls << KateHlManager::self()->hlName (i);

  return hls;
}

QString KateDocument::highlightingModeSection( int index ) const
{
  return KateHlManager::self()->hlSection( index );
}

QString KateDocument::modeSection( int index ) const
{
  return KateGlobal::self()->modeManager()->list()[ index ]->section;
}

void KateDocument::bufferHlChanged ()
{
  // update all views
  makeAttribs(false);

  // deactivate indenter if necessary
  m_indenter->checkRequiredStyle();

  emit highlightingModeChanged(this);
}


void KateDocument::setDontChangeHlOnSave()
{
  hlSetByUser = true;
}

void KateDocument::bomSetByUser()
{
  m_bomSetByUser=true;
}
//END

//BEGIN KTextEditor::SessionConfigInterface and KTextEditor::ParameterizedSessionConfigInterface stuff
void KateDocument::readSessionConfig(const KConfigGroup &kconfig)
{
  readParameterizedSessionConfig(kconfig, SkipNone);
}

void KateDocument::readParameterizedSessionConfig(const KConfigGroup &kconfig,
                                                  unsigned long configParameters)
{
  if(!(configParameters & KTextEditor::ParameterizedSessionConfigInterface::SkipEncoding)) {
    // get the encoding
    QString tmpenc=kconfig.readEntry("Encoding");
    if (!tmpenc.isEmpty() && (tmpenc != encoding()))
      setEncoding(tmpenc);
  }

  if(!(configParameters & KTextEditor::ParameterizedSessionConfigInterface::SkipUrl)) {
    // restore the url
    KUrl url (kconfig.readEntry("URL"));

    // open the file if url valid
    if (!url.isEmpty() && url.isValid())
      openUrl (url);
    else completed(); //perhaps this should be emitted at the end of this function
  }
  else {
    completed(); //perhaps this should be emitted at the end of this function
  }

  if(!(configParameters & KTextEditor::ParameterizedSessionConfigInterface::SkipMode)) {
    // restore the filetype
    updateFileType (kconfig.readEntry("Mode", "Normal"));
  }

  if(!(configParameters & KTextEditor::ParameterizedSessionConfigInterface::SkipHighlighting)) {
    // restore the hl stuff
    m_buffer->setHighlight(KateHlManager::self()->nameFind(kconfig.readEntry("Highlighting")));
  }

  // read only mode
  // todo: what does m_bReadOnly mean?
  setReadWrite(kconfig.readEntry("ReadWrite", true));

  // indent mode
  config()->setIndentationMode( kconfig.readEntry("Indentation Mode", config()->indentationMode() ) );

  // Restore Bookmarks
  const QList<int> marks = kconfig.readEntry("Bookmarks", QList<int>());
  for( int i = 0; i < marks.count(); i++ )
    addMark( marks[i], KateDocument::markType01 );
}

void KateDocument::writeSessionConfig(KConfigGroup &kconfig)
{
  writeParameterizedSessionConfig(kconfig, SkipNone);
}

void KateDocument::writeParameterizedSessionConfig(KConfigGroup &kconfig,
                                                   unsigned long configParameters)
{
  if ( this->url().isLocalFile() ) {
    const QString path = this->url().toLocalFile();
    if ( KGlobal::dirs()->relativeLocation( "tmp", path ) != path ) {
      return; // inside tmp resource, do not save
    }
  }

  if(!(configParameters & KTextEditor::ParameterizedSessionConfigInterface::SkipUrl)) {
    // save url
    kconfig.writeEntry("URL", this->url().prettyUrl() );
  }

  if(!(configParameters & KTextEditor::ParameterizedSessionConfigInterface::SkipEncoding)) {
    // save encoding
    kconfig.writeEntry("Encoding",encoding());
  }

  if(!(configParameters & KTextEditor::ParameterizedSessionConfigInterface::SkipMode)) {
    // save file type
    kconfig.writeEntry("Mode", m_fileType);
  }

  if(!(configParameters & KTextEditor::ParameterizedSessionConfigInterface::SkipHighlighting)) {
    // save hl
    kconfig.writeEntry("Highlighting", highlight()->name());
  }

  // read only mode
  kconfig.writeEntry("ReadWrite", isReadWrite());

  // indent mode
  kconfig.writeEntry("Indentation Mode", config()->indentationMode() );

  // Save Bookmarks
  QList<int> marks;
  for (QHash<int, KTextEditor::Mark*>::const_iterator i = m_marks.constBegin(); i != m_marks.constEnd(); ++i)
    if (i.value()->type & KTextEditor::MarkInterface::markType01)
      marks << i.value()->line;

  kconfig.writeEntry( "Bookmarks", marks );
}

//END KTextEditor::SessionConfigInterface and KTextEditor::ParameterizedSessionConfigInterface stuff

uint KateDocument::mark( int line )
{
  if( !m_marks.value(line) )
    return 0;

  return m_marks[line]->type;
}

void KateDocument::setMark( int line, uint markType )
{
  clearMark( line );
  addMark( line, markType );
}

void KateDocument::clearMark( int line )
{
  if( line > lastLine() )
    return;

  if( !m_marks.value(line) )
    return;

  KTextEditor::Mark* mark = m_marks.take( line );
  emit markChanged( this, *mark, MarkRemoved );
  emit marksChanged( this );
  delete mark;
  tagLines( line, line );
  repaintViews(true);
}

void KateDocument::addMark( int line, uint markType )
{
  if( line > lastLine())
    return;

  if( markType == 0 )
    return;

  if( m_marks.value(line) ) {
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
  emit markChanged( this, temp, MarkAdded );

  emit marksChanged( this );
  tagLines( line, line );
  repaintViews(true);
}

void KateDocument::removeMark( int line, uint markType )
{
  if( line > lastLine() )
    return;

  if( !m_marks.value(line) )
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
  emit markChanged( this, temp, MarkRemoved );

  if( mark->type == 0 )
    m_marks.remove( line );

  emit marksChanged( this );
  tagLines( line, line );
  repaintViews(true);
}

const QHash<int, KTextEditor::Mark*> &KateDocument::marks()
{
  return m_marks;
}

void KateDocument::requestMarkTooltip( int line, QPoint position )
{
  if(!mark(line))
    return;

  bool handled = false;
  emit markToolTipRequested( this, *marks()[line], position, handled );
}

bool KateDocument::handleMarkClick( int line )
{
  bool handled = false;

  if(!mark(line))
    return false;

  emit markClicked( this, *marks()[line], handled );

  return handled;
}

bool KateDocument::handleMarkContextMenu( int line, QPoint position )
{
  bool handled = false;

  if(!mark(line))
    return false;

  emit markContextMenuRequested( this, *marks()[line], position, handled );

  return handled;
}

void KateDocument::clearMarks()
{
  while (!m_marks.isEmpty())
  {
    QHash<int, KTextEditor::Mark*>::iterator it = m_marks.begin();
    KTextEditor::Mark mark = *it.value();
    delete it.value();
    m_marks.erase (it);

    emit markChanged( this, mark, MarkRemoved );
    tagLines( mark.line, mark.line );
  }

  m_marks.clear();

  emit marksChanged( this );
  repaintViews(true);
}

void KateDocument::setMarkPixmap( MarkInterface::MarkTypes type, const QPixmap& pixmap )
{
  m_markPixmaps.insert( type, pixmap );
}

void KateDocument::setMarkDescription( MarkInterface::MarkTypes type, const QString& description )
{
  m_markDescriptions.insert( type, description );
}

QPixmap KateDocument::markPixmap( MarkInterface::MarkTypes type ) const
{
  return m_markPixmaps.contains(type) ?
         m_markPixmaps[type] : QPixmap();
}

QColor KateDocument::markColor( MarkInterface::MarkTypes type ) const
{
  uint reserved = (0x1 << KTextEditor::MarkInterface::reservedMarkersCount()) - 1;
  if ((uint)type >= (uint)markType01 && (uint)type <= reserved) {
    return KateRendererConfig::global()->lineMarkerColor(type);
  } else {
    return QColor();
  }
}

QString KateDocument::markDescription( MarkInterface::MarkTypes type ) const
{
  return m_markDescriptions.contains(type) ?
         m_markDescriptions[type] : QString();
}

void KateDocument::setEditableMarks( uint markMask )
{
  m_editableMarks = markMask;
}

uint KateDocument::editableMarks() const
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
  if ( ! this->url().isEmpty() )
    result = KMimeType::findByUrl( this->url() );

  else if ( this->url().isEmpty() || ! this->url().isLocalFile() )
    result = mimeTypeForContent();

  return result->name();
}

KMimeType::Ptr KateDocument::mimeTypeForContent()
{
  QByteArray buf (1024,'\0');
  uint bufpos = 0;

  for (int i=0; i < lines(); ++i)
  {
    QString line = this->line( i );
    uint len = line.length() + 1;

    if (bufpos + len > 1024)
      len = 1024 - bufpos;

    QString ld (line + QChar::fromAscii('\n'));
    buf.replace(bufpos,len,ld.toLatin1()); //memcpy(buf.data() + bufpos, ld.toLatin1().constData(), len);

    bufpos += len;

    if (bufpos >= 1024)
      break;
  }
  buf.resize( bufpos );

  int accuracy = 0;
  KMimeType::Ptr mt = KMimeType::findByContent(buf, &accuracy);
  return mt ? mt : KMimeType::defaultMimeTypePtr();
}
//END KTextEditor::DocumentInfoInterface


//BEGIN KParts::ReadWrite stuff
bool KateDocument::openFile()
{
  // no open errors until now...
  setOpeningError(false);

  // add new m_file to dirwatch
  activateDirWatch ();

  //
  // mime type magic to get encoding right
  //
  QString mimeType = arguments().mimeType();
  int pos = mimeType.indexOf(';');
  if (pos != -1)
    setEncoding (mimeType.mid(pos+1));

  // do we have success ?
  history()->doEdit( new KateEditInfo(Kate::CloseFileEdit, documentRange(), QStringList(), KTextEditor::Range(0,0,0,0), QStringList()) );
  emit KTextEditor::Document::textRemoved(this, documentRange());

  bool success = m_buffer->openFile (localFilePath());

  history()->doEdit( new KateEditInfo(Kate::OpenFileEdit, KTextEditor::Range(0,0,0,0), QStringList(), documentRange(), QStringList()) );
  emit KTextEditor::Document::textInserted(this, documentRange());

  //
  // yeah, success
  //
  if (success)
  {
    // update file type
    updateFileType (KateGlobal::self()->modeManager()->fileType (this));

    // read dir config (if possible and wanted)
    readDirConfig ();

    // read vars
    readVariables();

    // update the md5 digest
    createDigest( m_digest );

    if (!m_postLoadFilterChecks.isEmpty())
    {
      LoadSaveFilterCheckPlugins *lscps=loadSaveFilterCheckPlugins();
      foreach(const QString& checkplugin, m_postLoadFilterChecks)
      {
         lscps->postLoadFilter(checkplugin,this);
      }
    }
  }

  // Inform that the text has changed (required as we're not inside the usual editStart/End stuff)
  emit textChanged (this);

  //
  // update views
  //
  foreach (KateView * view, m_views)
  {
    // This is needed here because inserting the text moves the view's start position (it is a SmartCursor)
    view->setCursorPosition(KTextEditor::Cursor());
    view->updateView(true);
  }

  if (!m_reloading)
  {
    //
    // emit the signal we need for example for kate app
    //
    emit documentUrlChanged (this);

    //
    // set doc name, dummy value as arg, don't need it
    //
    setDocName  (QString());
  }
  //
  // to houston, we are not modified
  //
  if (m_modOnHd)
  {
    m_modOnHd = false;
    m_modOnHdReason = OnDiskUnmodified;
    emit modifiedOnDisk (this, m_modOnHd, m_modOnHdReason);
  }

  //
  // display errors
  //
  QWidget *parentWidget(dialogParent());

  if (!suppressOpeningErrorDialogs())
  {
    if (!success)
      KMessageBox::error (parentWidget, i18n ("The file %1 could not be loaded, as it was not possible to read from it.\n\nCheck if you have read access to this file.", this->url().pathOrUrl()));
  }

  if (!success) {
    setOpeningError(true);
    setOpeningErrorMessage(i18n ("The file %1 could not be loaded, as it was not possible to read from it.\n\nCheck if you have read access to this file.",this->url().pathOrUrl()));
  }

  // warn -> opened binary file!!!!!!!
  if (m_buffer->binary())
  {
    // this file can't be saved again without killing it
    setReadWrite( false );

    if(!suppressOpeningErrorDialogs())
      KMessageBox::information (parentWidget
        , i18n ("The file %1 is a binary, saving it will result in a corrupt file.", this->url().pathOrUrl())
        , i18n ("Binary File Opened")
        , "Binary File Opened Warning");

    setOpeningError(true);
    setOpeningErrorMessage(i18n ("The file %1 is a binary, saving it will result in a corrupt file.", this->url().pathOrUrl()));
  }

  // warn: opened broken utf-8 file...
  // only warn if not already a binary file!
  else if (m_buffer->brokenUTF8())
  {
    // this file can't be saved again without killing it
    setReadWrite( false );

    if (!suppressOpeningErrorDialogs())
      KMessageBox::information (parentWidget
        , i18n ("The file %1 was opened with UTF-8 encoding but contained invalid characters."
                " It is set to read-only mode, as saving might destroy its content."
                " Either reopen the file with the correct encoding chosen or enable the read-write mode again in the menu to be able to edit it.", this->url().pathOrUrl())
        , i18n ("Broken UTF-8 File Opened")
        , "Broken UTF-8 File Opened Warning");
    setOpeningError(true);
    setOpeningErrorMessage(i18n ("The file %1 was opened with UTF-8 encoding but contained invalid characters."
              " It is set to read-only mode, as saving might destroy its content."
              " Either reopen the file with the correct encoding chosen or enable the read-write mode again in the menu to be able to edit it.", this->url().pathOrUrl()));
  }

  //
  // return the success
  //
  return success;
}

bool KateDocument::saveFile()
{
  QWidget *parentWidget(dialogParent());

  //
  // warn -> try to save binary file!!!!!!!
  //
  if (m_buffer->binary() && (KMessageBox::warningContinueCancel (parentWidget
        , i18n ("The file %1 is a binary, saving it will result in a corrupt file.", url().pathOrUrl())
        , i18n ("Trying to Save Binary File")
        , KGuiItem(i18n("Save Nevertheless"))
        , KStandardGuiItem::cancel(), "Binary File Save Warning") != KMessageBox::Continue))
    return false;

  // some warnings, if file was changed by the outside!
  if ( !url().isEmpty() )
  {
    if (s_fileChangedDialogsActivated && m_modOnHd)
    {
      QString str = reasonedMOHString() + "\n\n";

      if (!isModified())
      {
        if (KMessageBox::warningContinueCancel(parentWidget,
               str + i18n("Do you really want to save this unmodified file? You could overwrite changed data in the file on disk."),i18n("Trying to Save Unmodified File"),KGuiItem(i18n("Save Nevertheless"))) != KMessageBox::Continue)
          return false;
      }
      else
      {
        if (KMessageBox::warningContinueCancel(parentWidget,
               str + i18n("Do you really want to save this file? Both your open file and the file on disk were changed. There could be some data lost."),i18n("Possible Data Loss"),KGuiItem(i18n("Save Nevertheless"))) != KMessageBox::Continue)
          return false;
      }
    }
  }

  //
  // can we encode it if we want to save it ?
  //
  if (!m_buffer->canEncode ()
       && (KMessageBox::warningContinueCancel(parentWidget,
           i18n("The selected encoding cannot encode every unicode character in this document. Do you really want to save it? There could be some data lost."),i18n("Possible Data Loss"),KGuiItem(i18n("Save Nevertheless"))) != KMessageBox::Continue))
  {
    return false;
  }

  //
  // try to create backup file..
  //

  // local file or not is here the question
  bool l ( url().isLocalFile() );

  // does the user want any backup, if not, not our problem?
  if ( ( l && config()->backupFlags() & KateDocumentConfig::LocalFiles )
       || ( ! l && config()->backupFlags() & KateDocumentConfig::RemoteFiles ) )
  {
    KUrl u( url() );
    u.setFileName( config()->backupPrefix() + url().fileName() + config()->backupSuffix() );

    kDebug( 13020 ) << "backup src file name: " << url();
    kDebug( 13020 ) << "backup dst file name: " << u;

    // handle the backup...
    bool backupSuccess = false;

    // local file mode, no kio
    if (u.isLocalFile ())
    {
      if (QFile::exists (url().toLocalFile ()))
      {
        // first: check if backupFile is already there, if true, unlink it
        QFile backupFile (u.toLocalFile ());
        if (backupFile.exists()) backupFile.remove ();

        backupSuccess = QFile::copy (url().toLocalFile (), u.toLocalFile ());
      }
      else
        backupSuccess = true;
    }
    else // remote file mode, kio
    {
      QWidget *w = widget ();
      if (!w && !m_views.isEmpty ())
        w = m_views.first();

      // get the right permissions, start with safe default
      mode_t  perms = 0600;
      KIO::UDSEntry fentry;
      if (KIO::NetAccess::stat (url(), fentry, kapp->activeWindow()))
      {
        kDebug( 13020 ) << "stating succesfull: " << url();
        KFileItem item (fentry, url());
        perms = item.permissions();

        // do a evil copy which will overwrite target if possible
        KIO::FileCopyJob *job = KIO::file_copy ( url(), u, -1, KIO::Overwrite );
        backupSuccess = KIO::NetAccess::synchronousRun(job, w);
      }
      else
        backupSuccess = true;
    }

    // backup has failed, ask user how to proceed
    if (!backupSuccess && (KMessageBox::warningContinueCancel (parentWidget
        , i18n ("For file %1 no backup copy could be created before saving."
                " If an error occurs while saving, you might lose the data of this file."
                " A reason could be that the media you write to is full or the directory of the file is read-only for you.", url().pathOrUrl())
        , i18n ("Failed to create backup copy.")
        , KGuiItem(i18n("Try to Save Nevertheless"))
        , KStandardGuiItem::cancel(), "Backup Failed Warning") != KMessageBox::Continue))
    {
      return false;
    }
  }

  // update file type
  updateFileType (KateGlobal::self()->modeManager()->fileType (this));

  if (!m_preSavePostDialogFilterChecks.isEmpty())
  {
    LoadSaveFilterCheckPlugins *lscps=loadSaveFilterCheckPlugins();
    foreach(const QString& checkplugin, m_preSavePostDialogFilterChecks)
    {
       if (lscps->preSavePostDialogFilterCheck(checkplugin,this,parentWidget)==false)
         return false;
    }
  }

  // remember the oldpath...
  QString oldPath = m_dirWatchFile;

  // remove file from dirwatch
  deactivateDirWatch ();

  //
  // try to save
  //
  if (!m_buffer->saveFile (localFilePath()))
  {
    // add m_file again to dirwatch
    activateDirWatch (oldPath);

    KMessageBox::error (parentWidget, i18n ("The document could not be saved, as it was not possible to write to %1.\n\nCheck that you have write access to this file or that enough disk space is available.", this->url().pathOrUrl()));

    return false;
  }

  // update the md5 digest
  createDigest( m_digest );

  // add m_file again to dirwatch
  activateDirWatch ();

  // update file type
//  updateFileType (KateGlobal::self()->modeManager()->fileType (this));

  // read dir config (if possible and wanted)
  if ( url().isLocalFile())
  {
    QFileInfo fo (oldPath), fn (m_dirWatchFile);

    if (fo.path() != fn.path())
      readDirConfig();
  }

  // read our vars
  readVariables();

  //
  // we are not modified
  //
  if (m_modOnHd)
  {
    m_modOnHd = false;
    m_modOnHdReason = OnDiskUnmodified;
    emit modifiedOnDisk (this, m_modOnHd, m_modOnHdReason);
  }

  // update document name...
  setDocName( QString() );

  // url may have changed...
  emit documentUrlChanged (this);

  m_savingToUrl=true;

  // (dominik) mark last undo group as not mergeable, otherwise the next
  // edit action might be merged and undo will never stop at the saved state
  setUndoDontMerge(true);

  //
  // return success
  //
  return true;
}

void KateDocument::readDirConfig ()
{
  int depth = config()->searchDirConfigDepth ();

  if (this->url().isLocalFile() && (depth > -1))
  {
    QString currentDir = QFileInfo (localFilePath()).absolutePath();

    // only search as deep as specified or not at all ;)
    while (depth > -1)
    {
      //kDebug (13020) << "search for config file in path: " << currentDir;

      // try to open config file in this dir
      QFile f (currentDir + "/.kateconfig");

      if (f.open (QIODevice::ReadOnly))
      {
        QTextStream stream (&f);

        uint linesRead = 0;
        QString line = stream.readLine();
        while ((linesRead < 32) && !line.isNull())
        {
          readVariableLine( line );

          line = stream.readLine();

          linesRead++;
        }

        break;
      }

      QString newDir = QFileInfo (currentDir).absolutePath();

      // bail out on looping (for example reached /)
      if (currentDir == newDir)
        break;

      currentDir = newDir;
      --depth;
    }
  }
}

void KateDocument::activateDirWatch (const QString &useFileName)
{
  QString fileToUse = useFileName;
  if (fileToUse.isEmpty())
    fileToUse = localFilePath();

  // same file as we are monitoring, return
  if (fileToUse == m_dirWatchFile)
    return;

  // remove the old watched file
  deactivateDirWatch ();

  // add new file if needed
  if (url().isLocalFile() && !fileToUse.isEmpty())
  {
    KateGlobal::self()->dirWatch ()->addFile (fileToUse);
    m_dirWatchFile = fileToUse;
  }
}

void KateDocument::deactivateDirWatch ()
{
  if (!m_dirWatchFile.isEmpty())
    KateGlobal::self()->dirWatch ()->removeFile (m_dirWatchFile);

  m_dirWatchFile.clear();
}

bool KateDocument::closeUrl()
{
  //
  // file mod on hd
  //
  if ( !m_reloading && !url().isEmpty() )
  {
    if (s_fileChangedDialogsActivated && m_modOnHd)
    {
      QWidget *parentWidget(dialogParent());

      if (!(KMessageBox::warningContinueCancel(
            parentWidget,
            reasonedMOHString() + "\n\n" + i18n("Do you really want to continue to close this file? Data loss may occur."),
            i18n("Possible Data Loss"), KGuiItem(i18n("Close Nevertheless")), KStandardGuiItem::cancel(),
            QString("kate_close_modonhd_%1").arg( m_modOnHdReason ) ) == KMessageBox::Continue))
        return false;
    }
  }

  //
  // first call the normal kparts implementation
  //
  if (!KParts::ReadWritePart::closeUrl ())
    return false;

  // Tell the world that we're about to go ahead with the close
  if (!m_reloading)
    emit aboutToClose(this);

  // remove file from dirwatch
  deactivateDirWatch ();

  //
  // empty url + fileName
  //
  setUrl(KUrl());
  setLocalFilePath(QString());

  // we are not modified
  if (m_modOnHd)
  {
    m_modOnHd = false;
    m_modOnHdReason = OnDiskUnmodified;
    emit modifiedOnDisk (this, m_modOnHd, m_modOnHdReason);
  }

  emit KTextEditor::Document::textRemoved(this, documentRange());

  {
    QMutexLocker l(smartMutex());
    history()->doEdit( new KateEditInfo(Kate::CloseFileEdit, documentRange(), QStringList(), KTextEditor::Range(0,0,0,0), QStringList()) );

    // clear the buffer
    m_buffer->clear();

    // remove all marks
    clearMarks ();

    // clear undo/redo history
    m_undoManager->clearUndo();
    m_undoManager->clearRedo();
  }

  // no, we are no longer modified
  setModified(false);

  // we have no longer any hl
  m_buffer->setHighlight(0);

  // update all our views
  foreach (KateView * view, m_views )
  {
    view->clearSelection(); // fix bug #118588
    view->clear();
  }

  if (!m_reloading)
  {

    // uh, fileName changed
    emit documentUrlChanged (this);

    // update doc name
    setDocName (QString());
  }
  // success
  return true;
}

void KateDocument::setReadWrite( bool rw )
{
  if (isReadWrite() != rw)
  {
    KParts::ReadWritePart::setReadWrite (rw);

    foreach( KateView* view, m_views)
    {
      view->slotUpdateUndo();
      view->slotReadWriteChanged ();
    }
  }
}

void KateDocument::setModified(bool m) {

  if (isModified() != m) {
    KParts::ReadWritePart::setModified (m);

    foreach( KateView* view,m_views)
    {
      view->slotUpdateUndo();
    }

    emit modifiedChanged (this);
  }

  m_undoManager->setModified (m);
}
//END

//BEGIN Kate specific stuff ;)

void KateDocument::makeAttribs(bool needInvalidate)
{
  foreach(KateView *view,m_views)
    view->renderer()->updateAttributes ();

  if (needInvalidate)
    m_buffer->invalidateHighlighting();

  foreach(KateView *view,m_views)
  {
    view->tagAll();
    view->updateView (true);
  }
}

// the attributes of a hl have changed, update
void KateDocument::internalHlChanged()
{
  makeAttribs();
}

void KateDocument::addView(KTextEditor::View *view) {
  if (!view)
    return;

  m_views.append( static_cast<KateView*>(view) );
  m_textEditViews.append( view );

  foreach(KTextEditor::SmartRange* highlight, m_documentHighlights) {
    Q_ASSERT(dynamic_cast<KateView*>(view));
    static_cast<KateView*>(view)->addExternalHighlight(highlight, m_documentDynamicHighlights.contains(highlight));
}

  // apply the view & renderer vars from the file type
  if (!m_fileType.isEmpty())
      readVariableLine(KateGlobal::self()->modeManager()->fileType(m_fileType).varLine, true);

  // apply the view & renderer vars from the file
  readVariables (true);

  setActiveView(view);
}

void KateDocument::removeView(KTextEditor::View *view) {
  if (!view)
    return;

  if (activeView() == view)
    setActiveView(0L);

  m_views.removeAll( (KateView *) view );
  m_textEditViews.removeAll( view  );
}

void KateDocument::setActiveView(KTextEditor::View* view)
{
  if ( m_activeView == view ) return;

  if (m_activeView) {
    disconnect(m_activeView, SIGNAL(selectionChanged(KTextEditor::View*)), this, SIGNAL(activeViewSelectionChanged(KTextEditor::View*)));
  }

  m_activeView = (KateView*)view;

  if (m_activeView) {
    connect(m_activeView, SIGNAL(selectionChanged(KTextEditor::View*)), SIGNAL(activeViewSelectionChanged(KTextEditor::View*)));
  }
}

bool KateDocument::ownedView(KateView *view) {
  // do we own the given view?
  return (m_views.contains(view));
}

uint KateDocument::toVirtualColumn( const KTextEditor::Cursor& cursor )
{
  KateTextLine::Ptr textLine = m_buffer->plainLine(cursor.line());

  if (textLine)
    return textLine->toVirtualColumn(cursor.column(), config()->tabWidth());
  else
    return 0;
}

bool KateDocument::typeChars ( KateView *view, const QString &chars )
{
  // Because we want to access text before starting an edit, lock the smart mutex now
  QMutexLocker l(smartMutex());

  KateTextLine::Ptr textLine = m_buffer->plainLine(view->cursorPosition().line ());

  if (!textLine)
    return false;

  bool bracketInserted = false;
  QString buf;
  QChar c;
  for( int z = 0; z < chars.length(); z++ )
  {
    QChar ch = c = chars[z];

    if (ch.isPrint() || ch == QChar::fromAscii('\t'))
    {
      buf.append (ch);

      if (!bracketInserted && (config()->configFlags() & KateDocumentConfig::cfAutoBrackets))
      {
        if (ch == '(') { bracketInserted = true; buf.append (')'); }
        if (ch == '[') { bracketInserted = true; buf.append (']'); }
        if (ch == '{') { bracketInserted = true; buf.append ('}'); }
        if (ch == '"') { bracketInserted = true; buf.append ('"'); }
        if (ch == '\'') { bracketInserted = true; buf.append ('\''); }
      }
    }
  }

  if (buf.isEmpty())
    return false;

  l.unlock(); //editStart will lock the smart-mutex again, and it must be un-locked within editEnd. So unlock here.

  editStart ();

  if (!view->config()->persistentSelection() && view->selection() )
    view->removeSelectedText();

  KTextEditor::Cursor oldCur (view->cursorPosition());

  if (config()->configFlags() & KateDocumentConfig::cfOvr
      || (view->viInputMode() && view->getViInputModeManager()->getCurrentViMode() == ReplaceMode)) {

    KTextEditor::Range r = KTextEditor::Range(view->cursorPosition(), qMin(buf.length(),
          textLine->length() - view->cursorPosition().column()));

    // replace mode needs to know what was removed so it can be restored with backspace
    if (view->viInputMode() && view->getViInputModeManager()->getCurrentViMode() == ReplaceMode) {
      QChar removed = line( view->cursorPosition().line() ).at( r.start().column() );
      view->getViInputModeManager()->getViReplaceMode()->overwrittenChar( removed );
    }

    removeText(r);
  }

  insertText(view->cursorPosition(), buf);
  if (bracketInserted)
    view->setCursorPositionInternal (view->cursorPosition() - KTextEditor::Cursor(0,1));

  KTextEditor::Cursor b(view->cursorPosition());
  m_indenter->userTypedChar (view, b, c);

  editEnd ();

  view->slotTextInserted (view, oldCur, chars);
  return true;
}

void KateDocument::newLine( KateView *v )
{
  editStart();

  if( !v->config()->persistentSelection() && v->selection() )
    v->removeSelectedText();

  // query cursor position
  KTextEditor::Cursor c = v->cursorPosition();

  if (c.line() > (int)lastLine())
    c.setLine(lastLine());

  if (c.line() < 0)
    c.setLine(0);

  uint ln = c.line();

  KateTextLine::Ptr textLine = plainKateTextLine(ln);

  if (c.column() > (int)textLine->length())
    c.setColumn(textLine->length());

  // first: wrap line
  editWrapLine (c.line(), c.column());

  // second: indent the new line, if needed...
  m_indenter->userTypedChar(v, v->cursorPosition(), '\n');

  removeTrailingSpace( ln );

  editEnd();
}

void KateDocument::transpose( const KTextEditor::Cursor& cursor)
{
  KateTextLine::Ptr textLine = m_buffer->plainLine(cursor.line());

  if (!textLine || (textLine->length() < 2))
    return;

  uint col = cursor.column();

  if (col > 0)
    col--;

  if ((textLine->length() - col) < 2)
    return;

  uint line = cursor.line();
  QString s;

  //clever swap code if first character on the line swap right&left
  //otherwise left & right
  s.append (textLine->at(col+1));
  s.append (textLine->at(col));
  //do the swap

  // do it right, never ever manipulate a textline
  editStart ();
  editRemoveText (line, col, 2);
  editInsertText (line, col, s);
  editEnd ();
}

void KateDocument::backspace( KateView *view, const KTextEditor::Cursor& c )
{
  if ( !view->config()->persistentSelection() && view->selection() ) {
    view->removeSelectedText();
    return;
  }

  uint col = qMax( c.column(), 0 );
  uint line = qMax( c.line(), 0 );

  if ((col == 0) && (line == 0))
    return;

  int complement = 0;
  if (col > 0)
  {
    if (config()->configFlags() & KateDocumentConfig::cfAutoBrackets)
    {
      // if inside empty (), {}, [], '', "" delete both
      KateTextLine::Ptr tl = m_buffer->plainLine(line);
      if(!tl) return;
      QChar prevChar = tl->at(col-1);
      QChar nextChar = tl->at(col);

      if ( (prevChar == '"' && nextChar == '"') ||
           (prevChar == '\'' && nextChar == '\'') ||
           (prevChar == '(' && nextChar == ')') ||
           (prevChar == '[' && nextChar == ']') ||
           (prevChar == '{' && nextChar == '}') )
      {
        complement = 1;
      }
    }
    if (!(config()->configFlags() & KateDocumentConfig::cfBackspaceIndents))
    {
      // ordinary backspace
      //c.cursor.col--;
      removeText(KTextEditor::Range(line, col-1, line, col+complement));
    }
    else
    {
      // backspace indents: erase to next indent position
      KateTextLine::Ptr textLine = m_buffer->plainLine(line);

      // don't forget this check!!!! really!!!!
      if (!textLine)
        return;

      int colX = textLine->toVirtualColumn(col, config()->tabWidth());
      int pos = textLine->firstChar();
      if (pos > 0)
        pos = textLine->toVirtualColumn(pos, config()->tabWidth());

      if (pos < 0 || pos >= (int)colX)
      {
        // only spaces on left side of cursor
        indent( view, line, -1);
      }
      else
        removeText(KTextEditor::Range(line, col-1, line, col+complement));
    }
  }
  else
  {
    // col == 0: wrap to previous line
    if (line >= 1)
    {
      KateTextLine::Ptr textLine = m_buffer->plainLine(line-1);

      // don't forget this check!!!! really!!!!
      if (!textLine)
        return;

      if (config()->wordWrap() && textLine->endsWith(QLatin1String(" ")))
      {
        // gg: in hard wordwrap mode, backspace must also eat the trailing space
        removeText (KTextEditor::Range(line-1, textLine->length()-1, line, 0));
      }
      else
        removeText (KTextEditor::Range(line-1, textLine->length(), line, 0));
    }
  }
}

void KateDocument::del( KateView *view, const KTextEditor::Cursor& c )
{
  if ( !view->config()->persistentSelection() && view->selection() ) {
    view->removeSelectedText();
    return;
  }

  if( c.column() < (int) m_buffer->plainLine(c.line())->length())
  {
    removeText(KTextEditor::Range(c, 1));
  }
  else if ( c.line() < lastLine() )
  {
    removeText(KTextEditor::Range(c.line(), c.column(), c.line()+1, 0));
  }
}

void KateDocument::paste ( KateView* view, QClipboard::Mode mode )
{
  QString s = QApplication::clipboard()->text(mode);

  if (s.isEmpty())
    return;

  int lines = s.count (QChar::fromAscii ('\n'));

  m_undoManager->setUndoDontMerge (true);

  editStart (Kate::CutCopyPasteEdit);

  KTextEditor::Cursor pos = view->cursorPosition();
  if (!view->config()->persistentSelection() && view->selection()) {
    pos = view->selectionRange().start();
    view->removeSelectedText();
  }

  if (config()->configFlags() & KateDocumentConfig::cfOvr) {
    QStringList pasteLines = s.split(QLatin1Char('\n'));

    if (!view->blockSelectionMode()) {
      int endColumn = (pasteLines.count() == 1 ? pos.column() : 0) + pasteLines.last().length();
      removeText(KTextEditor::Range(pos,
                                    pos.line()+pasteLines.count()-1, endColumn));
    } else {
      int maxi = qMin(pos.line() + pasteLines.count(), this->lines());

      for (int i = pos.line(); i < maxi; ++i) {
        int pasteLength = pasteLines[i-pos.line()].length();
        removeText(KTextEditor::Range(i, pos.column(),
                                      i, qMin(pasteLength + pos.column(), lineLength(i))));
      }
    }
  }


  blockRemoveTrailingSpaces(true);
  insertText(pos, s, view->blockSelectionMode());
  blockRemoveTrailingSpaces(false);

  for (int i = pos.line(); i < pos.line() + lines; ++i)
    removeTrailingSpace(i);

  editEnd();

  // move cursor right for block select, as the user is moved right internal
  // even in that case, but user expects other behavior in block selection
  // mode !
  if (view->blockSelectionMode())
    view->setCursorPositionInternal(view->cursorPosition() + KTextEditor::Cursor(lines, 0));

  if (config()->configFlags() & KateDocumentConfig::cfIndentPastedText)
  {
    KTextEditor::Range range = KTextEditor::Range(KTextEditor::Cursor(pos.line(), 0),
                                                  KTextEditor::Cursor(pos.line() + lines, 0));

    int start = view->selectionRange().start().line();
    const int end = view->selectionRange().end().line();

    editStart();

    blockRemoveTrailingSpaces(true);
    m_indenter->indent(view, range);
    blockRemoveTrailingSpaces(false);

    for (; start <= end; ++start)
      removeTrailingSpace(start);

    editEnd();
  }

  if (!view->blockSelectionMode())
    emit charactersSemiInteractivelyInserted (pos, s);
  m_undoManager->setUndoDontMerge (true);
}

void KateDocument::indent ( KateView *v, uint line, int change)
{
  // dominik: if there is a selection, iterate afterwards over all lines and
  // remove trailing spaces
  const bool hasSelection = v->selection();
  int start = v->selectionRange().start().line();
  const int end = v->selectionRange().end().line();

  KTextEditor::Range range = hasSelection ? v->selectionRange() : KTextEditor::Range (KTextEditor::Cursor (line,0), KTextEditor::Cursor (line,0));

  editStart();
  blockRemoveTrailingSpaces(true);
  m_indenter->changeIndent(range, change);
  blockRemoveTrailingSpaces(false);

  if (hasSelection) {
    for (; start <= end; ++start)
      removeTrailingSpace(start);
  }
  editEnd();
}

void KateDocument::align(KateView *view, const KTextEditor::Range &range)
{
  editStart();

  blockRemoveTrailingSpaces(true);
  m_indenter->indent(view, range);
  blockRemoveTrailingSpaces(false);

  for (int start = range.start().line(); start <= range.end().line(); ++start) {
    removeTrailingSpace(start);
  }

  editEnd();
}

/*
  Remove a given string at the beginning
  of the current line.
*/
bool KateDocument::removeStringFromBeginning(int line, const QString &str)
{
  KateTextLine::Ptr textline = m_buffer->plainLine(line);

  KTextEditor::Cursor cursor (line, 0);
  bool there = textline->startsWith(str);

  if (!there)
  {
    cursor.setColumn(textline->firstChar());
    there = textline->matchesAt(cursor.column(), str);
  }

  if (there)
  {
    // Remove some chars
    removeText (KTextEditor::Range(cursor, str.length()));
  }

  return there;
}

/*
  Remove a given string at the end
  of the current line.
*/
bool KateDocument::removeStringFromEnd(int line, const QString &str)
{
  KateTextLine::Ptr textline = m_buffer->plainLine(line);

  KTextEditor::Cursor cursor (line, 0);
  bool there = textline->endsWith(str);

  if (there)
  {
    cursor.setColumn(textline->length() - str.length());
  }
  else
  {
    cursor.setColumn(textline->lastChar() - str.length() + 1);
    there = textline->matchesAt(cursor.column(), str);
  }

  if (there)
  {
    // Remove some chars
    removeText (KTextEditor::Range(cursor, str.length()));
  }

  return there;
}

/*
  Add to the current line a comment line mark at the beginning.
*/
void KateDocument::addStartLineCommentToSingleLine( int line, int attrib )
{
  QString commentLineMark = highlight()->getCommentSingleLineStart(attrib);
  int pos = -1;

  if (highlight()->getCommentSingleLinePosition(attrib) == KateHighlighting::CSLPosColumn0)
  {
    pos = 0;
    commentLineMark += ' ';
  } else {
    const KateTextLine::Ptr l = kateTextLine(line);
    pos = l->firstChar();
  }

  if (pos >= 0)
    insertText (KTextEditor::Cursor(line, pos), commentLineMark);
}

/*
  Remove from the current line a comment line mark at
  the beginning if there is one.
*/
bool KateDocument::removeStartLineCommentFromSingleLine( int line, int attrib )
{
  const QString shortCommentMark = highlight()->getCommentSingleLineStart( attrib );
  const QString longCommentMark = shortCommentMark + ' ';

  editStart();

  // Try to remove the long comment mark first
  bool removed = (removeStringFromBeginning(line, longCommentMark)
               || removeStringFromBeginning(line, shortCommentMark));

  editEnd();

  return removed;
}

/*
  Add to the current line a start comment mark at the
  beginning and a stop comment mark at the end.
*/
void KateDocument::addStartStopCommentToSingleLine( int line, int attrib )
{
  const QString startCommentMark = highlight()->getCommentStart( attrib ) + ' ';
  const QString stopCommentMark = ' ' + highlight()->getCommentEnd( attrib );

  editStart();

  // Add the start comment mark
  insertText (KTextEditor::Cursor(line, 0), startCommentMark);

  // Go to the end of the line
  const int col = m_buffer->plainLine(line)->length();

  // Add the stop comment mark
  insertText (KTextEditor::Cursor(line, col), stopCommentMark);

  editEnd();
}

/*
  Remove from the current line a start comment mark at
  the beginning and a stop comment mark at the end.
*/
bool KateDocument::removeStartStopCommentFromSingleLine( int line, int attrib )
{
  QString shortStartCommentMark = highlight()->getCommentStart( attrib );
  QString longStartCommentMark = shortStartCommentMark + ' ';
  QString shortStopCommentMark = highlight()->getCommentEnd( attrib );
  QString longStopCommentMark = ' ' + shortStopCommentMark;

  editStart();

#ifdef __GNUC__
#warning "that's a bad idea, can lead to stray endings, FIXME"
#endif
  // Try to remove the long start comment mark first
  bool removedStart = (removeStringFromBeginning(line, longStartCommentMark)
                    || removeStringFromBeginning(line, shortStartCommentMark));

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
  Add to the current selection a start comment mark at the beginning
  and a stop comment mark at the end.
*/
void KateDocument::addStartStopCommentToSelection( KateView *view, int attrib )
{
  const QString startComment = highlight()->getCommentStart( attrib );
  const QString endComment = highlight()->getCommentEnd( attrib );

  KTextEditor::Range range = view->selectionRange();

  if ((range.end().column() == 0) && (range.end().line() > 0))
    range.end().setPosition(range.end().line() - 1, lineLength(range.end().line() - 1));

  editStart();

  if (!view->blockSelection()) {
    insertText(range.end(), endComment);
    insertText(range.start(), startComment);
  } else {
    for (int line = range.start().line(); line <= range.end().line(); line++ ) {
      KTextEditor::Range subRange = rangeOnLine(range, line);
      insertText(subRange.end(), endComment);
      insertText(subRange.start(), startComment);
    }
  }

  editEnd ();
  // selection automatically updated (KateSmartRange)
}

/*
  Add to the current selection a comment line mark at the beginning of each line.
*/
void KateDocument::addStartLineCommentToSelection( KateView *view, int attrib )
{
  const QString commentLineMark = highlight()->getCommentSingleLineStart( attrib ) + ' ';

  int sl = view->selectionRange().start().line();
  int el = view->selectionRange().end().line();

  // if end of selection is in column 0 in last line, omit the last line
  if ((view->selectionRange().end().column() == 0) && (el > 0))
  {
    el--;
  }

  editStart();

  // For each line of the selection
  for (int z = el; z >= sl; z--) {
    //insertText (z, 0, commentLineMark);
    addStartLineCommentToSingleLine(z, attrib );
  }

  editEnd ();
  // selection automatically updated (KateSmartRange)
}

bool KateDocument::nextNonSpaceCharPos(int &line, int &col)
{
  for(; line < (int)m_buffer->count(); line++) {
    KateTextLine::Ptr textLine = m_buffer->plainLine(line);

    if (!textLine)
      break;

    col = textLine->nextNonSpaceChar(col);
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
    KateTextLine::Ptr textLine = m_buffer->plainLine(line);

    if (!textLine)
      break;

    col = textLine->previousNonSpaceChar(col);
    if(col != -1) return true;
    if(line == 0) return false;
    --line;
    col = textLine->length();
  }
  // No non-space char found
  line = -1;
  col = -1;
  return false;
}

/*
  Remove from the selection a start comment mark at
  the beginning and a stop comment mark at the end.
*/
bool KateDocument::removeStartStopCommentFromSelection( KateView *view, int attrib )
{
  const QString startComment = highlight()->getCommentStart( attrib );
  const QString endComment = highlight()->getCommentEnd( attrib );

  int sl = qMax<int> (0, view->selectionRange().start().line());
  int el = qMin<int>  (view->selectionRange().end().line(), lastLine());
  int sc = view->selectionRange().start().column();
  int ec = view->selectionRange().end().column();

  // The selection ends on the char before selectEnd
  if (ec != 0) {
    --ec;
  } else if (el > 0) {
    --el;
    ec = m_buffer->plainLine(el)->length() - 1;
  }

  const int startCommentLen = startComment.length();
  const int endCommentLen = endComment.length();

  // had this been perl or sed: s/^\s*$startComment(.+?)$endComment\s*/$2/

  bool remove = nextNonSpaceCharPos(sl, sc)
      && m_buffer->plainLine(sl)->matchesAt(sc, startComment)
      && previousNonSpaceCharPos(el, ec)
      && ( (ec - endCommentLen + 1) >= 0 )
      && m_buffer->plainLine(el)->matchesAt(ec - endCommentLen + 1, endComment);

  if (remove) {
    editStart();

    removeText (KTextEditor::Range(el, ec - endCommentLen + 1, el, ec + 1));
    removeText (KTextEditor::Range(sl, sc, sl, sc + startCommentLen));

    editEnd ();
    // selection automatically updated (KateSmartRange)
  }

  return remove;
}

bool KateDocument::removeStartStopCommentFromRegion(const KTextEditor::Cursor &start,const KTextEditor::Cursor &end,int attrib)
{
  const QString startComment = highlight()->getCommentStart( attrib );
  const QString endComment = highlight()->getCommentEnd( attrib );
  const int startCommentLen = startComment.length();
  const int endCommentLen = endComment.length();

  const bool remove = m_buffer->plainLine(start.line())->matchesAt(start.column(), startComment)
                   && m_buffer->plainLine(end.line())->matchesAt(end.column() - endCommentLen , endComment);
  if (remove) {
    editStart();
      removeText(KTextEditor::Range(end.line(), end.column() - endCommentLen, end.line(), end.column()));
      removeText(KTextEditor::Range(start, startCommentLen));
    editEnd();
  }
  return remove;
}

/*
  Remove from the beginning of each line of the
  selection a start comment line mark.
*/
bool KateDocument::removeStartLineCommentFromSelection( KateView *view, int attrib )
{
  const QString shortCommentMark = highlight()->getCommentSingleLineStart( attrib );
  const QString longCommentMark = shortCommentMark + ' ';

  int sl = view->selectionRange().start().line();
  int el = view->selectionRange().end().line();

  if ((view->selectionRange().end().column() == 0) && (el > 0))
  {
    el--;
  }

  bool removed = false;

  editStart();

  // For each line of the selection
  for (int z = el; z >= sl; z--)
  {
    // Try to remove the long comment mark first
    removed = (removeStringFromBeginning(z, longCommentMark)
            || removeStringFromBeginning(z, shortCommentMark)
            || removed);
  }

  editEnd();
  // selection automatically updated (KateSmartRange)

  return removed;
}

/*
  Comment or uncomment the selection or the current
  line if there is no selection.
*/
void KateDocument::comment( KateView *v, uint line,uint column, int change)
{
  // We need to check that we can sanely comment the selectino or region.
  // It is if the attribute of the first and last character of the range to
  // comment belongs to the same language definition.
  // for lines with no text, we need the attribute for the lines context.
  bool hassel = v->selection();
  int startAttrib, endAttrib;
  if ( hassel )
  {
    KateTextLine::Ptr ln = kateTextLine( v->selectionRange().start().line() );
    int l = v->selectionRange().start().line(), c = v->selectionRange().start().column();
    startAttrib = nextNonSpaceCharPos( l, c ) ? kateTextLine( l )->attribute( c ) : 0;

    ln = kateTextLine( v->selectionRange().end().line() );
    l = v->selectionRange().end().line(), c = v->selectionRange().end().column();
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

  if ( ! highlight()->canComment( startAttrib, endAttrib ) )
  {
    kDebug(13020)<<"canComment( "<<startAttrib<<", "<<endAttrib<<" ) returned false!";
    return;
  }

  bool hasStartLineCommentMark = !(highlight()->getCommentSingleLineStart( startAttrib ).isEmpty());
  bool hasStartStopCommentMark = ( !(highlight()->getCommentStart( startAttrib ).isEmpty())
      && !(highlight()->getCommentEnd( endAttrib ).isEmpty()) );

  bool removed = false;

  if (change > 0) // comment
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
           ( v->selectionRange().start().column() > m_buffer->plainLine( v->selectionRange().start().line() )->firstChar() ) ||
           ( v->selectionRange().end().column() < ((int)m_buffer->plainLine( v->selectionRange().end().line() )->length()) )
         ) ) )
        addStartStopCommentToSelection( v, startAttrib );
      else if ( hasStartLineCommentMark )
        addStartLineCommentToSelection( v, startAttrib );
    }
  }
  else // uncomment
  {
    if ( !hassel )
    {
      removed = ( hasStartLineCommentMark
                  && removeStartLineCommentFromSingleLine( line, startAttrib ) )
        || ( hasStartStopCommentMark
             && removeStartStopCommentFromSingleLine( line, startAttrib ) );
      if ((!removed) && foldingTree()) {
        kDebug(13020)<<"easy approach for uncommenting did not work, trying harder (folding tree)";
        int commentRegion=(highlight()->commentRegion(startAttrib));
        if (commentRegion){
           KateCodeFoldingNode *n=foldingTree()->findNodeForPosition(line,column);
           if (n) {
            KTextEditor::Cursor start,end;
            if ((n->nodeType()==(int)commentRegion) && n->getBegin(foldingTree(), &start) && n->getEnd(foldingTree(), &end)) {
                kDebug(13020)<<"Enclosing region found:"<<start.column()<<"/"<<start.line()<<"-"<<end.column()<<"/"<<end.line();
                removeStartStopCommentFromRegion(start,end,startAttrib);
             } else {
                  kDebug(13020)<<"Enclosing region found, but not valid";
                  kDebug(13020)<<"Region found: "<<n->nodeType()<<" region needed: "<<commentRegion;
             }
            //perhaps nested regions should be hadled here too...
          } else kDebug(13020)<<"No enclosing region found";
        } else kDebug(13020)<<"No comment region specified for current hl";
      }
    }
    else
    {
      // anders: this seems like it will work with above changes :)
      removed = ( hasStartStopCommentMark
          && removeStartStopCommentFromSelection( v, startAttrib ) )
      || ( hasStartLineCommentMark
          && removeStartLineCommentFromSelection( v, startAttrib ) );
    }
  }
}

void KateDocument::transform( KateView *v, const KTextEditor::Cursor &c,
                            KateDocument::TextTransform t )
{
  editStart();
  KTextEditor::Cursor cursor = c;

  if ( v->selection() )
  {
    // cache the selection and cursor, so we can be sure to restore.
    KTextEditor::Range selection = v->selectionRange();

    KTextEditor::Range range(selection.start(), 0);
    while ( range.start().line() <= selection.end().line() )
    {
      int start = 0;
      int end = lineLength( range.start().line() );

      if (range.start().line() == selection.start().line() || v->blockSelectionMode())
        start = selection.start().column();

      if (range.start().line() == selection.end().line() || v->blockSelectionMode())
        end = selection.end().column();

      if ( start > end )
      {
        int swapCol = start;
        start = end;
        end = swapCol;
      }
      range.start().setColumn( start );
      range.end().setColumn( end );

      QString s = text( range );
      QString old = s;

      if ( t == Uppercase )
        s = s.toUpper();
      else if ( t == Lowercase )
        s = s.toLower();
      else // Capitalize
      {
        KateTextLine::Ptr l = m_buffer->plainLine( range.start().line() );
        int p ( 0 );
        while( p < s.length() )
        {
          // If bol or the character before is not in a word, up this one:
          // 1. if both start and p is 0, upper char.
          // 2. if blockselect or first line, and p == 0 and start-1 is not in a word, upper
          // 3. if p-1 is not in a word, upper.
          if ( ( ! range.start().column() && ! p ) ||
                   ( ( range.start().line() == selection.start().line() || v->blockSelectionMode() ) &&
                   ! p && ! highlight()->isInWord( l->at( range.start().column() - 1 )) ) ||
                   ( p && ! highlight()->isInWord( s.at( p-1 ) ) )
             )
            s[p] = s.at(p).toUpper();
          p++;
        }
      }

      if ( s != old )
      {
        removeText( range );
        insertText( range.start(), s );
      }

      range.setBothLines(range.start().line() + 1);
    }

    // restore selection
    v->setSelection( selection );

  } else {  // no selection
    QString old = text( KTextEditor::Range(cursor, 1) );
    QString s;
    switch ( t ) {
      case Uppercase:
      s = old.toUpper();
      break;
      case Lowercase:
      s = old.toLower();
      break;
      case Capitalize:
      {
        KateTextLine::Ptr l = m_buffer->plainLine( cursor.line() );
        while ( cursor.column() > 0 && highlight()->isInWord( l->at( cursor.column() - 1 ), l->attribute( cursor.column() - 1 ) ) )
          cursor.setColumn(cursor.column() - 1);
        old = text( KTextEditor::Range(cursor, 1) );
        s = old.toUpper();
      }
      break;
      default:
      break;
    }
    if ( s != old )
    {
      removeText( KTextEditor::Range(cursor, 1) );
      insertText( cursor, s );
    }
  }

  editEnd();

  v->setCursorPosition( c );
}

void KateDocument::joinLines( uint first, uint last )
{
//   if ( first == last ) last += 1;
  editStart();
  int line( first );
  while ( first < last )
  {
    // Normalize the whitespace in the joined lines by making sure there's
    // always exactly one space between the joined lines
    // This cannot be done in editUnwrapLine, because we do NOT want this
    // behavior when deleting from the start of a line, just when explicitly
    // calling the join command
    KateTextLine::Ptr l = kateTextLine( line );
    KateTextLine::Ptr tl = kateTextLine( line + 1 );

    if ( !l || !tl )
    {
      editEnd();
      return;
    }

    int pos = tl->firstChar();
    if ( pos >= 0 )
    {
      if (pos != 0)
        editRemoveText( line + 1, 0, pos );
      if ( !( l->length() == 0 || l->at( l->length() - 1 ).isSpace() ) )
        editInsertText( line + 1, 0, " " );
    }
    else
    {
      // Just remove the whitespace and let Kate handle the rest
      editRemoveText( line + 1, 0, tl->length() );
    }

    editUnWrapLine( line );
    first++;
  }
  editEnd();
}

QString KateDocument::getWord( const KTextEditor::Cursor& cursor )
{
  int start, end, len;

  KateTextLine::Ptr textLine = m_buffer->plainLine(cursor.line());
  len = textLine->length();
  start = end = cursor.column();
  if (start > len)        // Probably because of non-wrapping cursor mode.
    return QString();

  while (start > 0 && highlight()->isInWord(textLine->at(start - 1), textLine->attribute(start - 1))) start--;
  while (end < len && highlight()->isInWord(textLine->at(end), textLine->attribute(end))) end++;
  len = end - start;
  return textLine->string().mid(start, len);
}

void KateDocument::tagLines(int start, int end)
{
  foreach(KateView *view,m_views)
    view->tagLines (start, end, true);
}

void KateDocument::tagLines(KTextEditor::Cursor start, KTextEditor::Cursor end)
{
  // May need to switch start/end cols if in block selection mode
/*  if (blockSelectionMode() && start.column() > end.column()) {
    int sc = start.column();
    start.setColumn(end.column());
    end.setColumn(sc);
  }
*/
  foreach (KateView* view, m_views)
    view->tagLines(start, end, true);
}

void KateDocument::repaintViews(bool paintOnlyDirty)
{
  foreach(KateView *view,m_views)
    view->repaintText(paintOnlyDirty);
}

inline bool isStartBracket( const QChar& c ) { return c == '{' || c == '[' || c == '('; }
inline bool isEndBracket  ( const QChar& c ) { return c == '}' || c == ']' || c == ')'; }
inline bool isBracket     ( const QChar& c ) { return isStartBracket( c ) || isEndBracket( c ); }

/*
   Bracket matching uses the following algorithm:
   If in overwrite mode, match the bracket currently underneath the cursor.
   Otherwise, if the character to the left is a bracket,
   match it. Otherwise if the character to the right of the cursor is a
   bracket, match it. Otherwise, don't match anything.
*/
void KateDocument::newBracketMark( const KTextEditor::Cursor& cursor, KTextEditor::Range& bm, int maxLines )
{
  bm.start() = cursor;

  if( findMatchingBracket( bm, maxLines ) )
    return;

  bm = KTextEditor::Range::invalid();

 // const int tw = config()->tabWidth();
 // const int indentStart = m_buffer->plainLine(bm.start().line())->indentDepth(tw);
 // const int indentEnd = m_buffer->plainLine(bm.end().line())->indentDepth(tw);
  //bm.setIndentMin(qMin(indentStart, indentEnd));
}

bool KateDocument::findMatchingBracket( KTextEditor::Range& range, int maxLines )
{
  KateTextLine::Ptr textLine = m_buffer->plainLine( range.start().line() );
  if( !textLine )
    return false;

  QChar right = textLine->at( range.start().column() );
  QChar left  = textLine->at( range.start().column() - 1 );
  QChar bracket;

  if ( config()->configFlags() & KateDocumentConfig::cfOvr ) {
    if( isBracket( right ) ) {
      bracket = right;
    } else {
      return false;
    }
  } else if ( isBracket( left ) ) {
    range.start().setColumn(range.start().column() - 1);
    bracket = left;
  } else if ( isBracket( right ) ) {
    bracket = right;
  } else {
    return false;
  }

  QChar opposite;

  switch( bracket.toAscii() ) {
  case '{': opposite = '}'; break;
  case '}': opposite = '{'; break;
  case '[': opposite = ']'; break;
  case ']': opposite = '['; break;
  case '(': opposite = ')'; break;
  case ')': opposite = '('; break;
  default: return false;
  }

  bool forward = isStartBracket( bracket );
  uint nesting = 0;

  int minLine = qMax( range.start().line() - maxLines, 0 );
  int maxLine = qMin( range.start().line() + maxLines, documentEnd().line() );

  range.end() = range.start();
  KateDocCursor cursor(range.start(), this);
  uchar validAttr = cursor.currentAttrib();

  while( cursor.line() >= minLine && cursor.line() <= maxLine ) {

    if (forward) {
      if (!cursor.moveForward(1))
        return false;
    } else {
      if (!cursor.moveBackward(1))
      return false;
    }

    if( cursor.currentAttrib() == validAttr )
    {
      /* Check for match */
      QChar c = cursor.currentChar();
      if( c == bracket ) {
        nesting++;
      } else if( c == opposite ) {
        if( nesting == 0 ) {
          if( forward )
            range.end() = cursor;
          else
            range.start() = cursor;
          return true;
        }
        nesting--;
      }
    }
  }

  return false;
}

void KateDocument::guiActivateEvent( KParts::GUIActivateEvent *ev )
{
  KParts::ReadWritePart::guiActivateEvent( ev );
  //if ( ev->activated() )
  //  emit selectionChanged();
}

void KateDocument::setDocName (QString name )
{
  if ( name == m_docName )
    return;

  if ( !name.isEmpty() )
  {
    // TODO check for similarly named documents
    m_docName = name;
    emit documentNameChanged (this);
    return;
  }

  // if the name is set, and starts with FILENAME, it should not be changed!
  if ( ! url().isEmpty()
       && (m_docName == url().fileName() || m_docName.startsWith (url().fileName() + " (") ) ) return;

  int count = -1;

  for (int z=0; z < KateGlobal::self()->kateDocuments().size(); ++z)
  {
    KateDocument *doc = (KateGlobal::self()->kateDocuments())[z];

    if ( (doc != this) && (doc->url().fileName() == url().fileName()) )
      if ( doc->m_docNameNumber > count )
        count = doc->m_docNameNumber;
  }

  m_docNameNumber = count + 1;

  m_docName = url().fileName();

  if (m_docName.isEmpty())
    m_docName = i18n ("Untitled");

  if (m_docNameNumber > 0)
    m_docName = QString(m_docName + " (%1)").arg(m_docNameNumber+1);

  emit documentNameChanged (this);
}

void KateDocument::slotModifiedOnDisk( KTextEditor::View * /*v*/ )
{
  if ( m_isasking < 0 )
  {
    m_isasking = 0;
    return;
  }

  if ( !s_fileChangedDialogsActivated || m_isasking )
    return;

  if (m_modOnHd && !url().isEmpty())
  {
    m_isasking = 1;

    QWidget *parentWidget(dialogParent());

    KateModOnHdPrompt p( this, m_modOnHdReason, reasonedMOHString(), parentWidget );
    switch ( p.exec() )
    {
      case KateModOnHdPrompt::Save:
      {
        m_modOnHd = false;
        KEncodingFileDialog::Result res=KEncodingFileDialog::getSaveUrlAndEncoding(config()->encoding(),
            url().url(),QString(),parentWidget,i18n("Save File"));

        kDebug(13020)<<"got "<<res.URLs.count()<<" URLs";
        if( ! res.URLs.isEmpty() && ! res.URLs.first().isEmpty() && checkOverwrite( res.URLs.first(), parentWidget ) )
        {
          setEncoding( res.encoding );

          if( ! saveAs( res.URLs.first() ) )
          {
            KMessageBox::error( parentWidget, i18n("Save failed") );
            m_modOnHd = true;
          }
          else
            emit modifiedOnDisk( this, false, OnDiskUnmodified );
        }
        else // the save as dialog was canceled, we are still modified on disk
        {
          m_modOnHd = true;
        }

        m_isasking = 0;
        break;
      }

      case KateModOnHdPrompt::Reload:
        m_modOnHd = false;
        emit modifiedOnDisk( this, false, OnDiskUnmodified );
        documentReload();
        m_isasking = 0;
        break;

      case KateModOnHdPrompt::Ignore:
        m_modOnHd = false;
        emit modifiedOnDisk( this, false, OnDiskUnmodified );
        m_isasking = 0;
        break;

      case KateModOnHdPrompt::Overwrite:
        m_modOnHd = false;
        emit modifiedOnDisk( this, false, OnDiskUnmodified );
        m_isasking = 0;
        save();
        break;

      default: // Delay/cancel: ignore next focus event
        m_isasking = -1;
    }
  }
}

void KateDocument::setModifiedOnDisk( ModifiedOnDiskReason reason )
{
  m_modOnHdReason = reason;
  m_modOnHd = (reason != OnDiskUnmodified);
  emit modifiedOnDisk( this, (reason != OnDiskUnmodified), reason );
}

class KateDocumentTmpMark
{
  public:
    QString line;
    KTextEditor::Mark mark;
};

void KateDocument::setModifiedOnDiskWarning (bool on)
{
  s_fileChangedDialogsActivated = on;
}

bool KateDocument::documentReload()
{
  if ( !url().isEmpty() )
  {
    if (m_modOnHd && s_fileChangedDialogsActivated)
    {
      QWidget *parentWidget(dialogParent());

      int i = KMessageBox::warningYesNoCancel
                (parentWidget, reasonedMOHString() + "\n\n" + i18n("What do you want to do?"),
                i18n("File Was Changed on Disk"), KGuiItem(i18n("&Reload File")), KGuiItem(i18n("&Ignore Changes")));

      if ( i != KMessageBox::Yes)
      {
        if (i == KMessageBox::No)
        {
          m_modOnHd = false;
          m_modOnHdReason = OnDiskUnmodified;
          emit modifiedOnDisk (this, m_modOnHd, m_modOnHdReason);
        }

        return false;
      }
    }

    emit aboutToReload(this);

    if (clearOnDocumentReload())
      m_smartManager->clear(false);

    QList<KateDocumentTmpMark> tmp;

    for (QHash<int, KTextEditor::Mark*>::const_iterator i = m_marks.constBegin(); i != m_marks.constEnd(); ++i)
    {
      KateDocumentTmpMark m;

      m.line = line (i.value()->line);
      m.mark = *i.value();

      tmp.append (m);
    }

    const QString oldMode = mode ();
    const bool byUser = m_fileTypeSetByUser;
    const QString hl_mode = highlightingMode ();
    KTextEditor::View* oldActiveView = activeView();

    m_storedVariables.clear();

    // save cursor positions for all views
    QVector<KTextEditor::Cursor> cursorPositions;
    cursorPositions.reserve(m_views.size());
    foreach (KateView *v, m_views)
      cursorPositions.append( v->cursorPosition() );

    m_reloading = true;
    KateDocument::openUrl( url() );
    m_reloading = false;


    // restore cursor positions for all views
    QLinkedList<KateView*>::iterator it = m_views.begin();
    for(int i = 0; i < m_views.size(); ++i, ++it) {
      setActiveView(*it);
      (*it)->setCursorPositionInternal( cursorPositions[i], m_config->tabWidth(), false );
      if ((*it)->isVisible()) {
        (*it)->repaintText(false);
      }
    }
    setActiveView(oldActiveView);

    for (int z=0; z < tmp.size(); z++)
    {
      if (z < (int)lines())
      {
        if (line(tmp[z].mark.line) == tmp[z].line)
          setMark (tmp[z].mark.line, tmp[z].mark.type);
      }
    }

    if (byUser)
      setMode (oldMode);
    setHighlightingMode (hl_mode);

    return true;
  }

  return false;
}

bool KateDocument::documentSave()
{
  if( !url().isValid() || !isReadWrite() )
    return documentSaveAs();

  return save();
}

bool KateDocument::documentSaveAs()
{
  QWidget *parentWidget(dialogParent());

  KEncodingFileDialog::Result res=KEncodingFileDialog::getSaveUrlAndEncoding(config()->encoding(),
                url().url(),QString(),parentWidget,i18n("Save File"));

  if( res.URLs.isEmpty() || !checkOverwrite( res.URLs.first(), parentWidget ) )
    return false;

  setEncoding( res.encoding );

  return saveAs( res.URLs.first() );
}

void KateDocument::setWordWrap (bool on)
{
  config()->setWordWrap (on);
}

bool KateDocument::wordWrap () const
{
  return config()->wordWrap ();
}

void KateDocument::setWordWrapAt (uint col)
{
  config()->setWordWrapAt (col);
}

unsigned int KateDocument::wordWrapAt () const
{
  return config()->wordWrapAt ();
}

void KateDocument::setPageUpDownMovesCursor (bool on)
{
  config()->setPageUpDownMovesCursor (on);
}

bool KateDocument::pageUpDownMovesCursor () const
{
  return config()->pageUpDownMovesCursor ();
}

void KateDocument::dumpRegionTree()
{
  m_buffer->foldingTree()->debugDump();
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

bool KateDocument::setEncoding (const QString &e)
{
  setProberTypeForEncodingAutoDetection(KEncodingProber::None);
  return m_config->setEncoding(e);
}

const QString &KateDocument::encoding() const
{
  return m_config->encoding();
}

void KateDocument::setProberTypeForEncodingAutoDetection (KEncodingProber::ProberType proberType)
{
  m_config->setEncodingProberType(proberType);
}

KEncodingProber::ProberType KateDocument::proberTypeForEncodingAutoDetection() const
{
  return m_config->encodingProberType();
}

void KateDocument::updateConfig ()
{
  m_undoManager->updateConfig ();

  // switch indenter if needed and update config....
  m_indenter->setMode (m_config->indentationMode());
  m_indenter->updateConfig();

  // set tab width there, too
  m_buffer->setTabWidth (config()->tabWidth());

  // update all views, does tagAll and updateView...
  foreach (KateView * view, m_views)
    view->updateDocumentConfig ();

  // update on-the-fly spell checking as spell checking defaults might have changes
  if(m_onTheFlyChecker) {
    m_onTheFlyChecker->updateConfig();
  }

  emit configChanged();
}

//BEGIN Variable reader
// "local variable" feature by anders, 2003
/* TODO
      add config options (how many lines to read, on/off)
      add interface for plugins/apps to set/get variables
      add view stuff
*/
QRegExp KateDocument::kvLine = QRegExp("kate:(.*)");
QRegExp KateDocument::kvLineWildcard = QRegExp("kate-wildcard\\((.*)\\):(.*)");
QRegExp KateDocument::kvLineMime = QRegExp("kate-mimetype\\((.*)\\):(.*)");
QRegExp KateDocument::kvVar = QRegExp("([\\w\\-]+)\\s+([^;]+)");

void KateDocument::readVariables(bool onlyViewAndRenderer)
{
  if (!onlyViewAndRenderer)
    m_config->configStart();

  // views!
  KateView *v;
  foreach (v,m_views)
  {
    v->config()->configStart();
    v->renderer()->config()->configStart();
  }
  // read a number of lines in the top/bottom of the document
  for (int i=0; i < qMin( 9, lines() ); ++i )
  {
    readVariableLine( line( i ), onlyViewAndRenderer );
  }
  if ( lines() > 10 )
  {
    for ( int i = qMax( 10, lines() - 10); i < lines(); i++ )
    {
      readVariableLine( line( i ), onlyViewAndRenderer );
    }
  }

  if (!onlyViewAndRenderer)
    m_config->configEnd();

  foreach (v,m_views)
  {
    v->config()->configEnd();
    v->renderer()->config()->configEnd();
  }
}

void KateDocument::readVariableLine( QString t, bool onlyViewAndRenderer )
{
  // simple check first, no regex
  // no kate inside, no vars, simple...
  if (!t.contains("kate"))
    return;

  // found vars, if any
  QString s;

  // now, try first the normal ones
  if ( kvLine.indexIn( t ) > -1 )
  {
    s = kvLine.cap(1);

    kDebug (13020) << "normal variable line kate: matched: " << s;
  }
  else if (kvLineWildcard.indexIn( t ) > -1) // regex given
  {
    const QStringList wildcards (kvLineWildcard.cap(1).split (';', QString::SkipEmptyParts));
    const QString nameOfFile = url().fileName();

    bool found = false;
    for (int i = 0; !found && i < wildcards.size(); ++i)
    {
      QRegExp wildcard (wildcards[i], Qt::CaseSensitive, QRegExp::Wildcard);

      found = wildcard.exactMatch (nameOfFile);
    }

    // nothing usable found...
    if (!found)
      return;

    s = kvLineWildcard.cap(2);

    kDebug (13020) << "guarded variable line kate-wildcard: matched: " << s;
  }
  else if (kvLineMime.indexIn( t ) > -1) // mime-type given
  {
    const QStringList types (kvLineMime.cap(1).split (';', QString::SkipEmptyParts));

    // no matching type found
    if (!types.contains (mimeType ()))
      return;

    s = kvLineMime.cap(2);

    kDebug (13020) << "guarded variable line kate-mimetype: matched: " << s;
  }
  else // nothing found
  {
    return;
  }

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
  int spaceIndent = -1;  // for backward compatibility; see below
  bool replaceTabsSet = false;
  int p( 0 );

  QString  var, val;
  while ( (p = kvVar.indexIn( s, p )) > -1 )
  {
    p += kvVar.matchedLength();
    var = kvVar.cap( 1 );
    val = kvVar.cap( 2 ).trimmed();
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
      // KateConfig::configFlags
      // FIXME should this be optimized to only a few calls? how?
      else if ( var == "backspace-indents" && checkBoolValue( val, &state ) )
        m_config->setConfigFlags( KateDocumentConfig::cfBackspaceIndents, state );
      else if ( var == "replace-tabs" && checkBoolValue( val, &state ) )
      {
        m_config->setConfigFlags( KateDocumentConfig::cfReplaceTabsDyn, state );
        replaceTabsSet = true;  // for backward compatibility; see below
      }
      else if ( var == "remove-trailing-space" && checkBoolValue( val, &state ) )
        m_config->setConfigFlags( KateDocumentConfig::cfRemoveTrailingDyn, state );
      else if ( var == "wrap-cursor" && checkBoolValue( val, &state ) )
        m_config->setConfigFlags( KateDocumentConfig::cfWrapCursor, state );
      else if ( var == "auto-brackets" && checkBoolValue( val, &state ) )
        m_config->setConfigFlags( KateDocumentConfig::cfAutoBrackets, state );
      else if ( var == "overwrite-mode" && checkBoolValue( val, &state ) )
        m_config->setConfigFlags( KateDocumentConfig::cfOvr, state );
      else if ( var == "keep-extra-spaces" && checkBoolValue( val, &state ) )
        m_config->setConfigFlags( KateDocumentConfig::cfKeepExtraSpaces, state );
      else if ( var == "tab-indents" && checkBoolValue( val, &state ) )
        m_config->setConfigFlags( KateDocumentConfig::cfTabIndents, state );
      else if ( var == "show-tabs" && checkBoolValue( val, &state ) )
        m_config->setConfigFlags( KateDocumentConfig::cfShowTabs, state );
      else if ( var == "show-trailing-spaces" && checkBoolValue( val, &state ) )
        m_config->setConfigFlags( KateDocumentConfig::cfShowSpaces, state );
      else if ( var == "space-indent" && checkBoolValue( val, &state ) )
      {
        // this is for backward compatibility; see below
        spaceIndent = state;
      }
      else if ( var == "smart-home" && checkBoolValue( val, &state ) )
        m_config->setConfigFlags( KateDocumentConfig::cfSmartHome, state );
      else if ( var == "replace-trailing-space-save" && checkBoolValue( val, &state ) )
        m_config->setConfigFlags( KateDocumentConfig::cfRemoveSpaces, state );

      // INTEGER SETTINGS
      else if ( var == "tab-width" && checkIntValue( val, &n ) )
        m_config->setTabWidth( n );
      else if ( var == "indent-width"  && checkIntValue( val, &n ) )
        m_config->setIndentationWidth( n );
      else if ( var == "indent-mode" )
      {
        m_config->setIndentationMode( val );
      }
      else if ( var == "word-wrap-column" && checkIntValue( val, &n ) && n > 0 ) // uint, but hard word wrap at 0 will be no fun ;)
        m_config->setWordWrapAt( n );

      // STRING SETTINGS
      else if ( var == "eol" || var == "end-of-line" )
      {
        QStringList l;
        l << "unix" << "dos" << "mac";
        if ( (n = l.indexOf( val.toLower() )) != -1 )
          m_config->setEol( n );
      }
      else if (var == "bom" || var =="byte-order-marker")
      {
          if (checkBoolValue(val,&state)) {
            m_config->setBom(state);
          }
      }
      else if ( var == "encoding" )
        m_config->setEncoding( val );
      else if (var == "presave-postdialog")
        setPreSavePostDialogFilterChecks(val.split(','));
      else if (var == "postsave")
        setPostSaveFilterChecks(val.split(','));
      else if (var == "postload")
        setPostLoadFilterChecks(val.split(','));
      else if ( var == "syntax" || var == "hl" )
      {
        setHighlightingMode( val );
      }
      else if ( var == "mode" )
      {
        setMode( val );
      }
      else if ( var == "default-dictionary" )
      {
        setDefaultDictionary( val );
      }
      else if ( var == "automatic-spell-checking" && checkBoolValue( val, &state ) )
      {
        onTheFlySpellCheckingEnabled( state );
      }

      // VIEW SETTINGS
      else if ( vvl.contains( var ) )
        setViewVariable( var, val );
      else
      {
        m_storedVariables.insert( var, val );
        emit variableChanged( this, var, val );
      }
    }
  }

  // Backward compatibility
  // If space-indent was set, but replace-tabs was not set, we assume
  // that the user wants to replace tabulators and set that flag.
  // If both were set, replace-tabs has precedence.
  // At this point spaceIndent is -1 if it was never set,
  // 0 if it was set to off, and 1 if it was set to on.
  // Note that if onlyViewAndRenderer was requested, spaceIndent is -1.
  if ( !replaceTabsSet && spaceIndent >= 0 )
  {
    m_config->setConfigFlags( KateDocumentConfig::cfReplaceTabsDyn, spaceIndent > 0 );
  }
}

void KateDocument::setViewVariable( QString var, QString val )
{
  KateView *v;
  bool state;
  int n;
  QColor c;
  foreach (v,m_views)
  {
    if ( var == "dynamic-word-wrap" && checkBoolValue( val, &state ) )
      v->config()->setDynWordWrap( state );
    else if ( var == "persistent-selection" && checkBoolValue( val, &state ) )
      v->config()->setPersistentSelection( state );
    else if ( var == "block-selection"  && checkBoolValue( val, &state ) )
          v->setBlockSelectionMode( state );
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
      QFont _f( v->renderer()->config()->font() );

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
      v->renderer()->config()->setSchema( val );
    }
  }
}

bool KateDocument::checkBoolValue( QString val, bool *result )
{
  val = val.trimmed().toLower();
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

  return QString();
}

//END

void KateDocument::slotModOnHdDirty (const QString &path)
{
  if ((path == m_dirWatchFile) && (!m_modOnHd || m_modOnHdReason != OnDiskModified))
  {
    // compare md5 with the one we have (if we have one)
    if ( ! m_digest.isEmpty() )
    {
      QByteArray tmp;
      if ( createDigest( tmp ) && tmp == m_digest )
        return;
    }

    m_modOnHd = true;
    m_modOnHdReason = OnDiskModified;

    // reenable dialog if not running atm
    if (m_isasking == -1)
      m_isasking = false;

    emit modifiedOnDisk (this, m_modOnHd, m_modOnHdReason);
  }
}

void KateDocument::slotModOnHdCreated (const QString &path)
{
  if ((path == m_dirWatchFile) && (!m_modOnHd || m_modOnHdReason != OnDiskCreated))
  {
    m_modOnHd = true;
    m_modOnHdReason = OnDiskCreated;

    // reenable dialog if not running atm
    if (m_isasking == -1)
      m_isasking = false;

    emit modifiedOnDisk (this, m_modOnHd, m_modOnHdReason);
  }
}

void KateDocument::slotModOnHdDeleted (const QString &path)
{
  if ((path == m_dirWatchFile) && (!m_modOnHd || m_modOnHdReason != OnDiskDeleted))
  {
    m_modOnHd = true;
    m_modOnHdReason = OnDiskDeleted;

    // reenable dialog if not running atm
    if (m_isasking == -1)
      m_isasking = false;

    emit modifiedOnDisk (this, m_modOnHd, m_modOnHdReason);
  }
}

bool KateDocument::createDigest( QByteArray &result )
{
  bool ret = false;
  result.clear();
  if ( url().isLocalFile() )
  {
    QFile f ( url().toLocalFile() );
    if ( f.open( QIODevice::ReadOnly) )
    {
      KMD5 md5;
      ret = md5.update( f );
      md5.hexDigest( result );
      f.close();
      ret = true;
    }
  }
  return ret;
}

QString KateDocument::reasonedMOHString() const
{
  // squeeze path
  QString str = KStringHandler::csqueeze(url().pathOrUrl());

  switch( m_modOnHdReason )
  {
    case OnDiskModified:
      return i18n("The file '%1' was modified by another program.", str );
      break;
    case OnDiskCreated:
      return i18n("The file '%1' was created by another program.", str );
      break;
    case OnDiskDeleted:
      return i18n("The file '%1' was deleted by another program.", str );
      break;
    default:
      return QString();
  }
  return QString();
}

void KateDocument::removeTrailingSpace(int line)
{
  // remove trailing spaces from left line if required
  if (m_blockRemoveTrailingSpaces
      || !(config()->configFlags() & KateDocumentConfig::cfRemoveTrailingDyn))
    return;

  KateTextLine::Ptr ln = plainKateTextLine(line);

  if (!ln || ln->length() == 0)
    return;

  if (line == activeView()->cursorPosition().line()
      && activeView()->cursorPosition().column() >= qMax(0, ln->lastChar()))
    return;

  const int p = ln->lastChar() + 1;
  const int l = ln->length() - p;
  if (l > 0) {
    m_blockRemoveTrailingSpaces = true;
    editRemoveText(line, p, l);
    m_blockRemoveTrailingSpaces = false;
  }
}

void KateDocument::updateFileType (const QString &newType, bool user)
{
  if (user || !m_fileTypeSetByUser)
  {
    if (!newType.isEmpty())
    {
          m_fileType = newType;

          m_config->configStart();

          if (!hlSetByUser && !KateGlobal::self()->modeManager()->fileType(newType).hl.isEmpty())
          {
            int hl (KateHlManager::self()->nameFind (KateGlobal::self()->modeManager()->fileType(newType).hl));

            if (hl >= 0)
              m_buffer->setHighlight(hl);
          }

          /**
           * set the indentation mode, if any in the mode...
           */
          if (!KateGlobal::self()->modeManager()->fileType(newType).indenter.isEmpty())
             config()->setIndentationMode (KateGlobal::self()->modeManager()->fileType(newType).indenter);

          // views!
          KateView *v;
          foreach (v,m_views)
          {
            v->config()->configStart();
            v->renderer()->config()->configStart();
          }

          bool bom_settings;
          if (m_bomSetByUser)
            bom_settings=m_config->bom();
          readVariableLine( KateGlobal::self()->modeManager()->fileType(newType).varLine );
          if (m_bomSetByUser)
             m_config->setBom(bom_settings);
          m_config->configEnd();
          foreach (v,m_views)
          {
            v->config()->configEnd();
            v->renderer()->config()->configEnd();
          }
    }
  }

  // fixme, make this better...
  emit modeChanged (this);
}

void KateDocument::slotQueryClose_save(bool *handled, bool* abortClosing) {
      *handled=true;
      *abortClosing=true;
      if (this->url().isEmpty())
      {
        QWidget *parentWidget(dialogParent());

        KEncodingFileDialog::Result res=KEncodingFileDialog::getSaveUrlAndEncoding(config()->encoding(),
                QString(),QString(),parentWidget,i18n("Save File"));

        if( res.URLs.isEmpty() || !checkOverwrite( res.URLs.first(), parentWidget ) ) {
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

bool KateDocument::checkOverwrite( KUrl u, QWidget *parent )
{
  if( !u.isLocalFile() )
    return true;

  QFileInfo info( u.path() );
  if( !info.exists() )
    return true;

  return KMessageBox::Cancel != KMessageBox::warningContinueCancel( parent,
    i18n( "A file named \"%1\" already exists. "
          "Are you sure you want to overwrite it?" ,  info.fileName() ),
    i18n( "Overwrite File?" ), KStandardGuiItem::overwrite(),
    KStandardGuiItem::cancel(), QString(), KMessageBox::Notify | KMessageBox::Dangerous );
}

//BEGIN KTextEditor::ConfigInterface

// BEGIN ConfigInterface stff
QStringList KateDocument::configKeys() const
{
  return QStringList() << "auto-brackets";
}

QVariant KateDocument::configValue(const QString &key)
{
  if (key == "auto-brackets") {
    return m_config->configFlags() & KateDocumentConfig::cfAutoBrackets;
  } else if (key == "backup-on-save-local") {
    return m_config->backupFlags() & KateDocumentConfig::LocalFiles;
  } else if (key == "backup-on-save-remote") {
    return m_config->backupFlags() & KateDocumentConfig::RemoteFiles;
  } else if (key == "backup-on-save-suffix") {
    return m_config->backupSuffix();
  } else if (key == "backup-on-save-prefix") {
    return m_config->backupPrefix();
  }

  // return invalid variant
  return QVariant();
}

void KateDocument::setConfigValue(const QString &key, const QVariant &value)
{
  if (value.type() == QVariant::String) {
    if (key == "backup-on-save-suffix") {
      m_config->setBackupSuffix(value.toString());
    } else if (key == "backup-on-save-prefix") {
      m_config->setBackupPrefix(value.toString());
    }
  } else if (value.canConvert(QVariant::Bool)) {
    const bool bValue = value.toBool();
    if (key == "auto-brackets") {
      m_config->setConfigFlags(KateDocumentConfig::cfAutoBrackets, bValue);
    } else if (key == "backup-on-save-local" && value.type() == QVariant::String) {
      uint f = m_config->backupFlags();
      if (bValue) {
        f |= KateDocumentConfig::LocalFiles;
      } else {
        f ^= KateDocumentConfig::LocalFiles;
      }

      m_config->setBackupFlags(f);
    } else if (key == "backup-on-save-remote") {
      uint f = m_config->backupFlags();
      if (bValue) {
        f |= KateDocumentConfig::RemoteFiles;
      } else {
        f ^= KateDocumentConfig::RemoteFiles;
      }

      m_config->setBackupFlags(f);
    }
  }
}

//END KTextEditor::ConfigInterface

//BEGIN KTextEditor::TemplateInterface
bool KateDocument::insertTemplateTextImplementation( const KTextEditor::Cursor &c, const QString &templateString,
                                                     const QMap<QString,QString> &initialValues, QWidget * )
{
  // the handler will delete itself when necessary
  KateTemplateHandler* handler = new KateTemplateHandler(this, c, templateString, initialValues, m_undoManager);

  return true;
}

KateView * KateDocument::activeKateView( ) const
{
  return static_cast<KateView*>(m_activeView);
}

KTextEditor::Cursor KateDocument::documentEnd( ) const
{
  return KTextEditor::Cursor(lastLine(), lineLength(lastLine()));
}
//END KTextEditor::TemplateInterface

//BEGIN KTextEditor::SmartInterface
int KateDocument::currentRevision() const
{
  return m_smartManager->currentRevision();
}

void KateDocument::releaseRevision(int revision) const
{
  m_smartManager->releaseRevision(revision);
}

void KateDocument::useRevision(int revision)
{
  m_smartManager->useRevision(revision);
}

KTextEditor::Cursor KateDocument::translateFromRevision(const KTextEditor::Cursor& cursor, KTextEditor::SmartCursor::InsertBehavior insertBehavior) const
{
  return m_smartManager->translateFromRevision(cursor, insertBehavior);
}

KTextEditor::Range KateDocument::translateFromRevision(const KTextEditor::Range& range, KTextEditor::SmartRange::InsertBehaviors insertBehavior) const
{
  return m_smartManager->translateFromRevision(range, insertBehavior);
}

KTextEditor::SmartCursor* KateDocument::newSmartCursor( const KTextEditor::Cursor & position, KTextEditor::SmartCursor::InsertBehavior insertBehavior )
{
  return m_smartManager->newSmartCursor(position, insertBehavior, false);
}

KTextEditor::SmartRange * KateDocument::newSmartRange( const KTextEditor::Range & range, KTextEditor::SmartRange * parent, KTextEditor::SmartRange::InsertBehaviors insertBehavior )
{
  return m_smartManager->newSmartRange( range, parent, insertBehavior, false );
}

KTextEditor::SmartRange * KateDocument::newSmartRange( KTextEditor::SmartCursor * start, KTextEditor::SmartCursor * end, KTextEditor::SmartRange * parent, KTextEditor::SmartRange::InsertBehaviors insertBehavior )
{
  KateSmartCursor* kstart = dynamic_cast<KateSmartCursor*>(start);
  KateSmartCursor* kend = dynamic_cast<KateSmartCursor*>(end);
  if (!kstart || !kend)
    return 0L;
  if (kstart->range() || kend->range())
    return 0L;
  return m_smartManager->newSmartRange(kstart, kend, parent, insertBehavior, false);
}

void KateDocument::unbindSmartRange( KTextEditor::SmartRange * range )
{
  m_smartManager->unbindSmartRange(range);
}

bool KateDocument::replaceText( const KTextEditor::Range & range, const QString & s, bool block )
{
  // TODO more efficient?
  editStart();
  bool changed = removeText(range, block);
  changed |= insertText(range.start(), s, block);
  editEnd();
  return changed;
}

void KateDocument::addHighlightToDocument( KTextEditor::SmartRange * topRange, bool supportDynamic )
{
  if (m_documentHighlights.contains(topRange))
    return;

  m_documentHighlights.append(topRange);

  // Deal with the range being deleted externally
  topRange->addWatcher(this);

  if (supportDynamic) {
    m_documentDynamicHighlights.append(topRange);
    emit dynamicHighlightAdded(static_cast<KateSmartRange*>(topRange));
  }

  foreach (KateView * view, m_views)
    view->addExternalHighlight(topRange, supportDynamic);
}

void KateDocument::removeHighlightFromDocument( KTextEditor::SmartRange * topRange )
{
  if (!m_documentHighlights.contains(topRange))
    return;

  foreach (KateView * view, m_views)
    view->removeExternalHighlight(topRange);

  m_documentHighlights.removeAll(topRange);
  topRange->removeWatcher(this);

  if (m_documentDynamicHighlights.contains(topRange)) {
    m_documentDynamicHighlights.removeAll(topRange);
    emit dynamicHighlightRemoved(static_cast<KateSmartRange*>(topRange));
  }
}

const QList< KTextEditor::SmartRange * > KateDocument::documentHighlights( ) const
{
  return m_documentHighlights;
}

void KateDocument::addHighlightToView( KTextEditor::View * view, KTextEditor::SmartRange * topRange, bool supportDynamic )
{
  static_cast<KateView*>(view)->addExternalHighlight(topRange, supportDynamic);
}

void KateDocument::removeHighlightFromView( KTextEditor::View * view, KTextEditor::SmartRange * topRange )
{
  static_cast<KateView*>(view)->removeExternalHighlight(topRange);
}

const QList< KTextEditor::SmartRange * > KateDocument::viewHighlights( KTextEditor::View * view ) const
{
  return static_cast<KateView*>(view)->externalHighlights();
}

void KateDocument::addActionsToDocument( KTextEditor::SmartRange * topRange )
{
  if (m_documentActions.contains(topRange))
    return;

  m_documentActions.append(topRange);

  // Deal with the range being deleted externally
  topRange->addWatcher(this);
}

void KateDocument::removeActionsFromDocument( KTextEditor::SmartRange * topRange )
{
  if (!m_documentActions.contains(topRange))
    return;

  m_documentActions.removeAll(topRange);
  topRange->removeWatcher(this);
}

const QList< KTextEditor::SmartRange * > KateDocument::documentActions( ) const
{
  return m_documentActions;
}

void KateDocument::addActionsToView( KTextEditor::View * view, KTextEditor::SmartRange * topRange )
{
  static_cast<KateView*>(view)->addActions(topRange);
}

void KateDocument::removeActionsFromView( KTextEditor::View * view, KTextEditor::SmartRange * topRange )
{
  static_cast<KateView*>(view)->removeActions(topRange);
}

const QList< KTextEditor::SmartRange * > KateDocument::viewActions( KTextEditor::View * view ) const
{
  return static_cast<KateView*>(view)->actions();
}

void KateDocument::attributeDynamic( KTextEditor::Attribute::Ptr )
{
  // TODO link in with cursor + mouse tracking
}

void KateDocument::attributeNotDynamic( KTextEditor::Attribute::Ptr )
{
  // TODO de-link cursor + mouse tracking
}

void KateDocument::clearSmartInterface( )
{
  clearDocumentHighlights();
  foreach (KateView* view, m_views)
    clearViewHighlights(view);

  clearDocumentActions();

  m_smartManager->clear(false);
}

void KateDocument::deleteCursors( )
{
  m_smartManager->deleteCursors(false);
}

void KateDocument::deleteRanges( )
{
  m_smartManager->deleteRanges(false);
}

void KateDocument::clearDocumentHighlights( )
{
  m_documentHighlights.clear();
}

void KateDocument::clearViewHighlights( KTextEditor::View * view )
{
  static_cast<KateView*>(view)->clearExternalHighlights();
}

void KateDocument::clearDocumentActions( )
{
  m_documentActions.clear();
}

void KateDocument::clearViewActions( KTextEditor::View * view )
{
  static_cast<KateView*>(view)->clearActions();
}

void KateDocument::ignoreModifiedOnDiskOnce( )
{
  m_isasking = -1;
}

KateHighlighting * KateDocument::highlight( ) const
{
  return m_buffer->highlight();
}

uint KateDocument::getRealLine( unsigned int virtualLine )
{
  return m_buffer->lineNumber (virtualLine);
}

uint KateDocument::getVirtualLine( unsigned int realLine )
{
  return m_buffer->lineVisibleNumber (realLine);
}

uint KateDocument::visibleLines( )
{
  return m_buffer->countVisible ();
}

KateTextLine::Ptr KateDocument::kateTextLine( uint i )
{
  m_buffer->ensureHighlighted (i);
  return m_buffer->plainLine (i);
}

KateTextLine::Ptr KateDocument::plainKateTextLine( uint i )
{
  return m_buffer->plainLine (i);
}

bool KateDocument::undoDontMerge( ) const
{
  return m_undoManager->undoDontMerge ();
}

void KateDocument::setUndoAllowComplexMerge(bool allow)
{
  m_undoManager->setAllowComplexMerge (allow);
}

bool KateDocument::undoAllowComplexMerge( ) const
{
  return m_undoManager->allowComplexMerge ();
}

void KateDocument::setUndoDontMerge(bool dontMerge)
{
  m_undoManager->setUndoDontMerge (dontMerge);
}

bool KateDocument::isEditRunning() const
{
  return editIsRunning;
}

void KateDocument::setMergeAllEdits(bool merge)
{
  setUndoDontMerge(true);
  setUndoAllowComplexMerge(merge);
}

void KateDocument::rangeDeleted( KTextEditor::SmartRange * range )
{
  removeHighlightFromDocument(range);
  removeActionsFromDocument(range);
}

//END KTextEditor::SmartInterface

// kate: space-indent on; indent-width 2; replace-tabs on;

bool KateDocument::simpleMode ()
{
  return KateGlobal::self()->simpleMode () && KateGlobal::self()->documentConfig()->allowSimpleMode ();
}

KateDocument::LoadSaveFilterCheckPlugins* KateDocument::loadSaveFilterCheckPlugins()
{
  K_GLOBAL_STATIC(KateDocument::LoadSaveFilterCheckPlugins, s_loadSaveFilterCheckPlugins)
  return s_loadSaveFilterCheckPlugins;
}

//BEGIN KTextEditor::AnnotationInterface
void KateDocument::setAnnotationModel( KTextEditor::AnnotationModel* model )
{
  KTextEditor::AnnotationModel* oldmodel = m_annotationModel;
  m_annotationModel = model;
  emit annotationModelChanged(oldmodel, m_annotationModel);
}

KTextEditor::AnnotationModel* KateDocument::annotationModel() const
{
  return m_annotationModel;
}
//END KTextEditor::AnnotationInterface

//TAKEN FROM kparts.h
bool KateDocument::queryClose()
{
    if ( !isReadWrite() || !isModified() )
        return true;

    QString docName = documentName();

    QWidget *parentWidget=widget();
    if(!parentWidget) parentWidget=QApplication::activeWindow();

    int res = KMessageBox::warningYesNoCancel( parentWidget,
                                               i18n( "The document \"%1\" has been modified.\n"
                                                     "Do you want to save your changes or discard them?" ,  docName ),
                                               i18n( "Close Document" ), KStandardGuiItem::save(), KStandardGuiItem::discard() );

    bool abortClose=false;
    bool handled=false;

    switch(res) {
    case KMessageBox::Yes :
        sigQueryClose(&handled,&abortClose);
        if (!handled)
        {
            if (url().isEmpty())
            {
                KUrl url = KFileDialog::getSaveUrl(KUrl(), QString(), parentWidget);
                if (url.isEmpty())
                    return false;

                saveAs( url );
            }
            else
            {
                save();
            }
        } else if (abortClose) return false;
        return waitSaveComplete();
    case KMessageBox::No :
        return true;
    default : // case KMessageBox::Cancel :
        return false;
    }
}


void KateDocument::slotCanceled() {
  m_savingToUrl=false;
  m_saveAs=false;
}

void KateDocument::slotCompleted() {
  if (m_savingToUrl) {
    if (!m_postSaveFilterChecks.isEmpty())
    {
      LoadSaveFilterCheckPlugins *lscps1=loadSaveFilterCheckPlugins();
      foreach(const QString& checkplugin, m_postSaveFilterChecks)
      {
        if (lscps1->postSaveFilterCheck(checkplugin,this,m_saveAs)==false)
          break;
      }
    }
    emit documentSavedOrUploaded(this,m_saveAs);
  }
  m_savingToUrl=false;
  m_saveAs=false;
}

bool KateDocument::save() {
  m_saveAs = false;
  return KTextEditor::Document::save();
}

bool KateDocument::saveAs( const KUrl &url ) {
  m_saveAs = true;
  return KTextEditor::Document::saveAs(url);
}

QString KateDocument::defaultDictionary() const
{
  return m_defaultDictionary;
}

QList<QPair<KTextEditor::SmartRange*, QString> > KateDocument::dictionaryRanges() const
{
  return m_dictionaryRanges;
}

void KateDocument::clearDictionaryRanges()
{
  QMutexLocker smartLock(smartMutex());
  for(QList<QPair<KTextEditor::SmartRange*, QString> >::iterator i = m_dictionaryRanges.begin();
      i != m_dictionaryRanges.end();++i)
  {
    delete (*i).first;
  }
  m_dictionaryRanges.clear();
  if(m_onTheFlyChecker)
  {
    m_onTheFlyChecker->refreshSpellCheck();
  }
  emit dictionaryRangesPresent(false);
}

void KateDocument::setDictionary(const QString& newDictionary, const KTextEditor::Range &range)
{
  KTextEditor::Range newDictionaryRange = range;
  if(!newDictionaryRange.isValid() || newDictionaryRange.isEmpty())
  {
    return;
  }
  QMutexLocker smartLock(smartMutex());
  QList<QPair<KTextEditor::SmartRange*, QString> > newRanges;
  // all ranges is 'm_dictionaryRanges' are assumed to be mutually disjoint
  for(QList<QPair<KTextEditor::SmartRange*, QString> >::iterator i = m_dictionaryRanges.begin();
      i != m_dictionaryRanges.end();)
  {
    kDebug(13000) << "new iteration" << newDictionaryRange;
    if(newDictionaryRange.isEmpty())
    {
      break;
    }
    QPair<KTextEditor::SmartRange*, QString> pair = *i;
    QString dictionarySet = pair.second;
    KTextEditor::SmartRange *dictionaryRange = pair.first;
    kDebug(13000) << *dictionaryRange << dictionarySet;
    if(dictionaryRange->contains(newDictionaryRange) && newDictionary == dictionarySet)
    {
      kDebug(13000) << "dictionaryRange contains newDictionaryRange";
      return;
    }
    if(newDictionaryRange.contains(*dictionaryRange))
    {
      delete dictionaryRange;
      i = m_dictionaryRanges.erase(i);
      kDebug(13000) << "newDictionaryRange contains dictionaryRange";
      continue;
    }

    KTextEditor::Range intersection = dictionaryRange->intersect(newDictionaryRange);
    if(!intersection.isEmpty() && intersection.isValid())
    {
      if(dictionarySet == newDictionary)  // we don't have to do anything for 'intersection'
      {                                   // except cut off the intersection
        QList<KTextEditor::Range> remainingRanges = KateSpellCheckManager::rangeDifference(newDictionaryRange, intersection);
        Q_ASSERT(remainingRanges.size() == 1);
        newDictionaryRange = remainingRanges.first();
        ++i;
        kDebug(13000) << "dictionarySet == newDictionary";
        continue;
      }
      QList<KTextEditor::Range> remainingRanges = KateSpellCheckManager::rangeDifference(*dictionaryRange, intersection);
      for(QList<KTextEditor::Range>::iterator j = remainingRanges.begin(); j != remainingRanges.end(); ++j)
      {
        KTextEditor::SmartRange *remainingRange = newSmartRange(*j, NULL,
                                                                KTextEditor::SmartRange::ExpandLeft | KTextEditor::SmartRange::ExpandRight);
        remainingRange->addNotifier(dictionaryRangeNotifier());
        newRanges.push_back(QPair<KTextEditor::SmartRange*, QString>(remainingRange, dictionarySet));
      }
      i = m_dictionaryRanges.erase(i);
      delete dictionaryRange;
    } else {
      ++i;
    }
  }
  m_dictionaryRanges += newRanges;
  if(!newDictionaryRange.isEmpty() && !newDictionary.isEmpty()) // we don't add anything for the default dictionary
  {
    KTextEditor::SmartRange *newDictionarySmartRange = newSmartRange(newDictionaryRange, NULL,
                                                                     KTextEditor::SmartRange::ExpandLeft | KTextEditor::SmartRange::ExpandRight);
    newDictionarySmartRange->addNotifier(dictionaryRangeNotifier());
    m_dictionaryRanges.push_back(QPair<KTextEditor::SmartRange*, QString>(newDictionarySmartRange, newDictionary));
  }
  if(m_onTheFlyChecker && !newDictionaryRange.isEmpty())
  {
    m_onTheFlyChecker->refreshSpellCheck(newDictionaryRange);
  }
  emit dictionaryRangesPresent(!m_dictionaryRanges.isEmpty());
}

void KateDocument::revertToDefaultDictionary(const KTextEditor::Range &range)
{
  setDictionary(QString(), range);
}

void KateDocument::setDefaultDictionary(const QString& dict)
{
  if(m_defaultDictionary == dict)
  {
    return;
  }
  m_defaultDictionary = dict;
  if(m_onTheFlyChecker)
  {
    m_onTheFlyChecker->updateConfig();
  }
  refreshOnTheFlyCheck();
  emit defaultDictionaryChanged(this);
}

void KateDocument::onTheFlySpellCheckingEnabled(bool enable)
{
  if (isOnTheFlySpellCheckingEnabled() == enable)
  {
    return;
  }
  if (enable)
  {
    if (!m_onTheFlyChecker) {
      m_onTheFlyChecker = new KateOnTheFlyChecker(this);
    }
  } else {
    delete m_onTheFlyChecker;
    m_onTheFlyChecker = 0;
  }
  foreach(KateView *view, m_views) {
    view->reflectOnTheFlySpellCheckStatus(enable);
  }
}

bool KateDocument::isOnTheFlySpellCheckingEnabled() const
{
  return m_onTheFlyChecker != 0;
}

QPair<KTextEditor::Range, QString> KateDocument::onTheFlyMisspelledItem(const KTextEditor::Cursor &cursor) const
{
  if (!m_onTheFlyChecker)
  {
    return QPair<KTextEditor::Range, QString>(KTextEditor::Range::invalid(), QString());
  } else {
    return m_onTheFlyChecker->getMisspelledItem(cursor);
  }
}

QString KateDocument::dictionaryForMisspelledRange(const KTextEditor::Range& range) const
{
  if (!m_onTheFlyChecker)
  {
    return QString();
  } else {
    return m_onTheFlyChecker->dictionaryForMisspelledRange(range);
  }
}

void KateDocument::clearMisspellingForWord(const QString& word)
{
  if(m_onTheFlyChecker) {
    m_onTheFlyChecker->clearMisspellingForWord(word);
  }
}

void KateDocument::refreshOnTheFlyCheck(const KTextEditor::Range &range)
{
  if(m_onTheFlyChecker) {
    m_onTheFlyChecker->refreshSpellCheck(range);
  }
}

void KateDocument::dictionaryRangeEliminated(KTextEditor::SmartRange *smartRange)
{
  QMutexLocker smartLock(smartMutex());
  kDebug(13020) << smartRange << "eliminated";
  bool found = false;
  for(QList<QPair<KTextEditor::SmartRange*, QString> >::iterator i = m_dictionaryRanges.begin();
      i != m_dictionaryRanges.end();)
  {
    KTextEditor::SmartRange *dictionaryRange = (*i).first;
    if(dictionaryRange == smartRange)
    {
      m_discardedSmartRanges.push_back(dictionaryRange); // we can't delete them here already
      found = true;
      i = m_dictionaryRanges.erase(i);
    } else {
      ++i;
    }
  }
  if(found)
  {
    QTimer::singleShot(0, this, SLOT(deleteDiscardedSmartRanges()));
  }
}

void KateDocument::deleteDiscardedSmartRanges()
{
  for(QList<KTextEditor::SmartRange*>::iterator i = m_discardedSmartRanges.begin();
                                                i != m_discardedSmartRanges.end(); ++i)
  {
    delete(*i);
  }
  m_discardedSmartRanges.clear();
}

KTextEditor::SmartRangeNotifier* KateDocument::dictionaryRangeNotifier()
{
  if(m_dictionaryRangeNotifier)
  {
    return m_dictionaryRangeNotifier;
  }
  m_dictionaryRangeNotifier = new KTextEditor::SmartRangeNotifier();
  connect(m_dictionaryRangeNotifier, SIGNAL(rangeEliminated(KTextEditor::SmartRange*)),
          this, SLOT(dictionaryRangeEliminated(KTextEditor::SmartRange*)));
  m_dictionaryRangeNotifier->setWantsDirectChanges(true);
  return m_dictionaryRangeNotifier;
}

bool KateDocument::containsCharacterEncoding(const KTextEditor::Range& range)
{
  KateHighlighting *highlighting = highlight();
  KateTextLine::Ptr textLine;

  const int rangeStartLine = range.start().line();
  const int rangeStartColumn = range.start().column();
  const int rangeEndLine = range.end().line();
  const int rangeEndColumn = range.end().column();

  for(int line = range.start().line(); line <= rangeEndLine; ++line) {
    textLine = kateTextLine(line);
    int startColumn = (line == rangeStartLine) ? rangeStartColumn : 0;
    int endColumn = (line == rangeEndLine) ? rangeEndColumn : textLine->length();
    for(int col = startColumn; col < endColumn; ++col) {
      int attr = textLine->attribute(col);
      const KatePrefixStore& prefixStore = highlighting->getCharacterEncodingsPrefixStore(attr);
      if(!prefixStore.findPrefix(textLine, col).isEmpty()) {
        return true;
      }
    }
  }

  return false;
}

int KateDocument::computePositionWrtOffsets(const OffsetList& offsetList, int pos)
{
  int previousOffset = 0;
  for(OffsetList::const_iterator i = offsetList.begin(); i != offsetList.end(); ++i) {
    if((*i).first > pos) {
      break;
    }
    previousOffset = (*i).second;
  }
  return pos + previousOffset;
}

QString KateDocument::decodeCharacters(const KTextEditor::Range& range, KateDocument::OffsetList& decToEncOffsetList,
                                                                        KateDocument::OffsetList& encToDecOffsetList)
{
  QString toReturn;
  KTextEditor::Cursor previous = range.start();
  int decToEncCurrentOffset = 0, encToDecCurrentOffset = 0;
  int i = 0;
  int newI = 0;

  KateHighlighting *highlighting = highlight();
  KateTextLine::Ptr textLine;

  const int rangeStartLine = range.start().line();
  const int rangeStartColumn = range.start().column();
  const int rangeEndLine = range.end().line();
  const int rangeEndColumn = range.end().column();

  for(int line = range.start().line(); line <= rangeEndLine; ++line) {
    textLine = kateTextLine(line);
    int startColumn = (line == rangeStartLine) ? rangeStartColumn : 0;
    int endColumn = (line == rangeEndLine) ? rangeEndColumn : textLine->length();
    for(int col = startColumn; col < endColumn;) {
      int attr = textLine->attribute(col);
      const KatePrefixStore& prefixStore = highlighting->getCharacterEncodingsPrefixStore(attr);
      const QHash<QString, QChar>& characterEncodingsHash = highlighting->getCharacterEncodings(attr);
      QString matchingPrefix = prefixStore.findPrefix(textLine, col);
      if(!matchingPrefix.isEmpty()) {
        toReturn += text(KTextEditor::Range(previous, KTextEditor::Cursor(line, col)));
        const QChar& c = characterEncodingsHash[matchingPrefix];
        const bool isNullChar = c.isNull();
        if(!c.isNull()) {
          toReturn += c;
        }
        i += matchingPrefix.length();
        col += matchingPrefix.length();
        previous = KTextEditor::Cursor(line, col);
        decToEncCurrentOffset = decToEncCurrentOffset - (isNullChar ? 0 : 1) + matchingPrefix.length();
        encToDecCurrentOffset = encToDecCurrentOffset - matchingPrefix.length() + (isNullChar ? 0 : 1);
        newI += (isNullChar ? 0 : 1);
        decToEncOffsetList.push_back(QPair<int, int>(newI, decToEncCurrentOffset));
        encToDecOffsetList.push_back(QPair<int, int>(i, encToDecCurrentOffset));
        continue;
      }
      ++col;
      ++i;
      ++newI;
    }
    ++i;
    ++newI;
  }
  if(previous < range.end()) {
    toReturn += text(KTextEditor::Range(previous, range.end()));
  }
  return toReturn;
}

void KateDocument::replaceCharactersByEncoding(const KTextEditor::Range& range)
{
  KateHighlighting *highlighting = highlight();
  KateTextLine::Ptr textLine;

  const int rangeStartLine = range.start().line();
  const int rangeStartColumn = range.start().column();
  const int rangeEndLine = range.end().line();
  const int rangeEndColumn = range.end().column();

  for(int line = range.start().line(); line <= rangeEndLine; ++line) {
    textLine = kateTextLine(line);
    int startColumn = (line == rangeStartLine) ? rangeStartColumn : 0;
    int endColumn = (line == rangeEndLine) ? rangeEndColumn : textLine->length();
    for(int col = startColumn; col < endColumn;) {
      int attr = textLine->attribute(col);
      const QHash<QChar, QString>& reverseCharacterEncodingsHash = highlighting->getReverseCharacterEncodings(attr);
      QHash<QChar, QString>::const_iterator it = reverseCharacterEncodingsHash.find(textLine->at(col));
      if(it != reverseCharacterEncodingsHash.end()) {
        replaceText(KTextEditor::Range(line, col, line, col + 1), *it);
        col += (*it).length();
        continue;
      }
      ++col;
    }
  }
}

//
// KTextEditor::HighlightInterface
//

KTextEditor::Attribute::Ptr KateDocument::defaultStyle(const KTextEditor::HighlightInterface::DefaultStyle ds) const
{
  ///TODO: move attributes to document, they are not view-dependant
  KateView* view = activeKateView();
  if ( !view ) {
    kWarning() << "ATTENTION: cannot access defaultStyle() without any View (will be fixed eventually)";
    return KTextEditor::Attribute::Ptr(0);
  }

  KateAttributeList list;
  return highlight()->attributes(view->renderer()->config()->schema()).at(ds);
}

QList< KTextEditor::HighlightInterface::AttributeBlock > KateDocument::lineAttributes(const unsigned int line)
{
  ///TODO: move attributes to document, they are not view-dependant

  QList< KTextEditor::HighlightInterface::AttributeBlock > attribs;

  KateView* view = activeKateView();
  if ( !view ) {
    kWarning() << "ATTENTION: cannot access lineAttributes() without any View (will be fixed eventually)";
    return attribs;
  }

  KateTextLine::Ptr kateLine = kateTextLine(line);

  if ( !kateLine ) {
    return attribs;
  }

  const QVector<int> & intAttrs = kateLine->attributesList();

  Q_ASSERT(intAttrs.size() % 3 == 0);

  for ( int i = 0; i < intAttrs.size(); i += 3 ) {
    attribs << KTextEditor::HighlightInterface::AttributeBlock(
      intAttrs[i],
      intAttrs[i+1],
      view->renderer()->attribute(intAttrs[i+2])
    );
  }

  return attribs;
}

QStringList KateDocument::embeddedHighlightingModes() const
{
  return highlight()->getEmbeddedModes();
}

QString KateDocument::highlightingModeAt(const KTextEditor::Cursor& position)
{
  KateTextLine::Ptr kateLine = kateTextLine(position.line());

  const QVector<int> & intAttrs = kateLine->attributesList();

  Q_ASSERT(intAttrs.size() % 3 == 0);

  for ( int i = 0; i < intAttrs.size(); i += 3 ) {
    if ( intAttrs[i] <= position.column() && intAttrs[i] + intAttrs[i+1] > position.column() ) {
      return KateHlManager::self()->nameForIdentifier(highlight()->hlKeyForAttrib(intAttrs[i+2]));
    }
  }

  return mode();
}

// kate: space-indent on; indent-width 2; replace-tabs on;
