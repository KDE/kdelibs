/* This file is part of the KDE libraries

   Copyright (C) 1996 Bernd Johannes Wuebben <wuebben@math.cornell.edu>
   Copyright (C) 2000 Waldo Bastian <bastian@kde.org>

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
#ifndef __KEDITCL_H__
#define __KEDITCL_H__

#include <qmultilineedit.h>
#include <qstring.h>
#include <kdialogbase.h>

class QDropEvent;
class QPushButton;
class QCheckBox;
class QRadioButton;
class QTextStream;
class KHistoryCombo;
class KIntNumInput;
class QVButtonGroup;

class KEdGotoLine : public KDialogBase
{
    Q_OBJECT

public:
    KEdGotoLine( QWidget *parent=0, const char *name=0, bool modal=true );
    int getLineNumber();

public slots:
    void selected( int );

private:
    KIntNumInput *lineNum;

protected:
    virtual void virtual_hook( int id, void* data );
private:
    class KEdGotoLinePrivate;
    KEdGotoLinePrivate *d;
};

///
class KEdFind : public KDialogBase
{
    Q_OBJECT
    Q_PROPERTY( QString text READ getText WRITE setText )
    Q_PROPERTY( bool caseSensitivity READ case_sensitive WRITE setCaseSensitive )
    Q_PROPERTY( bool direction READ get_direction WRITE setDirection )
public:

    KEdFind( QWidget *parent = 0, const char *name=0, bool modal=true);
    ~KEdFind();

    QString getText() const;
    void setText(QString string);
    void setCaseSensitive( bool b );
    bool case_sensitive() const;
    void setDirection( bool b );
    bool get_direction() const;

    /**
     * @returns the combobox containing the history of searches. Can be used
     * to save and restore the history.
     */
    KHistoryCombo *searchCombo() const;

protected slots:
    void slotCancel( void );
    void slotUser1( void );
    void textSearchChanged ( const QString & );

protected:
  QVButtonGroup* group;

private:
    QCheckBox *sensitive;
    QCheckBox *direction;

    virtual void done(int i ) { KDialogBase::done(i); }

signals:

    void search();
    void done();
protected:
    virtual void virtual_hook( int id, void* data );
private:
    class KEdFindPrivate;
    KEdFindPrivate *d;
};

///
class KEdReplace : public KDialogBase
{
    Q_OBJECT

public:

    KEdReplace ( QWidget *parent = 0, const char *name=0, bool modal=true );
    ~KEdReplace();

    QString 	getText();
    QString 	getReplaceText();
    void 	setText(QString);

    /**
     * @returns the combobox containing the history of searches. Can be used
     * to save and restore the history.
     */
    KHistoryCombo *searchCombo() const;

    /**
     * @returns the combobox containing the history of replaces. Can be used
     * to save and restore the history.
     */
    KHistoryCombo *replaceCombo() const;

    bool 	case_sensitive();
    bool 	get_direction();

protected slots:
    void slotCancel( void );
    void slotClose( void );
    void slotUser1( void );
    void slotUser2( void );
    void slotUser3( void );
    void textSearchChanged ( const QString & );

private:
    QCheckBox 	*sensitive;
    QCheckBox 	*direction;

	virtual void done(int i ) { KDialogBase::done(i); }

signals:
    void replace();
    void find();
    void replaceAll();
    void done();
protected:
  virtual void virtual_hook( int id, void* data );
private:
    class KEdReplacePrivate;
    KEdReplacePrivate *d;
};


/**
 * A simple text editor for the KDE project.
 * @deprecated. Use KTextEditor::Editor or QTextEdit instead.
 *
 * @author Bernd Johannes Wuebben <wuebben@math.cornell.edu>, Waldo Bastian <bastian@kde.org>
 **/

class KEdit : public QMultiLineEdit
{
    Q_OBJECT

public:
    /**
     * The usual constructor.
     **/
    KEdit (QWidget *_parent=NULL, const char *name=NULL);

    ~KEdit();

    /**
     * Search directions.
     * @internal
     **/
    enum { NONE,
	   FORWARD,
	   BACKWARD };
    /**
     * Insert text from the text stream into the edit widget.
     **/
    void insertText(QTextStream *);

    /**
     * Save text from the edit widget to a text stream.
     * If @p softWrap is false soft line wrappings are replaced with line-feeds
     * If @p softWrap is true soft line wrappings are ignored.
     **/
    void saveText(QTextStream *, bool softWrap);
    void saveText(QTextStream *); // KDE 4.0: remove

    /**
     *  Let the user select a font and set the font of the textwidget to that
     * selected font.
     **/
    void 	selectFont();

    /**
     * Present a search dialog to the user
     **/
    void 	search();

    /**
     * Repeat the last search specified on the search dialog.
     *
     *  If the user hasn't searched for anything until now, this method
     *   will simply return without doing anything.
     *
     * @return @p true if a search was done. @p false if no search was done.
     **/
    bool 	repeatSearch();

    /**
     * Present a Search and Replace Dialog to the user.
     **/
    void 	replace();

    /**
     * Present a "Goto Line" dialog to the user.
     */
    void 	doGotoLine();

    /**
     * Clean up redundant whitespace from selected text.
     */
    void        cleanWhiteSpace();

    /**
     * Install a context menu for KEdit.
     *
     *  The Popup Menu will be activated on a right mouse button press event.
     */
    void 	installRBPopup( QPopupMenu* );

    /**
     * Retrieve the current line number.
     *
     * The current line is the line the cursor is on.
     **/
    int 	currentLine();

    /**
     * Retrieve the actual column number the cursor is on.
     *
     *  This call differs
     *    from @ref QMultiLineEdit::getCursorPosition() in that it returns the actual cursor
     *    position and not the character position. Use @ref currentLine() and @ref currentColumn()
     *    if you want to display the current line or column in the status bar for
     *    example.
     */
    int 	currentColumn();


    /**
     * Start spellchecking mode.
     */
    void spellcheck_start();

    /**
     * Exit spellchecking mode.
     */
    void spellcheck_stop();

    /**
     * Allow the user to toggle between insert mode and overwrite mode with
     * the "Insert" key. See also @ref toggle_overwrite_signal();
     *
     * The default is false: the user can not toggle.
     */
    void setOverwriteEnabled(bool b);


signals:
    /** This signal is emitted if the user dropped a URL over the text editor
      * @ref QMultiLineEdit widget.
      *
      *  Note that the user can drop also Text on it, but
      * this is already handled internally by QMultiLineEdit.
      */
    void        gotUrlDrop(QDropEvent* e);

    /** This signal is emitted whenever the cursor position changes.
     *
     * Use this in conjunction with @ref currentLine(), @ref currentColumn()
     * if you need to know the cursor position.
     */
    void 	CursorPositionChanged();

    /**
     * This signal is emitted if the user toggles from insert to overwrite mode
     * or vice versa.
     *
     * The user can do so by pressing the "Insert" button on a PC keyboard.
     *
     * This feature must be activated by calling @ref setOverwriteEnabled(true)
     * first.
     */
    void 	toggle_overwrite_signal();

private slots:
      /**
       * @internal
       **/
    void corrected (const QString &originalword, const QString &newword, unsigned int pos);
      /**
       * @internal
       **/
    void misspelling (const QString &word, const QStringList &, unsigned int pos);

      /**
       * @internal
       * Called from search dialog.
       **/
    void search_slot();

      /**
       * @internal
       **/
    void searchdone_slot();

      /**
       * @internal
       **/
    void replace_slot();

      /**
       * @internal
       **/
    void replace_all_slot();

      /**
       * @internal
       **/
    void replace_search_slot();

      /**
       * @internal
       **/
    void replacedone_slot();

      /**
       * Cursor moved...
       */
    void slotCursorPositionChanged();

protected:
    void computePosition();
    int 	doSearch(QString s_pattern, bool case_sensitive,
			 bool regex, bool forward,int line, int col);

    int 	doReplace(QString s_pattern, bool case_sensitive,
			  bool regex, bool forward,int line, int col,bool replace);

      /**
       * Sets line and col to the position pos, considering word wrap.
       **/
    void	posToRowCol(unsigned int pos, unsigned int &line, unsigned int &col);

    /**
     * Reimplemented for internal reasons, the API is not affected.
     */
    virtual void create( WId = 0, bool initializeWindow = true,
                         bool destroyOldWindow = true );


protected:

    void 	keyPressEvent 	 ( QKeyEvent *  );

    // DnD inteface

    void        dragMoveEvent(QDragMoveEvent* e);
    void        dragEnterEvent(QDragEnterEvent* e);
    void        dropEvent(QDropEvent* e);

private:
    QTimer* repaintTimer;

    QString	killbufferstring;
    QWidget     *parent;
    KEdFind 	*srchdialog;
    KEdReplace 	*replace_dialog;
    KEdGotoLine *gotodialog;

    QString     pattern;

    bool 	can_replace;
    bool	killing;
    bool 	killtrue;
    bool 	lastwasanewline;
    bool        saved_readonlystate;
    int 	last_search;
    int 	last_replace;
    int 	replace_all_line;
    int 	replace_all_col;

    int 	line_pos, col_pos;
    bool        fill_column_is_set;
    bool        word_wrap_is_set;
    int         fill_column_value;

protected:
    virtual void virtual_hook( int id, void* data );
private:
    class KEditPrivate;
    KEditPrivate *d;
};

#endif
