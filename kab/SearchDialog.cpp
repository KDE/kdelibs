/* -*- C++ -*-
 * This file implements the search dialog..
 * 
 * the KDE addressbook
 * copyright:  (C) Mirko Sucker, 1998
 * license:    GNU Public License, Version 2
 * mail to:    Mirko Sucker <mirko.sucker@unibw-hamburg.de>
 * requires:   C++-compiler, STL, string class,
 *             Nana for debugging
 * $Revision$
 */

#include "SearchDialog.h"
#include <kapp.h>

#include "debug.h"

#undef Inherited
#define Inherited SearchDialogData

const char* SearchDialog::Keys[]= {
  "title",
  "firstname",
  "additionalName",
  "namePrefix",
  "name",
  "fn",
  "address",
  "town",
  "org",
  "role",
  "orgUnit",
  "orgSubUnit",
  "email",
  "email2",
  "email3",
  "telephone",
  "fax",
  "modem",
  "URL",
  "comment"
  /* , "deliveryLabel" */ };
  

SearchDialog::SearchDialog
(
	QWidget* parent,
	const char* name
)
	:
	Inherited( parent, name )
{
  // ########################################################
  setCaption(i18n("Search entries"));
  buttonSearch->setText(i18n("Search"));
  buttonCancel->setText(i18n("Cancel"));
  labelContains->setText(i18n("contains"));

  const char* Descriptions[]= {
    i18n("The title"),
    i18n("The first name"),
    i18n("The additional name"),
    i18n("The name prefix"),
    i18n("The name"),
    i18n("The formatted name"),
    i18n("The address"),
    i18n("The town"),
    i18n("The organization"),
    i18n("The role"),
    i18n("The org unit"),
    i18n("The org subunit"),
    i18n("The email address"),
    i18n("The second email address"),
    i18n("The third email address"),
    i18n("The telephone number"),
    i18n("The fax number"),
    i18n("The modem number"),
    i18n("The homepage URL"),
    i18n("The comment")
    /*, i18n("The delivery label") */ };
  const int Size
    =sizeof(Descriptions)/sizeof(Descriptions[0]);
  int count;
  // ---------
  for(count=0; count<Size; count++)
    {
      comboSelector->insertItem(Descriptions[count]);
    }
  connect(comboSelector, SIGNAL(activated(int)),
	  SLOT(keySelected(int)));
  connect(leWhat, SIGNAL(textChanged(const char*)),
	  SLOT(valueChanged(const char*)));
  keySelected(0);
  // ----------
  initializeGeometry();
  // ########################################################
}


SearchDialog::~SearchDialog()
{
}

void SearchDialog::keySelected(int index)
{
  REQUIRE(index>=0 
	  && (unsigned)index<(sizeof(Keys)
			      /sizeof(Keys[0])));
  // ########################################################
  L("SearchDialog::keySelected: key %s selected.\n",
    Keys[index]);
  key=Keys[index];
  // ########################################################
}

void SearchDialog::valueChanged(const char* value)
{
  // ########################################################
  L("SearchDialog::valueChanged: new value %s.\n",
    value);
  data=value;
  // ########################################################
}

void SearchDialog::initializeGeometry()
{
  ID(bool GUARD=true);
  LG(GUARD, "SearchDialog::initializeGeometry: called.\n");
  CHECK(comboSelector->count()>0);
  // ########################################################
  // ----- we assume the entries in comboSelector are 
  //       already inserted and the text for labelContains 
  //       to be set
  const int Grid=5;
  const int LeWidth=
    leWhat->fontMetrics().width("This is a long string.");
  const int ComboWidth=comboSelector->sizeHint().width();
  const int ComboHeight=comboSelector->sizeHint().height();
  const int LabelWidth=labelContains->sizeHint().width()
    // workaround: width of non-standard characters is 
    // measured wrong, adding width of an "a" (in German, the
    // test contains an "ae" (same width)
    // this will disappear after this problems are fixed
    // WORK_TO_DO
    +labelContains->fontMetrics().width("a");
  int cx, cy=Grid, x, y;
  // ----- find width:
  cx=4*Grid+ComboWidth+LabelWidth+LeWidth;
  // ----- now set geometries:
  x=Grid; 
  comboSelector->setGeometry(x, cy, ComboWidth, ComboHeight);
  x+=ComboWidth+Grid;
  labelContains->setGeometry(x, cy, LabelWidth, ComboHeight);
  x+=LabelWidth+Grid;
  leWhat->setGeometry(x, cy, LeWidth, ComboHeight);
  cy+=comboSelector->sizeHint().height()+Grid;
  // ----- the horizontal line
  frameHorLine->setGeometry(Grid, cy, cx-2*Grid, 2*Grid);
  cy+=2*Grid;
  // ----- the buttons
  buttonSearch->sizeHint().width()
    >buttonCancel->sizeHint().width()
    ? x=buttonSearch->sizeHint().width()
    : x=buttonCancel->sizeHint().width();
  y=buttonSearch->sizeHint().height();
  buttonSearch->setGeometry(Grid, cy, x, y);
  buttonCancel->setGeometry(cx-Grid-x, cy, x, y);
  cy+=y+Grid;
  // ------
  setFixedSize(cx, cy);
  LG(GUARD, "SearchDialog::initializeGeometry: done.\n");
  // ########################################################
}

#include "SearchDialog.moc"
#include "SearchDialogData.moc"
