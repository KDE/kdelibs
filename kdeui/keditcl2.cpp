/* This file is part of the KDE libraries

   Copyright (C) 1997 Bernd Johannes Wuebben <wuebben@math.cornell.edu>
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
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include <limits.h> // INT_MAX

#include <QFrame>
#include <qlabel.h>
#include <qlineedit.h>
#include <qcheckbox.h>
#include <qlayout.h>
#include <qpushbutton.h>

#include <q3popupmenu.h>
#include <Q3VButtonGroup>

#include <kcombobox.h>
#include <knuminput.h>
#include <kmessagebox.h>
#include <knotification.h>
#include <klocale.h>
#include <kdebug.h>
#include <kiconloader.h>
#include <kvbox.h>

#include "keditcl.h"


//////////////////////////////////////////////////////////////////////////
//
// Find Methods
//

void KEdit::search(){

  if( replace_dialog && replace_dialog->isVisible() )
  {
    replace_dialog->hide();
  }

  if( !srchdialog )
  {
    srchdialog = new KEdFind( this, "searchdialog", false);
    connect(srchdialog,SIGNAL(search()),this,SLOT(search_slot()));
    connect(srchdialog,SIGNAL(done()),this,SLOT(searchdone_slot()));
  }

  // If we already searched / replaced something before make sure it shows
  // up in the find dialog line-edit.

  QString string;
  string = srchdialog->getText();
  srchdialog->setText(string.isEmpty() ? pattern : string);

  deselect();
  last_search = NONE;

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
			 false, (!srchdialog->get_direction()),line,col);

  if(!result){
    if(!srchdialog->get_direction()){ // forward search

      int query = KMessageBox::questionYesNo(
			srchdialog,
                        i18n("End of document reached.\n"\
                             "Continue from the beginning?"),
                        i18n("Find"),KStdGuiItem::cont(),i18n("Stop"));
      if (query == KMessageBox::Yes){
	line = 0;
	col = 0;
	goto again;
      }
    }
    else{ //backward search

      int query = KMessageBox::questionYesNo(
			srchdialog,
                        i18n("Beginning of document reached.\n"\
                             "Continue from the end?"),
                        i18n("Find"),KStdGuiItem::cont(),i18n("Stop"));
      if (query == KMessageBox::Yes){
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
  setFocus();
  last_search = NONE;
}

/* antlarr: KDE 4: make it const QString & */
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

	setCursorPosition(i,pos,false);

	for(int l = 0 ; l < length; l++){
	  cursorRight(true);
	}

	setCursorPosition( i , pos + length, true );
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

	  setCursorPosition(i ,pos ,false );

	  for(int l = 0 ; l < length; l++){
	    cursorRight(true);
	  }

	  setCursorPosition(i ,pos + length ,true );
	  pattern = s_pattern;
	  last_search = BACKWARD;
	  return 1;

	}
      }

    }
  }

  return 0;

}



bool KEdit::repeatSearch() {

  if(!srchdialog || pattern.isEmpty())
  {
      search();
      return true;
  }

  search_slot();

  setFocus();
  return true;

}


//////////////////////////////////////////////////////////////////////////
//
// Replace Methods
//


void KEdit::replace()
{
  if( srchdialog && srchdialog->isVisible() )
  {
    srchdialog->hide();
  }

  if( !replace_dialog )
  {
    replace_dialog = new KEdReplace( this, "replace_dialog", false );
    connect(replace_dialog,SIGNAL(find()),this,SLOT(replace_search_slot()));
    connect(replace_dialog,SIGNAL(replace()),this,SLOT(replace_slot()));
    connect(replace_dialog,SIGNAL(replaceAll()),this,SLOT(replace_all_slot()));
    connect(replace_dialog,SIGNAL(done()),this,SLOT(replacedone_slot()));
  }

  QString string = replace_dialog->getText();
  replace_dialog->setText(string.isEmpty() ? pattern : string);


  deselect();
  last_replace = NONE;

  replace_dialog->show();
  replace_dialog->result();
}


void KEdit::replace_slot(){

  if (!replace_dialog)
    return;

  if(!can_replace){
    KNotification::beep();
    return;
  }

  int line,col, length;

  QString string = replace_dialog->getReplaceText();
  length = string.length();

  cut();

  getCursorPosition(&line,&col);

  insertAt(string,line,col);
  setModified(true);
  can_replace = false;

  if (replace_dialog->get_direction())
  {
    // Backward
    setCursorPosition(line,col+length);
    for( int k = 0; k < length; k++){
      cursorLeft(true);
    }
  }
  else
  {
    // Forward
    setCursorPosition(line,col);
    for( int k = 0; k < length; k++){
      cursorRight(true);
    }
  }
}

void KEdit::replace_all_slot(){

  if (!replace_dialog)
    return;

  QString to_find_string = replace_dialog->getText();

  int lineFrom, lineTo, colFrom, colTo;
  getSelection(&lineFrom, &colFrom, &lineTo, &colTo);

  // replace_dialog->get_direction() is true if searching backward
  if (replace_dialog->get_direction())
  {
    if (colTo != -1)
    {
      replace_all_col = colTo - to_find_string.length();
      replace_all_line = lineTo;
    }
    else
    {
      getCursorPosition(&replace_all_line,&replace_all_col);
      replace_all_col--;
    }
  }
  else
  {
    if (colFrom != -1)
    {
      replace_all_col = colFrom;
      replace_all_line = lineFrom;
    }
    else
    {
      getCursorPosition(&replace_all_line,&replace_all_col);
    }
  }

  deselect();

again:

  setAutoUpdate(false);
  int result = 1;

  while(result){

    result = doReplace(to_find_string, replace_dialog->case_sensitive(),
		       false, (!replace_dialog->get_direction()),
		       replace_all_line,replace_all_col,true);

  }

  setAutoUpdate(true);
  update();

  if(!replace_dialog->get_direction()){ // forward search

    int query = KMessageBox::questionYesNo(
			srchdialog,
                        i18n("End of document reached.\n"\
                             "Continue from the beginning?"),
                        i18n("Find"),KStdGuiItem::cont(),i18n("Stop"));
    if (query == KMessageBox::Yes){
      replace_all_line = 0;
      replace_all_col = 0;
      goto again;
    }
  }
  else{ //backward search

    int query = KMessageBox::questionYesNo(
			srchdialog,
                        i18n("Beginning of document reached.\n"\
                             "Continue from the end?"),
                        i18n("Find"),KStdGuiItem::cont(),i18n("Stop"));
    if (query == KMessageBox::Yes){
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

  int lineFrom, lineTo, colFrom, colTo;
  getSelection(&lineFrom, &colFrom, &lineTo, &colTo);

  // replace_dialog->get_direction() is true if searching backward
  if (replace_dialog->get_direction())
  {
    if (colFrom != -1)
    {
      col = colFrom - to_find_string.length();
      line = lineFrom;
    }
    else
    {
      getCursorPosition(&line,&col);
      col--;
    }
  }
  else
  {
    if (colTo != -1)
    {
      col = colTo;
      line = lineTo;
    }
    else
    {
      getCursorPosition(&line,&col);
    }
  }

again:

  int  result = doReplace(to_find_string, replace_dialog->case_sensitive(),
			 false, (!replace_dialog->get_direction()), line, col, false );

  if(!result){
    if(!replace_dialog->get_direction()){ // forward search

     int query = KMessageBox::questionYesNo(
			replace_dialog,
                        i18n("End of document reached.\n"\
                             "Continue from the beginning?"),
                        i18n("Replace"),KStdGuiItem::cont(),i18n("Stop"));
     if (query == KMessageBox::Yes){
	line = 0;
	col = 0;
	goto again;
      }
    }
    else{ //backward search

     int query = KMessageBox::questionYesNo(
			replace_dialog,
                        i18n("Beginning of document reached.\n"\
                             "Continue from the end?"),
                        i18n("Replace"),KStdGuiItem::cont(),i18n("Stop"));
      if (query == KMessageBox::Yes){
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

  setFocus();

  last_replace = NONE;
  can_replace  = false;

}



/* antlarr: KDE 4: make it const QString & */
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

      string = textLine(line_counter);

      if (replace_all){
	pos = string.find(s_pattern, replace_all_col, case_sensitive);
      }
      else{
	pos = string.find(s_pattern, line_counter == line ? col : 0, case_sensitive);
      }

      if (pos == -1 ){
	line_counter++;
	replace_all_col = 0;
	replace_all_line = line_counter;
      }

      if( pos != -1){

	length = s_pattern.length();

	if(replace_all){ // automatic

          stringnew = string.copy();
          do 
          {  
	    stringnew.replace(pos,length,replacement);

	    replace_all_col = pos + replacement.length();
	    replace_all_line = line_counter;

            pos = stringnew.find(s_pattern, replace_all_col, case_sensitive);
          }
          while( pos != -1); 

	  removeLine(line_counter);
	  insertLine(stringnew,line_counter);

	  setModified(true);
	}
	else{ // interactive

	  setCursorPosition( line_counter , pos, false );

	  for(int l = 0 ; l < length; l++){
	    cursorRight(true);
	  }

	  setCursorPosition( line_counter , pos + length, true );
	  pattern = s_pattern;
	  last_replace = FORWARD;
	  can_replace = true;

	  return 1;

	}

      }
    }
  }
  else{ // searching backwards

    while(line_counter >= 0){

      string = textLine(line_counter);

      int line_length = string.length();

      if( replace_all ){
        if (replace_all_col < 0)
          pos = -1;
        else
          pos = string.findRev(s_pattern, replace_all_col , case_sensitive);
      }
      else{
        if ((line == line_counter) && (col < 0))
          pos = -1;
        else
          pos = string.findRev(s_pattern,
			   line == line_counter ? col : line_length , case_sensitive);
      }

      if (pos == -1 ){
	line_counter--;

        replace_all_col = 0;
	if(line_counter >= 0){
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

	  replace_all_col = pos-length;
	  replace_all_line = line_counter;
	  if (replace_all_col < 0)
	  {
             line_counter--;

             if(line_counter >= 0){
                string = textLine(line_counter);
                replace_all_col = string.length();
             }
             replace_all_line = line_counter;
	  }

	  setModified(true);
	}
	else{ // interactive

	  //	  printf("line_counter %d pos %d col %d\n",line_counter, pos,col);
	  if( ! (line == line_counter && pos > col ) ){

	    setCursorPosition(line_counter, pos + length ,false );

	    for(int l = 0 ; l < length; l++){
	      cursorLeft(true);
	    }

	    setCursorPosition(line_counter, pos ,true );
	    pattern = s_pattern;

	    last_replace = BACKWARD;
	    can_replace = true;

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

class KEdFind::KEdFindPrivate
{
public:
    KEdFindPrivate( QWidget *parent ) {
	combo = new KHistoryCombo( parent );
	combo->setMaxCount( 20 ); // just some default
    }
    ~KEdFindPrivate() {
	delete combo;
    }

    KHistoryCombo *combo;
};


KEdFind::KEdFind( QWidget *parent, const char *name, bool modal )
  :KDialogBase( parent, name, modal, i18n("Find"),
		modal ? User1|Cancel : User1|Close, User1, false, KGuiItem( i18n("&Find"), "find") )
{
// is this really needed at all ?
//  setWFlags( Qt::WType_TopLevel );

  QWidget *page = new QWidget( this );
  setMainWidget(page);
  QVBoxLayout *topLayout = new QVBoxLayout( page, 0, spacingHint() );

  d = new KEdFindPrivate( page );

  QString text = i18n("Find:");
  QLabel *label = new QLabel( text, page , "find" );
  topLayout->addWidget( label );

  d->combo->setMinimumWidth(fontMetrics().maxWidth()*20);
  d->combo->setFocus();

  connect(d->combo, SIGNAL(textChanged ( const QString & )),
          this,SLOT(textSearchChanged ( const QString & )));

  topLayout->addWidget(d->combo);

  group = new Q3VButtonGroup( i18n("Options"), page );
  topLayout->addWidget( group );

  KHBox* row1 = new KHBox( group );

  text = i18n("Case &sensitive");
  sensitive = new QCheckBox( text, row1, "case");
  text = i18n("Find &backwards");
  direction = new QCheckBox( text, row1, "direction" );


  enableButton( KDialogBase::User1, !d->combo->currentText().isEmpty() );

  if ( !modal )
    connect( this, SIGNAL( closeClicked() ), this, SLOT( slotCancel() ) );
}

KEdFind::~KEdFind()
{
    delete d;
}

void KEdFind::textSearchChanged ( const QString &text )
{
   enableButton( KDialogBase::User1, !text.isEmpty() );
}

void KEdFind::slotCancel( void )
{
  emit done();
  KDialogBase::slotCancel();
}

void KEdFind::slotUser1( void )
{
  if( !d->combo->currentText().isEmpty() )
  {
    d->combo->addToHistory( d->combo->currentText() );
    emit search();
  }
}


QString KEdFind::getText() const
{
    return d->combo->currentText();
}


void KEdFind::setText(const QString &string)
{
  d->combo->setEditText(string);
  d->combo->lineEdit()->selectAll();
}

void KEdFind::setCaseSensitive( bool b )
{
  sensitive->setChecked( b );
}

bool KEdFind::case_sensitive() const
{
  return sensitive->isChecked();
}

void KEdFind::setDirection( bool b )
{
  direction->setChecked( b );
}

bool KEdFind::get_direction() const
{
  return direction->isChecked();
}

KHistoryCombo * KEdFind::searchCombo() const
{
    return d->combo;
}



////////////////////////////////////////////////////////////////////
//
//  Replace Dialog
//

class KEdReplace::KEdReplacePrivate
{
public:
    KEdReplacePrivate( QWidget *parent ) {
	searchCombo = new KHistoryCombo( parent );
	replaceCombo = new KHistoryCombo( parent );

	searchCombo->setMaxCount( 20 ); // just some defaults
	replaceCombo->setMaxCount( 20 );
    }
    ~KEdReplacePrivate() {
	delete searchCombo;
	delete replaceCombo;
    }

    KHistoryCombo *searchCombo, *replaceCombo;
};

KEdReplace::KEdReplace( QWidget *parent, const char *name, bool modal )
  :KDialogBase( parent, name, modal, i18n("Replace"),
		modal ? User3|User2|User1|Cancel : User3|User2|User1|Close,
                User3, false,
		i18n("Replace &All"), i18n("&Replace"), KGuiItem( i18n("&Find"), "find") )
{
// is this really needed at all ?
//  setWFlags( Qt::WType_TopLevel );

  setButtonBoxOrientation( Qt::Vertical );

  QFrame *page = makeMainWidget();
  QVBoxLayout *topLayout = new QVBoxLayout( page, 0, spacingHint() );

  d = new KEdReplacePrivate( page );

  QString text = i18n("Find:");
  QLabel *label = new QLabel( text, page, "find" );
  topLayout->addWidget( label );

  d->searchCombo->setMinimumWidth(fontMetrics().maxWidth()*20);
  d->searchCombo->setFocus();
  topLayout->addWidget(d->searchCombo);

  text = i18n("Replace with:");
  label = new QLabel( text, page, "replace" );
  topLayout->addWidget( label );

  d->replaceCombo->setMinimumWidth(fontMetrics().maxWidth()*20);
  topLayout->addWidget(d->replaceCombo);

  connect(d->searchCombo, SIGNAL(textChanged ( const QString & )),
          this,SLOT(textSearchChanged ( const QString & )));

  Q3ButtonGroup *group = new Q3ButtonGroup( i18n("Options"), page );
  topLayout->addWidget( group );

  QGridLayout *gbox = new QGridLayout( group, 3, 2, spacingHint() );
  gbox->addRowSpacing( 0, fontMetrics().lineSpacing() );

  text = i18n("Case &sensitive");
  sensitive = new QCheckBox( text, group, "case");
  text = i18n("Find &backwards");
  direction = new QCheckBox( text, group, "direction" );
  gbox->addWidget( sensitive, 1, 0 );
  gbox->addWidget( direction, 1, 1 );
  gbox->setRowStretch( 2, 10 );
}


KEdReplace::~KEdReplace()
{
    delete d;
}

void KEdReplace::textSearchChanged ( const QString &text )
{
    bool state=text.isEmpty();
    enableButton( KDialogBase::User1, !state );
    enableButton( KDialogBase::User2, !state );
    enableButton( KDialogBase::User3, !state );
}

void KEdReplace::slotCancel( void )
{
  emit done();
  d->searchCombo->clearEdit();
  d->replaceCombo->clearEdit();
  KDialogBase::slotCancel();
}

void KEdReplace::slotClose( void )
{
  slotCancel();
}

void KEdReplace::slotUser1( void )
{
    if( !d->searchCombo->currentText().isEmpty() )
    {
        d->replaceCombo->addToHistory( d->replaceCombo->currentText() );
        emit replaceAll();
    }
}


void KEdReplace::slotUser2( void )
{
    if( !d->searchCombo->currentText().isEmpty() )
    {
        d->replaceCombo->addToHistory( d->replaceCombo->currentText() );
        emit replace();
    }
}

void KEdReplace::slotUser3( void )
{
  if( !d->searchCombo->currentText().isEmpty() )
  {
    d->searchCombo->addToHistory( d->searchCombo->currentText() );
    emit find();
  }
}


QString KEdReplace::getText()
{
    return d->searchCombo->currentText();
}


QString KEdReplace::getReplaceText()
{
    return d->replaceCombo->currentText();
}


/* antlarr: KDE 4: make it const QString & */
void KEdReplace::setText(QString string)
{
  d->searchCombo->setEditText(string);
  d->searchCombo->lineEdit()->selectAll();
}


bool KEdReplace::case_sensitive()
{
  return sensitive->isChecked();
}


bool KEdReplace::get_direction()
{
  return direction->isChecked();
}

KHistoryCombo * KEdReplace::searchCombo() const
{
    return d->searchCombo;
}

KHistoryCombo * KEdReplace::replaceCombo() const
{
    return d->replaceCombo;
}


KEdGotoLine::KEdGotoLine( QWidget *parent, const char *name, bool modal )
  :KDialogBase( parent, name, modal, i18n("Go to Line"), modal ? Ok|Cancel : Ok|Close, Ok, false )
{
  QWidget *page = new QWidget( this );
  setMainWidget(page);
  QVBoxLayout *topLayout = new QVBoxLayout( page, 0, spacingHint() );

  lineNum = new KIntNumInput( 1,page);
  lineNum->setRange(1, 1000000, 1, false);
  lineNum->setLabel(i18n("Go to line:"), Qt::AlignVCenter | Qt::AlignLeft);
//  lineNum->setMinimumWidth(fontMetrics().maxWidth()*20);
  topLayout->addWidget( lineNum );

  topLayout->addStretch(10);
  lineNum->setFocus();
}


void KEdGotoLine::selected(int)
{
  accept();
}


int KEdGotoLine::getLineNumber()
{
  return lineNum->value();
}


//////////////////////////////////////////////////////////////////////////////
//
// Spell Checking
//

void KEdit::spellcheck_start()
{
   saved_readonlystate = isReadOnly();
   setReadOnly(true);
}

void KEdit::misspelling (const QString &word, const QStringList &, unsigned int pos)
{

  unsigned int l = 0;
  unsigned int cnt = 0;
  posToRowCol (pos, l, cnt);
  setSelection(l, cnt, l, cnt+word.length());

  /*
  if (cursorPoint().y()>height()/2)
    kspell->moveDlg (10, height()/2-kspell->heightDlg()-15);
  else
    kspell->moveDlg (10, height()/2 + 15);
  */

}

//need to use pos for insert, not cur, so forget cur altogether
void KEdit::corrected (const QString &originalword, const QString &newword, unsigned int pos)
{
  //we'll reselect the original word in case the user has played with
  //the selection in eframe or the word was auto-replaced

  unsigned int l = 0;
  unsigned int cnt = 0;

  if( newword != originalword )
  {
    posToRowCol (pos, l, cnt);
    setSelection(l, cnt, l, cnt+originalword.length());

    setReadOnly ( false );
    removeSelectedText();
    insert(newword);
    setReadOnly ( true );
  }
  else
  {
    deselect();
  }
}

void KEdit::posToRowCol(unsigned int pos, unsigned int &line, unsigned int &col)
{
  for (line = 0; line < static_cast<uint>(numLines()) && col <= pos; line++)
  {
    col += lineLength(line)+1;
  }
  line--;
  col = pos - col + lineLength(line) + 1;
}

void KEdit::spellcheck_stop()
{
  deselect();

  setReadOnly ( saved_readonlystate);
}

QString KEdit::selectWordUnderCursor( )
{
    int parag;
    int pos;

    getCursorPosition(&parag, &pos);

    QString txt = text(parag);

    // Find start
    int start = pos;
    while( start > 0 )
    {
       const QChar &ch = txt[start-1];
       if (ch.isSpace() || ch.isPunct())
          break;
       start--;
    }

    // Find end
    int end = pos;
    int len = txt.length();
    while( end < len )
    {
       const QChar &ch = txt[end];
       if (ch.isSpace() || ch.isPunct())
          break;
       end++;
    }
    setSelection(parag, start, parag, end);
    return txt.mid(start, end-start);
}

Q3PopupMenu *KEdit::createPopupMenu( const QPoint& pos )
{
    enum { IdUndo, IdRedo, IdSep1, IdCut, IdCopy, IdPaste, IdClear, IdSep2, IdSelectAll };

    Q3PopupMenu *menu = Q3MultiLineEdit::createPopupMenu( pos );
    
    if ( isReadOnly() )
      menu->changeItem( menu->idAt(0), SmallIconSet("editcopy"), menu->text( menu->idAt(0) ) );
    else {
      int id = menu->idAt(0);
      menu->changeItem( id - IdUndo, SmallIconSet("undo"), menu->text( id - IdUndo) );
      menu->changeItem( id - IdRedo, SmallIconSet("redo"), menu->text( id - IdRedo) );
      menu->changeItem( id - IdCut, SmallIconSet("editcut"), menu->text( id - IdCut) );
      menu->changeItem( id - IdCopy, SmallIconSet("editcopy"), menu->text( id - IdCopy) );
      menu->changeItem( id - IdPaste, SmallIconSet("editpaste"), menu->text( id - IdPaste) );
      menu->changeItem( id - IdClear, SmallIconSet("editclear"), menu->text( id - IdClear) );
    }

    return menu;
}
