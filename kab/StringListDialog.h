/* -*- C++ -*-
 * A dialog for editing string lists. Although this is usually the job 
 * of a combobox, this way is more comfortable for the user.
 * Declaration
 * the KDE addressbook
 * copyright:  (C) Mirko Sucker, 1998
 * license:    GNU Public License, Version 2
 * mail to:    Mirko Sucker <mirko.sucker@unibw-hamburg.de>
 * requires:   C++-compiler, STL, string class, Qt > 1.40
 *             NANA (for debugging)
 * $Revision$
 */

#ifndef StringListDialog_included
#define StringListDialog_included

#include <stl_headers.h>
#include "StringListDialogData.h"

class StringListDialog : public StringListDialogData
{
  // ############################################################################
  Q_OBJECT
  // ----------------------------------------------------------------------------
public:
  /** The constructor creates a dialog, nothing special.
    */
  StringListDialog(QWidget* parent=0, const char* name=0);
  virtual ~StringListDialog();
  /** {\tt set(..)} sets the contents of the stringlist
    * listbox to the strings in the list.
    */
  void set(const list<string>&);
  /** {\tt get(..)} returns the possibly changed contents of the listbox.
    */
  list<string> get();
  static const int ButtonSize;
  // ----------------------------------------------------------------------------
protected:
  void initializeGeometry();
  void addString();
  void upPressed();
  void downPressed();
  void deletePressed();
  // ############################################################################
};

#endif // StringListDialog_included




