/*
    Copyright 2011 John Layt <john@layt.net>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#ifndef KDATECOMBOBOXTEST_H
#define KDATECOMBOBOXTEST_H

#include <QWidget>

class KDateTimeEdit;

class KDateTimeEditTest : public QWidget
{
  Q_OBJECT

private Q_SLOTS:
  void testDefaults();
  void testValidNull();
  void testDateTimeRange();
  void testOptions();
  void testDateDisplayFormat();
  void testDateList();
  void testTimeListInterval();
  void testTimeList();
  void testTimeDisplayFormat();
  void testCalendarSystem();
  void testTimeSpec();

private:

  KDateTimeEdit* m_edit;
};

#endif // KDATECOMBOBOXTEST_H
