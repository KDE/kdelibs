/*-*- C++ -*-
 *********************************************************************

	--- Qt Architect generated file ---

	File: SearchDialog.h
	Last generated: Wed Feb 25 17:48:48 1998

 *********************************************************************/

#ifndef SearchDialog_included
#define SearchDialog_included

#include "SearchDialogData.h"
#include <string>

class SearchDialog : public SearchDialogData
{
    Q_OBJECT

public:

    SearchDialog
    (
        QWidget* parent = NULL,
        const char* name = NULL
    );

    virtual ~SearchDialog();
  string getKey() { return key; }
  string getValue() { return data; }
protected:
  void initializeGeometry();
  static const char* Keys[];
  // Search criteria (simple by now, only one):
  string key; // which field
  string data; // contains what?
public slots:
  void keySelected(int);
  void valueChanged(const char*);
};
#endif // SearchDialog_included
