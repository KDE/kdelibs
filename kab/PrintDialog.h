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

#ifndef PrintDialog_included
#define PrintDialog_included

#include "PrintDialogData.h"
#include "stl_headers.h"
#include "debug.h"

/* This dialog asks the user for the headline and footers when printing the 
 * database.
 */

class PrintDialog : public PrintDialogData
{
  // ############################################################################
  Q_OBJECT
  // ----------------------------------------------------------------------------
public:
  PrintDialog(QWidget* parent = 0, const char* name = 0);
  virtual ~PrintDialog();
  string getHeadline() { return leHeadline->text(); }
  string getRightFooter() { return leRightFooter->text(); }
  string getLeftFooter() { return leLeftFooter->text(); }
  void setHeadline(const string& t) { leHeadline->setText(t.c_str()); }
  void setRightFooter(const string& t) { leRightFooter->setText(t.c_str()); }
  void setLeftFooter(const string& t) { leLeftFooter->setText(t.c_str()); }
  // ----------------------------------------------------------------------------
protected slots:
  void initializeGeometry();
  // ############################################################################
};

#endif // PrintDialog_included





