/* -*- C++ -*-
 * The new dialog to search the entries for patterns.
 * Implementation.
 *
 * the KDE addressbook
 * copyright:  (C) Mirko Sucker, 1998
 * license:    GNU Public License, Version 2
 * mail to:    Mirko Sucker <mirko.sucker@unibw-hamburg.de>
 * requires:   C++-compiler, STL, string class, Qt > 1.40
 *             NANA (for debugging)
 * $Revision$
 */

#include "SearchDialogMainWidget.h"
#include <qtooltip.h>
#include "debug.h" // includes kapp.h

SearchDialogMainWidget::SearchDialogMainWidget(QWidget* parent, const char* name)
  : SearchDialogMainWidgetData(parent, name)
{
  // ############################################################################
  labelMatches->setText(i18n("matches"));
  QToolTip::add(lePattern, i18n("Use shell wildcard patterns here"));
  QToolTip::add(comboSelector, i18n("Select field to match the pattern"));
  connect(kapp, SIGNAL(appearanceChanged()), SLOT(initializeGeometry()));
  initializeGeometry();
  // ############################################################################
}

SearchDialogMainWidget::~SearchDialogMainWidget()
{
  // ############################################################################
  // ############################################################################
}

void SearchDialogMainWidget::initializeGeometry()
{
  // ############################################################################
  int cx, cy;
  // ----- set minimum size:
  cx=QMAX(labelMatches->sizeHint().width(),
	  QMAX(comboSelector->sizeHint().width(),
	       lePattern->sizeHint().width()));
  cx=QMAX(cx, lePattern->fontMetrics().width("This is a very long string."));
  cy=QMAX(QMAX(labelMatches->sizeHint().height(),lePattern->sizeHint().height()),
	  comboSelector->sizeHint().height())*3;
  // cy=(int)(1.1*cy+0.5); // respect box layout stretch factors
  // -----
  setMinimumSize(cx, cy);
  emit(sizeChanged());
  // ############################################################################
}

// ##############################################################################
// MOC OUTPUT FILES:
#include "SearchDialogMainWidget.moc"
#include "SearchDialogMainWidgetData.moc"
// ##############################################################################
