/* -*- C++ -*-
 * This file implements the a general dialog for editing string 
 * lists in a listbox allowing the user to reorder the items.
 * 
 * the KDE addressbook.
 * copyright:  (C) Mirko Sucker, 1998
 * license:    GNU Public License, Version 2
 * mail to:    Mirko Sucker <mirko.sucker@unibw-hamburg.de>
 * requires:   C++-compiler, STL, string class,
 *             Nana for debugging
 * $Revision$
 */

#include "StringListDialog.h"
#include "debug.h"
#include <qbitmap.h>

// the bitmaps
#include "arrow_up.xbm"
#include "arrow_down.xbm"
#include "trash_open.xbm"

const int StringListDialog::ButtonSize=24;

StringListDialog::StringListDialog(QWidget* parent, const char* name)
  : StringListDialogData(parent, name)
{
  // ############################################################################
  // ----- load bitmaps:
  QBitmap up(16, 16, (unsigned char*)uparrow_bits, true);
  QBitmap down(16, 16, (unsigned char*)arrow_down_bits, true);
  QBitmap erase(16, 16, (unsigned char*)trashcan_bits, true);
  // ----- set button texts and bitmaps
  buttonOK->setText(i18n("OK"));
  buttonCancel->setText(i18n("Cancel"));
  buttonUp->setPixmap(up);
  buttonDown->setPixmap(down);
  buttonDelete->setPixmap(erase);
  // ----- setup geometry:
  initializeGeometry();
  // ############################################################################
}

StringListDialog::~StringListDialog()
{
  // ############################################################################
  // ############################################################################
}

void StringListDialog::initializeGeometry()
{
  // ############################################################################
  const int Grid=5;
  int cx, cy, tempy, tempx, buttonWidth, buttonHeight, lbHeight;
  // ----- calculate size:
  tempy=leLine->sizeHint().height();
  tempx=(int)(1.5*leLine->sizeHint().width());
  buttonWidth=QMAX(buttonOK->sizeHint().width(),
		   buttonCancel->sizeHint().width());
  buttonHeight=buttonOK->sizeHint().height();
  tempx=QMAX(tempx, 2*buttonWidth+Grid);
  cx=2*Grid+tempx;
  // ----- the line edit:
  leLine->setGeometry(Grid, Grid, tempx, tempy);
  cy=tempy+2*Grid;
  // ----- the buttons and the listbox:
  lbHeight=7*lbStrings->itemHeight()+4;
  lbStrings->setGeometry(Grid, cy, cx-3*Grid-ButtonSize, lbHeight);
  tempx=2*Grid+lbStrings->width();
  tempy=0;
  buttonUp->setGeometry(tempx, cy+tempy, ButtonSize, ButtonSize);
  tempy+=ButtonSize+Grid;
  buttonDown->setGeometry(tempx, cy+tempy, ButtonSize, ButtonSize);
  tempy+=ButtonSize+Grid;
  buttonDelete->setGeometry(tempx, cy+tempy, ButtonSize, ButtonSize);
  tempy+=ButtonSize+Grid;
  cy+=QMAX(tempy, lbHeight);
  // ----- the horizontal line:
  frameLine->setGeometry(Grid, cy, cx-2*Grid, Grid);
  cy+=2*Grid;
  // ----- the cancel & the ok button:
  buttonOK->setGeometry(Grid, cy, buttonWidth, buttonHeight);
  buttonCancel->setGeometry(cx-Grid-buttonWidth, cy, buttonWidth, buttonHeight);
  cy+=buttonHeight+Grid;
  // ----- resize the widget:
  setFixedSize(cx, cy);
  // ############################################################################
}

void StringListDialog::set(const list<string>& strings)
{
  // ############################################################################
  list<string>::const_iterator pos;
  // -----
  lbStrings->clear();
  for(pos=strings.begin(); pos!=strings.end(); pos++)
    {
      lbStrings->insertItem((*pos).c_str());
    }
  CHECK(lbStrings->count()==strings.size());
  // ############################################################################
}

list<string> StringListDialog::get()
{
  // ############################################################################
  list<string> strings;
  unsigned int count;
  // -----
  for(count=0; count<lbStrings->count(); count++)
    {
      strings.push_back(lbStrings->text(count));
    }
  ENSURE(strings.size()==lbStrings->count());
  return strings;
  // ############################################################################
}

void StringListDialog::addString()
{
  // ############################################################################
  string text=leLine->text();
  // ----
  if(text.empty())
    {
      qApp->beep();
    } else {
      lbStrings->insertItem(text.c_str());
    }
  // ############################################################################
}

void StringListDialog::deletePressed()
{
  // ############################################################################
  int index=lbStrings->currentItem();
  // -----
  if(index==-1)
    {
      qApp->beep();
    } else {
      lbStrings->removeItem(index);
    }
  // ############################################################################
}

void StringListDialog::upPressed()
{
  ID(bool GUARD=true);
  // ############################################################################
  int index=lbStrings->currentItem();
  string text;
  // -----
  LG(GUARD, "StringListDialog::upPressed: moving item %i up.\n", index);
  if(index==-1 || lbStrings->count()<2)
    {
      LG(GUARD, "StringListDialog::upPressed: nothing selected.\n");
      qApp->beep();
    } else {
      if(index!=0)
	{
	  text=lbStrings->text(index);
	  lbStrings->removeItem(index);
	  lbStrings->insertItem(text.c_str(), index-1);
	  lbStrings->setCurrentItem(index-1);
	  lbStrings->centerCurrentItem();
	} else {
	  qApp->beep();
	}
    }	  
  // ############################################################################
}

void StringListDialog::downPressed()
{
  ID(bool GUARD=true);
  // ############################################################################
  int index=lbStrings->currentItem();
  string text;
  // -----
  LG(GUARD, "StringListDialog::downPressed: moving item %i down.\n", index);
  if(index==-1 || lbStrings->count()<2)
    {
      LG(GUARD, "StringListDialog::downPressed: nothing selected.\n");
      qApp->beep();
    } else {
      if((unsigned)index+1!=lbStrings->count())
	{
	  text=lbStrings->text(index);
	  lbStrings->removeItem(index);
	  lbStrings->insertItem(text.c_str(), index+1);
	  lbStrings->setCurrentItem(index+1);
	  lbStrings->centerCurrentItem();
	} else {
	  qApp->beep();
	}
    }
  // ############################################################################
}

// #############################################################################
// MOC OUTPUT FILES:
#include "StringListDialog.moc"
#include "StringListDialogData.moc"
// #############################################################################

