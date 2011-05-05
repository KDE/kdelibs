/* This file is part of the KDE libraries
    Copyright (c) 2005 Thomas Braxton <brax108@cox.net>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#ifndef KLOCALETEST_H
#define KLOCALETEST_H

#include <QtCore/QObject>

class KLocaleTest : public QObject
{
	Q_OBJECT
private Q_SLOTS:
	void initTestCase();
    void languages();
    void formatNumbers();
	void formatNumberStrings();
	void readNumber();
    void formatMoney();
    void readMoney();
	void formatDate();
	void readDate();
	void formatTime();
	void formatDateTime();
	void readTime();
    void testDayPeriods();
    void testCalendarSystemType();
    void testCountryDivision();
	void prettyFormatDuration();
	void formatByteSize();
	void bug95511();
	void weekDays();
	void removeAcceleratorMarker();
	void formatByteSize2();
};

#endif // KLOCALETEST_H
