/* -*- C++ -*-
 * This file implements a dialog for selecting a date.
 * It uses KDatePicker.
 * 
 * the KDE addressbook.
 * copyright:  (C) Mirko Sucker, 1998
 * license:    GNU Public License, Version 2
 * mail to:    Mirko Sucker <mirko.sucker@unibw-hamburg.de>
 * requires:   C++-compiler, STL, string class,
 *             Nana for debugging
 * $Revision$
 */

#include "datepickerdialog.h"
#include <kdatepik.h>
#include <kapp.h>
#include <klocale.h>
#include <qpushbutton.h>
#include <qdialog.h>

DatePickerDialog::DatePickerDialog(QWidget* parent, const char* name)
  : QDialog(parent, name, TRUE)
{
  // ############################################################################
  datePicker=new KDatePicker(this);
  ok=new QPushButton(i18n("OK"), this);
  cancel=new QPushButton(i18n("Cancel"), this);
  dateLabel=new DateLabel(this);
  dateLabel->setAlignment(AlignHCenter | AlignVCenter);
  initializeGeometry();
  connect(datePicker, SIGNAL(dateSelected(QDate)), 
	  dateLabel, SLOT(setDate(QDate)));
  connect(cancel, SIGNAL(clicked()), SLOT(reject()));
  connect(ok, SIGNAL(clicked()), SLOT(accept()));
  setCaption(i18n("Select birthday"));
  // ############################################################################
}

void DatePickerDialog::initializeGeometry()
{
  // ############################################################################
  const int Grid=5;
  QSize size;
  int x=0, y=Grid, cx, cy;
  // -----
  size=datePicker->sizeHint();
  datePicker->setGeometry(Grid, y, size.width(), size.height());
  y+=size.height()+Grid;
  x+=2*Grid+size.width();
  // -----
  dateLabel->setGeometry(Grid, y, x-2*Grid, dateLabel->sizeHint().height());
  y+=dateLabel->sizeHint().height()+Grid;
  // -----
  cy=ok->sizeHint().height();
  ok->sizeHint().width()>cancel->sizeHint().width()
    ? cx=ok->sizeHint().width() : cx=cancel->sizeHint().width();
  ok->setGeometry(Grid, y, cx, cy);
  cancel->setGeometry(x-Grid-cx, y, cx, cy);
  y+=cy+Grid;
  // -----
  setFixedSize(x, y);
  // ############################################################################
}  

const QDate& DatePickerDialog::getDate()
{
  // ############################################################################
  return dateLabel->getDate();
  // ############################################################################
}

bool DatePickerDialog::setDate(const QDate& newdate)
{
  // ############################################################################
  if(newdate.isValid())
    {
      datePicker->setDate(newdate);
      dateLabel->setDate(newdate);
      return true;
    } else {
      return false;
    }
  // ############################################################################
}
DatePickerDialog::~DatePickerDialog()
{
  // ############################################################################
  // ############################################################################
}

DateLabel::DateLabel(QWidget* parent, const char* name, 
		     const QDate& dateToSet)
  : QLabel(parent, name),
    date(dateToSet),
    changeDate(false)
{
  // ############################################################################
  setAlignment(AlignVCenter | AlignCenter);
  setBackgroundColor("peachpuff");
  setFrameStyle(QFrame::Box | QFrame::Plain);
  setText(date.toString());
  // ############################################################################
}

void DateLabel::setDate(QDate newDate)
{
  // ############################################################################
  date=newDate;
  if(newDate.isValid())
    {
      setText(date.toString());
    } else {
      setText("");
    }
  emit(dateSelected(date));
  // ############################################################################
}

void DateLabel::setDate()
{
  // ############################################################################
  DatePickerDialog* datePicker=new DatePickerDialog(this);
  // -----
  datePicker->setDate(date);
  if(datePicker->exec())
    { // ok was pressed
      setDate(datePicker->getDate());
    }
  delete datePicker;
  // ############################################################################
}

void DateLabel::enableChangeDialog(bool state)
{
  // ############################################################################
  changeDate=state;
  // ############################################################################
}

void DateLabel::mouseDoubleClickEvent(QMouseEvent*)
{
  // ############################################################################
  if(changeDate)
    {
      setDate();
    }
  // ############################################################################
}

const QDate& DateLabel::getDate()
{
  // ############################################################################
  return date;
  // ############################################################################
}

// #############################################################################
// MOC OUTPUT FILES:
#include "datepickerdialog.moc"
// #############################################################################
