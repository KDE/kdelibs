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
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#include <qdragobject.h>
#include <qpopupmenu.h>
#include <qtextstream.h>
#include <qtimer.h>

#include <kapplication.h>
#include <kcursor.h>
#include <kdebug.h>
#include <kcmenumngr.h>
#include <kfontdialog.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kstdaccel.h>

#include "keditcl.h"
#include "keditcl.moc"

class KEdit::KEditPrivate
{
public:
    bool overwriteEnabled;
};


KEdit::KEdit(QWidget *_parent, const char *name)
   : QMultiLineEdit(_parent, name)
{
    d = new KEditPrivate;
    d->overwriteEnabled = false;

    parent = _parent;

    // fancy optimized refreshing (Matthias and Paul)
    repaintTimer = new QTimer(this);
    connect(repaintTimer, SIGNAL(timeout()), this, SLOT(repaintAll()));

    // set some defaults

    line_pos = col_pos = 0;

    srchdialog = NULL;
    replace_dialog= NULL;
    gotodialog = NULL;

    setAcceptDrops(true);
    KCursor::setAutoHideCursor( this, true );
}


KEdit::~KEdit()
{
  delete d;
}

void
KEdit::insertText(QTextStream *stream)
{
   setAutoUpdate(FALSE);
   int line, col;
   getCursorPosition(&line, &col);
   int saveline = line;
   int savecol = col;
   QString textLine;

   // MS: Patch by Martin Schenk <martin@schenk.com>
   // MS: disable UNDO, or QMultiLineEdit remembers every textLine !!!
   // memory usage is:
   //   textLine: 2*size rounded up to nearest power of 2 (520Kb -> 1024Kb)
   //   widget:   about (2*size + 60bytes*lines)
   // -> without disabling undo, it often needs almost 8*size
#if QT_VERSION < 300
   bool oldUndo = isUndoEnabled();
   setUndoEnabled( FALSE );
#else
   int oldUndoDepth = undoDepth();
   setUndoDepth( 0 ); // ### -1?
#endif

   // MS: read everything at once if file <= 1MB,
   // else read in 5000-line chunks to keep memory usage acceptable.
   QIODevice *dev=stream->device();
   if (dev && dev->size()>(1024*1024)) {
      while(1) {
        int i;
        textLine="";
        for (i=0; i<5000; i++) {
                QString line=stream->readLine();
                if (line.isNull()) break;  // EOF
                textLine+=line+'\n';
        }
        insertAt(textLine, line, col);
        line+=i; col=0;
        if (i!=5000) break;
      }
   }
   else {
        textLine = stream->read(); // Read all !
        insertAt( textLine, line, col);
   }
#if QT_VERSION < 300
   setUndoEnabled(oldUndo);
#else
   setUndoDepth( oldUndoDepth );
#endif

   setCursorPosition(saveline, savecol);
   setAutoUpdate(true);

   if (!repaintTimer->isActive())
      repaintTimer->start(0,TRUE);
   repaint();

   setModified(true);
   setFocus();

   // Bernd: Please don't leave debug message like that lying around
   // they cause ENORMOUSE performance hits. Once upon a day
   // kedit used to be really really fast using memmap etc .....
   // oh well ....

   //   QString str = text();
   //   for (int i = 0; i < (int) str.length(); i++)
   //     printf("KEdit: U+%04X\n", str[i].unicode());

}

void
KEdit::cleanWhiteSpace()
{
   setAutoUpdate(FALSE);
   if (!hasMarkedText())
      selectAll();
   QString oldText = markedText();
   QString newText;
   QStringList lines = QStringList::split('\n', oldText, true);
   bool addSpace = false;
   bool firstLine = true;
   QChar lastChar = oldText[oldText.length()-1];
   QChar firstChar = oldText[0];
   for(QStringList::Iterator it = lines.begin();
       it != lines.end();)
   {
      QString line = (*it).simplifyWhiteSpace();
      if (line.isEmpty())
      {
         if (addSpace)
            newText += QString::fromLatin1("\n\n");
         if (firstLine)
         {
            if (firstChar.isSpace())
               newText += '\n';
            firstLine = false;
         }
         addSpace = false;
      }
      else
      {
         if (addSpace)
            newText += ' ';
         if (firstLine)
         {
            if (firstChar.isSpace())
               newText += ' ';
            firstLine = false;
         }
         newText += line;
         addSpace = true;
      }
      it = lines.remove(it);
   }
   if (addSpace)
   {
      if (lastChar == '\n')
         newText += '\n';
      else if (lastChar.isSpace())
         newText += ' ';
   }

   if (oldText == newText)
   {
      deselect();
      setAutoUpdate(TRUE);
      if (!repaintTimer->isActive())
         repaintTimer->start(0,TRUE);
      repaint();
      return;
   }
   if (wordWrap() == NoWrap)
   {
      // If wordwrap is off, we have to do some line-wrapping ourselves now
      // We use another QMultiLineEdit for this, so that we get nice undo
      // behaviour.
      QMultiLineEdit *we = new QMultiLineEdit();
      we->setWordWrap(FixedColumnWidth);
      we->setWrapColumnOrWidth(78);
      we->setText(newText);
      newText = QString::null;
      for(int i = 0; i < we->numLines(); i++)
      {
        QString line = we->textLine(i);
        if (line.right(1) != "\n")
           line += '\n';
        newText += line;
      }
      delete we;
   }

   insert(newText);
   setAutoUpdate(TRUE);
   if (!repaintTimer->isActive())
      repaintTimer->start(0,TRUE);
   repaint();

   setModified(true);
   setFocus();
}

void
KEdit::saveText(QTextStream *stream)
{
   int line_count = numLines()-1;
   if (line_count < 0)
      return;
   for(int i = 0; i < line_count; i++)
   {
      (*stream) << textLine(i) << '\n';
   }
   (*stream) << textLine(line_count);
}

void KEdit::repaintAll(){
    repaint(FALSE);
}

int KEdit::currentLine(){

  computePosition();
  return line_pos;

};

int KEdit::currentColumn(){

  computePosition();
  return col_pos;

}

void KEdit::computePosition(){

  int line, col, coltemp;

  getCursorPosition(&line,&col);
  QString linetext = textLine(line);

  // O.K here is the deal: The function getCursorPositoin returns the character
  // position of the cursor, not the screenposition. I.e,. assume the line
  // consists of ab\tc then the character c will be on the screen on position 8
  // whereas getCursorPosition will return 3 if the cursors is on the character c.
  // Therefore we need to compute the screen position from the character position.
  // That's what all the following trouble is all about:

  coltemp  = 	col;
  int pos  = 	0;
  int find = 	0;
  int mem  = 	0;
  bool found_one = false;

  // if you understand the following algorithm you are worthy to look at the
  // kedit+ sources -- if not, go away ;-)

  while(find >=0 && find <= coltemp- 1 ){
    find = linetext.find('\t', find, TRUE );
    if( find >=0 && find <= coltemp - 1 ){
      found_one = true;
      pos = pos + find - mem;
      pos = pos + 8  - pos % 8;
      mem = find;
      find ++;
    }
  }

  pos = pos + coltemp - mem ;  // add the number of characters behind the
                               // last tab on the line.

  if (found_one){
    pos = pos - 1;
  }

  line_pos = line;
  col_pos = pos;

}


void KEdit::keyPressEvent ( QKeyEvent *e){


  if ((e->state() & ControlButton ) && (e->key() == Key_K) ){

    int line = 0;
    int col  = 0;
    QString killstring;

    if(!killing){
      killbufferstring = "";
      killtrue = false;
      lastwasanewline = false;
    }

    getCursorPosition(&line,&col);
    killstring = textLine(line);
    killstring = killstring.mid(col,killstring.length());


    if(!killbufferstring.isEmpty() && !killtrue && !lastwasanewline){
      killbufferstring += '\n';
    }

    if( (killstring.length() == 0) && !killtrue){
      killbufferstring += '\n';
      lastwasanewline = true;
    }

    if(killstring.length() > 0){

      killbufferstring += killstring;
      lastwasanewline = false;
      killtrue = true;

    }else{

      lastwasanewline = false;
      killtrue = !killtrue;

    }

    killing = true;

    QMultiLineEdit::keyPressEvent(e);
    setModified(true);
    emit CursorPositionChanged();
    return;
  }

  if ((e->state() & ControlButton ) && (e->key() == Key_Y) ){

    int line = 0;
    int col  = 0;

    getCursorPosition(&line,&col);

    if(!killtrue)
      killbufferstring += '\n';

    insertAt(killbufferstring,line,col);

    killing = false;
    setModified(true);
    emit CursorPositionChanged();
    return;
  }

  killing = false;

  if ((e->state() & ControlButton ) && (e->key() == Key_Insert) ){
    copy();
    return;
  }

  if ((e->state() & ShiftButton ) && (e->key() == Key_Insert) ){
    paste();
    setModified(true);
    emit CursorPositionChanged();
    return;
  }

  if ((e->state() & ShiftButton ) && (e->key() == Key_Delete) ){
    cut();
    setModified(true);
    emit CursorPositionChanged();
    return;
  }

  if (d->overwriteEnabled)
  {
    if (e->key() == Key_Insert){
       this->setOverwriteMode(!this->isOverwriteMode());
       emit toggle_overwrite_signal();
       return;
    }
  }

  if ( KStdAccel::isEqual( e, KStdAccel::deleteWordBack()) ) {
      deleteWordBack();  // to be replaced with QT3 function
      e->accept();
      setModified(true);
      emit CursorPositionChanged();
      return;
  }
  else if ( KStdAccel::isEqual( e, KStdAccel::deleteWordForward()) ) {
    deleteWordForward(); // to be replaced with QT3 function
    e->accept();
    setModified(true);
    emit CursorPositionChanged();
    return;
  }

  QMultiLineEdit::keyPressEvent(e);
  emit CursorPositionChanged();
}


void KEdit::mousePressEvent (QMouseEvent* e){


  QMultiLineEdit::mousePressEvent(e);
  emit CursorPositionChanged();

}

void KEdit::mouseMoveEvent (QMouseEvent* e){

  QMultiLineEdit::mouseMoveEvent(e);
  emit CursorPositionChanged();


}


void KEdit::installRBPopup(QPopupMenu *p) {
  KContextMenuManager::insert( this, p );
}

void KEdit::mouseReleaseEvent (QMouseEvent* e){


  QMultiLineEdit::mouseReleaseEvent(e);
  emit CursorPositionChanged();

}

void KEdit::selectFont(){

  QFont font = this->font();
  KFontDialog::getFont(font);
  this->setFont(font);

}

#if QT_VERSION < 300
void KEdit::setModified(bool _mod){
  setEdited(_mod);
}

bool KEdit::isModified(){
    return edited();
}
#endif

// ### KDE3: remove!
bool KEdit::eventFilter(QObject* o, QEvent* ev)
{
  return QMultiLineEdit::eventFilter( o, ev );
}

QString KEdit::markedText(){
  return QMultiLineEdit::markedText();
}

void KEdit::doGotoLine() {

	if( !gotodialog )
		gotodialog = new KEdGotoLine( parent, "gotodialog" );

	this->clearFocus();

	gotodialog->show();
	// this seems to be not necessary
	// gotodialog->setFocus();
	if( gotodialog->result() ) {
		setCursorPosition( gotodialog->getLineNumber()-1 , 0, FALSE );
		emit CursorPositionChanged();
		setFocus();
	}
}


void  KEdit::dragMoveEvent(QDragMoveEvent* e) {

  if(QUriDrag::canDecode(e))
    e->accept();
  else if(QTextDrag::canDecode(e))
    QMultiLineEdit::dragMoveEvent(e);
}


void  KEdit::dragEnterEvent(QDragEnterEvent* e) {

  kdDebug() << "KEdit::dragEnterEvent()" << endl;
  e->accept(QUriDrag::canDecode(e) || QTextDrag::canDecode(e));
}


void  KEdit::dropEvent(QDropEvent* e) {

  kdDebug() << "KEdit::dropEvent()" << endl;

  if(QUriDrag::canDecode(e)) {
   emit gotUrlDrop(e);
  }
  else if(QTextDrag::canDecode(e))
    QMultiLineEdit::dropEvent(e);
}

void KEdit::setOverwriteEnabled(bool b)
{
  d->overwriteEnabled = b;
}

// QWidget::create() turns off mouse-Tracking which would break auto-hiding
void KEdit::create( WId id, bool initializeWindow, bool destroyOldWindow )
{
  QMultiLineEdit::create( id, initializeWindow, destroyOldWindow );
  KCursor::setAutoHideCursor( this, true, true );
}

// Temporary functions until QT3 appears. - Seth Chaiklin 20 may 2001
void KEdit::deleteWordForward()
{
  cursorWordForward(TRUE);
  if ( hasMarkedText() )
    del();
}

void KEdit::deleteWordBack()
{
  cursorWordBackward(TRUE);
  if ( hasMarkedText() )
    del();
}

