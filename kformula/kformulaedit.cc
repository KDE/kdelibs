#include "kformulaedit.h"
#include "kformula.h"
#include "box.h"
#include <qkeycode.h>
#include <stdio.h>

//initialize the static clipboard
QString KFormulaEdit::clipText;

//cursor blink interval:
#define BLINK 500

//possible chacheStates:
#define ALL_DIRTY 0
#define SOME_DIRTY 1
#define ALL_CLEAN 2

//easier.
#define IS_LGROUP(x) ((x) == L_GROUP || (x) == L_BRACE_UNSEEN)
#define IS_RGROUP(x) ((x) == R_GROUP || (x) == R_BRACE_UNSEEN)

//  The widget works by having an internal string which the user
//  unwittingly edits.  This string is re-parsed into a formula
//  object whenever it is changed.  The cursorCache is for computing
//  cursor positions in idle time because it is slow for larger formulas.
//  For some reason, strchr(anything, '\0') returns 1 (perhaps it counts
//  the terminating character of anything) so before every strchr,
//  I check that the input is not '\0' (as may happen if a QChar is cast
//  into char). 


//-----------------------CONSTRUCTOR--------------------
//Plain vanilla constructor--just initialization

KFormulaEdit::KFormulaEdit(QWidget * parent, const char *name, WFlags f) :
  QWidget(parent, name, f)
{
  form = new KFormula;
  pm.resize(width(), height());
  setBackgroundMode(PaletteBase);
  setFocusPolicy(StrongFocus);
  setCursor(ibeamCursor);
  cursorPos = 0;
  cacheState = ALL_DIRTY;
  isSelecting = 0;
  textSelected = 0;
  undo.setAutoDelete(TRUE); //delete strings as soon as we're done with 'em
  redo.setAutoDelete(TRUE);
  setFont(QFont("times", 19)); //just default
  clipText.sprintf("");

  formText.sprintf("");

  form->parse(formText, &info); // initialize info

  QPainter p;

  p.begin(&pm);
  p.fillRect(0, 0, pm.width(), pm.height(), QColor(255,255,255));
  p.end();

  connect(&t, SIGNAL(timeout()), this, SLOT(toggleCursor()));
  connect(&fast, SIGNAL(timeout()), this, SLOT(computeCache()));
  fast.start(1); // computes cursor positions in the background
}

//---------------------------FOCUS IN EVENT---------------------
//Start it blinking:
void KFormulaEdit::focusInEvent(QFocusEvent *)
{
  cursorDrawn = 1;
  t.start(BLINK);
  redraw(0);
}

//---------------------------FOCUS OUT EVENT--------------------
//Stop the blinking:
void KFormulaEdit::focusOutEvent(QFocusEvent *)
{
  cursorDrawn = 0;
  t.stop();
  redraw(0);
}

//--------------------------DESTRUCTOR-------------------------
//Duh
KFormulaEdit::~KFormulaEdit()
{
  if(form) delete form;
  form = NULL;
}

//-------------------------SET TEXT (slot)----------------------
//reset the text for the formula--clear all undo and redo as well
void KFormulaEdit::setText(QString text)
{
  formText = text;
  form->parse(formText, &info);
  cacheState = ALL_DIRTY;
  while(undo.remove());
  while(redo.remove());
  redraw();
}

//--------------------------RESIZE EVENT--------------------------
void KFormulaEdit::resizeEvent(QResizeEvent *)
{
  pm.resize(width(), height());
  cacheState = ALL_DIRTY;
  redraw();
}

//------------------------------REDRAW----------------------------
// This calls the redraw function for the formula object.
// Then it draws the selection rectangle
// all = 1, by default.  if all is 1, just reblit and redraw
// the cursor.  If all is 0, do the whole thing.

void KFormulaEdit::redraw(int all)
{
  QPainter p;

  if(all == 0) {
    repaint(FALSE);
    return;
  }

  p.begin(&pm);
  p.setFont(font());
  p.fillRect(0, 0, pm.width(), pm.height(), backgroundColor());

  //temp is just for debugging.
  /*QString temp = formText;

  temp.insert(cursorPos, '$');

  fprintf(stderr, "\r%s       ", temp.ascii());*/

  form->setPos(pm.width() / 2, pm.height() / 2);
  form->redraw(p);

  if(textSelected && selectStart != cursorPos) {
    //draw selection with white brush, since XORing
    p.setRasterOp(XorROP);
    int i;
    QRect tmp;

    //The selection rectangle is the union of all cursor positions
    //inside the selection.  It also includes the bounding rectangles
    //for boxes like roots, division, absolute value, and parentheses
    //if the entire box is included in the selection.  Not that pretty,
    //but it works.

    for(i = QMIN(cursorPos, selectStart);
	i <= QMAX(cursorPos, selectStart); i++) {

      if(isInString(i, KFormula::delim() + QChar(SQRT) + QChar(DIVIDE))) {
	if(tmp.isNull()) {
	  tmp = info[i].where->getLastRect();
	}
	else {
	  tmp = tmp.unite(info[i].where->getLastRect());
	}
      }
      else {
	if(tmp.isNull()) {
	  tmp = getCursorPos(i);
	}
	else {
	  tmp = tmp.unite(getCursorPos(i));
	}
	if(i < QMAX(cursorPos, selectStart) && //actually inside the selection
	   (isInString(i, QString(QChar(SLASH))) ||
	   formText[i].unicode() >= SYMBOL_ABOVE)) {
	  //we need its height
	  box *b = info[i].where;

	  if(formText[i].unicode() < SYMBOL_ABOVE) // if slash
	    while(b->getType() != SLASH) {
	      if(b->getParent() != NULL) b = b->getParent();
	      else break;
	    }
	  
	  tmp = tmp.unite(QRect(tmp.x(), b->getLastRect().y(),
				1, b->getLastRect().height()));
	}
      }
    }

    p.fillRect(tmp, QBrush(backgroundColor()));
  }

  p.end();

  repaint(FALSE);
}

//-----------------------IS IN STRING-----------------------
//checks if str contains formText[pos].  For readability.
int KFormulaEdit::isInString(int pos, const QString &str)
{
  return pos < (int)formText.length() && str.contains(formText[pos]);
}

//-----------------------GET CURSOR POS----------------------
//If the cursor position is already computed and in the cache,
//return it.  Otherwise, compute the cursor position and add it
//to the cache.

QRect KFormulaEdit::getCursorPos(int pos)
{
  //If it's all dirty, set each individual element to dirty.
  if(cacheState == ALL_DIRTY) computeCache();

  if(cursorCache[pos].dirty) {
    cursorCache[pos].pos = form->getCursorPos(info[pos]);
    cursorCache[pos].dirty = 0;
  }

  return cursorCache[pos].pos;
}

//-------------------------PAINT EVENT------------------------
//Blits and if there is no text selected draws the cursor

void KFormulaEdit::paintEvent(QPaintEvent *)
{
  bitBlt(this, 0, 0, &pm, 0, 0, -1, -1);

  if((!textSelected || cursorPos == selectStart) && cursorDrawn) {
    QRect r;
    QPainter p(this);
    r = getCursorPos(cursorPos);
    p.drawLine(r.left(), r.top(), r.left(), r.bottom());
    p.drawLine(r.left() - 2, r.top(), r.left() + 2, r.top());
    p.drawLine(r.left() - 2, r.bottom(), r.left() + 2, r.bottom());
  }
}

//------------------------IS VALID CURSOR POS------------------
//The cursor cannot be anywhere.  It cannot be between an
//operation symbol and the opening brace for it
//so "x^${2}" is impossible (with $ representing the cursor).
//Also, the cursor cannot be in the left group (which must be
//always empty) of parentheses or absolute value, like this:
// "x+{$}|{y}".  The beginning and the end of the string are always
//valid positions.

int KFormulaEdit::isValidCursorPos(int pos)
{
  if(pos == 0 || pos == (int)formText.length()) return 1;

  if(formText[pos - 1] == L_BRACE_UNSEEN) return 0;
  if(formText[pos] == R_BRACE_UNSEEN) return 0;

  if(formText[pos] == L_GROUP && isInString(pos - 1, KFormula::delim() +
					    KFormula::loc() + QChar(SQRT) +
					    QChar(DIVIDE))) return 0;
  
  if(formText[pos - 1] == R_GROUP && isInString(pos, KFormula::delim() +
						QChar(SQRT) + QChar(ABOVE) +
						QChar(BELOW) +
						QChar(DIVIDE))) return 0;

  if(formText[pos] == R_GROUP && isInString(pos + 1, KFormula::delim()))
    return 0;

  return 1;
}

//----------------------DELETE AT CURSOR--------------------
//This is surprisingly painful.  See individual comments.
//returns 1 if text has been deleted, 0 if it wasn't in
//position to delete anything.

int KFormulaEdit::deleteAtCursor()
{
  int maxpos = formText.length() - 1; //easier to type "maxpos"
  int ncpos = cursorPos; //this stores the position at which
                        //the actual deletion will be made
                        //so if we alter it, we don't have to change
                        //cursorPos.

  if(cursorPos > maxpos) return 0;

  //If we are just deleting part of a literal (or +-*), do it and go away.
  if(!isInString(cursorPos, KFormula::delim() +
		 KFormula::loc() + L_GROUP + R_GROUP +
		 QChar(DIVIDE) + QChar(SQRT))) {
    formText.remove(cursorPos, 1);
    return 1;
  }

  //we only delete an operator if both its operands are empty.
  //if we are not in an empty group, the following if statement
  //catches it and returns.
  if((cursorPos > 0 && IS_RGROUP(formText[cursorPos]) &&
     !IS_LGROUP(formText[cursorPos - 1])) ||
     (cursorPos < maxpos && IS_LGROUP(formText[cursorPos]) &&
      !IS_RGROUP(formText[cursorPos + 1])))
    return 0;

  //if we happen to be in a right group, this shifts cursor position from
  //"{}/{$}" to "{}$/{}" with $ representing ncpos.  Notice that
  //this may be an invalid cursor position but that doesn't matter.
  if(cursorPos > 1 && formText[cursorPos] == R_GROUP &&
     isInString(cursorPos - 2, KFormula::delim() + KFormula::loc() +
		QChar(SQRT) + QChar(DIVIDE)))
    ncpos -= 2;
  else if(cursorPos > 0 && cursorPos <= maxpos &&
	  IS_LGROUP(formText[cursorPos]) &&
	  isInString(cursorPos - 1, KFormula::delim() + KFormula::loc() +
		     QChar(SQRT) + QChar(DIVIDE)))
    ncpos--;
  //the else shifts from "{}/${}" to "{}$/{}".  Even though the
  //former is an invalid cursor position, it may still happen
  //because the backspace key decrements cursorPos without checking
  //for validity

  //The following handles the case where the operator has only
  //the right operand grouped (exponents and subscripts).
  //It also checks whether the group is empty before deleting.
  if(isInString(ncpos, QString(QChar(POWER)) + QChar(SUB)) &&
     ncpos <= maxpos - 2 &&
     IS_LGROUP(formText[ncpos + 1]) && IS_RGROUP(formText[ncpos + 2])) {
    formText.remove(ncpos, 3);
    cursorPos = ncpos;
    return 1;
  }

  //Shifts position from "{$}/{}" and "${}/{}" to "{}$/{}".
  if(IS_LGROUP(formText[ncpos]) &&
     isInString(ncpos + 2, KFormula::delim() + QChar(SQRT) + QChar(DIVIDE)))
    {
      ncpos += 2;
    }
  else if(ncpos > 0 && ncpos < maxpos &&
	  IS_RGROUP(formText[ncpos]) &&
	  (formText[ncpos + 1] == QChar(ABOVE) ||
	   formText[ncpos + 1] == QChar(BELOW) ||
	   formText[ncpos + 1] == QChar(SQRT) ||
	   formText[ncpos + 1] == QChar(DIVIDE)))
    ncpos++;

  //The following removes the division operator and leaves the
  //numerator intact.
  if(ncpos <= maxpos - 2 && IS_RGROUP(formText[ncpos + 2]) &&
     IS_LGROUP(formText[ncpos + 1])) {
    if(ncpos <= 1) return 0;
    if(formText[ncpos] == QChar(DIVIDE) ||
       formText[ncpos] == QChar(ABOVE) ||
       formText[ncpos] == QChar(BELOW)) {
      int i, level = 0;
      ncpos--;
      formText.remove(ncpos, 4); // "{hello$}/{}" -> "{hello$"

      //now find the curly brace which marks the start of the numerator,
      //remove it, and go away.
      for(i = ncpos - 1; i >= 0; i--) {
	if(IS_RGROUP(formText[i])) level++;
	if(IS_LGROUP(formText[i])) level--;
	if(level < 0) {
	  formText.remove(i, 1); //removes the curly brace.
	  ncpos--;
	  cursorPos = ncpos;
	  return 1;
	}
      }
    }

    //If we are have "{3}$@{}", we delete the 3 even though the group
    //is not empty.  I thought this was best.
    if(isInString(ncpos, KFormula::delim() + QChar(SQRT))) {
      int i, level = 0;
      ncpos--;
      formText.remove(ncpos, 4);
      for(i = ncpos - 1; i >= 0; i--) {
	if(IS_RGROUP(formText[i])) level++;
	if(IS_LGROUP(formText[i])) level--;
	formText.remove(i, 1); //remove everything including the curly brace.
	ncpos--;
	if(level < 0) {
	  cursorPos = ncpos;
	  return 1;
	}
      }
    }
  }

  return 0;
  
}

//----------------------COMPUTE CACHE----------------------
//does one step of the cursor position computation.
void KFormulaEdit::computeCache()
{
  if(cacheState == ALL_CLEAN) return; //we're done
  int i;

  //if it's ALL_DIRTY, at the first step we simply change each
  //individual cache slot to dirty and leave.
  if(cacheState == ALL_DIRTY) {
    cursorCache.resize(formText.length() + 1);
    for(i = 0; i < (int)cursorCache.size(); i++) {
      cursorCache[i].dirty = 1;
    }
    cacheState = SOME_DIRTY;
    return;
  }

  //find the first dirty slot
  for(i = 0; i < (int)cursorCache.size(); i++) {
    if(cursorCache[i].dirty == 1) break;
  }

  //if it's also the last dirty slot, we're now clean.
  if(i >= (int)cursorCache.size() - 1)
    cacheState = ALL_CLEAN;

  //if there's no dirty slot, leave.
  if(i == (int)cursorCache.size())
    return;

  //calculate it and mark it clean.
  cursorCache[i].pos = form->getCursorPos(info[i]);
  cursorCache[i].dirty = 0;

}

//----------------------POS AT POINT-------------------
//When Joe User clicks the mouse, this figures out the nearest
//cursor position to his mouse click.  Nothing intelligent about
//this routine.

int KFormulaEdit::posAtPoint(QPoint p)
{
  int i;
  QPoint tmp;
  int mini, mindist, dist;

  mindist = 99999999; //They don't make screens that big (yet).
  mini = cursorPos;

  for(i = 0; i <= (int)formText.length(); i++) {
    if(!isValidCursorPos(i)) continue;
    tmp = getCursorPos(i).center() - p;
    dist = tmp.x() * tmp.x() + tmp.y() * tmp.y();
    if(dist < mindist) {
      mindist = dist;
      mini = i;
    }
  }

  return mini;
}

//CURSOR_RESET is what we say after we move the cursor or do something
//and we want to have it drawn immediately.
#define CURSOR_RESET cursorDrawn = 1; t.start(BLINK);

//-----------------MOUSE PRESS EVENT-------------------
//simple--shifts the cursor to where the click happened and deselects
//all text.

void KFormulaEdit::mousePressEvent(QMouseEvent *e)
{
  int oldcpos = cursorPos;
  cursorPos = posAtPoint(e->pos());
  if(oldcpos != cursorPos) {
    CURSOR_RESET
  }
  if(textSelected) {
    textSelected = 0;
    redraw();
  }
  else redraw(0);
}

//-----------------EXPAND SELECTION--------------------
//we want to only select complete blocks.  This function enforces
//this

void KFormulaEdit::expandSelection()
{
  selectStart = selectStartOrig; //so that selection is "unexpanded"
                                 //when it shrinks

  if(cursorPos == selectStart) return;
  int dir; //which way the user is selecting
  int i;

  //1 if selecting to the right, -1 if to the left
  if(cursorPos < selectStart) dir = -1;
  else dir = 1;

  int level = 0; //level is (depth of nested groups) relative to
                 //cursorPos and selectStart

  i = selectStart;

  if(dir == 1) {
    //expand to the right until level is 0.  If level is negative,
    //expand selection to the left.
    while(i <= (int)formText.length() &&
	  (i < cursorPos || level != 0 || !isValidCursorPos(i))) {
      if(i < (int)formText.length() && IS_LGROUP(formText[i])) level++;
      if(i < (int)formText.length() && IS_RGROUP(formText[i])) level--;
      if(level == -1) {
	while(selectStart > 0 &&
	      (level < 0 || !isValidCursorPos(selectStart))) {
	  selectStart--;
	  if(IS_LGROUP(formText[selectStart])) level++;
	  if(IS_RGROUP(formText[selectStart])) level--;
	}
      }
      i++;
    }
  }

  if(dir == -1) {
    while(i > 0 &&
	  (i > cursorPos || level != 0 || !isValidCursorPos(i))) {
      i--;
      if(IS_LGROUP(formText[i])) level--;
      if(IS_RGROUP(formText[i])) level++;
      if(level == -1) {
	while(selectStart <= (int)formText.length() &&
	      (level < 0 || !isValidCursorPos(selectStart))) {
	  if(selectStart < (int)formText.length() && 
	     IS_LGROUP(formText[selectStart])) level--;
	  if(selectStart < (int)formText.length() &&
	     IS_RGROUP(formText[selectStart])) level++;
	  selectStart++;
	}
      }
    }
  }

  cursorPos = i;

  return;
}

//--------------------MOUSE MOVE EVENT-----------------
//Just select more text

void KFormulaEdit::mouseMoveEvent(QMouseEvent *e)
{
  int oldcpos = cursorPos;
  cursorPos = posAtPoint(e->pos());
  if(oldcpos != cursorPos) {
    if(!isSelecting) { //if selection is just starting
      textSelected = 1;
      isSelecting = 1;
      selectStartOrig = selectStart = oldcpos;
    }
    expandSelection();
    CURSOR_RESET
    redraw();
  }
}

//-------------------------MOUSE RELEASE EVENT--------------------
//this concludes a drag.
void KFormulaEdit::mouseReleaseEvent(QMouseEvent *)
{
  if(isSelecting && cursorPos == selectStart) {
    textSelected = 0;
  }

  isSelecting = 0;
}

//---------------------------TOGGLE CURSOR------------------------
//duh.
void KFormulaEdit::toggleCursor()
{
  cursorDrawn = !cursorDrawn;
  redraw(0);
}

//----------------------------KEY PRESS EVENT---------------------
//Key event handler.

//MODIFIED reparses the string, resets the cursor, invalidates the
//cache, adds an undo step, and removes all redo.
#define MODIFIED { form->parse(formText, &info); CURSOR_RESET \
  cacheState = ALL_DIRTY; undo.push(new QString(oldText)); \
  while(redo.remove()); }

void KFormulaEdit::keyPressEvent(QKeyEvent *e)
{
  int shift = (e->state() & ShiftButton); //easier to type "shift"
  QString oldText = formText; // for undo

  //LEFT ARROW:

  if(e->key() == Key_Left) {
    int oldc = cursorPos;

    if(!textSelected || shift) { //if we are not removing a selection
      //move left to the next valid cursor position.
      while(cursorPos > 0 && !isValidCursorPos(--cursorPos));
      
      if(oldc != cursorPos) {
	CURSOR_RESET
      }

      if(shift && !textSelected) { //start selection.
	textSelected = 1;
	selectStartOrig = selectStart = oldc;
      }

      if(textSelected) {
	expandSelection();
	redraw();

      }
      else if(oldc != cursorPos) redraw(0);
    }
    else { // we remove the selection
      cursorPos = QMIN(selectStart, cursorPos);
      textSelected = 0;
      CURSOR_RESET
      redraw();
    }
    return;
  }

  //RIGHT ARROW:

  if(e->key() == Key_Right) {
    int oldc = cursorPos;

    if(!textSelected || shift) { //if we are not removing a selection
      //move right to the next valid cursor position.
      while(cursorPos < (int)formText.length() &&
	    !isValidCursorPos(++cursorPos));
      
      if(oldc != cursorPos) {
	CURSOR_RESET
      }

      if(shift && !textSelected) { //start selection.
	textSelected = 1;
	selectStartOrig = selectStart = oldc;
      }

      if(textSelected) {
	expandSelection();
	redraw();

      }
      else if(oldc != cursorPos) redraw(0);
    }
    else { // we remove the selection
      cursorPos = QMAX(selectStart, cursorPos);
      textSelected = 0;
      CURSOR_RESET
      redraw();
    }
    return;
  }

  //HOME KEY or CTRL+A:
  
  if(e->key() == Key_Home ||
     (e->state() & ControlButton && e->key() == Key_A)) {
    if(cursorPos != 0) {
      if(shift && !textSelected) {
	textSelected = 1;
	selectStartOrig = selectStart = cursorPos;
      }
      cursorPos = 0;
      CURSOR_RESET
    }
    if(textSelected) {
      if(!shift) {
	textSelected = 0;
	CURSOR_RESET;
      }
      else {
	expandSelection();
      }
      redraw();
    }
    else redraw(0);
    return;
  }

  //END KEY or CTRL+E:  

  if(e->key() == Key_End ||
     (e->state() & ControlButton && e->key() == Key_E)) {
    if(cursorPos < (int)formText.length()) {
      if(shift && !textSelected) {
	textSelected = 1;
	selectStartOrig = selectStart = cursorPos;
      }
      cursorPos = formText.length();
      CURSOR_RESET
    }
    if(textSelected) {
      if(!shift) {
	textSelected = 0;
	CURSOR_RESET;
      }
      else {
	expandSelection();
      }
      redraw();
    }
    else redraw(0);
    return;
  }

  //BACKSPACE KEY:

  if(e->key() == Key_Backspace) {
    if(textSelected) { //if there's text selected, kill it.
      formText.remove(QMIN(selectStart, cursorPos),
		      QMAX(selectStart - cursorPos, cursorPos - selectStart));
      cursorPos = QMIN(selectStart, cursorPos);
      textSelected = 0;
      MODIFIED
      redraw();
      return;
    }
    if(cursorPos == 0) return;
    cursorPos--; //who cares if it's invalid, we're deleting anyway

    if(deleteAtCursor()) {
      MODIFIED
    }

    //possibly shift it to a valid cursor position.
    while(!isValidCursorPos(cursorPos)) cursorPos--; //now if it's invalid,
                                                     //we shift it left.

    redraw();
    return;    
  }

  //DELETE KEY:

  if(e->key() == Key_Delete) {
    if(textSelected) {
      formText.remove(QMIN(selectStart, cursorPos),
		      QMAX(selectStart - cursorPos, cursorPos - selectStart));
      cursorPos = QMIN(selectStart, cursorPos);
      textSelected = 0;
      MODIFIED
      redraw();
      return;
    }

    if(cursorPos >= (int)formText.length()) return;

    if(deleteAtCursor()) {
      MODIFIED
      redraw();
    }
    return;    
  }

  //CUT, COPY, PASTE, UNDO, REDO:

  if(e->state() & ControlButton) {

    //Copy:
    if(e->key() == Key_C) {
      if(textSelected) {
	clipText =
	  QString(formText.mid(QMIN(selectStart, cursorPos),
		  QMAX(selectStart - cursorPos + 1, \
		      cursorPos - selectStart + 1) - 1));
      }
      return;
    }

    //Cut: copy and remove
    if(e->key() == Key_X) {
      if(textSelected) {
	clipText =
	  QString(formText.mid(QMIN(selectStart, cursorPos),
		  QMAX(selectStart - cursorPos + 1, \
		      cursorPos - selectStart + 1) - 1));
	formText.remove(QMIN(selectStart, cursorPos),
			QMAX(selectStart - cursorPos, \
			    cursorPos - selectStart));
	cursorPos = QMIN(selectStart, cursorPos);
	textSelected = 0;
	MODIFIED
	redraw();
	return;
      }
    }

    //Paste: just insert it into cursorPos, deleting any selected text.
    if(e->key() == Key_V) {
      if(clipText.length() > 0) {
	if(textSelected) {
	  formText.remove(QMIN(selectStart, cursorPos),
			  QMAX(selectStart - cursorPos, \
			      cursorPos - selectStart));
	  cursorPos = QMIN(selectStart, cursorPos);
	  textSelected = 0;
	}
	formText.insert(cursorPos, clipText);
	cursorPos += clipText.length();
	MODIFIED
        redraw();
	return;
      }
    }

    if(e->key() == Key_Z) { // undo
      //pop the undo stack and push the current string onto redo.
      if(!undo.isEmpty()) {
	if(textSelected) textSelected = 0;
	redo.push(new QString(oldText));
	formText = *undo.top(); //we don't want it deleted
	                        //until a shallow copy is made--
	                        //so we don't pop right away.
	undo.pop();
	form->parse(formText, &info); //can't use MODIFIED
	cacheState = ALL_DIRTY;
	if(cursorPos == (int)oldText.length() ||
	   cursorPos > (int)formText.length()) cursorPos = formText.length();
	else while(!isValidCursorPos(cursorPos)) cursorPos++;
	redraw();
      }
      return;
    }

    if(e->key() == Key_R) { // redo
      //same thing as undo but backwards
      if(!redo.isEmpty()) {
	if(textSelected) textSelected = 0;
	undo.push(new QString(oldText));
	formText = *redo.top();
	redo.pop();
	form->parse(formText, &info);
	cacheState = ALL_DIRTY;
	if(cursorPos == (int)oldText.length() ||
	   cursorPos > (int)formText.length()) cursorPos = formText.length();
	else while(!isValidCursorPos(cursorPos)) cursorPos++;
	redraw();
      }
      return;	
    }
  }
  //NORMAL KEY:
  //remove the selection and insert what the user types and
  //perhaps some curly braces
  if(!(e->state() & (ControlButton | AltButton))  && e->ascii() >= 32 &&
     !strchr("{})#", e->ascii())) {
    // the {})# are chars that can't be typed

    insertChar(QChar(e->ascii()));

    MODIFIED
    redraw();
    return;
  }

  if(e->state() & ControlButton) {
    switch(e->key()) {

    case Key_BracketLeft:
      insertChar(QChar(ABOVE));
      break;

    case Key_BracketRight:
      insertChar(QChar(BELOW));
      break;

    case Key_Slash:
      insertChar(QChar(DIVIDE));
      break;

    case Key_6:
      insertChar(QChar(POWER));
      break;

    case Key_2:
      insertChar(QChar(SQRT));
      break;
      
    case Key_Minus:
      insertChar(QChar(SUB));
      break;

    case Key_4:
      insertChar(QChar(INTEGRAL));
      break;

    case Key_5:
      insertChar(QChar(SUM));
      break;

    case Key_8:
      insertChar(QChar(PRODUCT));
      break;

    case Key_Period:
      insertChar(QChar(ARROW));
      break;
      
    }

    MODIFIED
    redraw();
    return;
  }

  e->ignore(); //follow the rules...
}

//---------------------------INSERT CHAR--------------------
//slot.  Inserts c and whatever braces it needs (and removes the
//selection perhaps) into the string.  Useful to connect to a
//tool bar.

void KFormulaEdit::insertChar(QChar c)
{

  if(!(KFormula::loc() + KFormula::delim() + QChar(DIVIDE) +
       QChar(SQRT) + KFormula::bigop()).contains(c)) {
    // "/^_@(|" are chars that need groups

    if(textSelected) {
      formText.remove(QMIN(selectStart, cursorPos),
		      QMAX(selectStart - cursorPos, \
			   cursorPos - selectStart));
      cursorPos = QMIN(selectStart, cursorPos);
      textSelected = 0;
    }
    formText.insert(cursorPos++, c);

  }
  else { //if we need to auto insert curly braces
    //if user entered a '/' (DIVIDE) then insert curly braces after it
    //and surround some previous text with curly braces.  Example:
    //"1+2^{3}$" -> (user types '/') -> "1+{2^{3}}/{$}".

    if(KFormula::bigop().contains(c)) { // we need to add limits
      if(textSelected) {
	formText.remove(QMIN(selectStart, cursorPos),
			QMAX(selectStart - cursorPos, \
			     cursorPos - selectStart));
	cursorPos = QMIN(selectStart, cursorPos);
	textSelected = 0;
      }
      //insert "{{c}]{$}}[{}#"
      formText.insert(cursorPos++, L_GROUP);
      formText.insert(cursorPos++, L_GROUP);
      formText.insert(cursorPos++, c);
      formText.insert(cursorPos++, R_GROUP);
      formText.insert(cursorPos++, QChar(BELOW));
      formText.insert(cursorPos++, L_GROUP);
      formText.insert(cursorPos++, R_GROUP);
      formText.insert(cursorPos++, R_GROUP);
      formText.insert(cursorPos++, QChar(ABOVE));
      formText.insert(cursorPos++, L_GROUP);
      formText.insert(cursorPos++, R_GROUP);
      cursorPos -= 5;
    }

    if(c == DIVIDE || c == QChar(ABOVE) || c == QChar(BELOW)) {
      int i, level;

      //if there is selected text, put curly braces around that so
      //that the entire selection is the numerator:
      if(textSelected) {
	formText.insert(QMAX(selectStart, cursorPos), R_GROUP);
	formText.insert(QMIN(selectStart, cursorPos), L_GROUP);
	cursorPos = QMAX(selectStart, cursorPos) + 2;
      }
      
      formText.insert(cursorPos, c); //insert the slash or whatever
      cursorPos++;
      
      formText.insert(cursorPos, L_GROUP);
      cursorPos++;
      formText.insert(cursorPos, R_GROUP);

      //if there is no selection we identify the numerator:
      if(!textSelected) {
	formText.insert(cursorPos - 2, R_GROUP);
	
	level = 0;
	
	for(i = cursorPos - 3; i >= 0; i--) {
	  if(formText[i] == R_GROUP) level++;
	  if(formText[i] == L_GROUP) level--;
	  if(level < 0) break;
	  //figure out how to do precedence for bigop!
	  if(level == 0 && isInString(i, KFormula::intext())) break;
	  //the "+-#=<>" are all operators with lower precedence than
	  //the divide.  if they are encoundered, they don't end up in
	  //the numerator (unless they are selected but then we wouldn't
	  //be here).
	}
	formText.insert(i + 1, L_GROUP);
	
	cursorPos += 2;
      }
      textSelected = 0;

      return;
    }
    
    //these just need a pair of curly braces after the operator.
    if(KFormula::loc().contains(c)) { // "x$" -> "x^{$}"
      if(textSelected) {
	cursorPos = QMAX(cursorPos, selectStart);
      }
      textSelected = 0;
      formText.insert(cursorPos, c);
      cursorPos++;
      formText.insert(cursorPos, L_GROUP);
      cursorPos++;
      formText.insert(cursorPos, R_GROUP);
    }
    
    //these guys need an explicit group preceding, but it's
    //initially (and for "(|" always) empty.
    //So: "x+$" -> "x+{}({$}"
    if((KFormula::delim() + QChar(SQRT)).contains(c)) {
      if(!textSelected) {
	formText.insert(cursorPos, c);
	cursorPos++;
	formText.insert(cursorPos - 1, L_GROUP);
	cursorPos++;
	formText.insert(cursorPos - 1, R_GROUP);
	cursorPos++;
	formText.insert(cursorPos, L_GROUP);
	cursorPos++;
	formText.insert(cursorPos, R_GROUP);
      }
      else { //the entire selection ends up as the right operand.
	formText.insert(QMAX(cursorPos, selectStart), R_GROUP);
	formText.insert(QMIN(cursorPos, selectStart), L_GROUP);
	formText.insert(QMIN(cursorPos, selectStart), c);
	formText.insert(QMIN(cursorPos, selectStart), R_GROUP);
	formText.insert(QMIN(cursorPos, selectStart), L_GROUP);
	cursorPos = QMAX(cursorPos, selectStart) + 5;
	textSelected = 0;
      }
    }
  }

  return;
}

#include "kformulaedit.moc"



