
/*
 * KEdit.h
 * defines KEdit class
 *
 * $Id$
 */

/*@ManDoc:
 This is a simple dialog for gathering a text value, used by KEdit search functions.
  */
  

#ifndef __KEDIT_H__
#define __KEDIT_H__

#include <qdialog.h>
#include <qcombo.h>
#include <qpushbt.h>
#include <qchkbox.h>
#include <qgrpbox.h>
#include <qmlined.h>
#include <qpopmenu.h>
#include <qfiledlg.h>

class KEdSrch : public QDialog
{
    Q_OBJECT;
public:

    /*@ManDoc:
     
     construct a new KEdSrch dialog and all its child widgets. The function has
     no special parameters.
     */
    
    KEdSrch ( QWidget *parent = 0, const char *name=0);

    /*@ManDoc:
     
     return the index value of the currently selected item.
     */
    
    int getSrchItem();

    /*@ManDoc:
     
     get the currently selected text from the history list
     */

    const char *getSrchItemText();
private:
    QComboBox *values;
    QPushButton *ok, *cancel;
    QCheckBox *sensitive;
    QGroupBox *frame1;
    void resizeEvent(QResizeEvent *);

    /*@ManDoc:
     This slot function is called, whenever a selection from the list is made.
     */
public slots:
    void selected(int);
};

/*@ManDoc:
 The base editor class. It inherits QMultiLineEdit and provides several
 additional features like its own fileselector box, a little context menu
 and a collection of methods for load, save, search and so on.

 KEdit provides a number of creation-time options. You may use them to control the behaviour
 of the editor object
 */

class KEdit : public QMultiLineEdit
{
    Q_OBJECT;
    
public:

    enum { ALLOW_OPEN = 1, ALLOW_SAVE = 2, ALLOW_SAVEAS = 4, ALLOW_SEARCH = 8,
            HAS_POPUP = 16,
            SHOW_ERRORS = 32
    };

    enum { MENU_ID_OPEN = 1,
            MENU_ID_INSERT = 2,
            MENU_ID_SAVE = 3,
            MENU_ID_SAVEAS = 4,
            MENU_ID_SEARCH = 5,
            MENU_ID_SEARCHAGAIN = 6
    };

    enum { KEDIT_OK = 0, KEDIT_OS_ERROR = 1, KEDIT_USER_CANCEL = 2 };
    enum { OPEN_READWRITE = 1, OPEN_READONLY = 2, OPEN_INSERT = 4 };
    
    //@Include: KEdit.cpp
    KEdit ( QWidget *parent=0, const char *name=0, const char *filename=0, unsigned flags = 0);
    ~KEdit();

    void loadFile(const char *, int);
    int saveFile();
    int saveAs();
    int openFile(int);
    bool isModified();
    const char *getName();
    /// Set the name of the file
    /**
      You may mention an URL here, but you need not.
      */
    void setName( const char *_name );
    int doSave();
    /// Save the data to the file named '_filename'
    int doSave( const char *_filename );
    int setEditMode(int);
    int doSearch(const char *, int);
    int repeatSearch();
    void initSearch();
    /// Set the modified flag
    void toggleModified( bool );
private:
    //@Include: KEdit.cpp
    bool modified;
    QPopupMenu *context;
    QFileDialog *fbox;
    int edit_mode;
    char filename[1024];
    char pattern[256];
    bool eventFilter(QObject *, QEvent *);
    /*@ManDoc:
     This method is used to set the menu-item sensitivities for KEdit's popup menu. The
     availability of certain menu entries depends on how the object has been created.
     For example: If KEdit::ALLOW_SAVE is NOT set, the menuentry {\bf Save} will be grayed out.
     */
    void setContextSens();
    KEdSrch *srchdialog;
    unsigned k_flags;

signals:
    void fileChanged();

public slots:
    void contextCallback(int);
    void setModified();
};



#endif
