/* -*- C++ -*-
 * The dialog for searching the database.
 * Declaration
 *
 * the KDE addressbook
 * copyright:  (C) Mirko Sucker, 1998
 * license:    GNU Public License, Version 2
 * mail to:    Mirko Sucker <mirko.sucker@unibw-hamburg.de>
 * requires:   C++-compiler, STL, string class, Qt > 1.40
 *             NANA (for debugging)
 * $Revision$
 */

#ifndef SearchDialog_included
#define SearchDialog_included

#include <dialogbase.h>
#include "SearchDialogMainWidget.h"
#include "stl_headers.h"

class SearchDialog : public DialogBase
{
  // ############################################################################
  Q_OBJECT
  // ----------------------------------------------------------------------------
public:
  SearchDialog(QWidget* parent=0, const char* name=0);
  virtual ~SearchDialog();
  string getKey() { return key; }
  string getValue() { return data; }
  // ----------------------------------------------------------------------------
protected:
  static const char* Keys[];
  // Search criteria (simple by now, only one):
  string key; // which field
  string data; // matches what?
  SearchDialogMainWidget* widget;
  // ----------------------------------------------------------------------------
public slots:
  void keySelected(int);
  void valueChanged(const QString&);
  // ############################################################################
};

#endif // SearchDialog_included


