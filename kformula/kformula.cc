#include "kformula.h"
#include "box.h"
#include <stdio.h>

//initialize the static members:
QString *KFormula::SPECIAL = NULL;
QString *KFormula::DELIM = NULL;
QString *KFormula::INTEXT = NULL;
QString *KFormula::LOC = NULL;

//This class stores and displays the formula

//---------------------CONSTRUCTORS AND DESTRUCTORS-----------------
KFormula::KFormula()
{
  posx = posy = 0;
}

KFormula::KFormula(int x, int y)
{
  posx = x;
  posy = y;
}

KFormula::~KFormula()
{
  while(boxes.size()) {
    delete boxes[boxes.size() - 1];
    boxes.resize(boxes.size() - 1);
  }
}

//---------------------------GET CURSOR POS-------------------------
//simply calls the function on the last (top level) box.
QRect KFormula::getCursorPos(charinfo i)
{
  QRect tmp = boxes[boxes.size() - 1]->getRect();
  return boxes[boxes.size() - 1]->getCursorPos(i, posx - tmp.center().x(),
				 posy - tmp.center().y());
}

//--------------------------------REDRAW----------------------------
//first call calculate, figure out the center, and draw the boxes
void KFormula::redraw(QPainter &p)
{
  if(boxes.size() == 0) return;
  boxes[boxes.size() - 1]->calculate(p, p.font().pointSize());
  QRect tmp = boxes[boxes.size() - 1]->getRect();
  boxes[boxes.size() - 1]->draw(p, posx - tmp.center().x(),
			       posy - tmp.center().y());

  return;
}

//-------------------------------SET BOXES-------------------------
//duh
void KFormula::setBoxes(QArray<box *> newBoxes)
{
  while(boxes.size()) {
    delete boxes[boxes.size() - 1];
    boxes.resize(boxes.size() - 1);
  }
  boxes = newBoxes.copy();
}

//--------------------------------GET BOXES------------------------
//an even bigger DUH than the previous one.
QArray<box *> KFormula::getBoxes()
{
  return boxes.copy();
}

void KFormula::setPos(int x, int y)
{
  posx = x;
  posy = y;
}

//-------------------------------UNPARSE----------------------------

//recursively output a string that, when parsed, results in the
//formula.  Pretty self-explanatory.
//it's not used anywhere now, but it might be useful.
//it will need to be fixed before it's used

QString KFormula::unparse(box *b)
{
  if(b == NULL) b = boxes[boxes.size() - 1];

  if(b->type == TEXT) return b->text;

  QString x;
  
  if(b->b1 != NULL && b->b2 != NULL) {
    x.sprintf("{}%c{}", b->type);

    x.insert(1, unparse(b->b1));
    x.insert(x.length() - 1, unparse(b->b2));
    return x;
  }

  if(b->b1 != NULL) {
    x.sprintf("{}%c{}", b->type);
    x.insert(1, unparse(b->b1));
    return x;
  }

  if(b->b2 != NULL) {
    x.sprintf("{}%c{}", b->type);
    x.insert(x.length() - 1, unparse(b->b2));
    return x;
  }
  //if both are NULL
  x.sprintf("{}%c{}", b->type);
  return x;
}

//INSERTED goes through the charinfo array and increments posinstr
//for all the characters after the one that was inserted.

#define INSERTED(xxi) { int tmpx; if(info) for(tmpx = 0; \
						 tmpx < (int)info->size(); \
						 tmpx++) \
  if((*info)[tmpx].posinstr >= (xxi)) (*info)[tmpx].posinstr++; }

// The parser works by first fully parenthesizing (with {}) the string
// and then removing the parentheses one by one in make_boxes,
// adding a box each time.  Not especially efficient or versatile
// but speed is not critical and this was easy to write.

void KFormula::parse(QString text, QArray<charinfo> *info)
{
  int i, j;

  info->resize(text.length() + 1);

  if(info) { //initialize info
    for(i = 0; i <= (int)text.length(); i++) {
      charinfo inf;
      
      inf.where = NULL;
      inf.posinbox = 0;
      inf.posinstr = i;
      
      info->at(i) = inf;
    }
  }

  //delete the existing boxes
  while(boxes.size() > 0) {
    delete boxes[boxes.size() - 1];
    boxes.resize(boxes.size() - 1);
  }

  //make "unseen" braces into regular ones:
  for(i = 0; i < (int)text.length(); i++) {
    if(text[i] == L_BRACE_UNSEEN) text[i] = QChar(L_GROUP);
    if(text[i] == R_BRACE_UNSEEN) text[i] = QChar(R_GROUP);
  }

  //search for implicit concatenation:
  //insert a CAT (#) symbol at every concatenation:
  //"a{b}/{c}d" -> "a#{b}/{c}#d".

  for(i = 0; i < ((int)text.length() - 1); i++)
    {
      j = i + 1;
      if(j > (int)text.length() - 1) continue;
      if(text[j] != L_GROUP && text[i] != R_GROUP) continue;
      if(text[i] == R_GROUP && text[j] == L_GROUP) {
	text.insert(j, CAT);
	INSERTED(j);
      }

      if((char)text[i] && (char)text[j] &&
          special().contains(text[i]) && special().contains(text[j])) continue;
      text.insert(j, CAT);
      INSERTED(j);
    }

  //now parenthesize everything in reverse order (think about it).
  //After this, "x+y*z^{2}" becomes "{x}+{{y}*{{z}^{{2}}}}"

  //equal and gt lt signs have lowest priority
  for(i = (int)text.length() - 1; i >= 0; i--) {
    if(!(char)text[i] || !strchr("=<>", text[i])) continue;
    parenthesize(text, i, info);
    i++;
  }
  
  //addition and subtraction
  for(i = (int)text.length() - 1; i >= 0; i--) {
    if(text[i] != PLUS && text[i] != MINUS) continue;
    parenthesize(text, i, info);
    i++;
  }

  //concatenation
  for(i = (int)text.length() - 1; i >= 0; i--) {
    if(text[i] != CAT) continue;
    parenthesize(text, i, info);
    i++;
  }

  //multiplication and slash
  for(i = (int)text.length() - 1; i >= 0; i--) {
    if(text[i] != TIMES && text[i] != SLASH) continue;
    parenthesize(text, i, info);
    i++;
  }

  //locational things: should not be reversed despite order of ops.
  for(i = (int)text.length() - 1; i >= 0; i--) {
    if(text[i] != POWER && text[i] != SUB &&
       text[i] != ABOVE && text[i] != BELOW) continue;
    parenthesize(text, i, info);
    i++;
  }

  //this is not in usual order of operations but it works:
  //we want "@@x" to become "{}@{{}@{x}}" not "{{}@{}}@{x}" or anything
  //like that

  //roots and division and parentheses:
  for(i = 0; i < (int)text.length(); i++) {
    if(!(char)text[i] || !strchr("@/(|", text[i])) continue;
    parenthesize(text, i, info);
    i += 3;
  }

  //Now just make the boxes.
  makeBoxes(text, 0, (int)text.length(), info);

}

//---------------------------PARENTHESIZE--------------------------
//given a string and an operator in position i, this puts
//curly braces around the two groups near the operator.  e.g.:
// Before: "{x}+{y*3}".  Then parenthesize is called with
// i = 6.  After: "{x}+{{y}*{3}}"
void KFormula::parenthesize(QString &temp, int i, QArray<charinfo> *info)
{
  int j;
  int level = 0;

  //search for the left end of the left group
  for(j = i; j >= 0; j--) {
    if(temp[j] == R_GROUP) level++;
    if(temp[j] == L_GROUP) level--;
    if(level < 0) {
      temp.insert(j + 1, L_GROUP);
      INSERTED(j + 1);
      i++;
      break;
    }
    if(j == 0) {
      temp.insert(j, L_GROUP);
      INSERTED(j);
      i++;
      break;
    }
  }
  //insert the right brace of the left group and the left brace of the
  //right group:
  temp.insert(i, R_GROUP);
  INSERTED(i); i++;
  temp.insert(i + 1, L_GROUP);
  INSERTED(i + 1); i++;
  level = 0;

  //now search for the right end of the right group
  for(j = i + 1; j <= (int)temp.length(); j++) {
    if(j < (int)temp.length() && temp[j] == L_GROUP) level++;
    if(j < (int)temp.length() && temp[j] == R_GROUP) level--;

    if(level < 0) {
      temp.insert(j, R_GROUP);
      INSERTED(j);
      i++;
      break;
    }

    if(j == (int)temp.length()) {
      temp.insert(j, R_GROUP);
      INSERTED(j);
      i++;
      break;
    }
  }
}

//-----------------------------MAKE BOXES-----------------------------
//creates the boxes from a fully parenthesized str.
//calls itself recursively.
box * KFormula::makeBoxes(QString str, int offset,
			  int maxlen, QArray<charinfo> *info)
{
  int toplevel = -1; //the location of the toplevel operator
  int level = 0;
  int i;

  //when the posinstr bug is fixed, change the "maxlen + 1" in the info
  //comparison to "maxlen"

  if(maxlen <= 0) { // make empty box:
    boxes.resize(boxes.size() + 1);
    boxes[boxes.size() - 1] = new box("");
    if(info) { //update the charinfo for all characters in this box
      for(i = 0; i < (int)info->size(); i++) {
	if((*info)[i].posinstr >= offset &&
	   (*info)[i].posinstr - offset <= maxlen + 1) {
	  (*info)[i].where = boxes[boxes.size() - 1];
	  (*info)[i].posinbox = 0;
	}
      }
    }
    
    return boxes[boxes.size() - 1];
  }

  if(str[0] != L_GROUP) { //we have a literal--make a TEXT box:
    boxes.resize(boxes.size() + 1);
    boxes[boxes.size() - 1] = new box(str.left(maxlen));

    if(info) {
      for(i = 0; i < (int)info->size(); i++) {
	if((*info)[i].posinstr >= offset &&
	   (*info)[i].posinstr - offset <= maxlen + 1) {
	  (*info)[i].where = boxes[boxes.size() - 1];
	  (*info)[i].posinbox = (*info)[i].posinstr - offset;
	}
      }
    }
    
    return boxes[boxes.size() - 1];
  }

  //find toplevel:  in "{{x}*{y}}+{3}" toplevel = 9, the +
  for(i = 0; i < maxlen; i++) {
    if(str[i] == L_GROUP) level++;
    if(str[i] == R_GROUP) level--;

    if(level == 0 && i < maxlen - 1) {
      toplevel = i + 1;
      break;
    }
  }

  //if there is no toplevel operand, strip the outside curly braces.  e.g.:
  //"{{x}+{y}}" -> "{x}+{y}".  Done by recursively calling makeBoxes
  //on the appropriate substring.
  if(toplevel == -1) return makeBoxes(str.mid(1), offset + 1,
				      maxlen - 2, info);

  //this stores the returned pointer so we don't add it to the
  //boxes array until all the children have been added.
  box *tmpbox;

  tmpbox = new box((BoxType)(char)str[toplevel], //that's the operator.
	    makeBoxes(str, offset, toplevel, info),
	    makeBoxes(str.mid(toplevel + 1), offset + toplevel + 1,
		       maxlen - toplevel - 1, info));

  boxes.resize(boxes.size() + 1);
  boxes[boxes.size() - 1] = tmpbox;

  if(info) {
    for(i = 0; i < (int)info->size(); i++) {
      //if the character has been assigned nowhere else, assign it to
      //this box.
      if((*info)[i].where == NULL && (*info)[i].posinstr >= offset &&
	 (*info)[i].posinstr - offset <= maxlen + 1) {
	(*info)[i].where = boxes[boxes.size() - 1];
	//for non-TEXT boxes, posinbox is 0 if the character is
	//to the left of the box and 1 if it is to the right.
	if((*info)[i].posinstr - offset > toplevel) (*info)[i].posinbox = 1;
	else (*info)[i].posinbox = 0;
      }
    }
  }

  return boxes[boxes.size() - 1];
}






