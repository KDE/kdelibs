/*
 *
 *
 * NOTE THIS CLASS IS NOT THE SAME AS THE ONE IN THE KDEUI LIB
 * File handling functions have been removed from this class.
 * 
 * This class is to replace the version in KDEUI.

  KEdit, a simple text editor for the KDE project

  Copyright (C) 1996 Bernd Johannes Wuebben
                     wuebben@math.cornell.edu

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

#warning !!This version (kdelibs/kdeui/keditcl.h) of keditcl.h  will be replaced 
#warning with the one in kdeutils/kedit after the release of KRASH!!

#include <qdialog.h>
#include <qstring.h>
#include <qmultilineedit.h>

#include <ksconfig.h>
#include <kspell.h>

class QLabel;
class QPushButton;
class QCheckBox;
class QRadioButton;
class QGroupBox;
class QLineEdit;
class QTextStream;
class KIntNumInput;

///
class KEdGotoLine : public QDialog
{
	Q_OBJECT

public:

	KEdGotoLine( QWidget *parent = 0, const char *name = 0 );

	int getLineNumber();
	KIntNumInput *lineNum;

private:
	QPushButton *ok, *cancel;
	QGroupBox *frame;
	void resizeEvent(QResizeEvent *);
	void focusInEvent(QFocusEvent *);

public slots:

	void selected( int );
};

///
class KEdSrch : public QDialog
{
    Q_OBJECT

public:

    KEdSrch ( QWidget *parent = 0, const char *name=0);

    QString getText();
    void setText(QString string);
    bool case_sensitive();
    bool get_direction();

protected:
    void focusInEvent( QFocusEvent *);

private:

    QPushButton *ok, *cancel;
    QCheckBox *sensitive;
    QCheckBox *direction;
    QGroupBox *frame1;
    QLineEdit *value;

signals:

    void search_signal();
    void search_done_signal();

public slots:

    void done_slot();
    void ok_slot();

};

///
class KEdReplace : public QDialog
{
    Q_OBJECT

public:

    KEdReplace ( QWidget *parent = 0, const char *name=0);

    QString 	getText();
    QString 	getReplaceText();
    void 	setText(QString);
    QLineEdit 	*value;
    QLineEdit 	*replace_value;
    QLabel 	*label;
    bool 	case_sensitive();
    bool 	get_direction();

protected:

    void focusInEvent( QFocusEvent *);

private:

    QPushButton *ok, *cancel, *replace, *replace_all;
    QCheckBox 	*sensitive;
    QCheckBox 	*direction;
    QGroupBox 	*frame1;


signals:

    void replace_signal();
    void find_signal();
    void replace_all_signal();
    void replace_done_signal();

public slots:

    void done_slot();
    void replace_slot();
    void replace_all_slot();
    void ok_slot();

};


///
class KEdit : public QMultiLineEdit
{
    Q_OBJECT

public:

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
     * Inserts text from the text stream into the edit widget
     **/
    void insertText(QTextStream *);

    /**
     * Save text from the edit widget to a text stream.
     **/
    void saveText(QTextStream *);

    /// Returns the currently marked text.
    /** Returns the currently marked text.
     */
    QString 	markedText();

    /// Lets the user select a font and sets the font of the textwidget.
    /** Lets the user select a font and sets the font of the textwidget to that
        selected font.
    */
    void 	selectFont();

    /// Presents a search dialog to the user
    /** Presents a search dialog to the user
     */
    void 	search();

    /// Repeats the last search specified on the search dialog.
    /** Repeasts the last search specified on the search dialog. If the user
        hasn't searched for anything until now, this method will simply return
	without doing anything.
    */
    int 	repeatSearch();

    /// Presents a Search and Replace Dialog to the User.
    /**  Presents a Search and Replace Dialog to the User.
     */
    void 	replace();

    /// Presents a "Goto Line" dialog to the User
    /** Presents a "Goto Line" dialog to the User
     */
    void 	doGotoLine();

    /**
     * @return true if the document has been modified;.
     */
    bool 	isModified();

    /** Sets the modification status of the document. 
     *  true = Modified, false = UNMODIFIED. 
     *  This can be used to test whether the document needs to be saved.
     **/
    void 	setModified( bool = true);

    /// Install a Popup Menue for KEdit.
    /** Install a Popup Menue for KEdit. The Popup Menu will be activated on
        a right mouse button press event.
     */
    void 	installRBPopup( QPopupMenu* );

    /// Return the current Line number
	  /** Returns the current line number, that is the line the cursor is on.
	   */
    int 	currentLine();

    /// Returns the current Column number
	  /** This returns the actual column number the cursor is on. This call differs
	    from QMultiLineEdit::getCursorPosition in that it returns the actual cursor
	    position and not the character position. Use currentLine() and currentColumn()
	    if you want to display the current line or column in the status bar for
	    example.
	    */
    int 	currentColumn();

    /**
     * Returns the KSpellConfig object used by KEdit.
     */
    KSpellConfig *ksConfig(void) { return ksc;}
    /**
     * Sets the KSpellConfig object used by KEdit.  (The object is
     *  copied internally.)
     */
    void setKSConfig (const KSpellConfig _ksc) {if (!ksc) ksc = new KSpellConfig(); *ksc=_ksc;}

signals:

     /**
     * This says spellchecking is <i>percent</i> done.
     * It is emitted every 10% during spellchecking.
     **/
    void  spellcheck_progress (unsigned int percent);

    /**
     * Emitted when spellcheck is complete.
     **/
    void spellcheck_done ();

    ///  This signal is emitted whenever the cursor position changed.
	   /** This signal is emitted whenever the cursor position changed.
	       Use this in conjunction with currentLine(), currentColumn()
	       if you need to know the cursor position.
	       */
    void 	CursorPositionChanged();

    /// This signal is emitted if the user toggles from overwrite to insert mode
	 /** This signal is emitted if the user toggles from overwrite to insert mode.
	     He can do so by pressing the "Insert" Button on a PC keyboard.
	     */
    void 	toggle_overwrite_signal();

    // This signal is emitted if the Editor widget received a Url Drop
         /** This signal is emitted if the user dropped a Url over the text editor
             QMultiLineEdit widget. Note that the user can drop also Text on it, but
             this is already handled internally by QMultiLineEdit.
             */
    void        gotUrlDrop(QDropEvent* e);


public slots:

    void spellResult (const QString &newtext);
    void corrected (QString originalword, QString newword, unsigned pos);
    void misspelling (QString word, QStringList *, unsigned pos);
    void spellcheck();
    void spellcheck2(KSpell*);
    void spellCleanDone();

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

    // Spellchecking
    KSpell *kspell;
    KSpellConfig *ksc;
    int spell_offset;

    QString	killbufferstring;
    QWidget     *parent;
    KEdSrch 	*srchdialog;
    KEdReplace 	*replace_dialog;
    KEdGotoLine *gotodialog;
    QPopupMenu  *rb_popup;

    QString     pattern;

    bool 	can_replace;
    bool	killing;
    bool 	killtrue;
    bool 	lastwasanewline;
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
