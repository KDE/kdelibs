/* -*- C++ -*-
 */

#ifndef STRINGLISTEDITDIALOG_H
#define STRINGLISTEDITDIALOG_H

#include "stl_headers.h"
#include <qlist.h>
#include <qsize.h>
#include <kdialogbase.h>
#include <qstrlist.h>

class StringListEditWidget;

class StringListEditDialog
  : public KDialogBase
{
  // ############################################################################
  Q_OBJECT
  // ----------------------------------------------------------------------------
public:
    /** The constructor creates a dialog, nothing special.
    */
  StringListEditDialog(QWidget* parent=0, const char* name=0);
  virtual ~StringListEditDialog();
  /** {\tt set(..)} sets the contents of the stringlist
    * listbox to the strings in the list.
    */
  void setStrings(const list<string>& strings);
  void setStrings(const QStrList& strings);
  /** {\tt get(..)} returns the possibly changed contents of the listbox.
    */
  void getStrings(list<string>& result);
  void getStrings(QStrList& result);
protected:
  StringListEditWidget* widget;
  // ############################################################################
};

#endif // STRINGLISTEDITDIALOG_H

