/* -*- C++ -*-
 */

#ifndef StringListEditWidget_included
#define StringListEditWidget_included

#include "StringListEditWidgetData.h"
#include <qstrlist.h>
#include "stl_headers.h"

class StringListEditWidget : public StringListEditWidgetData
{
  // ############################################################################
  Q_OBJECT
  // ----------------------------------------------------------------------------
public:
  StringListEditWidget(QWidget* parent=0, const char* name=0);
  virtual ~StringListEditWidget();
  QSize sizeHint() const;
  void setStrings(const list<string>& strings);
  void setStrings(const QStrList& strings);
  void getStrings(list<string>& result);
  void getStrings(QStrList& result);
  // ----------------------------------------------------------------------------
public slots:
  void initializeGeometry();
  // ----------------------------------------------------------------------------
protected slots:
  void itemSelected(int);
  // ----------------------------------------------------------------------------
protected:
  void resizeEvent(QResizeEvent*);
  void newItem(); // overloaded slot
  void itemChanged(const char*); // dito
  void upPressed();
  void downPressed();
  void deletePressed();
  void enableButtons();
  // ############################################################################
};

#endif // StringListEditWidget_included




