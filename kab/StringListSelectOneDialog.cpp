/* -*- C++ -*-
 * §Header§
 */

#include "StringListSelectOneDialog.h"
#include <qcombobox.h>
#include <kapp.h>

extern "C" {
#include <assert.h>
	   }

#define Inherited StringListSelectOneDialogData

StringListSelectOneDialog::StringListSelectOneDialog
(QWidget* parent,
 const char* name)
  : Inherited( parent, name )
{
  // ########################################################
  buttonOK->setDefault(true);
  buttonOK->setFocus();
  initializeGeometry();
  buttonOK->setText(i18n("OK"));
  buttonCancel->setText(i18n("Cancel"));
  // -----
  connect(kapp, SIGNAL(appearanceChanged()),
	  SLOT(initializeGeometry()));
  // ########################################################    
}


StringListSelectOneDialog::~StringListSelectOneDialog()
{
  // ########################################################  
  // ########################################################  
}

void StringListSelectOneDialog::initializeGeometry()
{
  // ########################################################
  const int Grid=5;
  const int HLWidth=QMAX(labelHeadline->sizeHint().width(),
			 200),
    HLHeight=labelHeadline->sizeHint().height(),
    ComboWidth=comboStrings->sizeHint().width(),
    ComboHeight=comboStrings->sizeHint().height(),
    ButtonHeight=buttonOK->sizeHint().height();
  int buttonWidth=QMAX(buttonOK->sizeHint().width(),
		       buttonCancel->sizeHint().width());
  int tempx, tempy, cx, cy, x;
  // ----- determine combo and label width and height:
  tempx=QMAX(ComboWidth, HLWidth);
  tempy=ComboHeight+Grid+HLHeight;
  x=tempx;
  // ----- determine inner frame width and height:
  tempx=tempx+2*frameInner->frameWidth()+2*Grid;
  if(tempx<2*buttonWidth)
    { //       enlarge inner frame:
      tempx=2*buttonWidth;
    } else { 
      //       enlarge buttons:
      buttonWidth=tempx/2;
    }
  tempy=tempy+2*frameInner->frameWidth()+2*Grid;
  // ----- determine best dialog size:
  cx=2*Grid+2*frameBase->frameWidth()+tempx;
  cy=3*Grid+2*frameInner->frameWidth()+tempy+ButtonHeight;
  // ----- now set the geometry of the dialog:
  setFixedSize(cx, cy);
  frameBase->setGeometry(0, 0, cx, cy);
  frameInner->setGeometry
    (frameBase->frameWidth()+Grid, 
     frameBase->frameWidth()+Grid,
     tempx, tempy);
  labelHeadline->setGeometry
    (frameInner->x()+Grid+frameInner->frameWidth(), 
     frameInner->y()+Grid+frameInner->frameWidth(), 
     x, HLHeight);
  comboStrings->setGeometry
    (labelHeadline->x(), labelHeadline->y()+HLHeight+Grid,
     x, ComboHeight);
  buttonOK->setGeometry
    (frameInner->x(), 
     frameInner->y()+frameInner->height()+Grid,
     buttonWidth, ButtonHeight);
  buttonCancel->setGeometry
    (frameInner->x()+buttonWidth, 
     frameInner->y()+frameInner->height()+Grid,
     buttonWidth, ButtonHeight);     
  // ########################################################  
}

bool StringListSelectOneDialog::setValues
(const list<string>& strings)
{
  assert(comboStrings!=0);
  // ########################################################  
  list<string>::const_iterator pos;
  // -----
  comboStrings->clear();
  for(pos=strings.begin(); pos!=strings.end(); pos++)
    {
      comboStrings->insertItem((*pos).c_str());
    }
  assert((unsigned)comboStrings->count()==strings.size());
  return true;
  // ########################################################  
}

bool StringListSelectOneDialog::setValues
(const QStrList& strings)
{
  assert(comboStrings!=0);
  // ########################################################  
  comboStrings->clear();
  comboStrings->insertStrList(&strings, 0);
  assert((unsigned)comboStrings->count()==strings.count());
  return true;
  // ########################################################  
}

bool StringListSelectOneDialog::getValues
(list<string>& strings)
{
  assert(comboStrings!=0);
  // ########################################################  
  int index;
  // -----
  strings.erase(strings.begin(), strings.end());
  assert(strings.empty());
  for(index=0; index<comboStrings->count(); index++)
    {
      strings.push_back(comboStrings->text(index));
    }
  assert(strings.size()==(unsigned)comboStrings->count());
  return true;
  // ########################################################  
}

bool StringListSelectOneDialog::getValues(QStrList& strings)
{
  assert(comboStrings!=0);
  // ########################################################  
  int index;
  // -----
  strings.clear();
  assert(strings.count()==0);
  for(index=0; index<comboStrings->count(); index++)
    {
      strings.insert(index, comboStrings->text(index));
    }
  assert(strings.count()==(unsigned)comboStrings->count());
  return true;
  // ########################################################  
}

bool StringListSelectOneDialog::getSelection(int& index)
{  
  // ########################################################  
  int i=comboStrings->currentItem();
  // -----
  if(i<0 || i>=comboStrings->count())
    { //      nothing is selected:
      return false;
    } else {
      index=i;
      return true;
    }
  // ########################################################  
}

bool StringListSelectOneDialog::getSelection(string& text)
{
  // ########################################################  
  text=comboStrings->currentText();
  return true;
  // ########################################################  
}

bool StringListSelectOneDialog::getSelection(QString& text)
{
  // ########################################################  
  text=comboStrings->currentText();
  return true;
  // ########################################################  
}

void StringListSelectOneDialog::setHeadline(const char* hl)
{
  // ########################################################  
  labelHeadline->setText(hl);
  // ########################################################  
}

void StringListSelectOneDialog::setHeadline(const string& hl)
{
  // ########################################################  
  setHeadline(hl.c_str());
  // ########################################################  
}

void StringListSelectOneDialog::setHeadline(const QString& s)
{
  // ########################################################  
  setHeadline((const char*)s);
  // ########################################################  
}

void StringListSelectOneDialog::editable(bool state)
{
  // ########################################################  
  if(state)
    {
      comboStrings->setInsertionPolicy(QComboBox::AtBottom);
    } else {
      comboStrings->setInsertionPolicy
	(QComboBox::NoInsertion);
    }
  // ########################################################  
}  

bool StringListSelectOneDialog::isEditable()
{
  // ########################################################  
  if(comboStrings->insertionPolicy()==QComboBox::NoInsertion)
    {
      return false;
    } else {
      return true;
    }
  // ########################################################    
}

// -----
#include "StringListSelectOneDialog.moc"
#include "StringListSelectOneDialogData.moc"
