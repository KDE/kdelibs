/*
    $Id$

    Copyright (C) 2000 Michael Koch <koch@kde.org>

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

#ifndef __kwritepart_h__
#define __kwritepart_h__

#include <kparts/part.h>
#include <kparts/browserextension.h>

class KWCursor;
class KWrite;
class KWriteDoc;
class KAction;
class KSelectAction;
class KConfig;
class KTextPrint;
class HlManager;

class KWritePart : public KParts::ReadWritePart
{
  Q_OBJECT

public:

  KWritePart( QWidget * parentWidget, QObject *parent, bool bBrowser = false );
  virtual ~KWritePart();

  QWidget* editorWidget();

  virtual void setReadWrite( bool rw = true );

  virtual void saveConfig( KConfig* config );
  virtual void restoreConfig( KConfig* config );

  KWriteDoc* doc() { return m_doc; };

protected:

  virtual bool openFile();
  virtual bool saveFile();

public slots:

  virtual void newDoc();
  virtual void slotPrint();
  virtual void slotCut();
  virtual void slotCopy();
  virtual void slotPaste();
  virtual void slotUndo();
  virtual void slotRedo();
  virtual void slotUndoRedoHistory();
  virtual void slotIndent();
  virtual void slotUnindent();
  virtual void slotCleanIndent();
  virtual void slotSelectAll();
  virtual void slotDeselectAll();
  virtual void slotInvertSelection();
  virtual void slotSpellChecking();

  virtual void slotFind();
  virtual void slotReplace();
  virtual void slotFindAgain();
  virtual void slotGotoLine();

  virtual void slotSetBookmark();
  virtual void slotAddBookmark();
  virtual void slotClearBookmarks();

  virtual void slotHighlighting( int );
  virtual void slotOptions();
  virtual void slotEOL( int );
  virtual void slotVerticalSelections();

  virtual void newStatus();
  virtual void newCurPos();
  virtual void newCaption();
  virtual void newUndo();
  //virtual void slotDropEvent( QDropEvent* );

  void doPrint( KTextPrint& );

signals:

  void newCursorPos( const QPoint& );

private:

  static HlManager * hlManager;

  KWrite        *m_kwrite;
  KWriteDoc     *m_doc;
  KSelectAction *m_langAction;
  KAction       *m_cut, *m_copy, *m_paste, *m_undo, *m_redo, *m_replace;
  KAction       *m_indent, *m_unindent, *m_cleanIndent, *m_spell;
};

class KWriteBrowserExtension : public KParts::BrowserExtension
{
  Q_OBJECT

public:

  KWriteBrowserExtension( KWritePart *part );
};

#endif // __kwritepart_h__
