/*-*- C++ -*-
 **********************************************************************

	--- Qt Architect generated file ---

	File: StringListDialog.h
	Last generated: Sun Apr 12 00:28:01 1998

*********************************************************************/

#ifndef StringListDialog_included
#define StringListDialog_included

#include <stl_headers.h>
#include "StringListDialogData.h"

class StringListDialog : public StringListDialogData
{
  Q_OBJECT
public:
  /** The constructor creates a dialog, nothing special.
    */
  StringListDialog(QWidget* parent=0, const char* name=0);
  virtual ~StringListDialog();
  /** {\tt set(..)} sets the contents of the stringlist
    * listbox to the strings in the list.
    */
  void set(const list<string>&);
  /** {\tt get(..)} returns the possibly changed contents
    * of the listbox.
    */
  list<string> get();
  static const int ButtonSize;
protected:
  void initializeGeometry();
  void addString();
  void upPressed();
  void downPressed();
  void deletePressed();
};
#endif // StringListDialog_included
