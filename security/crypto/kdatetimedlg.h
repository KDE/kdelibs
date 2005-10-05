/**
 * kdatetimedlg.h
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

#ifndef _KDATETIMEDLG_H
#define _KDATETIMEDLG_H

#include <kdialog.h>

class KDatePicker;
class KIntNumInput;
class KPushButton;
class QDate;
class QTime;
class QDateTime;

class KDateTimeDlgPrivate;

class KDateTimeDlg : public KDialog
{
  Q_OBJECT
public:
  KDateTimeDlg(QWidget *parent = 0L, const char *name = 0L);
  virtual ~KDateTimeDlg();

  virtual QTime     getTime();
  virtual QDate     getDate();
  virtual QDateTime getDateTime();

  virtual void      setDate(const QDate& qdate);
  virtual void      setTime(const QTime& qtime);
  virtual void      setDateTime(const QDateTime& qdatetime);

protected slots:

private:
   KPushButton *_ok, *_cancel;
   KDatePicker *_date;
   KIntNumInput *_hours, *_mins, *_secs;

   KDateTimeDlgPrivate *d;
};

#endif
