#ifndef DATEPICKERDIALOG_H_INCL
#define DATEPICKERDIALOG_H_INCL

#include <qdialog.h>
#include <qdatetm.h>
#include "widgets.h"
#include <qlabel.h>
#include <qcolor.h>
#include <qdatetm.h>

class QPushButton;

// KDatePicker widget is used in DateLabel
#include <kdatepik.h>


class DateLabel : public QLabel
{ // not necessary but may be enhanced
  Q_OBJECT
public:
  DateLabel(QWidget* parent=0, 
	    const char* name=0, 
	    const QDate& dateToSet=QDate::currentDate());
public slots:
  virtual void setDate(); // uses KDatePicker
  virtual void setDate(QDate);
  const QDate& getDate(); 
  void enableChangeDialog(bool state=true);
protected:
  QDate date;
  bool changeDate; // if true, doubleclick shows a dialog 
  // Events
  void mouseDoubleClickEvent(QMouseEvent*);
signals:
  void dateSelected(QDate);
};

class DatePickerDialog : public QDialog
{
 Q_OBJECT
public:
  DatePickerDialog(QWidget* parent=0, const char* name=0);
  ~DatePickerDialog();
  const QDate& getDate();
  bool setDate(const QDate&);
protected:
  DateLabel* dateLabel;
  QPushButton* ok;
  QPushButton* cancel;
  KDatePicker* datePicker;
  void initializeGeometry();
};

#endif // DATEPICKERDIALOG_H_INCL
