/* -*- C++ -*-
 * The new dialog to search the entries for patterns.
 * Declaration.
 *
 * the KDE addressbook
 * copyright:  (C) Mirko Sucker, 1998
 * license:    GNU Public License, Version 2
 * mail to:    Mirko Sucker <mirko.sucker@unibw-hamburg.de>
 * requires:   C++-compiler, STL, string class, Qt > 1.40
 *             NANA (for debugging)
 * $Revision$
 */

#ifndef SearchDialogMainWidget_included
#define SearchDialogMainWidget_included

#include "SearchDialogMainWidgetData.h"

class SearchDialogMainWidget : public SearchDialogMainWidgetData
{
  // ############################################################################
  Q_OBJECT
  // ----------------------------------------------------------------------------
public:
  SearchDialogMainWidget(QWidget* parent=0, const char* name=0);
  virtual ~SearchDialogMainWidget();
  // make all subwidgets public to the SearchDialog class:
  friend class SearchDialog;
  // ----------------------------------------------------------------------------
signals:
  // emitted from appearanceChanged signal handler:
  void sizeChanged();
  // ----------------------------------------------------------------------------
protected slots:
  void initializeGeometry();
  // ############################################################################
};

#endif // SearchDialogMainWidget_included


