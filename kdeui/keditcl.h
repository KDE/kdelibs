/*
  KEdit, a simple text editor for the KDE project

  Copyright (C) 1996 Bernd Johannes Wuebben
                     wuebben@math.cornell.edu
  Copyright (C) 2000 Waldo Bastian <bastian@kde.org>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

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
class QLineEdit;
class QTextStream;
class KIntNumInput;

///
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
};

///
class KEdFind : public KDialogBase
{
    Q_OBJECT

public:

    KEdFind( QWidget *parent = 0, const char *name=0, bool modal=true);

    QString getText();
    void setText(QString string);
    bool case_sensitive();
    bool get_direction();

protected slots:
    void slotCancel( void );
    void slotUser1( void );

private:
    QCheckBox *sensitive;
    QCheckBox *direction;
    QLineEdit *value;

signals:

    void search();
    void done();
};

///
class KEdReplace : public KDialogBase
{
    Q_OBJECT

public:

    KEdReplace ( QWidget *parent = 0, const char *name=0, bool modal=true );

    QString 	getText();
    QString 	getReplaceText();
    void 	setText(QString);
    QLineEdit 	*value;
    QLineEdit 	*replace_value;
    bool 	case_sensitive();
    bool 	get_direction();

protected slots:
    void slotCancel( void );
    void slotUser1( void );
    void slotUser2( void );
    void slotUser3( void );

private:
    QCheckBox 	*sensitive;
    QCheckBox 	*direction;

signals:
    void replace();
    void find();
    void replaceAll();
    void done();
};


/**
 * A simple text editor for the KDE project.
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


    enum { NONE,
	   FORWARD,
	   BACKWARD };

    enum { KEDIT_OK 		= 0,
	   KEDIT_OS_ERROR 	= 1,
	   KEDIT_USER_CANCEL 	= 2 ,
	   KEDIT_RETRY 		= 3,
	   KEDIT_NOPERMISSIONS 	= 4};

    enum { OPEN_READWRITE 	= 1,
	   OPEN_READONLY 	= 2,
	   OPEN_INSERT 		= 4 };

    /**
     * Insert text from the text stream into the edit widget.
     **/
    void insertText(QTextStream *);

    /**
     * Save text from the edit widget to a text stream.
     **/
    void saveText(QTextStream *);

    /** 
     * Retrieve the currently marked text.
     */
    QString 	markedText();

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
     **/
    int 	repeatSearch();

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
     * @return @p true if the document has been modified.
     */
    bool 	isModified();

    /**
     * Set the modification status of the document. 
     *  @true = Modified, @false = UNMODIFIED. 
     *  This can be used to test whether the document needs to be saved.
     **/
    void 	setModified( bool = true);

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

signals:
    /** This signal is emitted if the user dropped a URL over the text editor
      * QMultiLineEdit widget.
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

    /** This signal is emitted if the user toggles from overwrite to insert mode.
     *
     * The user can do so by pressing the "Insert" button on a PC keyboard.
     */
    void 	toggle_overwrite_signal();

public slots:

    void corrected (QString originalword, QString newword, unsigned pos);
    void misspelling (QString word, QStringList *, unsigned pos);

    void search_slot();

    void searchdone_slot();

    void replace_slot();

    void replace_all_slot();

    void replace_search_slot();

    void replacedone_slot();

    void computePosition();

    void repaintAll();


protected:
    QTimer* repaintTimer;

    int 	saveFile();

    int 	doSearch(QString s_pattern, bool case_sensitive,
			 bool regex, bool forward,int line, int col);

    int 	doReplace(QString s_pattern, bool case_sensitive,
			  bool regex, bool forward,int line, int col,bool replace);




protected:

    bool 	eventFilter	 ( QObject *, QEvent * );
    void 	keyPressEvent 	 ( QKeyEvent *  );
    void 	mousePressEvent  ( QMouseEvent* );
    void 	mouseReleaseEvent( QMouseEvent* );
    void 	mouseMoveEvent 	 ( QMouseEvent* );

    // DnD inteface
    
    void        dragMoveEvent(QDragMoveEvent* e);
    void        dragEnterEvent(QDragEnterEvent* e);
    void        dropEvent(QDropEvent* e);

private:

    void 	setContextSens();
    void        mynewLine();
    QString 	prefixString(QString);


private:
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
};



#endif
