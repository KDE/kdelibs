/**
 * kdatetimedlg.cpp
 *
 * Copyright (c) 2001 George Staikos <staikos@kde.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published
 *  by the Free Software Foundation; either version 2.1 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "kdatetimedlg.h"

#include <QLayout>
#include <QLabel>
#include <QSpinBox>

#include <klocalizedstring.h>
#include <kdebug.h>
#include <kdatepicker.h>

KDateTimeDlg::KDateTimeDlg(QWidget *parent)
 : KDialog(parent)
{
   QWidget *widget = new QWidget(this);
   setMainWidget(widget);

   QGridLayout *grid = new QGridLayout;
   setButtons(Ok|Cancel);
   widget->setLayout(grid);
   setCaption(i18nc("Select Time and Date", "Date & Time Selector"));

   _date = new KDatePicker(this);
   grid->addWidget(_date, 0, 0, 6, 6);

   grid->addWidget(new QLabel(i18nc("Set Hours of Time", "Hour:"), this), 7, 0);
   _hours = new QSpinBox(this);
   _hours->setRange(0, 23);
   _hours->setSingleStep(1);
   _hours->setSliderEnabled(false);
   grid->addWidget(_hours, 7, 1);

   grid->addWidget(new QLabel(i18nc("Set Minutes of Time", "Minute:"), this), 7, 2);
   _mins = new QSpinBox(this);
   _mins->setRange(0, 59);
   _mins->setSingleStep(1);
   _mins->setSliderEnabled(false);
   grid->addWidget(_mins, 7, 3);

   grid->addWidget(new QLabel(i18nc("Set Seconds of Time", "Second:"), this), 7, 4);
   _secs = new QSpinBox(this);
   _secs->setRange(0, 59);
   _secs->setSingleStep(1);
   _secs->setSliderEnabled(false);
   grid->addWidget(_secs, 7, 5);

}


KDateTimeDlg::~KDateTimeDlg() {

}


QDate KDateTimeDlg::getDate() {
   return _date->date();
}


QTime KDateTimeDlg::getTime() {
QTime rc(_hours->value(), _mins->value(), _secs->value());
return rc;
}


QDateTime KDateTimeDlg::getDateTime() {
QDateTime qdt;
QTime qtime(_hours->value(), _mins->value(), _secs->value());

   qdt.setDate(_date->date());
   qdt.setTime(qtime);

return qdt;
}


void KDateTimeDlg::setDate(const QDate& qdate) {
   _date->setDate(qdate);
}


void KDateTimeDlg::setTime(const QTime& qtime) {
   _hours->setValue(qtime.hour());
   _mins->setValue(qtime.minute());
   _secs->setValue(qtime.second());
}


void KDateTimeDlg::setDateTime(const QDateTime& qdatetime) {
   _date->setDate(qdatetime.date());
   _hours->setValue(qdatetime.time().hour());
   _mins->setValue(qdatetime.time().minute());
   _secs->setValue(qdatetime.time().second());
}



