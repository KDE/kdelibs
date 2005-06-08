/* This file is part of the KDE libraries
   Copyright (C) 2002 John Firebaugh <jfirebaugh@kde.org>
   Copyright (C) 2001 Christoph Cullmann <cullmann@kde.org>
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

#ifndef kate_view_h
#define kate_view_h

#include "katedocument.h"
#include "kateviewinternal.h"
#include "kateconfig.h"

#include <ktexteditor/view.h>
#include <ktexteditor/texthintinterface.h>
#include <ktexteditor/markinterface.h>
#include <ktexteditor/codecompletioninterface.h>
#include <ktexteditor/sessionconfiginterface.h>

#include <qpointer.h>
#include <Q3PopupMenu>

class KateDocument;
class KateBookmarks;
class KateSearch;
class KateCmdLine;
class KateCodeCompletion;
class KateViewConfig;
class KateViewSchemaAction;
class KateRenderer;
class KateRangeList;
class KateSpell;

class KToggleAction;
class KAction;
class KRecentFilesAction;
class KSelectAction;

class QVBoxLayout;

//
// Kate KTextEditor::View class ;)
//
class KateView : public KTextEditor::View,
                 public KTextEditor::TextHintInterface,
                 public KTextEditor::CodeCompletionInterface,
                 public KTextEditor::SessionConfigInterface
{
    Q_OBJECT

    friend class KateViewInternal;
    friend class KateIconBorder;
    friend class KateCodeCompletion;

  public:
    KateView( KateDocument* doc, QWidget* parent );
    ~KateView ();

    KTextEditor::Document *document () { return m_doc; }

    QString viewMode () const;

  signals:
    void viewModeChanged ( KTextEditor::View *view );

  //
  // KTextEditor::ClipboardInterface
  //
  public slots:
    // TODO: Factor out of m_viewInternal
    void paste()         {  m_doc->paste( this ); m_viewInternal->repaint(); }
    void cut();
    void copy() const;

    /**
     * internal use, copy text as HTML to clipboard
     */
    void copyHTML();

  // helper to export text as html stuff
  private:
    QString selectionAsHtml ();
    QString textAsHtml ( uint startLine, uint startCol, uint endLine, uint endCol, bool blockwise);
    void textAsHtmlStream ( uint startLine, uint startCol, uint endLine, uint endCol, bool blockwise, QTextStream *ts);

    /**
     * Gets a substring in valid-xml html.
     * Example:  "<b>const</b> b = <i>34</i>"
     * It won't contain <p> or <body> or <html> or anything like that.
     *
     * @param startCol start column of substring
     * @param length length of substring
     * @param renderer The katerenderer.  This will have the schema
     *                 information that describes how to render the
     *                 attributes.
     * @param outputStream A stream to write the html to
     */
    void lineAsHTML (KateTextLine::Ptr line, uint startCol, uint length, QTextStream *outputStream);

  public slots:
    void exportAsHTML ();

  //
  // KTextEditor::PopupMenuInterface
  //
  public:
    void setContextMenu( QMenu* menu ) { m_rmbMenu = menu; }
    QMenu* contextMenu()              { return m_rmbMenu; }

  //
  // KTextEditor::ViewCursorInterface
  //
  public:
    bool setCursorPosition (const KTextEditor::Cursor &position)
      { return setCursorPositionInternal( position.line(), position.column(), 1, true ); }

    const KTextEditor::Cursor &cursorPosition () const
     { return m_viewInternal->getCursor(); }

    KTextEditor::Cursor cursorPositionVirtual () const
     { return KTextEditor::Cursor (cursorLine(), cursorColumn()); }

    QPoint cursorPositionCoordinates() const
        { return m_viewInternal->cursorCoordinates(); }

    void cursorPosition( int& l, int& c ) const
        { l = cursorLine(); c = cursorColumn();     }
    void cursorPositionReal( int& l, int& c ) const
        { l = cursorLine(); c = cursorColumnReal(); }
    bool setCursorPosition( int line, int col )
        { return setCursorPositionInternal( line, col, tabWidth(), true );  }
    bool setCursorPositionReal( int line, int col)
        { return setCursorPositionInternal( line, col, 1, true );           }
    int cursorLine() const
        { return m_viewInternal->getCursor().line();                    }
    int cursorColumn() const;
    int cursorColumnReal() const
        { return m_viewInternal->getCursor().column();                     }

  signals:
    void cursorPositionChanged(KTextEditor::View *view);
    void caretPositionChanged(const KTextEditor::Cursor& newPosition);
    void mousePositionChanged(const KTextEditor::Cursor& newPosition);

  private slots:
    void slotMousePositionChanged();
    void slotCaretPositionChanged();

  //
  // KTextEditor::CodeCompletionInterface
  //
  public slots:
    void showArgHint( QStringList arg1, const QString& arg2, const QString& arg3 );
    void showCompletionBox( Q3ValueList<KTextEditor::CompletionEntry> arg1, int offset = 0, bool cs = true );

  signals:
    void completionAborted();
    void completionDone();
    void argHintHidden();
    void completionDone(KTextEditor::CompletionEntry);
    void filterInsertString(KTextEditor::CompletionEntry*,QString *);
    void aboutToShowCompletionBox();

  //
  // KTextEditor::TextHintInterface
  //
  public:
    void enableTextHints(int timeout);
    void disableTextHints();

  signals:
    void needTextHint(int line, int col, QString &text);

  //
  // KTextEditor::DynWordWrapInterface
  //
  public:
    void setDynWordWrap( bool b );
    bool dynWordWrap() const      { return m_hasWrap; }

  //
  // KTextEditor::SelectionInterface stuff
  //
  public slots:
    virtual bool setSelection ( const KTextEditor::Cursor &startPosition, const KTextEditor::Cursor &endPosition );

    virtual bool selection () const { return hasSelection(); }

    virtual QString selectionText () const;

    virtual bool removeSelection () { return clearSelection(); }

    virtual bool removeSelectionText () { return removeSelectedText(); }

    virtual const KTextEditor::Cursor &selectionStart () const { return selectStart; }

    virtual const KTextEditor::Cursor &selectionEnd () const { return selectEnd; }

    virtual bool setBlockSelection (bool on) { return setBlockSelectionMode (on); }

    virtual bool blockSelection () const { return blockSelectionMode (); }

    bool setSelection ( int startLine, int startCol,
      int endLine, int endCol );
    bool clearSelection ();
    bool clearSelection (bool redraw, bool finishedChangingSelection = true);

    bool hasSelection () const;

    bool removeSelectedText ();

    bool selectAll();

    //
    // KTextEditor::SelectionInterfaceExt
    //
    int selectionStartLine() const { return selectStart.line(); };
    int selectionStartColumn() const { return selectStart.column(); };
    int selectionEndLine() const  { return selectEnd.line(); };
    int selectionEndColumn()  const  { return selectEnd.column(); };

  signals:
    void selectionChanged (KTextEditor::View *view);

  //
  // internal helper stuff, for katerenderer and so on
  //
  public:
    /**
     * accessors to the selection start
     * @return selection start cursor (read-only)
     */
    inline const KateSuperCursor &selStart () const { return selectStart; }

    /**
     * accessors to the selection end
     * @return selection end cursor (read-only)
     */
    inline const KateSuperCursor &selEnd () const { return selectEnd; }

    // should cursor be wrapped ? take config + blockselection state in account
    bool wrapCursor ();

    // some internal functions to get selection state of a line/col
    bool lineColSelected (int line, int col);
    bool lineSelected (int line);
    bool lineEndSelected (int line, int endCol);
    bool lineHasSelected (int line);
    bool lineIsSelection (int line);

    void tagSelection (const KTextEditor::Cursor &oldSelectStart, const KTextEditor::Cursor &oldSelectEnd);

    void selectWord(   const KTextEditor::Cursor& cursor );
    void selectLine(   const KTextEditor::Cursor& cursor );
    void selectLength( const KTextEditor::Cursor& cursor, int length );

  //
  // KTextEditor::BlockSelectionInterface stuff
  //
  public slots:
    bool blockSelectionMode () const;
    bool setBlockSelectionMode (bool on);
    bool toggleBlockSelectionMode ();


  //BEGIN EDIT STUFF
  public:
    void editStart ();
    void editEnd (int editTagLineStart, int editTagLineEnd, bool tagFrom);

    void editSetCursor (const KTextEditor::Cursor &cursor);
  //END

  //BEGIN TAG & CLEAR
  public:
    bool tagLine (const KTextEditor::Cursor& virtualCursor);

    bool tagRange (const KateRange& range, bool realLines = false);
    bool tagLines (int start, int end, bool realLines = false );
    bool tagLines (KTextEditor::Cursor start, KTextEditor::Cursor end, bool realCursors = false);

    void tagAll ();

    void clear ();

    void repaintText (bool paintOnlyDirty = false);

    void updateView (bool changed = false);
  //END

  //
  // KTextEditor::View
  //
  public:
    /**
     Return values for "save" related commands.
    */
    bool isOverwriteMode() const;
    void setOverwriteMode( bool b );

    QString currentTextLine()
        { return m_doc->line( cursorLine() ); }
    QString currentWord()
        { return m_doc->getWord( m_viewInternal->getCursor() ); }
    void insertText( const QString& text )
        { m_doc->insertText( m_viewInternal->getCursor(), text ); }
    int tabWidth()                { return m_doc->config()->tabWidth(); }
    void setTabWidth( int w )     { m_doc->config()->setTabWidth(w);  }
    void setEncoding( QString e ) { m_doc->setEncoding(e);       }
    bool isLastView()             { return m_doc->isLastView(1); }

  public slots:
    void indent()             { m_doc->indent( this, cursorLine(), 1 );  }
    void unIndent()           { m_doc->indent( this, cursorLine(), -1 ); }
    void cleanIndent()        { m_doc->indent( this, cursorLine(), 0 );  }
    void align()              { m_doc->align( this, cursorLine() ); }
    void comment()            { m_doc->comment( this, cursorLine(), cursorColumnReal(), 1 );  }
    void uncomment()          { m_doc->comment( this, cursorLine(), cursorColumnReal(),-1 ); }
    void killLine()           { m_doc->removeLine( cursorLine() ); }

    /**
      Uppercases selected text, or an alphabetic character next to the cursor.
    */
    void uppercase() { m_doc->transform( this, m_viewInternal->cursor, KateDocument::Uppercase ); }
    /**
      Lowercases selected text, or an alphabetic character next to the cursor.
    */
    void lowercase() { m_doc->transform( this, m_viewInternal->cursor, KateDocument::Lowercase ); }
    /**
      Capitalizes the selection (makes each word start with an uppercase) or
      the word under the cursor.
    */
    void capitalize() { m_doc->transform( this, m_viewInternal->cursor, KateDocument::Capitalize ); }
    /**
      Joins lines touched by the selection
    */
    void joinLines();


    void keyReturn()          { m_viewInternal->doReturn();          }
    void backspace()          { m_viewInternal->doBackspace();       }
    void deleteWordLeft()     { m_viewInternal->doDeleteWordLeft();  }
    void keyDelete()          { m_viewInternal->doDelete();          }
    void deleteWordRight()    { m_viewInternal->doDeleteWordRight(); }
    void transpose()          { m_viewInternal->doTranspose();       }
    void cursorLeft()         { m_viewInternal->cursorLeft();        }
    void shiftCursorLeft()    { m_viewInternal->cursorLeft(true);    }
    void cursorRight()        { m_viewInternal->cursorRight();       }
    void shiftCursorRight()   { m_viewInternal->cursorRight(true);   }
    void wordLeft()           { m_viewInternal->wordLeft();          }
    void shiftWordLeft()      { m_viewInternal->wordLeft(true);      }
    void wordRight()          { m_viewInternal->wordRight();         }
    void shiftWordRight()     { m_viewInternal->wordRight(true);     }
    void home()               { m_viewInternal->home();              }
    void shiftHome()          { m_viewInternal->home(true);          }
    void end()                { m_viewInternal->end();               }
    void shiftEnd()           { m_viewInternal->end(true);           }
    void up()                 { m_viewInternal->cursorUp();          }
    void shiftUp()            { m_viewInternal->cursorUp(true);      }
    void down()               { m_viewInternal->cursorDown();        }
    void shiftDown()          { m_viewInternal->cursorDown(true);    }
    void scrollUp()           { m_viewInternal->scrollUp();          }
    void scrollDown()         { m_viewInternal->scrollDown();        }
    void topOfView()          { m_viewInternal->topOfView();         }
    void shiftTopOfView()     { m_viewInternal->topOfView(true);     }
    void bottomOfView()       { m_viewInternal->bottomOfView();      }
    void shiftBottomOfView()  { m_viewInternal->bottomOfView(true);  }
    void pageUp()             { m_viewInternal->pageUp();            }
    void shiftPageUp()        { m_viewInternal->pageUp(true);        }
    void pageDown()           { m_viewInternal->pageDown();          }
    void shiftPageDown()      { m_viewInternal->pageDown(true);      }
    void top()                { m_viewInternal->top_home();          }
    void shiftTop()           { m_viewInternal->top_home(true);      }
    void bottom()             { m_viewInternal->bottom_end();        }
    void shiftBottom()        { m_viewInternal->bottom_end(true);    }
    void toMatchingBracket()  { m_viewInternal->cursorToMatchingBracket();}
    void shiftToMatchingBracket()  { m_viewInternal->cursorToMatchingBracket(true);}

    void gotoLine();

  // config file / session management functions
  public:
    void readSessionConfig(KConfig *);
    void writeSessionConfig(KConfig *);

  public slots:
    int getEol();
    void setEol( int eol );
    void find();
    void find( const QString&, long, bool add=true ); ///< proxy for KateSearch
    void replace();
    void replace( const QString&, const QString &, long ); ///< proxy for KateSearch
    void findAgain( bool back );
    void findAgain()              { findAgain( false );          }
    void findPrev()               { findAgain( true );           }

    void setFoldingMarkersOn( bool enable ); // Not in KTextEditor::View, but should be
    void setIconBorder( bool enable );
    void setLineNumbersOn( bool enable );
    void setScrollBarMarks( bool enable );
    void showCmdLine ( bool enable );
    void toggleFoldingMarkers();
    void toggleIconBorder();
    void toggleLineNumbersOn();
    void toggleScrollBarMarks();
    void toggleDynWordWrap ();
    void toggleCmdLine ();
    void setDynWrapIndicators(int mode);

  public:
    KateRenderer *renderer ();

    bool iconBorder();
    bool lineNumbersOn();
    bool scrollBarMarks();
    int dynWrapIndicators();
    bool foldingMarkersOn();
    KTextEditor::Document* getDoc()    { return m_doc; }

  public slots:
    void gotoMark( KTextEditor::Mark* mark ) { setCursorPositionInternal ( mark->line, 0, 1 ); }
    void slotSelectionChanged ();

  signals:
    void focusIn( KTextEditor::View* );
    void focusOut( KTextEditor::View* );

  //
  // Extras
  //
  public:

    KateDocument*  doc() const       { return m_doc; }

    KActionCollection* editActionCollection() const { return m_editActions; }

  public slots:
    void slotNewUndo();
    void slotUpdate();
    void toggleInsert();
    void reloadFile();
    void toggleWWMarker();
    void toggleWriteLock();
    void switchToCmdLine ();
    void slotReadWriteChanged ();

  signals:
    void dropEventPass(QDropEvent*);
    void viewStatusMsg (const QString &msg);

  public:
    bool setCursorPositionInternal( uint line, uint col, uint tabwidth = 1, bool calledExternally = false );

  protected:
    void contextMenuEvent( QContextMenuEvent* );

  public slots:
    void slotSelectionTypeChanged();

  private slots:
    void slotGotFocus();
    void slotLostFocus();
    void slotDropEventPass( QDropEvent* ev );
    void slotSaveCanceled( const QString& error );
    void slotExpandToplevel();
    void slotCollapseLocal();
    void slotExpandLocal();

  private:
    void setupConnections();
    void setupActions();
    void setupEditActions();
    void setupCodeFolding();
    void setupCodeCompletion();

    KActionCollection*     m_editActions;
    KAction*               m_editUndo;
    KAction*               m_editRedo;
    KRecentFilesAction*    m_fileRecent;
    KToggleAction*         m_toggleFoldingMarkers;
    KToggleAction*         m_toggleIconBar;
    KToggleAction*         m_toggleLineNumbers;
    KToggleAction*         m_toggleScrollBarMarks;
    KToggleAction*         m_toggleDynWrap;
    KSelectAction*         m_setDynWrapIndicators;
    KToggleAction*         m_toggleWWMarker;
    KAction*               m_switchCmdLine;

    KSelectAction*         m_setEndOfLine;

    KAction *m_cut;
    KAction *m_copy;
    KAction *m_copyHTML;
    KAction *m_paste;
    KAction *m_selectAll;
    KAction *m_deSelect;

    KToggleAction *m_toggleBlockSelection;
    KToggleAction *m_toggleInsert;
    KToggleAction *m_toggleWriteLock;

    KateDocument*          m_doc;
    KateViewInternal*      m_viewInternal;
    KateRenderer*          m_renderer;
    KateSearch*            m_search;
    KateSpell             *m_spell;
    KateBookmarks*         m_bookmarks;
    QPointer<QMenu>  m_rmbMenu;
    KateCodeCompletion*    m_codeCompletion;

    KateCmdLine *m_cmdLine;
    bool m_cmdLineOn;

    QVBoxLayout *m_vBox;

    bool       m_hasWrap;

  private slots:
    void slotNeedTextHint(int line, int col, QString &text);
    void slotHlChanged();

  /**
   * Configuration
   */
  public:
    inline KateViewConfig *config () { return m_config; };

    void updateConfig ();

    void updateDocumentConfig();

    void updateRendererConfig();

  private slots:
    void updateFoldingConfig ();

  private:
    KateViewConfig *m_config;
    bool m_startingUp;
    bool m_updatingDocumentConfig;
    KateRangeList* m_internalHighlights;

    // stores the current selection
    KateSuperCursor selectStart;
    KateSuperCursor selectEnd;

    // do we select normal or blockwise ?
    bool blockSelect;

  /**
   * IM input stuff
   */
  public:
    void setIMSelectionValue( uint imStartLine, uint imStart, uint imEnd,
                              uint imSelStart, uint imSelEnd, bool m_imComposeEvent );
    void getIMSelectionValue( uint *imStartLine, uint *imStart, uint *imEnd,
                              uint *imSelStart, uint *imSelEnd );
    bool isIMSelection( int _line, int _column );
    bool isIMEdit( int _line, int _column );
    bool imComposeEvent () const { return m_imComposeEvent; }

  private:
    uint m_imStartLine;
    uint m_imStart;
    uint m_imEnd;
    uint m_imSelStart;
    uint m_imSelEnd;
    bool m_imComposeEvent;
};

#endif
