#ifndef PrintDialog_included
#define PrintDialog_included
// #############################################################################
#include "PrintDialogData.h"
#include "stl_headers.h"
// #############################################################################
/* This dialog asks the user for the headline and footers when printing the 
 * database.
 */
class PrintDialog : public PrintDialogData
{
  Q_OBJECT
public:
  PrintDialog(QWidget* parent = 0, const char* name = 0);
  virtual ~PrintDialog();
  string getHeadline() { return leHeadline->text(); }
  string getRightFooter() { return leRightFooter->text(); }
  string getLeftFooter() { return leLeftFooter->text(); }
  void setHeadline(const string& t) { leHeadline->setText(t.c_str()); }
  void setRightFooter(const string& t) { leRightFooter->setText(t.c_str()); }
  void setLeftFooter(const string& t) { leLeftFooter->setText(t.c_str()); }
protected slots:
  void initializeGeometry();
};
// #############################################################################
#endif // PrintDialog_included
