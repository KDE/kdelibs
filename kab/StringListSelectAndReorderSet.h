/* -*- C++ -*-
 * This  dialog allows the user to select a  set of strings out of a string
 * list,  again providing interfaces for STL and Qt string lists.  The list
 * is displayed in a  listbox using  multiple selection,  and it does allow
 * reordering of the selected strings.
 * For convenience and  consistency,  the methods provided for querying the 
 * users selection remain the same as in 
 *   StringListSelectSetDialog.
 * Thus you can query the selected strings or their indizes in the original
 * string list. 
 * The COMPLETE list must be handed over to the dialog for determination of
 * the strings after reordering.
 *
 * The dialog is derived from a widget that can be used separately, it pro-
 * vides the basic string operations. 
 *
 * §Header§
 */

#ifndef StringListSelectAndReorderSet_included
#define StringListSelectAndReorderSet_included

#include "StringListSelectAndReorderSetData.h"
#include "stl_headers.h"
#include <qlist.h>
#include <qsize.h>
#include <qdialog.h>

class StringListSelectAndReorderSet // a widget!, see dialog class below
  : public StringListSelectAndReorderSetData
{
  Q_OBJECT
public:
  StringListSelectAndReorderSet(QWidget* parent=0, const char* name=0); 
  virtual ~StringListSelectAndReorderSet(); 
  // methods to set the strings:
  bool setValues(const list<string>&); 
  bool setValues(const QStrList&); 
  // methods to retrieve the selected indizes:
  bool getSelection(list<int>&); 
  bool getSelection(QList<int>&); 
  // methods to retrieve the selected strings:
  bool getSelection(list<string>&); 
  bool getSelection(QStrList&); 
  // methods for selecting specific values out of the ORIGINAL value list,
  // returns false if the item is already selected or the index is to large 
  // or below zero
  bool select(int index); // a single one
  // this methods return false if at least ONE item could not be selected
  bool select(int* indizes, int no); // indizes in an array
  bool select(const list<int>& indizes);
  bool select(const QList<int>& indizes);
  // the sizehint
  QSize sizeHint() const;
protected slots:
   // enable or disable buttons
  void enableButtons(int);
protected:
  // events:
  void resizeEvent(QResizeEvent*); 
  // selects an item in the "possible" listbox, the index is (thus) NOT the 
  // index in the original list!!
  bool selectItem(int index); 
  // dito, but unselect an item in the "selected" box
  bool unselectItem(int index); 
  // members:
  static const int Grid;
  list<string> original; // for temporary storing
  list<int> possible; // indizes of the elements in the "possible" listbox
  list<int> selected; // indizes of the elements in the "selected" listbox
  // overloaded slots from base class:
  void up();
  void down();
  void selectPressed();
  void unselectPressed();
};

// class name abbreviated...

class StringListSAndRSetDialog
  : public QDialog
{
  Q_OBJECT
public:
  StringListSAndRSetDialog
  (QWidget* par=0, const char* text=0, bool modal=true);
  // this method returns the "core" selector widget:
  // (use its methods)
  StringListSelectAndReorderSet* selector();
  /** You may allow the user to resize the dialog by calling
    * fixSize(false);
    * By default the dialog has a fixed size computed at startup.
    * With variable sizes, the dialog sets a minimum size that 
    * the window manager hopefully respects (kwm does).
    */
  // ----- method for switching "fixed size" state of the dialog:
  void fixSize(bool state); 
  // ----- method for querying "fixed size" state of the dialog:
  bool isSizeFixed();
protected slots:
  void initializeGeometry();
protected:
  StringListSelectAndReorderSet* sar;
  // events:
  void resizeEvent(QResizeEvent*);
  // the buttons:
  QPushButton *buttonOK, *buttonCancel;
  // others: 
  bool sizeIsFixed; 
};

#endif // StringListSelectAndReorderSet_included
