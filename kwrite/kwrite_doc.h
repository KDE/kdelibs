/*
  $Id$

   Copyright (C) 1998, 1999 Jochen Wilhelmy
                            digisnap@cs.tu-berlin.de

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

#ifndef _KWDOC_H_
#define _KWDOC_H_

#include <qlist.h>
#include <qfont.h>
#include <qcolor.h>
#include <qobject.h>
#include <qfontmetrics.h>

#include <kwrite/kwrite_view.h>

#define MAX_COLORS  5
#define MAX_ATTRIBS 32

class Highlight;
class HlManager;
class KWritePart;

/**
  The TextLine represents a line of text. A text line that contains the
  text, an attribute for each character, an attribute for the free space
  behind the last character and a context number for the syntax highlight.
  The attribute stores the index to a table that contains fonts and colors
  and also if a character is selected.
*/
class TextLine {
  public:

    /**
      Creates an empty text line with given attribute and syntax highlight
      context
    */
    TextLine(int attribute = 0, int context = 0);
    ~TextLine();

    /**
      Returns the length
    */
    int length() const {return len;}

    /**
      Universal text manipulation method. It can be used to insert, delete
      or replace text.
    */
    void replace(int pos, int delLen, const QChar *insText, int insLen, uchar *insAttribs = 0L);

    /**
      Appends a string of length l to the textline
    */
    void append(const QChar *s, int l) {replace(len, 0, s, l);}

    /**
      Wraps the text from the given position to the end to the next line
    */
    void wrap(TextLine *nextLine, int pos);

    /**
      Wraps the text of given length from the beginning of the next line to
      this line at the given position
    */
    void unWrap(int pos, TextLine *nextLine, int len);

    /**
      Truncates the textline to the new length
    */
    void truncate(int newLen) {if (newLen < len) len = newLen;}

    /**
      Returns the position of the first character which is not a white space
    */
    int firstChar() const;

    /**
      Returns the position of the last character which is not a white space
    */
    int lastChar() const;

    /**
      Removes trailing spaces
    */
    void removeSpaces();

    /**
      Gets the char at the given position
    */
    QChar getChar(int pos) const;

    /**
      Gets the text. WARNING: it is not null terminated
    */
    const QChar *getText() const {return text;}

    /**
      Gets a C-like null terminated string
    */
    const QChar *getString();

    /**
      Returns the x position of the cursor at the given position, which
      depends on the number of tab characters
    */
    int cursorX(int pos, int tabChars) const;

    /**
      Sets the attributes from start to end -1
    */
    void setAttribs(int attribute, int start, int end);

    /**
      Sets the attribute for the free space behind the last character
    */
    void setAttr(int attribute);

    /**
      Gets the attribute at the given position
    */
    int getAttr(int pos) const;

    /**
      Gets the attribute for the free space behind the last character
    */
    int getAttr() const;

    /**
      Gets the attribute, including the select state, at the given position
    */
    int getRawAttr(int pos) const;

    /**
      Gets the attribute, including the select state, for the free space
      behind the last character
    */
    int getRawAttr() const;

    /**
      Sets the syntax highlight context number
    */
    void setContext(int context);

    /**
      Gets the syntax highlight context number
    */
    int getContext() const;

    /**
      Sets the select state from start to end -1
    */
    void select(bool sel, int start, int end);

    /**
      Sets the select state from the given position to the end, including
      the free space behind the last character
    */
    void selectEol(bool sel, int pos);

    /**
      Toggles the select state from start to end -1
    */
    void toggleSelect(int start, int end);

    /**
      Toggles the select state from the given position to the end, including
      the free space behind the last character
    */
    void toggleSelectEol(int pos);

    /**
      Returns the number of selected characters
    */
    int numSelected() const;

    /**
      Returns if the character at the given position is selected
    */
    bool isSelected(int pos) const;

    /**
      Returns true if the free space behind the last character is selected
    */
    bool isSelected() const;

    /**
      Finds the next selected character, starting at the given position
    */
    int findSelected(int pos) const;

    /**
      Finds the next unselected character, starting at the given position
    */
    int findUnselected(int pos) const;

    /**
      Finds the previous selected character, starting at the given position
    */
    int findRevSelected(int pos) const;

    /**
      Finds the previous unselected character, starting at the given position
    */
    int findRevUnselected(int pos) const;

    /**
      Marks the text from the given position and length as found
    */
    void markFound(int pos, int l);

    /**
      Removes the found marks
    */
    void unmarkFound();

  protected:

    /**
      Length of the text line
    */
    int len;

    /**
      Memory Size of the text line
    */
    int size;

    /**
      The text
    */
    QChar *text;

    /**
      The attributes
    */
    uchar *attribs;

    /**
      The attribute of the free space behind the end
    */
    uchar attr;

    /**
      The syntax highlight context
    */
    int ctx;
};

class Attribute {
  public:

    Attribute();
//    Attribute(const char *aName, const QColor &, const QColor &, const QFont &);
//    QString name;
    QColor col;
    QColor selCol;
    void setFont(const QFont &);
    QFont font;
    QFontMetrics fm;
    //workaround for slow QFontMetrics::width()
    int width(QChar c) {return(fontWidth < 0) ? fm.width(c) : fontWidth;};
    int width(QString s) {return(fontWidth < 0) ? fm.width(s) : s.length()*fontWidth;};

  protected:

    int fontWidth;
};

class KWAction {
  public:

    enum Action {replace, wordWrap, wordUnWrap, newLine, delLine,
      insLine, killLine};//, doubleLine, removeLine};

    KWAction(Action, KWCursor &cursor, int len = 0,
      const QString &text = QString::null);

    Action action;
    KWCursor cursor;
    int len;
    QString text;
    KWAction *next;
};

class KWActionGroup {
  public:

    // the undo group types
    enum {
      ugNone,         //
      ugPaste,        // paste
      ugDelBlock,     // delete/replace selected text
      ugIndent,       // indent
      ugUnindent,     // unindent
      ugReplace,      // text search/replace
      ugSpell,        // spell check
      ugInsChar,      // char type/deleting
      ugDelChar,      // ''  ''
      ugInsLine,      // line insert/delete
      ugDelLine       // ''  ''
    };

    KWActionGroup(KWCursor &aStart, int type = ugNone);
    ~KWActionGroup();
    void insertAction(KWAction *);

    static QString typeName(int type);

    KWCursor start;
    KWCursor end;
    KWAction *action;
    int undoType;
};

/**
  The text document. It contains the textlines, controls the
  document changing operations and does undo/redo. WARNING: do not change
  the text contents directly in methods where this is not explicitly
  permitted. All changes have to be made with some basic operations,
  which are recorded by the undo/redo system.
  @see TextLine
  @author Jochen Wilhelmy
*/
class KWriteDoc : QObject {
    Q_OBJECT

    friend KWriteView;
    friend KWrite;
    friend KWritePart;

  public:

    KWriteDoc(HlManager *, const QString &path = QString::null);
    virtual ~KWriteDoc();

    void incRefCount() {m_refCount++;}
    void decRefCount();

    /**
      gets the number of lines
    */
    int numLines() const {return(int) m_contents.count();}

    /**
      gets the last line number(numLines() -1)
    */
    int lastLine() const {return(int) m_contents.count() -1;}

    /**
      gets the given line
      @return  the TextLine object at the given line
      @see     TextLine
    */
    TextLine *textLine(int line);

    /**
      get the length in pixels of the given line
    */
    int textLength(int line);

    void setTabWidth(int);
    int tabWidth() {return m_tabChars;}
    void setReadWrite(bool);
    bool isReadWrite() {return m_readWrite;}
    void setModified(bool);
    bool isModified() {return m_modified;}
    void setSingleSelectMode(bool ss) {m_singleSelectMode = ss;}
    bool isSingleSelectMode() {return m_singleSelectMode;}

    void readConfig(KConfig *);
    void writeConfig(KConfig *);
    void readSessionConfig(KConfig *);
    void writeSessionConfig(KConfig *);

// highlight

    Highlight *highlight();
    int highlightNum();
    int numAttribs();
    Attribute *attribs() {return m_attribs;}

  protected slots:

    void hlChanged();

  protected:

    void findHighlight(const QString &filename);
    void setHighlight(int n);
    void makeAttribs();
    void updateFontData();

// view interaction

    void registerView(KWriteView *);
    void removeView(KWriteView *);
    bool ownedView(KWriteView *);
    bool isLastView(int numViews);

    int textWidth(TextLine *, int cursorX);
    int textWidth(KWCursor &cursor);
    int textWidth(bool wrapCursor, KWCursor &cursor, int xPos);
    int textPos(TextLine *, int xPos);
//    int textPos(TextLine *, int xPos, int &newXPos);
    int textWidth();
    int textHeight();

    void insert(VConfig &, const QString &);
    void insertFile(VConfig &, QIODevice &);
    void loadFile(QIODevice &);
    void writeFile(QIODevice &);

    int currentColumn(KWCursor &cursor);
    bool insertChars(VConfig &, const QString &chars);
    void newLine(VConfig &);
    void killLine(VConfig &);
    void backspace(VConfig &);
    void backspaceWord(VConfig &);
    void del(VConfig &);
    void delWord(VConfig &);
    void clear();
    void cut(VConfig &);
    void copy(int flags);
    void paste(VConfig &);

    void toggleRect(int, int, int, int);
    void selectTo(VConfig &c, KWCursor &cursor, int cXPos);
    void selectAll();
    void deselectAll();
    void invertSelection();
    void selectWord(KWCursor &cursor, int flags);

    void indent(VConfig &c) {doIndent(c, 1);}
    void unIndent(VConfig &c) {doIndent(c, -1);}
    void cleanIndent(VConfig &c) {doIndent(c, 0);}
    // called by indent/unIndent/cleanIndent
    // just does some setup and then calls optimizeLeadingSpace()
    void doIndent(VConfig &, int change);
    // optimize leading whitespace on a single line - see kwdoc.cpp for full description
//    bool optimizeLeadingSpace(VConfig &, TextLine *, int, bool);
    void optimizeLeadingSpace(int line, int flags, int change);

  public:

    QString text();
    void setText(const QString &);
    
  protected:

    QString getWord(KWCursor &cursor);
    bool hasMarkedText() {return(m_selectEnd >= m_selectStart);}
    QString markedText(int flags);
    void delMarkedText(VConfig &/*, bool undo = true*/);

    void tagLineRange(int line, int x1, int x2);
    void tagLines(int start, int end);
    void tagAll();
    void updateLines(int startLine = 0, int endLine = 0xffffff, int flags = 0, int cursorY = -1);
    void updateMaxLength(TextLine *);
    void updateViews(KWriteView *exclude = 0L);

    QColor &cursorCol(int x, int y);
    void paintTextLine2(QPainter*, int line, int xStart, int xEnd, bool showTabs);
    void paintTextLine(QPainter*, int line, int xStart, int xEnd, bool showTabs);
//    void printTextLine(QPainter &, int line, int xEnd, int y);

    bool hasFileName();
    const QString fileName();
    void setFileName(const QString &);
    void clearFileName();

    bool doSearch(SConfig &s, const QString &searchFor);
    void unmarkFound();
    void markFound(KWCursor &cursor, int len);

// internal
    void tagLine(int line);
    void insLine(int line);
    void delLine(int line);
    void optimizeSelection();

    void doAction(KWAction *);
    void doReplace(KWAction *);
    void doWordWrap(KWAction *);
    void doWordUnWrap(KWAction *);
    void doNewLine(KWAction *);
    void doDelLine(KWAction *);
    void doInsLine(KWAction *);
    void doKillLine(KWAction *);
    void newUndo();

    void recordStart(VConfig &, int newUndoType);
    void recordStart(KWriteView *, KWCursor &, int flags, int newUndoType,
      bool keepModal = false, bool mergeUndo = false);
    void recordAction(KWAction::Action, KWCursor &);
    void recordInsert(VConfig &, const QString &text);
    void recordReplace(VConfig &, int len, const QString &text);
    void recordInsert(KWCursor &, const QString &text);
    void recordDelete(KWCursor &, int len);
    void recordReplace(KWCursor &, int len, const QString &text);
    void recordEnd(VConfig &);
    void recordEnd(KWriteView *, KWCursor &, int flags);
    void doActionGroup(KWActionGroup *, int flags);
    int nextUndoType();
    int nextRedoType();
    void undoTypeList(QValueList<int> &lst);
    void redoTypeList(QValueList<int> &lst);
    void undo(VConfig &, int count = 1);
    void redo(VConfig &, int count = 1);
    void clearRedo();
    void setUndoSteps(int steps);
    void setPseudoModal(QWidget *);
    void newBracketMark(KWCursor &, BracketMark &);

  protected slots:

    void clipboardChanged();

// member variables
  public:

    QColor *colors() {return m_colors;};
    void setColors(QColor *colors);
    int fontHeight() {return m_fontHeight;};
    QList<TextLine> contents() {return m_contents;};
    int eolMode() {return m_eolMode;};
    void setEolMode(int mode) {m_eolMode = mode;};
    HlManager *hlManager() {return m_hlManager;};
    int undoSteps() {return m_undoSteps;};
    int undoState() {return m_undoState;};

  protected:

    int                   m_refCount; // reference counter for multiple views

    QList<TextLine>       m_contents;
    QColor                m_colors[MAX_COLORS];
    HlManager            *m_hlManager;
    Highlight            *m_highlight;
    int                   m_numAttribs;
    Attribute             m_attribs[MAX_ATTRIBS];
    int                   m_eolMode;

    int                   m_tabChars; // number of characters for one tab (e.g. 8)
    int                   m_tabWidth; // tab width in pixel (for internal use)

    int                   m_fontHeight;
    int                   m_fontAscent;
    QList<KWriteView>     m_views;
    bool                  m_newDocGeometry;
    TextLine             *m_longestLine;
    int                   m_maxLength;
    KWCursor              m_select;
    KWCursor              m_anchor;
    int                   m_aXPos;
    int                   m_selectStart;
    int                   m_selectEnd;
    bool                  m_oldMarkState;
    bool                  m_readWrite;
    bool                  m_modified;
    bool                  m_singleSelectMode; // false: windows-like, true: X11-like
    QString               m_fName;
    int                   m_foundLine;
    QList<KWActionGroup>  m_undoList;
    int                   m_currentUndo;
    int                   m_undoState;
    int                   m_undoSteps;
    int                   m_tagStart;
    int                   m_tagEnd;
    int                   m_undoCount; // counts merged undo steps
    QWidget              *m_pseudoModal; // the replace prompt is pseudo modal
};

#endif //KWDOC_H
