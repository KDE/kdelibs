/* A dialog to handle assertions raised in the code.
 *
 * the KDE addressbook
 * copyright:  (C) Mirko Sucker, 1998
 * license:    GNU Public License, Version 2
 * mail to:    Mirko Sucker <mirko.sucker@unibw-hamburg.de>
 * requires:   C++-compiler, STL, string class, Qt > 1.40
 *             NANA (for debugging)
 * $Revision$
 */

#ifndef AssertDialog_included
#define AssertDialog_included

#include <qradiobutton.h>
#include "stl_headers.h"
#include "AssertDialogData.h"

class AssertDialog : public AssertDialogData
{
  Q_OBJECT
public:
  AssertDialog(QWidget* parent=0, const char* name=0);
  virtual ~AssertDialog();
public slots:
  void initializeGeometry();
  void setFile(string);
  void setLine(string);
  void setCondition(string);
  // actions may be Ignore (0), Kill (1), Mail (2)
  void setAction(int i);
  int getAction();
protected:
  // the radio button:
  QRadioButton *buttonIgnore, *buttonKill, *buttonMail;
  int action;
  string file, line, condition;
  void setErrorText();
};

#endif // AssertDialog_included
