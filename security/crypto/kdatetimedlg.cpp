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
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "kdatetimedlg.h"
#include <qlayout.h>
#include <qlabel.h>
#include <klocale.h>
#include <kdebug.h>
#include <kdatepicker.h>
#include <qpushbutton.h>
#include <qdatetime.h>
#include <knuminput.h>


KDateTimeDlg::KDateTimeDlg(QWidget *parent, const char *name) 
                             : KDialog(parent, name, true) {
QGridLayout *grid = new QGridLayout(this, 9, 6, marginHint(), spacingHint());

   setCaption(i18n("Date & Time Selector"));

   _date = new KDatePicker(this);
   grid->addMultiCellWidget(_date, 0, 5, 0, 5);

   grid->addWidget(new QLabel(i18n("Hour:"), this), 7, 0);
   _hours = new KIntNumInput(this);
   _hours->setRange(0, 23, 1, false);
   grid->addWidget(_hours, 7, 1);

   grid->addWidget(new QLabel(i18n("Minute:"), this), 7, 2);
   _mins = new KIntNumInput(this);
   _mins->setRange(0, 59, 1, false);
   grid->addWidget(_mins, 7, 3);

   grid->addWidget(new QLabel(i18n("Second:"), this), 7, 4);
   _secs = new KIntNumInput(this);
   _secs->setRange(0, 59, 1, false);
   grid->addWidget(_secs, 7, 5);

   _ok = new QPushButton(i18n("&OK"), this);
   grid->addWidget(_ok, 8, 4);
   connect(_ok, SIGNAL(clicked()), SLOT(accept()));

   _cancel = new QPushButton(i18n("&Cancel"), this);
   grid->addWidget(_cancel, 8, 5);
   connect(_cancel, SIGNAL(clicked()), SLOT(reject()));

}


KDateTimeDlg::~KDateTimeDlg() {

}


QDate KDateTimeDlg::getDate() {
   return _date->getDate();
}


QTime KDateTimeDlg::getTime() {
QTime rc(_hours->value(), _mins->value(), _secs->value());
return rc;
}


QDateTime KDateTimeDlg::getDateTime() {
QDateTime qdt;
QTime qtime(_hours->value(), _mins->value(), _secs->value());

   qdt.setDate(_date->getDate());
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


#include "kdatetimedlg.moc"

