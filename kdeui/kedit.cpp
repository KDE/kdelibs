/*
 * KEdit, simple editor class.
 * Nov 96, Alexander Sanda <alex@darkstar.ping.at>
 *
 * $Id$
 *
 * $Log$
 * Revision 1.1.1.1  1997/04/09 00:28:09  cvsuser
 * Sources imported
 *
 * Revision 1.1  1997/03/15 22:41:30  kalle
 * Initial revision
 *
 * Revision 1.6  1997/01/07 18:15:19  alex
 * pre 0-2
 *
 */

#include <qpopmenu.h>
#include <qmenubar.h>
#include <qapp.h>
#include <qkeycode.h>
#include <qaccel.h>
#include <qobject.h>
#include <qmlined.h>
#include <qradiobt.h>
#include <qfiledlg.h>
#include <qchkbox.h>
#include <qmsgbox.h>
#include <qcombo.h>
#include <qpushbt.h>
#include <qgrpbox.h>
#include <qregexp.h>

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>

#include <pwd.h>

#include "keditconstants.h"
#include "kedit.h"
#include "kedit.moc"

/*@ManDoc:
 Construct a KEdit widget. The constructor does not initialize the fileselect box
 and the search dialog by default. They are initialized when needed.

 The first two parameters are passed directly to QMultiLineEdit and specify the parent
 of this widget and its name.
 The third parameter {\bf fname} may name a file, which should be opened right after
 the widget has been created. If {\bf fname} is NULL, an empty KEdit widget will be
 created.
 
 With the fourth parameter {\bf flags}, you can control the behaviour of the KEdit object.
 It may contain any combination (ORed together) of the following constants:

 \begin{itemize}
 \item {\tt KEdit::ALLOW_OPEN: allow the user to open a file, using KEdit's openFile()
 method. Note, that you also have to set HAS_POPUP in order to make the popup menu accessible.}
 \item {\tt KEdit::ALLOW_SAVE: allow the user to select the Save entry from the popup menu.}
 \item {\tt KEdit::ALLOW_SAVEAS: same for the {\bf Save As...} entry.}
 \item {\tt KEdit::HAS_POPUP: enables the internal popup menu.}
 \item {\tt KEdit::ALLOW_SEARCH: allows the user to use the internal search method (including
 the builtin search dialog).}
 \end{itemize}

 By default, the constructor sets edit mode to KEdit::OPEN_READWRITE. The method
 {\bf setEditMode()} may be used to change this. This can be done either before or after
 a file has been loaded into the editor.
 */

KEdit::KEdit(QWidget *parent, const char *name, const char *fname, unsigned flags)
    : QMultiLineEdit(parent, name)
{
    /*
     * we want to know, which file we are working on
     */
    
    qstrncpy(filename, fname, 1023);
    modified = FALSE;
    k_flags = flags;
    
    /*
     * KEdit has its own fileselection box, but it'll only be created at first
     * time use
     */
    
    fbox = 0;
    srchdialog = 0;
    connect(this, SIGNAL(textChanged()), this, SLOT(setModified()));
    if(k_flags & HAS_POPUP) {
        installEventFilter(this);
        context = new QPopupMenu(0, "context");
        context->insertItem("Open...", MENU_ID_OPEN, -1);
        context->insertItem("Insert file...", MENU_ID_INSERT, -1);
        context->insertItem("Save", MENU_ID_SAVE, -1);
        context->insertItem("Save As...", MENU_ID_SAVEAS, -1);
        context->insertSeparator(-1);
        context->insertItem("Search...", MENU_ID_SEARCH, -1);
        context->insertItem("Repeat last search", MENU_ID_SEARCHAGAIN, -1);
        connect(context, SIGNAL(activated(int)), this, SLOT(contextCallback(int)));
    }
    setContextSens();
}

KEdit::~KEdit()
{
    if(k_flags & HAS_POPUP)
        delete context;
}


/*@ManDoc:
 Load the given file into the editor widget. The second parameter is used to specify one
 of the following modes:

 \begin{itemize}
 \item {\tt OPEN_READWRITE  (value = 1). The file may be modified.}
 \item {\tt OPEN_READONLY   (value = 2). The file cannot be modified. This makes it also
 impossible to save the file (but you can SAVEAS it)}
 \item {\tt OPEN_INSERT     (value = 4). The file will be inserted at the current cursor
 position. The status of {\bf edit_mode} will remain unchanged.}
 \end{itemize}

 The function does not check, wheter the file has been modified or not. This can be done
 by using {\bf isModified()}.
 */

void KEdit::loadFile(const char *name, int mode)
{
    FILE *fd;
    int fdesc;
    struct stat s;
    char *mb_caption = "Load file:";
    char *addr;
    
    fdesc = open(name, O_RDONLY);
    if(fdesc == -1) {
        switch(errno) {
        case EACCES:
            QMessageBox::message(mb_caption, "You have no permission to read this file", "Ok");
            return;

        default:
            QMessageBox::message(mb_caption, "An error occured, while opening this file", "Ok");
            return;
        }
    }
    fstat(fdesc, &s);
    addr = (char *)mmap(0, s.st_size, PROT_READ | PROT_WRITE, MAP_PRIVATE, fdesc, 0);
    
    printf("Mapped file\n");
    
    char *addr2 = (char*)malloc( s.st_size + 1 );
    memcpy( addr2, addr, s.st_size );
    munmap(addr, s.st_size);
    addr = addr2;
    
    *(addr + s.st_size) = '\0';
    setAutoUpdate(FALSE);
    if(mode & OPEN_INSERT) {
        int line, col;
        getCursorPosition(&line, &col);
        insertLine(addr, line);
    }
    else
        setText(addr);
    setAutoUpdate(TRUE);
    repaint();
    // munmap(addr, s.st_size);
    free( addr );
    
    printf("Doing the rest\n");
    
    modified = mode & OPEN_INSERT;
    setFocus();
    if(!(mode & OPEN_INSERT))
        qstrncpy(filename, name, 1023);
    
    setEditMode(edit_mode);
    emit(fileChanged());
}

/*@ManDoc:
 Display a fileselector box and let the user select a filename. This function does not
 perform the load operation itself, it calls {\bf loadFile()} for this purpose.

 The argument {\bf mode} has the same meaning as the second parameter to {\bf loadFile()}.
 It determines, wheter the new file should replace the current text, or should only be
 inserted at the current cursor position.
 */

int KEdit::openFile(int mode)
{
    char fname[1024];
    struct passwd *pwent;
    
    if(modified && !(mode & OPEN_INSERT)) {           /* save old file */
        if((QMessageBox::query("Warning", "The current file has been modified.\n\rDo you want to save it ?"))) {
            if (doSave() != KEDIT_OK)
                return KEDIT_USER_CANCEL;       /* TODO: better checking */
        }
//        if(mb_result == QMessageBox::Cancel)
//            return KEDIT_USER_CANCEL;
    }
            
    if( !fbox ) {
        uid_t ui = getuid();
        pwent = getpwuid(ui);
        fbox = new QFileDialog(pwent->pw_dir, "*.*", this, "fbox", TRUE);
    }
    fbox->setCaption("Select file to load");
    if(k_flags & ALLOW_OPEN) {
        fbox->show();
        if (!fbox->result())   /* cancelled */
            return 0;
        if(fbox->selectedFile().isNull()) {  /* no selection */
            return 0;
        }
        strcpy(fname, fbox->selectedFile().data());
        loadFile(fname, mode);
        return 1;
    }
    else {
        QMessageBox::message("Information", "You are not allowd to open a file", "Yes", this);
        return 0;
    }
        
}

int KEdit::saveFile()
{
    FILE *fd;
    unsigned len;
    int result = 0;

    if(modified) {
        fd = fopen(filename, "w");
        if(fd) {
            len = text().length();
            if(fwrite(text().data(), len, 1, fd) != 1) {  /* check for write errors */
                result = KEDIT_OS_ERROR;
            }
            else {
                modified = FALSE;
                result = KEDIT_OK;
            }
            fclose(fd);
        } else {
            result = KEDIT_OS_ERROR;
        }
        
    }
    return result;
}

int KEdit::saveAs()
{
    char fname[1024];
    struct stat s;
    
    if( !fbox ) {
        fbox = new QFileDialog("/usr/lib/news", "*.*", this, "fbox", TRUE);
    }
    fbox->setCaption("Save file as");
try_again:
    fbox->show();
    if (!fbox->result())
        return KEDIT_USER_CANCEL;
    if(fbox->selectedFile().isNull())
        return KEDIT_USER_CANCEL;
    if(!stat(fbox->selectedFile().data(), &s)) {       /* file exists */
        if(!(QMessageBox::query("Warning:", "The specified file already exists.\n\rDo you want to overwrite it ?")))
            goto try_again;  /* ugly, but it works :) */
//        if(mb_result == QMessageBox::Cancel)
//            return KEDIT_USER_CANCEL;  /* do nothing */
    }
    qstrncpy(filename, fbox->selectedFile().data(), 1023);
    modified = TRUE; /* let the saveFile() believe, our file is modified, even if it is not */
    return saveFile();
}

int KEdit::doSave()
{
    int result = 0;
    
    if(!strcmp(filename, "Untitled")) {
        if(result = saveAs()) {
            setCaption(filename);
            return result;
        }
    } else
        return saveFile();
}

int KEdit::doSave( const char *_name )
{
    QString n = filename;
    strcpy( filename, _name );

    int erg = saveFile();

    strcpy( filename, n.data() );
    return erg;
}

void KEdit::setName( const char *_name )
{
    strcpy( filename, _name );
}

const char *KEdit::getName()
{
    return filename;
}

void KEdit::setModified()
{
    modified = TRUE;
}

void KEdit::toggleModified( bool _mod )
{
    modified = _mod;
}

/*@ManDoc:
 This function returns TRUE, if the text has been modified, FALSE otherwise. It takes no
 arguments.
 */

bool KEdit::isModified()
{
    return modified;
}

/*@ManDoc:
 This function sets the new mode for the editor widget and returns the old mode.
 */

int KEdit::setEditMode(int mode)
{
    int oldmode = edit_mode;
    edit_mode = mode;
    setReadOnly(mode & OPEN_READONLY);
    setContextSens();
    return oldmode;
}

void KEdit::setContextSens()
{
    context->setItemEnabled(MENU_ID_OPEN, k_flags & ALLOW_OPEN);
    context->setItemEnabled(MENU_ID_INSERT, (k_flags & ALLOW_SAVE) && (edit_mode & OPEN_READWRITE));
    context->setItemEnabled(MENU_ID_SAVE, (k_flags & ALLOW_SAVE) && (edit_mode & OPEN_READWRITE));
    context->setItemEnabled(MENU_ID_SAVEAS, k_flags & ALLOW_SAVEAS);
}

bool KEdit::eventFilter(QObject *o, QEvent *ev)
{
    if(ev->type() == Event_MouseButtonPress) {  /* only intersted event */
        QMouseEvent *e = (QMouseEvent *)ev;
        if(e->button() == RightButton) {          /* check button */
            context->popup(mapToGlobal(pos()) + e->pos(), 1);    /* ok, got it, display the popup */
            return TRUE;
        }
    }

    return FALSE;
}

/*@ManDoc:
 This member function is a public slot function. It handles the internal popup-menu
 selections and takes exactly one argument: {\bf item} is the internal menu-item ID
 which corresponds to the selected item.
 */
//@Memo: slot function

void KEdit::contextCallback(int item)
{
    switch(item) {
    case MENU_ID_OPEN:        /* open */
        openFile(0);
        break;

    case MENU_ID_INSERT:
        openFile(OPEN_INSERT);
        break;
        
    case MENU_ID_SAVE:
        if(!strcmp(filename, "Untitled"))
            saveAs();
        else
            saveFile();
        break;

    case MENU_ID_SAVEAS:
        saveAs();
        break;

    case MENU_ID_SEARCH:
        initSearch();
        break;

    case MENU_ID_SEARCHAGAIN:
        doSearch(NULL, FALSE);
        break;
        
    default:
        break;
    }
}

/*@ManDoc:
 This member function initiates a search. The first time it is called, it creates the
 searchdialog. Then, the dialog will be shown as application-modal. After the dialog
 terminates, the function doSearch() is called with mode set to TRUE.

 This function takes no arguments.
 */
 
void KEdit::initSearch()
{
    if(!srchdialog)
        srchdialog = new KEdSrch(this, "searchdialog");
    srchdialog->move(width()/2 - 150 + x(), height()/2 - 50 + y());
    srchdialog->show();
    if(srchdialog->result()) {
        doSearch(srchdialog->getSrchItemText(), TRUE);
    }
}

/*@ManDoc:
 This function performs a search (regular expressions allowed), starting
 at the current cursor position. The argument {\bf s_pattern} is limited to a maximum
 of 254 characters, and should contain a valid regular expression.
 The argument {\bf mode} specifies, how the search should be done:

 \begin{itemize}
 \item {\tt TRUE means: start a new search with s_pattern}
 \item {\tt FALSE means: repeat the last search (reuse old pattern)}
 \end{itemize}

 KEdit remembers the search value in an internal (private) variable. When performing
 a "search again" (with mode set to FALSE), the search also starts at the current
 cursor position.
 */

int KEdit::doSearch(const char *s_pattern, int mode)
{
    int line, col, i, length;
    QRegExp re;
    
    if(mode) {
        re = QRegExp(s_pattern, FALSE, FALSE);
        *pattern = '\0';
    }
    else  /* search again, reuse old pattern */
        re = QRegExp(pattern, FALSE, FALSE);

    
    getCursorPosition(&line, &col);
    for(i = line; i < numLines(); i++) {
        if((col = re.match(textLine(i), i == line ? col : 0, &length)) != -1) {
            setCursorPosition(i, col, FALSE);
            update();
            qstrncpy(pattern, s_pattern, 255);  /* accept the pattern */
            return TRUE;
        }
    }
    QMessageBox::message("Search", "No (more) matches found", "Ok");
    return FALSE;
}

int KEdit::repeatSearch() {

    QRegExp re(pattern, FALSE, FALSE);
}

void KEdSrch::selected(int)
{
    accept();
}


KEdSrch::KEdSrch(QWidget *parent, const char *name)
     : QDialog(parent, name, TRUE)

{
    frame1 = new QGroupBox("Search for", this, "frame1");
    values = new QComboBox(TRUE, this, "values");
    values->setInsertionPolicy(QComboBox::AtTop);
    values->setFocusPolicy(QWidget::StrongFocus);
    values->setStyle(WindowsStyle);
    connect(values, SIGNAL(activated(int)), this, SLOT(selected(int)));
    sensitive = new QCheckBox("Case sensitive", this, "sens");
    ok = new QPushButton("Find", this, "find");
    cancel = new QPushButton("Cancel", this, "cancel");
    connect(cancel, SIGNAL(clicked()), this, SLOT(reject()));
    connect(ok, SIGNAL(clicked()), this, SLOT(accept()));
    resize(300, 120);
}

int KEdSrch::getSrchItem() { return values->currentItem(); }

const char *KEdSrch::getSrchItemText() { return values->text(values->currentItem()); }

void KEdSrch::resizeEvent(QResizeEvent *)
{
    frame1->setGeometry(5, 5, width() - 10, 80);
    cancel->setGeometry(width() - 80, height() - 30, 70, 25);
    ok->setGeometry(10, height() - 30, 70, 25);
    values->setGeometry(10, 23, width() - 20, 25);
    sensitive->setGeometry(10, 50, 100, 25);
}
