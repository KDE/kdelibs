/* -*- C++ -*-
 * §Header§
 */

#include "StringListSelectSetDialog.h"
#include <qlistbox.h>
#include <kapp.h>

extern "C" {
#include <assert.h>
	   }

StringListSelectSetDialog::StringListSelectSetDialog
(QWidget* parent, const char* name)
  : DialogBase(parent, name)
{
  // ############################################################################
  lbStrings=new QListBox(this);
  lbStrings->setMultiSelection(true);
  setMainWidget(lbStrings);
  enableButtonApply(false);
  initializeGeometry();
  resize(minimumSize());
  // ############################################################################
}

StringListSelectSetDialog::~StringListSelectSetDialog()
{
  // ############################################################################  
  // ############################################################################  
}

void StringListSelectSetDialog::initializeGeometry()
{
  // ############################################################################  
  QSize size=lbStrings->sizeHint();
  // -----
  if(size.height()<=10)
    { 
      size.setHeight(5*lbStrings->itemHeight()+2*lbStrings->frameWidth());
    }
  if(size.width()<=10) 
    {
      size.setWidth(lbStrings->fontMetrics().width("This is a long string."));
    }
  lbStrings->setMinimumSize(size);
  DialogBase::initializeGeometry();
  // ############################################################################
}

bool StringListSelectSetDialog::setValues(const list<string>& values)
{
  // ############################################################################
  list<string>::const_iterator pos;
  // -----
  lbStrings->clear();
  for(pos=values.begin(); pos!=values.end(); pos++)
    {
      lbStrings->insertItem((*pos).c_str());
    }
  assert(lbStrings->count()==values.size());
  return true;
  // ############################################################################
}

bool StringListSelectSetDialog::setValues(const QStrList& values)
{
  // ############################################################################
  lbStrings->clear();
  lbStrings->insertStrList(&values);
  assert(lbStrings->count()==values.count());
  return true;
  // ############################################################################
}

bool StringListSelectSetDialog::getSelection(list<int>& s)
{
  // ############################################################################
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
  // ############################################################################
}

bool StringListSelectSetDialog::getSelection(QList<int>& s)
{
  // ############################################################################
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
  // ############################################################################
}

bool StringListSelectSetDialog::getSelection(list<string>& s)
{
  // ############################################################################
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
  // ############################################################################
}

bool StringListSelectSetDialog::getSelection(QStrList& s)
{
  // ############################################################################
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
  // ############################################################################
}

// #############################################################################
// MOC OUTPUT FILES:
#include "StringListSelectSetDialog.h"
// #############################################################################


