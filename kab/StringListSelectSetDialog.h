/* -*- C++ -*-
 * This dialog allows the user to select a  set of strings out of a string
 * list, again providing interfaces for STL and Qt string lists.  The list
 * is displayed in a listbox using multiple selection,  it DOES NOT  allow
 * reordering of the selected strings.  If you need a dialog that provides
 * selecting and reordering use StringListSelectAndReorderSetDialog. After
 * finishing the dialog,  it is possible to retrieve the selected  strings
 * directly or their indizes in a list of integers (the indizes are stored
 * in the same order as the items are selected).
 * The setSelection methods return  false on errors,  getSelection methods
 * return false if nothing is selected (please note the difference!).
 *
 * §Header§
 */

#ifndef StringListSelectSetDialog_included
#define StringListSelectSetDialog_included

#include "StringListSelectSetDialogData.h"
#include "stl_headers.h"
#include <qstring.h>
#include <qstrlist.h> 

class StringListSelectSetDialog : public StringListSelectSetDialogData
{
  Q_OBJECT
public:
  StringListSelectSetDialog(QWidget* parent=0, const char* name=0);
  virtual ~StringListSelectSetDialog();
  // methods to set the strings:
  bool setValues(const list<string>&);
  bool setValues(const QStrList&);
  // methods to retrieve the selected indizes:
  bool getSelection(list<int>&);
  bool getSelection(QList<int>&);
  // methods to retrieve the selected strings:
  bool getSelection(list<string>&);
  bool getSelection(QStrList&);
protected:
  void initializeGeometry();
};

#endif // StringListSelectSetDialog_included
