 /*

  $Id$
 
  kedit, a simple text editor for the KDE project
  
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
  
  KEdit, simple editor class, hacked version of the original by 

 
  */

#include <qlayout.h>
#include <qlabel.h>
#include <qapplication.h>
#include <klocale.h>

#include "keditcl.h"


//////////////////////////////////////////////////////////////////////////
//
// Find Methods
//
 
void KEdit::search(){

  if (replace_dialog)
	if (replace_dialog->isVisible())
		replace_dialog->hide();
  

  if(!srchdialog){
    srchdialog = new KEdSrch(0, "searchdialog");
    connect(srchdialog,SIGNAL(search_signal()),this,SLOT(search_slot()));
    connect(srchdialog,SIGNAL(search_done_signal()),this,SLOT(searchdone_slot()));
  }

  // If we already searched / replaced something before make sure it shows
  // up in the find dialog line-edit.

  if(srchdialog->getText().isEmpty())
    srchdialog->setText(pattern);

  this->deselect();
  last_search = NONE;

  this->clearFocus();

  QPoint point = this->mapToGlobal (QPoint (0,0));

  QRect pos = this->geometry();
  srchdialog->setGeometry(point.x() + pos.width()/2  - srchdialog->width()/2,
			   point.y() + pos.height()/2 - srchdialog->height()/2, 
			   srchdialog->width(),
			   srchdialog->height());
   
  srchdialog->show();
  srchdialog->result();
}


void KEdit::search_slot(){

  int line, col;

  if (!srchdialog)
    return;

  QString to_find_string = srchdialog->getText();
  getCursorPosition(&line,&col);
  
  // srchdialog->get_direction() is true if searching backward

  if (last_search != NONE && srchdialog->get_direction()){
    col = col  - pattern.length() - 1 ;
  }

again:
  int  result = doSearch(to_find_string, srchdialog->case_sensitive(),
			 FALSE, (!srchdialog->get_direction()),line,col);
    
  if(result == 0){
    if(!srchdialog->get_direction()){ // forward search
    
      int query = QMessageBox::information(
					   srchdialog,
					   i18n("Find"), 
					   i18n("End of document reached.\n"\
                                                "Continue from the beginning?"), 
					   i18n("Yes"),
					   i18n("No"),
					   "",
					   0,1);
      if (query == 0){
	line = 0;
	col = 0;
	goto again;
      }
    }
    else{ //backward search
      
      int query = QMessageBox::information(
					   srchdialog,
					   i18n("Find"), 
					   i18n("Beginning of document reached.\n"\
                                                "Continue from the end?"), 
					   i18n("Yes"),
					   i18n("No"),
					   "",
					   0,1);
      if (query == 0){
	QString string = textLine( numLines() - 1 );
	line = numLines() - 1;
	col  = string.length();
	last_search = BACKWARD;
	goto again;
      }
    }
  }
  else{
    emit CursorPositionChanged(); 
  }
}



void KEdit::searchdone_slot(){

  if (!srchdialog)
    return;

  srchdialog->hide();
  this->setFocus();
  last_search = NONE;

}

int KEdit::doSearch(QString s_pattern, bool case_sensitive, 
		    bool wildcard, bool forward, int line, int col){

  (void) wildcard; // reserved for possible extension to regex


  int i, length;  
  int pos = -1;

  if(forward){

    QString string;

    for(i = line; i < numLines(); i++) {

      string = textLine(i);

      pos = string.find(s_pattern, i == line ? col : 0, case_sensitive);
      
      if( pos != -1){
      
	length = s_pattern.length();

	setCursorPosition(i,pos,FALSE);
      
	for(int l = 0 ; l < length; l++){
	  cursorRight(TRUE);
	}
      
	setCursorPosition( i , pos + length, TRUE );
	pattern = s_pattern;
	last_search = FORWARD;

	return 1;
      }
    }
  }
  else{ // searching backwards

    QString string;
    
    for(i = line; i >= 0; i--) {

      string = textLine(i);
      int line_length = string.length();

      pos = string.findRev(s_pattern, line == i ? col : line_length , case_sensitive);

      if (pos != -1){

	length = s_pattern.length();

	if( ! (line == i && pos > col ) ){

	  setCursorPosition(i ,pos ,FALSE );
      
	  for(int l = 0 ; l < length; l++){
	    cursorRight(TRUE);
	  }	
      
	  setCursorPosition(i ,pos + length ,TRUE );
	  pattern = s_pattern;
	  last_search = BACKWARD;
	  return 1;

	}
      }

    }
  }
  
  return 0;

}



int KEdit::repeatSearch() {
  
  if(!srchdialog)
      return 0;


  if(pattern.isEmpty()) // there wasn't a previous search
    return 0;

  search_slot();

  this->setFocus();
  return 1;

}


//////////////////////////////////////////////////////////////////////////
//
// Replace Methods
//


void KEdit::replace(){

  if (srchdialog)
	if (srchdialog->isVisible())
		srchdialog->hide();
  

  if (!replace_dialog){
    
    replace_dialog = new KEdReplace(0, "replace_dialog");
    connect(replace_dialog,SIGNAL(find_signal()),this,SLOT(replace_search_slot()));
    connect(replace_dialog,SIGNAL(replace_signal()),this,SLOT(replace_slot()));
    connect(replace_dialog,SIGNAL(replace_all_signal()),this,SLOT(replace_all_slot()));
    connect(replace_dialog,SIGNAL(replace_done_signal()),this,SLOT(replacedone_slot()));
  
  }

  if(replace_dialog->getText().isEmpty())
    replace_dialog->setText(pattern);


  this->deselect();
  last_replace = NONE;

  this->clearFocus();

  QPoint point = this->mapToGlobal (QPoint (0,0));

  QRect pos = this->geometry();
  replace_dialog->setGeometry(point.x() + pos.width()/2  - replace_dialog->width()/2,
			   point.y() + pos.height()/2 - replace_dialog->height()/2, 
			   replace_dialog->width(),
			   replace_dialog->height());
  replace_dialog->show();
  replace_dialog->result();
}


void KEdit::replace_slot(){

  if (!replace_dialog)
    return;

  if(!can_replace){
    QApplication::beep();
    return;
  }

  int line,col, length;

  QString string = replace_dialog->getReplaceText();
  length = string.length();

  this->cut();

  getCursorPosition(&line,&col);

  insertAt(string,line,col);
  setModified();
  can_replace = FALSE;

  setCursorPosition(line,col);
  for( int k = 0; k < length; k++){
    cursorRight(TRUE);
  }

}

void KEdit::replace_all_slot(){

  if (!replace_dialog)
    return;

  QString to_find_string = replace_dialog->getText();
  getCursorPosition(&replace_all_line,&replace_all_col);
  
  // replace_dialog->get_direction() is true if searching backward

  if (last_replace != NONE && replace_dialog->get_direction()){
    replace_all_col = replace_all_col  - pattern.length() - 1 ;
  }
  
  deselect();

again:

  setAutoUpdate(FALSE);
  int result = 1;

  while(result){

    result = doReplace(to_find_string, replace_dialog->case_sensitive(),
		       FALSE, (!replace_dialog->get_direction()),
		       replace_all_line,replace_all_col,TRUE);

  }

  setAutoUpdate(TRUE);
  update();
    
  if(!replace_dialog->get_direction()){ // forward search
    
    int query = QMessageBox::information(
				   replace_dialog,
				   i18n("Find"), 
				   i18n("End of document reached.\n"\
                                        "Continue from the beginning?"), 
				   i18n("Yes"),
				   i18n("No"),
				   "",
				   0,1);
    if (query == 0){
      replace_all_line = 0;
      replace_all_col = 0;
      goto again;
    }
  }
  else{ //backward search
    
    int query = QMessageBox::information(
				   replace_dialog,
				   i18n("Find"), 
				   i18n("Beginning of document reached.\n"\
                                        "Continue from the end?"), 
				   i18n("Yes"),
				   i18n("No"),
				   "",
				   0,1);
    if (query == 0){
      QString string = textLine( numLines() - 1 );
      replace_all_line = numLines() - 1;
      replace_all_col  = string.length();
      last_replace = BACKWARD;
      goto again;
    }
  }

  emit CursorPositionChanged(); 

}


void KEdit::replace_search_slot(){

  int line, col;

  if (!replace_dialog)
    return;

  QString to_find_string = replace_dialog->getText();
  getCursorPosition(&line,&col);
  
  // replace_dialog->get_direction() is true if searching backward

  //printf("col %d length %d\n",col, pattern.length());

  if (last_replace != NONE && replace_dialog->get_direction()){
    col = col  - pattern.length() -1;
    if (col < 0 ) {
      if(line !=0){
	col = strlen(textLine(line - 1));
	line --;
      }
      else{

	int query = QMessageBox::information(
			 replace_dialog,
			 i18n("Replace"), 
			 i18n("Beginning of document reached.\n"\
                              "Continue from the end?"), 
			 i18n("Yes"),
			 i18n("No"),
			 "",
			 0,1);

	if (query == 0){
	  QString string = textLine( numLines() - 1 );
	  line = numLines() - 1;
	  col  = string.length();
	  last_replace = BACKWARD;
	}
      }
    }
  }

again:

  //  printf("Col %d \n",col);

  int  result = doReplace(to_find_string, replace_dialog->case_sensitive(),
			 FALSE, (!replace_dialog->get_direction()), line, col, FALSE );
    
  if(result == 0){
    if(!replace_dialog->get_direction()){ // forward search
    
      int query = QMessageBox::information(
				     replace_dialog,
				     i18n("Replace"), 
				     i18n("End of document reached.\n"\
                                          "Continue from the beginning?"), 
				     i18n("Yes"),
				     i18n("No"),
				     "",
				     0,1);
      if (query == 0){
	line = 0;
	col = 0;
	goto again;
      }
    }
    else{ //backward search
      
      int query = QMessageBox::information(
					   replace_dialog,
					   i18n("Replace"), 
					   i18n("Beginning of document reached.\n"\
                                                "Continue from the end?"), 
					   i18n("Yes"),
					   i18n("No"),
					   "",
					   0,1);
      if (query == 0){
	QString string = textLine( numLines() - 1 );
	line = numLines() - 1;
	col  = string.length();
	last_replace = BACKWARD;
	goto again;
      }
    }
  }
  else{

    emit CursorPositionChanged(); 
  }
}



void KEdit::replacedone_slot(){

  if (!replace_dialog)
    return;
  
  replace_dialog->hide();
  //  replace_dialog->clearFocus();

  this->setFocus();

  last_replace = NONE;
  can_replace  = FALSE;

}



int KEdit::doReplace(QString s_pattern, bool case_sensitive, 
	   bool wildcard, bool forward, int line, int col, bool replace_all){


  (void) wildcard; // reserved for possible extension to regex

  int line_counter, length;  
  int pos = -1;

  QString string;
  QString stringnew;
  QString replacement;
  
  replacement = replace_dialog->getReplaceText();
  line_counter = line;
  replace_all_col = col;

  if(forward){

    int num_lines = numLines();

    while (line_counter < num_lines){
      
      string = "";
      string = textLine(line_counter);

      if (replace_all){
	pos = string.find(s_pattern, replace_all_col, case_sensitive);
      }
      else{
	pos = string.find(s_pattern, line_counter == line ? col : 0, case_sensitive);
      }

      if (pos == -1 ){
	line_counter ++;
	replace_all_col = 0;
	replace_all_line = line_counter;
      }

      if( pos != -1){

	length = s_pattern.length();
	
	if(replace_all){ // automatic

	  stringnew = string.copy();
	  stringnew.replace(pos,length,replacement);

	  removeLine(line_counter);
	  insertLine(stringnew,line_counter);

	  replace_all_col = replace_all_col + replacement.length();
	  replace_all_line = line_counter;

	  setModified();
	}
	else{ // interactive

	  setCursorPosition( line_counter , pos, FALSE );

	  for(int l = 0 ; l < length; l++){
	    cursorRight(TRUE);
	  }

	  setCursorPosition( line_counter , pos + length, TRUE );
	  pattern = s_pattern;
	  last_replace = FORWARD;
	  can_replace = TRUE;

	  return 1;

	}
	
      }
    }
  }
  else{ // searching backwards

    while(line_counter >= 0){

      string = "";
      string = textLine(line_counter);

      int line_length = string.length();

      if( replace_all ){
      	pos = string.findRev(s_pattern, replace_all_col , case_sensitive);
      }
      else{
	pos = string.findRev(s_pattern, 
			   line == line_counter ? col : line_length , case_sensitive);
      }

      if (pos == -1 ){
	line_counter --;

	if(line_counter >= 0){
	  string = "";
	  string = textLine(line_counter);
	  replace_all_col = string.length();
	  
	}
	replace_all_line = line_counter;
      }


      if (pos != -1){
	length = s_pattern.length();

	if(replace_all){ // automatic

	  stringnew = string.copy();
	  stringnew.replace(pos,length,replacement);

	  removeLine(line_counter);
	  insertLine(stringnew,line_counter);

	  replace_all_col = replace_all_col - replacement.length();
	  replace_all_line = line_counter;

	  setModified();

	}
	else{ // interactive

	  //	  printf("line_counter %d pos %d col %d\n",line_counter, pos,col);
	  if( ! (line == line_counter && pos > col ) ){

	    setCursorPosition(line_counter ,pos ,FALSE );
      
	    for(int l = 0 ; l < length; l++){
	      cursorRight(TRUE);
	    }	
	
	    setCursorPosition(line_counter ,pos + length ,TRUE );
	    pattern = s_pattern;

	    last_replace = BACKWARD;
	    can_replace = TRUE;

	    return 1;
	  }
	}
      }
    }
  }
  
  return 0;

}





////////////////////////////////////////////////////////////////////
//
// Find Dialog
//


KEdSrch::KEdSrch(QWidget *parent, const char *name)
    : QDialog(parent, name,FALSE){

    this->setFocusPolicy(QWidget::StrongFocus);
    int fontHeight = 2*fontMetrics().height();
    QVBoxLayout * mainLayout = new QVBoxLayout(this, 10);

    frame1 = new QGroupBox(i18n("Find"), this, "frame1");
    mainLayout->addWidget(frame1);
    QVBoxLayout * frameLayout = new QVBoxLayout(frame1, 15);

    value = new QLineEdit( frame1, "value");
    value->setFocus();
    value->setMinimumWidth(200);
    value->setFixedHeight(fontHeight);
    frameLayout->addWidget(value);
    connect(value, SIGNAL(returnPressed()), this, SLOT(ok_slot()));

    QHBoxLayout * hLay = new QHBoxLayout();
    frameLayout->addLayout(hLay);
    sensitive = new QCheckBox(i18n("Case Sensitive"), frame1, "case");
    sensitive->setFixedSize( sensitive->sizeHint() );
    hLay->addWidget(sensitive);
    direction = new QCheckBox(i18n("Find Backwards"), frame1, "direction");
    direction->setFixedSize( direction->sizeHint() );
    hLay->addWidget(direction);

    frameLayout->addStretch(10); // so that frame doesn't grow
    mainLayout->addStretch(10);

    hLay = new QHBoxLayout();
    mainLayout->addLayout(hLay);
    ok = new QPushButton(i18n("Find"), this, "find");
    connect(ok, SIGNAL(clicked()), this, SLOT(ok_slot()));

    hLay->addStretch();
    hLay->addWidget(ok);
    hLay->addStretch();

    cancel = new QPushButton(i18n("Done"), this, "cancel");
    hLay->addWidget(cancel);
    hLay->addStretch();
    cancel->setFocus();

    connect(cancel, SIGNAL(clicked()), this, SLOT(done_slot()));
    //    connect(cancel, SIGNAL(clicked()), this, SLOT(reject()));

    resize(sizeHint());

}

void KEdSrch::focusInEvent( QFocusEvent *)
{
    value->setFocus();
    //value->selectAll();
}

QString KEdSrch::getText() { return value->text(); }

void KEdSrch::setText(QString string){

  value->setText(string);

}

void KEdSrch::done_slot(){

  emit search_done_signal();

}


bool KEdSrch::case_sensitive(){

  return sensitive->isChecked();

}

bool KEdSrch::get_direction(){

  return direction->isChecked();

}


void KEdSrch::ok_slot(){

  QString text;

  text = value->text();

  if (!text.isEmpty())
    emit search_signal();

}


////////////////////////////////////////////////////////////////////
//
//  Replace Dialog
//


KEdReplace::KEdReplace(QWidget *parent, const char *name)
    : QDialog(parent, name,FALSE){


    this->setFocusPolicy(QWidget::StrongFocus);
    int fontHeight = 2*fontMetrics().height();
    QVBoxLayout * mainLayout = new QVBoxLayout(this, 10);

    frame1 = new QGroupBox(i18n("Find"), this, "frame1");
    mainLayout->addWidget(frame1);
    QVBoxLayout * frameLayout = new QVBoxLayout(frame1, 20);

    value = new QLineEdit( frame1, "value");
    value->setFocus();
    value->setMinimumWidth(200);
    value->setFixedHeight(fontHeight);
    frameLayout->addWidget(value);
    connect(value, SIGNAL(returnPressed()), this, SLOT(ok_slot()));

    label = new QLabel(frame1,"Rlabel");
    label->setText(i18n("Replace with:"));
    label->setFixedSize( label->sizeHint() );
    frameLayout->addWidget(label);

    replace_value = new QLineEdit( frame1, "replac_value");
    replace_value->setMinimumWidth(200);
    replace_value->setFixedHeight(fontHeight);
    frameLayout->addWidget(replace_value);
    connect(replace_value, SIGNAL(returnPressed()), this, SLOT(ok_slot()));

    QHBoxLayout * hLay = new QHBoxLayout();
    frameLayout->addLayout(hLay);

    sensitive = new QCheckBox(i18n("Case Sensitive"), frame1, "case");
    sensitive->setChecked(TRUE);
    hLay->addWidget(sensitive);

    direction = new QCheckBox(i18n("Find Backwards")
			      ,frame1 , "direction");
    hLay->addWidget(direction);

    frameLayout->addStretch(10); // so that frame doesn't grow
    mainLayout->addStretch(10);

    hLay = new QHBoxLayout();
    mainLayout->addLayout(hLay);
    
    ok = new QPushButton(i18n("Find"), this, "find");
    connect(ok, SIGNAL(clicked()), this, SLOT(ok_slot()));
    hLay->addStretch();
    hLay->addWidget(ok);
    hLay->addStretch();

    replace = new QPushButton(i18n("Replace"), this, "rep");
    connect(replace, SIGNAL(clicked()), this, SLOT(replace_slot()));
    hLay->addWidget(replace);
    hLay->addStretch();

    replace_all = new QPushButton(i18n("Replace All"), this, "repall");
    connect(replace_all, SIGNAL(clicked()), this, SLOT(replace_all_slot()));
    hLay->addWidget(replace_all);
    hLay->addStretch();

    cancel = new QPushButton(i18n("Done"), this, "cancel");
    connect(cancel, SIGNAL(clicked()), this, SLOT(done_slot()));
    hLay->addWidget(cancel);
    hLay->addStretch();

    resize(sizeHint());
}


void KEdReplace::focusInEvent( QFocusEvent *){

    value->setFocus();
    // value->selectAll();
}

QString KEdReplace::getText() { return value->text(); }

QString KEdReplace::getReplaceText() { return replace_value->text(); }

void KEdReplace::setText(QString string) { 

  value->setText(string); 

}

void KEdReplace::done_slot(){

  emit replace_done_signal();

}


void KEdReplace::replace_slot(){

  emit replace_signal();

}

void KEdReplace::replace_all_slot(){

  emit replace_all_signal();

}


bool KEdReplace::case_sensitive(){

  return sensitive->isChecked();

}


bool KEdReplace::get_direction(){

  return direction->isChecked();

}


void KEdReplace::ok_slot(){

  QString text;
  text = value->text();

  if (!text.isEmpty())
    emit find_signal();

}


KEdGotoLine::KEdGotoLine( QWidget *parent, const char *name)
	: QDialog( parent, name, TRUE )
{
	frame = new QGroupBox( i18n("Goto Line"), this );
	lineNum = new KIntLineEdit( this );
	this->setFocusPolicy( QWidget::StrongFocus );
	connect(lineNum, SIGNAL(returnPressed()), this, SLOT(accept()));

	ok = new QPushButton(i18n("Go"), this );
	cancel = new QPushButton(i18n("Cancel"), this ); 

	connect(cancel, SIGNAL(clicked()), this, SLOT(reject()));
	connect(ok, SIGNAL(clicked()), this, SLOT(accept()));
	resize(300, 120); 

}

void KEdGotoLine::selected(int)
{
	accept();
}

void KEdGotoLine::resizeEvent(QResizeEvent *)
{
    frame->setGeometry(5, 5, width() - 10, 80);
    cancel->setGeometry(width() - 80, height() - 30, 70, 25);
    ok->setGeometry(10, height() - 30, 70, 25);
    lineNum->setGeometry(20, 35, width() - 40, 25);
}

void KEdGotoLine::focusInEvent( QFocusEvent *)
{
    lineNum->setFocus();
    lineNum->selectAll();
}

int KEdGotoLine::getLineNumber()
{
	return lineNum->getValue();
}
