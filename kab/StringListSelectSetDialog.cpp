/* -*- C++ -*-
 * §Header§
 */

#include "StringListSelectSetDialog.h"
#include <qcombo.h>
#include <kapp.h>

extern "C" {
#include <assert.h>
	   }

#define Inherited StringListSelectSetDialogData

StringListSelectSetDialog::StringListSelectSetDialog
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

StringListSelectSetDialog::~StringListSelectSetDialog()
{
  // ########################################################  
  // ########################################################  
}

void StringListSelectSetDialog::initializeGeometry()
{
  // ########################################################  
  const int Grid=5;
  const int     ButtonHeight=buttonOK->sizeHint().height(),
    ButtonWidth=QMAX(buttonOK->sizeHint().width(),
		 buttonCancel->sizeHint().width()),
    LBWidth=QMAX(lbStrings->sizeHint().width(),
		 3*ButtonWidth),
    LBHeight=QMAX(lbStrings->sizeHint().height(),
		  7*lbStrings->itemHeight()
		  +2*lbStrings->frameWidth());
  int cx, cy;
  // ----- determine preferred height and width:
  cx=LBWidth+2*Grid+2*frameBase->frameWidth();
  cy=LBHeight+3*Grid+ButtonHeight+2*frameBase->frameWidth();
  setFixedSize(cx, cy);
  // ----- set subwidget geometries:
  frameBase->setGeometry(0, 0, cx, cy);
  lbStrings->setGeometry
    (frameBase->frameWidth()+Grid, 
     frameBase->frameWidth()+Grid, 
     cx-2*(Grid+frameBase->frameWidth()),
     cy-3*Grid-2*frameBase->frameWidth()-ButtonHeight);
  buttonOK->setGeometry
    (lbStrings->x(), 
     cy-Grid-frameBase->frameWidth()-ButtonHeight,
     ButtonWidth, ButtonHeight);
  buttonCancel->setGeometry
    (lbStrings->x()+lbStrings->width()-ButtonWidth, 
     cy-Grid-frameBase->frameWidth()-ButtonHeight,
     ButtonWidth, ButtonHeight);
  // ########################################################    
}

bool StringListSelectSetDialog::setValues
(const list<string>& values)
{
  // ########################################################    
  list<string>::const_iterator pos;
  // -----
  lbStrings->clear();
  for(pos=values.begin(); pos!=values.end(); pos++)
    {
      lbStrings->insertItem((*pos).c_str());
    }
  assert(lbStrings->count()==values.size());
  return true;
  // ########################################################    
}

bool StringListSelectSetDialog::setValues
(const QStrList& values)
{
  // ########################################################    
  lbStrings->clear();
  lbStrings->insertStrList(&values);
  assert(lbStrings->count()==values.count());
  return true;
  // ########################################################    
}

bool StringListSelectSetDialog::getSelection(list<int>& s)
{
  // ########################################################    
  unsigned int index;
  // -----
  for(index=0; index<lbStrings->count(); index++)
    {
      if(lbStrings->isSelected(index))
	{
	  s.push_back(index);
	}
    }
  if(s.size()==0)
    { //       nothing selected
      return false;
    } else {
      return true;
    }
  // ########################################################    
}

bool StringListSelectSetDialog::getSelection(QList<int>& s)
{
  // ########################################################    
  int index;
  // -----
  for(index=0; (unsigned)index<lbStrings->count(); index++)
    {
      if(lbStrings->isSelected(index))
	{
	  s.append(&index);
	}
    }
  if(s.isEmpty())
    { 
      return true;
    } else { //       nothing selected
      return false;
    }      
  // ########################################################    
}

bool StringListSelectSetDialog::getSelection(list<string>& s)
{
  // ########################################################    
  unsigned int index;
  // -----
  for(index=0; index<lbStrings->count(); index++)
    {
      if(lbStrings->isSelected(index))
	{
	  s.push_back(lbStrings->text(index));
	}
    }
  if(s.size()==0)
    { //       nothing selected
      return false;
    } else {
      return true;
    }  
  // ########################################################    
}

bool StringListSelectSetDialog::getSelection(QStrList& s)
{
  // ########################################################    
  unsigned int index;
  // -----
  for(index=0; index<lbStrings->count(); index++)
    {
      if(lbStrings->isSelected(index))
	{
	  s.append(lbStrings->text(index));
	}
    }
  if(s.isEmpty())
    { 
      return true;
    } else { //       nothing selected
      return false;
    }      
  // ########################################################    
}

#include "StringListSelectSetDialog.moc"
#include "StringListSelectSetDialogData.moc"
