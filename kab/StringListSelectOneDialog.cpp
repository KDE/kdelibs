/* -*- C++ -*-
 * §Header§
 */

#include "StringListSelectOneDialog.h" 
#include <qwidget.h>
#include <qcombobox.h>
#include <qlayout.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qcombobox.h>
#include <qframe.h>
#include <kapp.h>

extern "C" {
#include <assert.h>
	   }

StringListSelectOneDialog::StringListSelectOneDialog(QWidget* parent,
						     const char* name)
  : DialogBase(parent, name),
    sizeIsFixed(false)
{
  // ############################################################################
  widget=new QWidget(this);
  layout=new QVBoxLayout(widget);
  labelHeadline=new QLabel(widget);
  layout->addWidget(labelHeadline, 2);
  comboStrings=new QComboBox(widget);
  layout->addWidget(comboStrings, 3);
  setMainWidget(widget);
  enableButtonApply(false);
  // showButtonApply(false);
  // -----
  initializeGeometry();
  resize(minimumSize());
  // ############################################################################
}

QSize StringListSelectOneDialog::sizeHint() const
{
  // ############################################################################
  QSize size;
  int ulx, uly, lrx, lry, cx, cy;
  // -----
  getBorderWidths(ulx, uly, lrx, lry);
  cx=QMAX(comboStrings->sizeHint().width(), labelHeadline->sizeHint().width());
  cy=QMAX(comboStrings->sizeHint().height(), labelHeadline->sizeHint().height());
  cy=cy<<1; // *2
  if(cx>0) size.setWidth(cx+ulx+lrx);
  if(cy>0) size.setHeight(cy+uly+lry);
  // -----
  return size;
  // ############################################################################
}

StringListSelectOneDialog::~StringListSelectOneDialog()
{
  // ############################################################################
  // ############################################################################
}

void StringListSelectOneDialog::fixSize(bool state)
{
  // ############################################################################
  sizeIsFixed=state;
  initializeGeometry();
  // ############################################################################
}

bool StringListSelectOneDialog::isSizeFixed()
{
  // ############################################################################
  return sizeIsFixed;
  // ############################################################################
}

void StringListSelectOneDialog::initializeGeometry()
{
  // ############################################################################
  int cx, cy;
  // -----
  cx=QMAX(comboStrings->sizeHint().width(), labelHeadline->sizeHint().width());
  cy=QMAX(comboStrings->sizeHint().height(), labelHeadline->sizeHint().height());
  cy=cy<<1;
  widget->setMinimumSize(cx, cy);
  DialogBase::initializeGeometry();
  // ############################################################################
}

bool StringListSelectOneDialog::setValues
(const list<string>& strings)
{
  assert(comboStrings!=0);
  // ############################################################################
  list<string>::const_iterator pos;
  // -----
  comboStrings->clear();
  for(pos=strings.begin(); pos!=strings.end(); pos++)
    {
      comboStrings->insertItem((*pos).c_str());
    }
  assert((unsigned)comboStrings->count()==strings.size());
  return true;
  // ############################################################################
}

bool StringListSelectOneDialog::setValues
(const QStrList& strings)
{
  assert(comboStrings!=0);
  // ############################################################################
  comboStrings->clear();
  comboStrings->insertStrList(&strings, 0);
  assert((unsigned)comboStrings->count()==strings.count());
  return true;
  // ############################################################################
}

bool StringListSelectOneDialog::getValues
(list<string>& strings)
{
  assert(comboStrings!=0);
  // ############################################################################
  int index;
  // -----
  strings.erase(strings.begin(), strings.end());
  assert(strings.empty());
  for(index=0; index<comboStrings->count(); index++)
    {
      strings.push_back(comboStrings->text(index).ascii());
    }
  assert(strings.size()==(unsigned)comboStrings->count());
  return true;
  // ############################################################################
}

bool StringListSelectOneDialog::getValues(QStrList& strings)
{
  assert(comboStrings!=0);
  // ############################################################################
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
  // ############################################################################
}

bool StringListSelectOneDialog::getSelection(int& index)
{
  // ############################################################################
  int i=comboStrings->currentItem();
  // -----
  if(i<0 || i>=comboStrings->count())
    { //      nothing is selected:
      return false;
    } else {
      index=i;
      return true;
    }
  // ############################################################################
}

bool StringListSelectOneDialog::getSelection(string& text)
{
  // ############################################################################
  text=comboStrings->currentText();
  return true;
  // ############################################################################
}

bool StringListSelectOneDialog::getSelection(QString& text)
{
  // ############################################################################
  text=comboStrings->currentText();
  return true;
  // ############################################################################
}

void StringListSelectOneDialog::setHeadline(const char* hl)
{
  // ############################################################################
  labelHeadline->setText(hl);
  // ############################################################################
}

void StringListSelectOneDialog::setHeadline(const string& hl)
{
  // ############################################################################
  setHeadline(hl.c_str());
  // ############################################################################
}

void StringListSelectOneDialog::setHeadline(const QString& s)
{
  // ############################################################################
  setHeadline((const char*)s);
  // ############################################################################
}

void StringListSelectOneDialog::editable(bool state)
{
  // ############################################################################
  if(state)
    {
      comboStrings->setInsertionPolicy(QComboBox::AtBottom);
    } else {
      comboStrings->setInsertionPolicy(QComboBox::NoInsertion);
    }
  // ############################################################################
}

bool StringListSelectOneDialog::isEditable()
{
  // ############################################################################
  if(comboStrings->insertionPolicy()==QComboBox::NoInsertion)
    {
      return false;
    } else {
      return true;
    }
  // ############################################################################
}

// #############################################################################
// MOC OUTPUT FILES:
#include "StringListSelectOneDialog.moc"
#include "StringListSelectOneDialogData.moc"
// #############################################################################

