/* -*- C++ -*-
 * §Header§
 */

#include "StringListEditDialog.h"
#include "StringListEditWidget.h"
#include "debug.h"

StringListEditDialog::StringListEditDialog(QWidget* parent, const char* name)
  : DialogBase(parent, name)
{
  // ############################################################################
  enableButtonApply(false);
  widget=new StringListEditWidget(this);
  CHECK(widget!=0);
  setMainWidget(widget);
  // ----- set up geometry:
  resize(minimumSize());
  // ############################################################################
}

StringListEditDialog::~StringListEditDialog()
{
  // ############################################################################
  // ############################################################################
}

void StringListEditDialog::setStrings(const list<string>& strings)
{
  REQUIRE(widget!=0);
  // ############################################################################
  widget->setStrings(strings);
  // ############################################################################
}


void StringListEditDialog::setStrings(const QStrList& strings)
{
  REQUIRE(widget!=0);
  // ############################################################################
  widget->setStrings(strings);
  // ############################################################################
}

void StringListEditDialog::getStrings(list<string>& result)
{
  REQUIRE(widget!=0);
  // ############################################################################
  widget->getStrings(result);
  // ############################################################################
}

void StringListEditDialog::getStrings(QStrList& result)
{
  REQUIRE(widget!=0);
  // ############################################################################
  widget->getStrings(result);
  // ############################################################################
}

// #############################################################################
// MOC OUTPUT FILES:
#include "StringListEditDialog.h"
// #############################################################################
