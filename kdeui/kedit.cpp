/*
 * KEdit, simple editor class.
 * Nov 96, Alexander Sanda <alex@darkstar.ping.at>
 *
 * $Id$
 *
 * $Log$
 * Revision 1.4  1997/05/08 22:53:18  kalle
 * Kalle:
 * KPixmap gone for good
 * Eliminated static objects from KApplication and HTML-Widget
 *
 * Revision 1.1.1.1  1997/04/13 14:42:42  cvsuser
 * Source imported
 *
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

<<<<<<< kedit.cpp
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
=======
  $Id$
 
         kedit+ , a simple text editor for the KDE project
>>>>>>> 1.3

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>



#include "kedit.moc"
#include "kedit.h"
#include "kfontdialog.h"



extern KApplication *a;

KEdit::KEdit(QWidget *parent, const char *name, const char *fname, unsigned flags)
    : QMultiLineEdit(parent, name){
    
    p_parent = parent;

    qstrncpy(filename, fname, 1023);
    modified = FALSE;
    k_flags = flags;
    
    installEventFilter( this );     
    srchdialog = 0;
    connect(this, SIGNAL(textChanged()), this, SLOT(setModified()));
    setContextSens();
}



KEdit::~KEdit(){

}


int KEdit::loadFile(const char *name, int mode){

    int fdesc;
    struct stat s;
    char *mb_caption = "Load file:";
    char *addr;
    QFileInfo info(name);

    if(!info.exists()){
      QMessageBox::message("Sorry","The specified File does not exist","OK");
      return KEDIT_RETRY;
    }

    if(!info.isReadable()){
      QMessageBox::message("Sorry","You do not have read permission to this file.","OK");
      return KEDIT_RETRY;
    }


    fdesc = open(name, O_RDONLY);

    if(fdesc == -1) {
        switch(errno) {
        case EACCES:
            QMessageBox::message("Sorry", 
				 "You have do not have Permission to \n"\
				 "read this Document", "Ok");
            return KEDIT_OS_ERROR;

        default:
            QMessageBox::message(mb_caption, 
				 "An Error occured while trying to open this Document", 
				 "Ok");
            return KEDIT_OS_ERROR;
        }
    }
    
    emit loading();
    a->processEvents();

    fstat(fdesc, &s);
    addr = (char *)mmap(0, s.st_size, PROT_READ | PROT_WRITE, MAP_FILE |
			MAP_PRIVATE, fdesc, 0);
    
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
    else{
        setText(addr);
    }

    setAutoUpdate(TRUE);
    repaint();

    free( addr );
    
    modified = mode & OPEN_INSERT;
    setFocus();
    
    if(!(mode & OPEN_INSERT))
        qstrncpy(filename, name, 1023);
    
    setEditMode(edit_mode);
    emit(fileChanged());

    return KEDIT_OK;
}


int KEdit::insertFile(){

    QFileDialog *box;
    QString filename;
      
    box = new QFileDialog( this, "fbox", TRUE);
    
    box->setCaption("Select Document to Insert");

    if(k_flags & ALLOW_OPEN) {
      
      box->show();
      
      if (!box->result()) {
	delete box;
	return KEDIT_USER_CANCEL;
      }

      if(box->selectedFile().isEmpty()) {  /* no selection */
	delete box;
	return KEDIT_USER_CANCEL;
      }

      filename = box->selectedFile();
      filename.detach();

      delete box;

    }
    
    return loadFile(filename.data(), OPEN_INSERT);

}

int KEdit::openFile(int mode)
{
    char fname[1024];
    QFileDialog *box;
    

    if(modified && !(mode & OPEN_INSERT)) {           
      if((QMessageBox::query("Message", 
			     "The current Document has been modified.\n"\
			     "Would you like to save it?"))) {

	if (doSave() != KEDIT_OK){
	  
	  QMessageBox::message("Sorry", "Could not Save the Document", "OK");
	  return KEDIT_OS_ERROR;     

	}
      }
    }
            
    box = new QFileDialog( this, "fbox", TRUE);
    
    box->setCaption("Select Document to Open");

    if(k_flags & ALLOW_OPEN) {
        box->show();
        if (!box->result())   /* cancelled */
            return KEDIT_USER_CANCEL;
        if(box->selectedFile().isNull()) {  /* no selection */
            return KEDIT_USER_CANCEL;
        }

        strcpy(fname, box->selectedFile().data());

	delete box;

        return loadFile(fname, mode);
    }
    else {
        QMessageBox::message("Sorry", 
			     "You do not have permision to open a Document", 
			     "Yes", this);
        return KEDIT_OS_ERROR;
    }
        
}

int KEdit::newFile(int mode){


    if(modified && !(mode & OPEN_INSERT)) {           
      if((QMessageBox::query("Message", 
			     "The current Document has been modified.\n"\
			     "Would you like to save it?"))) {

	if (doSave() != KEDIT_OK){
	  
	  QMessageBox::message("Sorry", "Could not Save the Document", "OK");
	  return KEDIT_OS_ERROR;     

	}
      }
    }

    this->clear();

    
    modified = FALSE;
    setFocus();

    qstrncpy(filename, "Untitled", 1023);
       
    setEditMode(HAS_POPUP | ALLOW_OPEN | ALLOW_SAVE | ALLOW_SAVEAS | OPEN_READWRITE);

    emit(fileChanged());

    return KEDIT_OK;
            
        
}


void KEdit::keyPressEvent ( QKeyEvent *e){

  int line;
  int column;


  if (e->key() == Key_Tab){

    QMultiLineEdit::getCursorPosition(&line, &column);
    column = 8 - column % 8;
    for(int i = 0; i< column;i++){
      QMultiLineEdit::insertChar((char)' ');
    }
    emit update_status_bar();
    return;
  }
  
  QMultiLineEdit::keyPressEvent(e);
  emit update_status_bar();
}

void KEdit::mousePressEvent (QMouseEvent* e){

  
  QMultiLineEdit::mousePressEvent(e);
  emit update_status_bar();

}

void KEdit::mouseMoveEvent (QMouseEvent* e){

  QMultiLineEdit::mouseMoveEvent(e);
  emit update_status_bar();
  

}


void KEdit::mouseReleaseEvent (QMouseEvent* e){

  
  QMultiLineEdit::mouseReleaseEvent(e);
  emit update_status_bar();

}


int KEdit::saveFile()
{
  struct stat st;
  int stat_ok = -1;

    QFile file(filename);

    emit saving();
    a->processEvents();
    
    if(!modified) {
      return KEDIT_OK;
    }

	if( file.exists() )
	  stat_ok = stat( filename, &st );

    if( !file.open( IO_WriteOnly | IO_Truncate )) {
      QMessageBox::message("Sorry","Could not create the file\n","OK");
      return KEDIT_OS_ERROR;
    }

    if(file.writeBlock(text().data(), text().length() ) == -1) {  
      QMessageBox::message("Sorry","Could not save the file\n","OK");
      return KEDIT_OS_ERROR;    
    }	    

    modified = FALSE;    
    file.close();
    
	if( stat_ok == 0 )
	  chmod( filename, st.st_mode & 07777 );

    return KEDIT_OK;

}

int KEdit::saveAs()
{

    QFileDialog *box;
    QFileInfo info;
    QString tmpfilename;
    int result;
    
    box = new QFileDialog( this, "box", TRUE);
    box->setCaption("Save Document As");

try_again:

    box->show();

    if (!box->result()){
      delete box;
      return KEDIT_USER_CANCEL;
    }

    if(box->selectedFile().isNull()){
      delete box;
      return KEDIT_USER_CANCEL;
    }

    info.setFile(box->selectedFile());

    if(info.exists()){
        if(!(QMessageBox::query("Warning:", 
				"A Document with this Name exists already\n"\
				"Do you want to overwrite it ?")))
	  goto try_again;  

    }


    tmpfilename = filename;

    qstrncpy(filename, box->selectedFile().data(), 1023);

    // we need this for saveFile();

    modified = TRUE; 
    
    delete box;

    result =  saveFile();
    
    if( result != KEDIT_OK)
      qstrncpy(filename,tmpfilename.data(),1023); // revert filename
	
    return result;
      
}



int KEdit::doSave()
{

    
  int result = 0;
    
    if(strcmp(filename, "Untitled") == 0) {
      result = saveAs();

      if(result == KEDIT_OK)
	setCaption(filename);

      return result;
    }

    QFileInfo info(filename);
    if(!info.isWritable()){
      QMessageBox::message("Sorry:", 
			   "You do not have write permission to this file.\n","OK");
      return KEDIT_NOPERMISSIONS;
    }
    

    result =  saveFile();
    return result;

}

int KEdit::doSave( const char *_name ){

    QString n = filename;
    strcpy( filename, _name );

    int erg = saveFile();

    strcpy( filename, n.data() );
    return erg;
}


void KEdit::setName( const char *_name ){

    strcpy( filename, _name );
}


const char *KEdit::getName(){

    return filename;
}


void KEdit::selectFont(){
 

  QFont font = this->font();//QFont("Times",18,QFont::Bold);
  KFontDialog::getFont(font);
  this->setFont(font);

}

void KEdit::setModified(){

    modified = TRUE;
}

void KEdit::toggleModified( bool _mod ){

    modified = _mod;
}



bool KEdit::isModified(){

    return modified;
}



int KEdit::setEditMode(int mode){

    int oldmode = edit_mode;
    edit_mode = mode;
    //    setReadOnly(mode & OPEN_READONLY);
    setContextSens();
    return oldmode;
}


void KEdit::setContextSens(){
  /*
    context->setItemEnabled(MENU_ID_OPEN, k_flags & ALLOW_OPEN);
    context->setItemEnabled(MENU_ID_INSERT, (k_flags & ALLOW_SAVE) 
			    && (edit_mode & OPEN_READWRITE));

    context->setItemEnabled(MENU_ID_SAVE, (k_flags & ALLOW_SAVE) 
			    && (edit_mode & OPEN_READWRITE));

    context->setItemEnabled(MENU_ID_SAVEAS, k_flags & ALLOW_SAVEAS);
    */
}


bool KEdit::eventFilter(QObject *o, QEvent *ev){

  static QPoint tmp_point;

  (void) o;

  if(ev->type() != Event_MouseButtonPress) 
    return FALSE;
    
  QMouseEvent *e = (QMouseEvent *)ev;
  
  if(e->button() != RightButton) 
    return FALSE;

  tmp_point = QCursor::pos();
  
  if(p_parent)
    ((TopLevel*)p_parent)->right_mouse_button->popup(tmp_point);   


  return TRUE;

}


QString KEdit::markedText(){

  return QMultiLineEdit::markedText();

}


 
void KEdit::initSearch(){
  
  int result = 1;

  if(!srchdialog)
    srchdialog = new KEdSrch(this, "searchdialog");
  this->clearFocus();
  srchdialog->show();
  srchdialog->setFocus();
  if(srchdialog->result()) {
    result = doSearch(srchdialog->getText(), TRUE);
  }
  
  if(result == 0)
    QMessageBox::message("Search", "No  matches found", "Ok");
  
  this->setFocus();
}



int KEdit::doSearch(const char *s_pattern, int mode)
{
    int line, col, i, length;
    QRegExp re;
    
    if(mode) {
        re = QRegExp(s_pattern, FALSE, FALSE);
        *pattern = '\0';
	getCursorPosition(&line, &col);
    }
    else {
      
      if(strcmp(pattern,"") == 0)
	return 1;
	
      re = QRegExp(pattern, FALSE, FALSE);
	getCursorPosition(&line, &col);
	col +=1;
    }

    for(i = line; i < numLines(); i++) {
        if((col = re.match(textLine(i), i == line ? col : 0, &length)) != -1) {
            setCursorPosition(i, col, FALSE);
	    // update();
            qstrncpy(pattern, s_pattern, 255);  /* accept the pattern */
            return 1;
        }
    }
    return 0;
}



int KEdit::repeatSearch() {
  
  int result;

  if(!srchdialog)
      return 0;
  
  result = doSearch(srchdialog->getText(), FALSE);
  
  if(result == 0)
    QMessageBox::message("Search", "No  more matches found", "Ok");

  this->setFocus();
  
  return result;
}


void KEdSrch::selected(int)
{
    accept();
}


KEdSrch::KEdSrch(QWidget *parent, const char *name)
     : QDialog(parent, name, TRUE)

{
    frame1 = new QGroupBox("Search for", this, "frame1");
    values = new QLineEdit( this, "values");
    this->setFocusPolicy(QWidget::StrongFocus);
    connect(values, SIGNAL(returnPressed()), this, SLOT(accept()));
    sensitive = new QCheckBox("Case sensitive", this, "sens");
    ok = new QPushButton("Find", this, "find");
    cancel = new QPushButton("Cancel", this, "cancel");
    connect(cancel, SIGNAL(clicked()), this, SLOT(reject()));
    connect(ok, SIGNAL(clicked()), this, SLOT(accept()));
    resize(300, 120);
}

QString KEdSrch::getText() { return values->text(); }


void KEdSrch::resizeEvent(QResizeEvent *)
{
    frame1->setGeometry(5, 5, width() - 10, 80);
    cancel->setGeometry(width() - 80, height() - 30, 70, 25);
    ok->setGeometry(10, height() - 30, 70, 25);
    values->setGeometry(20, 25, width() - 40, 25);
    sensitive->setGeometry(20, 55, 100, 25);
}

#include "kfontdialog.moc"

