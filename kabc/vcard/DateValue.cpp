/*
	libvcard - vCard parsing library for vCard version 3.0

	Copyright (C) 1998 Rik Hemsley rik@kde.org
	
	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#include <qregexp.h>

#include <VCardDefines.h>
#include <VCardDateValue.h>
#include <VCardValue.h>

using namespace VCARD;

DateValue::DateValue()
	:	Value()
{
}

DateValue::DateValue(
		unsigned int	year,
		unsigned int	month,
		unsigned int	day,
		unsigned int	hour,
		unsigned int	minute,
		unsigned int	second,
		double		secFrac,
		bool		zonePositive,
		unsigned int	zoneHour,
		unsigned int	zoneMinute)
	:	Value			(),
		year_			(year),
		month_			(month),
		day_			(day),
		hour_			(hour),
		minute_			(minute),
		second_			(second),
		zoneHour_		(zoneHour),
		zoneMinute_		(zoneMinute),
		secFrac_		(secFrac),
		zonePositive_	(zonePositive),
		hasTime_(false)
{
	parsed_		= true;
}

DateValue::DateValue(const QDate & d)
	:	Value		(),
		year_		(d.year()),
		month_		(d.month()),
		day_		(d.day())
{
	parsed_		= true;
}

DateValue::DateValue(const DateValue & x)
	:	Value(x)
{
}

DateValue::DateValue(const QCString & s)
	:	Value(s)
{
}

	DateValue &
DateValue::operator = (DateValue & x)
{
	if (*this == x) return *this;

	Value::operator = (x);
	return *this;
}

	DateValue &
DateValue::operator = (const QCString & s)
{
	Value::operator = (s);
	return *this;
}

	bool
DateValue::operator == (DateValue & x)
{
	x.parse();
	return false;
}

DateValue::~DateValue()
{
}

	void
DateValue::_parse()
{
	vDebug("parse");
	// date = date-full-year ["-"] date-month ["-"] date-mday
	// time = time-hour [":"] time-minute [":"] time-second [":"]
	// [time-secfrac] [time-zone]
	
	int timeSep = strRep_.find('T');
	
	QCString dateStr;
	QCString timeStr;
	
	if (timeSep == -1) {
		
		dateStr = strRep_;
		vDebug("Has date string \"" + dateStr + "\"");
		
	} else {
		
		dateStr = strRep_.left(timeSep);
		vDebug("Has date string \"" + dateStr + "\"");
		
		timeStr = strRep_.mid(timeSep + 1);
		vDebug("Has time string \"" + timeStr + "\"");
	}
	
	/////////////////////////////////////////////////////////////// DATE
	
	dateStr.replace(QRegExp("-"), "");

	year_	= dateStr.left(4).toInt();
	month_	= dateStr.mid(4, 2).toInt();
	day_	= dateStr.right(2).toInt();
	
	if (timeSep == -1) {
		hasTime_ = false;
		return; // No time, done.
	}
	else
		hasTime_ = true;
	
	/////////////////////////////////////////////////////////////// TIME

	/////////////////////////////////////////////////////////////// ZONE
	
	int zoneSep = timeStr.find('Z');
	
	if (zoneSep != -1 && timeStr.length() - zoneSep > 3) {
		
		QCString zoneStr(timeStr.mid(zoneSep + 1));
		vDebug("zoneStr == " + zoneStr);

		zonePositive_	= (zoneStr[0] == '+');
		zoneHour_		= zoneStr.mid(1, 2).toInt();
		zoneMinute_		= zoneStr.right(2).toInt();
		
		timeStr.remove(zoneSep, timeStr.length() - zoneSep);
	}

	//////////////////////////////////////////////////// SECOND FRACTION
	
	int secFracSep = timeStr.findRev(',');
	
	if (secFracSep != -1 && zoneSep != -1) { // zoneSep checked to avoid errors.
		QCString quirkafleeg = "0." + timeStr.mid(secFracSep + 1, zoneSep);
		secFrac_ = quirkafleeg.toDouble();
	}
	
	/////////////////////////////////////////////////////////////// HMS

	timeStr.replace(QRegExp(":"), "");
	
	hour_	= timeStr.left(2).toInt();
	minute_	= timeStr.mid(2, 2).toInt();
	second_	= timeStr.mid(4, 2).toInt();
}

	void
DateValue::_assemble()
{
	strRep_ =	QCString().setNum(year_)	+ '-' +
				QCString().setNum(month_)	+ '-' +
				QCString().setNum(day_);
}

	unsigned int
DateValue::year()
{
	parse();
	return year_;
}

	unsigned int
DateValue::month()
{
	parse();
	return month_;
}

	unsigned int
DateValue::day()
{
	parse();
	return day_;
}
	unsigned int
DateValue::hour()
{
	parse();
	return hour_;
}

	unsigned int
DateValue::minute()
{
	parse();
	return minute_;
}

	unsigned int
DateValue::second()
{
	parse();
	return second_;
}

	double
DateValue::secondFraction()
{
	parse();
	return secFrac_;
}

	bool
DateValue::zonePositive()
{
	parse();
	return zonePositive_;
}

	unsigned int
DateValue::zoneHour()
{
	parse();
	return zoneHour_;
}

	unsigned int
DateValue::zoneMinute()
{
	parse();
	return zoneMinute_;
}
	
	void
DateValue::setYear(unsigned int i)
{
	year_ = i;
	assembled_ = false;
}

	void
DateValue::setMonth(unsigned int i)
{
	month_ = i;
	assembled_ = false;
}

	void
DateValue::setDay(unsigned int i)
{
	day_ = i;
	assembled_ = false;
}

	void
DateValue::setHour(unsigned int i)
{
	hour_ = i;
	assembled_ = false;
}

	void
DateValue::setMinute(unsigned int i)
{
	minute_ = i;
	assembled_ = false;
}

	void
DateValue::setSecond(unsigned int i)
{
	second_ = i;
	assembled_ = false;
}

	void
DateValue::setSecondFraction(double d)
{
	secFrac_ = d;
	assembled_ = false;
}

	void
DateValue::setZonePositive(bool b)
{
	zonePositive_ = b;
	assembled_ = false;
}	

	void
DateValue::setZoneHour(unsigned int i)
{
	zoneHour_ = i;
	assembled_ = false;
}

	void
DateValue::setZoneMinute(unsigned int i)
{
	zoneMinute_ = i;
	assembled_ = false;
}

	QDate
DateValue::qdate()
{
	QDate d(year_, month_, day_);
	return d;
}

	QTime
DateValue::qtime()
{
	QTime t(hour_, minute_, second_);
//	t.setMs(1 / secFrac_);
	return t;
}

	QDateTime
DateValue::qdt()
{
	QDateTime dt;
	dt.setDate(qdate());
	dt.setTime(qtime());
	return dt;
}

	bool
DateValue::hasTime()
{
	parse();
	return hasTime_;
}

