/* -*- C++ -*-
 * This dialog allows the user to select one string out of a string list,
 * no matter whether it is a QStrLing or a STL-like list<string>.
 * The OK and Cancel buttons have their labels translated to the selected
 * locale.  There is a label above the QComboBox that contains the string
 * values,  usually showing a  description  of what the user should do in
 * the shown dialog. Its text can be set by the programmer.
 * The geometry management is done automatically, the dialog resizes when 
 * the KDE signal appearanceChanged() is received.
 * The dialog may be editable, allowing the user to enter a new string if
 * nothing appropriate is n the list, or not editable. 
 *
 * If the user adds a string in the editable state, the signal 
 *   void stringAdded(string);
 * is sent. With this you can keep track of the changes.
 *
 * §Header§
 */

#ifndef StringListSelectOneDialog_included
#define StringListSelectOneDialog_included

#include "StringListSelectOneDialogData.h"
#include "stl_headers.h"
#include <qstring.h>
#include <qstrlist.h> 


class StringListSelectOneDialog : public StringListSelectOneDialogData
{
  Q_OBJECT
public:
  StringListSelectOneDialog(QWidget* parent=0, const char* name=0);
  virtual ~StringListSelectOneDialog();
  // ----- methods for setting the strings
  //       (the setValues and getValues methods return false on errors):
  bool setValues(const list<string>& strings);
  bool setValues(const QStrList& strings);
  // ----- methods for retrieving the strings 
  //       (only useful in editable state):
  bool getValues(list<string>& strings);
  bool getValues(QStrList& strings);
  // ----- methods for retrieving the selection:
  bool getSelection(int& index);
  bool getSelection(string& text);
  bool getSelection(QString& text);
  // ----- methods for setting the headline text:
  void setHeadline(const string& headline);
  void setHeadline(const QString& headline);
  void setHeadline(const char* headline);
  // ----- method for switching "editable" state of the dialog:
  void editable(bool state);
  // ----- method for querying "editable" state of the dialog:
  bool isEditable();
protected slots:
  void initializeGeometry();
signals:
  void stringAdded(string); // not implemented!
  // void stringAdded(int); // not implemented!
};

#endif // StringListSelectOneDialog_included
