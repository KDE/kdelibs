/* -*- C++ -*-
 * §Header§
 */

#include "StringListSelectAndReorderSet.h"
#include <qbitmap.h>
// the bitmaps
#include "arrow_left.xbm"
#include "arrow_right.xbm"
#include "arrow_up.xbm"
#include "arrow_down.xbm"
extern "C" {
#include <assert.h>
	   }
#include <kapp.h>

const int StringListSelectAndReorderSet::Grid=3;

StringListSelectAndReorderSet::StringListSelectAndReorderSet(QWidget* parent, 
							     const char* name)
  : StringListSelectAndReorderSetData(parent, name)
{
  // ############################################################################
  // ----- create the bitmaps:
  QBitmap previous(16, 16, (unsigned char*)arrow_left_bits, true);
  QBitmap next(16, 16, (unsigned char*)arrow_right_bits, true);  
  QBitmap up(16, 16, (unsigned char*)uparrow_bits, true);
  QBitmap down(16, 16, (unsigned char*)arrow_down_bits, true);
  // ----- set the button faces:
  buttonSelect->setPixmap(next);
  buttonUnselect->setPixmap(previous);
  buttonUp->setPixmap(up);
  buttonDown->setPixmap(down);
  labelPossible->setText(i18n("Possible values:"));
  labelSelected->setText(i18n("Selected values:"));
  // ----- setup the dialog:
  connect(lbSelected, SIGNAL(highlighted(int)), SLOT(enableButtons(int)));
  enableButtons(0);
  // ----- 
  setMinimumSize(sizeHint());
  // ############################################################################
}


StringListSelectAndReorderSet::~StringListSelectAndReorderSet
()
{
  // ############################################################################
  // ############################################################################
}

void StringListSelectAndReorderSet::resizeEvent(QResizeEvent*)
{
  // ############################################################################
  const int ButtonSize=22,
    LabelHeight=labelPossible->sizeHint().height();
  int lbHeight, x, y, tempx;
  // ----- calculate listbox width and height:
  tempx=(width()-2*ButtonSize-3*Grid)/2;
  lbHeight=height()-Grid-LabelHeight;
  if(tempx<10 || lbHeight<10) 
    { //       do nothing
      return;
    }
  x=0; // current placement x
  y=0; // current placement y
  // ----- place the labels:
  labelPossible->setGeometry(x, y, tempx, LabelHeight);
  labelSelected->setGeometry(x+tempx+2*Grid+ButtonSize, y, tempx, LabelHeight);
  y+=LabelHeight+Grid;
  // ----- place the listboxes and the buttons:
  lbPossible->setGeometry(x, y, tempx, lbHeight);
  buttonSelect->setGeometry(x+Grid+tempx, y, ButtonSize, ButtonSize);
  buttonUnselect->setGeometry
    (x+Grid+tempx, y+ButtonSize, ButtonSize, ButtonSize);
  lbSelected->setGeometry(x+2*Grid+tempx+ButtonSize, y, tempx, lbHeight);
  buttonUp->setGeometry(x+3*Grid+2*tempx+ButtonSize, y, ButtonSize, ButtonSize);
  buttonDown->setGeometry
    (x+3*Grid+2*tempx+ButtonSize, y+ButtonSize, ButtonSize, ButtonSize);
  // ############################################################################
}

bool StringListSelectAndReorderSet::setValues(const list<string>& values)
{ 
  // ############################################################################
  list<string>::const_iterator pos;
  int i=0;
  // ----- copy values:
  possible.erase(possible.begin(), possible.end());
  selected.erase(selected.begin(), selected.end());
  lbPossible->clear();
  lbSelected->clear();
  original=values;
  assert(values.size()==original.size());
  // -----
  for(pos=values.begin(); pos!=values.end(); pos++)
    {
      lbPossible->insertItem((*pos).c_str());
      possible.push_back(i++);
    }
  assert(values.size()==possible.size());
  enableButtons(0);
  return true;
  // ############################################################################
}

bool StringListSelectAndReorderSet::setValues(const QStrList& values)
{ 
  // ############################################################################
  int index;
  // -----
  possible.erase(possible.begin(), possible.end());
  selected.erase(selected.begin(), selected.end());
  lbPossible->clear();
  lbSelected->clear();
  lbPossible->insertStrList(&values);
  assert(values.count()==lbPossible->count());
  // -----
  for(index=0; (unsigned)index<values.count(); index++)
    {
      possible.push_back(index);
    }
  assert(values.count()==possible.size()); 
  enableButtons(0);
  return false; 
  // ############################################################################
}

bool StringListSelectAndReorderSet::getSelection(list<int>& indizes)
{ 
  // ############################################################################
  if(selected.empty())
    {
      return false;
    } else {
      indizes.erase(indizes.begin(), indizes.end());
      indizes=selected;
      assert(indizes.size()==selected.size());
      return true;
    }
  // ############################################################################
}

bool StringListSelectAndReorderSet::getSelection(QList<int>& indizes)
{ 
  // ############################################################################
  list<int>::iterator pos;
  // -----
  if(selected.empty())
    {
      return false;
    } else {
      indizes.clear();
      for(pos=selected.begin(); pos!=selected.end(); pos++)
	{
	  indizes.append(&(*pos));
	}
      assert(indizes.count()==selected.size());
      return true;
    }
  // ############################################################################
}

bool StringListSelectAndReorderSet::getSelection(list<string>& values)
{
  // ############################################################################
  list<int>::iterator pos;
  list<string>::iterator text;
  // -----
  if(selected.empty())
    {
      return false; 
    } else {
      values.erase(values.begin(), values.end());
      for(pos=selected.begin(); pos!=selected.end(); pos++)
	{
	  text=original.begin();
	  assert((unsigned)(*pos)<original.size());
	  advance(text, *pos);
	  values.push_back(*text);
	}
      assert(values.size()==selected.size());
      return true;
    }
  // ############################################################################
}

bool StringListSelectAndReorderSet::getSelection(QStrList& values)
{ 
  // ############################################################################
  list<int>::iterator pos;
  list<string>::iterator text;
  // -----
  if(selected.empty())
    {
      return false; 
    } else {
      values.clear();
      for(pos=selected.begin(); pos!=selected.end(); pos++)
	{
	  text=original.begin();
	  assert((unsigned)(*pos)<original.size());
	  advance(text, *pos);
	  values.append((*text).c_str());
	}
      assert(values.count()==selected.size());
      return true;
    }
  // ############################################################################
}

bool StringListSelectAndReorderSet::select(int index)
{
  // ############################################################################
  list<int>::iterator pos;
  int number=-1;
  // -----
  if(index<0 || (unsigned)index>=original.size())
    {
      return false; 
    } else {
      for(pos=possible.begin(); pos!=possible.end(); pos++)
	{
	  ++number;
	  if(*pos==index) break;
	}
      if(pos!=possible.end())
	{
	  return selectItem(number);
	} else {
	  return false;
	}
    }
  enableButtons(0);
  // ############################################################################
}

bool StringListSelectAndReorderSet::select(int* indizes, int no)
{ 
  // ############################################################################
  bool error=false;
  int index;
  // -----
  for(index=0; index<no; index++)
    {
      if(!select(indizes[index]))
	{
	  error=true;
	}
    }
  enableButtons(0);
  return !error;
  // ############################################################################
}
bool StringListSelectAndReorderSet::select(const list<int>& indizes)
{
  // ############################################################################
  list<int>::const_iterator pos;
  bool error=false;
  // -----
  for(pos=indizes.begin(); pos!=indizes.end(); pos++)
    {
      if(!select(*pos))
	{
	  error=true;
	}
    }
  enableButtons(0);
  return !error;
  // ############################################################################
}

bool StringListSelectAndReorderSet::select(const QList<int>& indizes)
{ 
  // ############################################################################
  bool error=false;
  int index;
  // -----
  for(index=0; (unsigned)index<indizes.count(); index++)
    {
      if(!select(*((QList<int>&)indizes).at(index)))
	{
	  error=true;
	}
    }
  enableButtons(0);
  return !error;
  // ############################################################################
}

bool StringListSelectAndReorderSet::selectItem(int index)
{
  // ############################################################################
  string text;
  int temp;
  list<int>::iterator pos=possible.begin();
  // -----
  assert(lbPossible->count()==possible.size());
  assert(lbSelected->count()==(unsigned)selected.size());
  if(index<0 || (unsigned)index>=lbPossible->count())
    {
      return false;
    }
  // -----
  text=lbPossible->text(index);
  lbPossible->removeItem(index);
  advance(pos, index);
  temp=*pos;
  possible.erase(pos);
  lbSelected->insertItem(text.c_str());
  selected.push_back(temp);
  // -----
  assert(lbPossible->count()==possible.size());
  assert(lbSelected->count()==selected.size());
  enableButtons(0);
  return true;
  // ############################################################################
}

bool StringListSelectAndReorderSet::unselectItem(int index)
{
  // ############################################################################
  int temp;
  string text;
  list<int>::iterator pos=selected.begin();
  // -----
  assert(lbPossible->count()==possible.size());
  assert(lbSelected->count()==selected.size());
  if(index<0 || (unsigned)index>=lbSelected->count())
    {
      return false;
    }  
  // -----
  text=lbSelected->text(index);
  lbSelected->removeItem(index);
  advance(pos, index);
  temp=*pos;
  selected.erase(pos);
  lbPossible->insertItem(text.c_str());
  possible.push_back(temp);
  // -----
  assert(lbPossible->count()==possible.size());
  assert(lbSelected->count()==selected.size());
  enableButtons(0);
  return true;
  // ############################################################################
}

void StringListSelectAndReorderSet::selectPressed()
{
  // ############################################################################
  int index;
  // -----
  if(lbPossible->count()==0)
    { //       listbox is empty
      qApp->beep();
      return;
    }
  index=lbPossible->currentItem();
  if(index==-1)
    { //       nothing selected
      qApp->beep();
      return;
    }
  assert(index>=0 && (unsigned)index<lbPossible->count());
  if(!selectItem(index))
    {
      qApp->beep();
      return;
    }
  // ############################################################################
}

void StringListSelectAndReorderSet::unselectPressed()
{
  // ############################################################################
  int index;
  // -----
  if(lbSelected->count()==0)
    { //       listbox is empty
      qApp->beep();
      return;
    }
  index=lbSelected->currentItem();
  if(index==-1)
    { //       nothing selected
      qApp->beep();
      return;
    }
  assert(index>=0 && (unsigned)index<lbSelected->count());
  if(!unselectItem(index))
    {
      qApp->beep();
      return;
    }
  // ############################################################################
}

void StringListSelectAndReorderSet::up()
{
  // ############################################################################
  int index, temp;
  string text;
  list<int>::iterator pos;
  // -----
  if(lbSelected->count()==0)
    {
      return;
    }
  index=lbSelected->currentItem();
  if(index==-1)
    {
      qApp->beep();
      return;
    }
  assert(index>=0 && (unsigned)index<lbSelected->count());
  if(index==0)
    { //       already first element
      return;
    }
  // ----- get text from list box
  text=lbSelected->text(index);
  // ----- move position in list box:
  lbSelected->removeItem(index);
  lbSelected->insertItem(text.c_str(), index-1);
  // ----- move position in index list:
  pos=selected.begin();
  advance(pos, index);
  temp=*pos;
  selected.erase(pos);
  pos=selected.begin();
  advance(pos, index-1);
  selected.insert(pos, temp);
  // ----- keep the same current item:
  lbSelected->setCurrentItem(index-1);
  enableButtons(0);
  // ############################################################################
}

void StringListSelectAndReorderSet::down()
{
  // ############################################################################
  int index, temp;
  string text;
  list<int>::iterator pos;
  // -----
  if(lbSelected->count()==0)
    { //       no entries in listbox
      return;
    }
  index=lbSelected->currentItem();
  if(index==-1)
    { //       nothing selected
      qApp->beep();
      return;
    }
  assert(index>=0 && (unsigned)index<lbSelected->count());
  if((unsigned)index==lbSelected->count()-1)
    { //       already last element
      return;
    }
  // ----- get description from list box:
  text=lbSelected->text(index);
  // ----- move position in list box:
  lbSelected->removeItem(index);
  lbSelected->insertItem(text.c_str(), index+1);
  // ----- move position in index list:
  pos=selected.begin();
  advance(pos, index);
  temp=*pos;
  selected.erase(pos);
  pos=selected.begin();
  advance(pos, index+1);
  selected.insert(pos, temp);
  // ----- keep the same current item:
  lbSelected->setCurrentItem(index+1);
  enableButtons(0);
  // ############################################################################
}

void StringListSelectAndReorderSet::enableButtons(int) 
{ 
  // ############################################################################
  if(lbPossible->count()==0)
    {
      buttonSelect->setEnabled(false);
    } else {
      buttonSelect->setEnabled(true);
    }
  if(lbSelected->count()==0)
    {
      buttonUnselect->setEnabled(false);
    } else {
      buttonUnselect->setEnabled(true);
    }
  if(lbSelected->currentItem()==0 || lbSelected->count()<2)
    {
      buttonUp->setEnabled(false);
    } else {
      buttonUp->setEnabled(true);
    }
  if((unsigned)lbSelected->currentItem()
     ==lbSelected->count()-1 || lbSelected->count()<2)
    {
      buttonDown->setEnabled(false);
    } else {
      buttonDown->setEnabled(true);
    }
  // ############################################################################
}

QSize StringListSelectAndReorderSet::sizeHint() const
{
  // ############################################################################
  const int ButtonSize=22;
  int x;
  // -----
  x=lbPossible->fontMetrics().width("This is a very very long string");
  return QSize
    (/* x */ 
     3*Grid+2*ButtonSize
     +2*QMAX(lbPossible->sizeHint().width(), x+2*lbPossible->frameWidth()),
     /* y */ 
     Grid+QMAX(lbPossible->sizeHint().height(),
	       7*lbPossible->itemHeight()+2*lbPossible->frameWidth()));
  // ############################################################################
}

// ----- definition of the dialog class:

StringListSAndRSetDialog::StringListSAndRSetDialog
(QWidget* parent, const char* text, bool modal)
  : DialogBase(parent, text, modal),
    sar(new StringListSelectAndReorderSet(this)),
    sizeIsFixed(false)
{
  // ############################################################################
  // ----- manage subwidgets:
  // buttonOK->setText(i18n("OK"));
  // buttonCancel->setText(i18n("Cancel"));
  enableButtonApply(false);
  setMainWidget(sar);
  // ----- set up geometry:
  resize(minimumSize());
  // ############################################################################
}

void StringListSAndRSetDialog::fixSize(bool state)
{
  // ############################################################################
  if(state!=sizeIsFixed)
    {
      sizeIsFixed=state;
      initializeGeometry();
      if(state==true)
	{
	  setFixedSize(minimumSize());
	}
    }
  // ############################################################################
}

bool StringListSAndRSetDialog::isSizeFixed()
{
  // ############################################################################
  return sizeIsFixed;
  // ############################################################################
}

StringListSelectAndReorderSet* StringListSAndRSetDialog::selector() 
{ 
  // ############################################################################
  return sar; 
  // ############################################################################
}

// #############################################################################
// MOC OUTPUT FILES:
#include "StringListSelectAndReorderSet.h"
#include "StringListSelectAndReorderSetData.h"
// #############################################################################

