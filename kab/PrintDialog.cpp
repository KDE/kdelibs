/*-*- C++ -*-
 * A dialog to configure printing.
 * Deklaration
 *
 * the KDE addressbook
 * copyright:  (C) Mirko Sucker, 1998
 * license:    GNU Public License, Version 2
 * mail to:    Mirko Sucker <mirko.sucker@unibw-hamburg.de>
 * requires:   C++-compiler, STL, string class, Qt > 1.40
 *             NANA (for debugging)
 * $Revision$
 */

#include "PrintDialog.h"
#include "debug.h"

#define Inherited PrintDialogData

PrintDialog::PrintDialog(QWidget* parent, const char* name)
  : Inherited( parent, name )
{
  // ###########################################################################
  buttonOK->setDefault(true);
  buttonOK->setAutoDefault(true);
  buttonOK->setFocus();
  labelHeadline->setText(i18n("Headline text:"));
  labelRightFooter->setText(i18n("Right footer text:"));
  labelLeftFooter->setText(i18n("Left footer text:"));
  connect(kapp, SIGNAL(appearanceChanged()), SLOT(initializeGeometry()));
  initializeGeometry();
  // ###########################################################################
}


PrintDialog::~PrintDialog()
{
  // ###########################################################################
  // ###########################################################################
}

void PrintDialog::initializeGeometry()
{
  // ###########################################################################
  // CONSTANTS:
  const int Grid=5;
  const int LabelWidth=QMAX(QMAX(labelHeadline->sizeHint().width(), 
				 labelRightFooter->sizeHint().width()),
			    labelLeftFooter->sizeHint().width());
  const int LabelHeight=leHeadline->sizeHint().height();
  const int LabelLeft=Grid+frameBase->frameWidth();
  const int LineEditWidth=leHeadline
    ->fontMetrics().width("This should be long enough for a headline.");
  const int LineEditLeft=2*Grid+frameBase->frameWidth()+LabelWidth;
  const int LineEditHeight=LabelHeight;
  const int ButtonWidth=QMAX(buttonOK->sizeHint().width(),
			     buttonCancel->sizeHint().width());
  const int ButtonHeight=buttonOK->sizeHint().height();
  // VARIABLES:
  int cx, cy, y;
  // CODE:
  // ----- set geometry of base frame, labels and line-edits:
  y=frameBase->frameWidth()+Grid; // (x, y) is upper corner of top label
  labelHeadline->setGeometry(LabelLeft, y, LabelWidth, LabelHeight);
  leHeadline->setGeometry(LineEditLeft, y, LineEditWidth, LineEditHeight);
  y+=leHeadline->sizeHint().height()+Grid;
  labelRightFooter->setGeometry(LabelLeft, y, LabelWidth, LabelHeight);
  leRightFooter->setGeometry(LineEditLeft, y, LineEditWidth, LineEditHeight);
  y+=leHeadline->sizeHint().height()+Grid;
  labelLeftFooter->setGeometry(LabelLeft, y, LabelWidth, LabelHeight);
  leLeftFooter->setGeometry(LineEditLeft, y, LineEditWidth, LineEditHeight);
  cx=3*Grid+2*frameBase->frameWidth()+LabelWidth+LineEditWidth;
  cy=4*Grid+3*leHeadline->sizeHint().height()+2*frameBase->frameWidth();
  frameBase->setGeometry(0, 0, cx, cy);
  // ----- set geometry of the two buttons:
  cy+=Grid;
  buttonOK->setGeometry(0, cy, ButtonWidth, ButtonHeight);
  buttonCancel->setGeometry(cx-ButtonWidth, cy, ButtonWidth, ButtonHeight);
  // ----- fix dialog size:
  cy+=ButtonHeight;
  setFixedSize(cx, cy);
  // ###########################################################################
}

// #############################################################################
// MOC OUTPUT FILES:
#include "PrintDialog.moc"
#include "PrintDialogData.moc"
// #############################################################################






















