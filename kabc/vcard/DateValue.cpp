/*
	libvcard - vCard parsing library for vCard version 3.0

	Copyright (C) 1998 Rik Hemsley rik@kde.org
	
  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to
  deal in the Software without restriction, including without limitation the
  rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
  sell copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in
  all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
  AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
  ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
  WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include <qregexp.h>

#include <kdebug.h>

#include <VCardDefines.h>
#include <VCardDateValue.h>
#include <VCardValue.h>

using namespace VCARD;

DateValue::DateValue()
	:	Value()
{
	vDebug("DateValue::DateValue()");
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
		hasTime_(true)
{
	parsed_ = true;
	assembled_ = false;
}

DateValue::DateValue(const QDate & d)
	:	Value		(),
		year_		(d.year()),
		month_		(d.month()),
		day_		(d.day()),
		hasTime_(false)
{
	parsed_ = true;
	assembled_ = false;
}

DateValue::DateValue(const QDateTime & d)
	:	Value		(),
		year_		(d.date().year()),
		month_		(d.date().month()),
		day_		(d.date().day()),
		hour_		(d.time().hour()),
		minute_		(d.time().minute()),
		second_		(d.time().second()),
		hasTime_(true)
{
	parsed_ = true;
	assembled_ = false;
}

DateValue::DateValue(const DateValue & x)
	:	Value(x)
{
	year_ = x.year_;
	month_ = x.month_;
	day_ = x.day_;
	hour_ = x.hour_;
	minute_ = x.minute_;
	second_ = x.second_;
	zoneHour_ = x.zoneHour_;
	zoneMinute_ = x.zoneMinute_;
	secFrac_ = x.secFrac_;
	hasTime_ = x.hasTime_;
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

	DateValue *
DateValue::clone()
{
	return new DateValue( *this );
}

	void
DateValue::_parse()
{
	vDebug("DateValue::_parse()");

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

	kdDebug(5710) << "dateStr: " << dateStr << endl;

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
	vDebug("DateValue::_assemble");

	QCString year;
	QCString month;
	QCString day;
	
	year.setNum( year_ );
	month.setNum( month_ );
	day.setNum( day_ );

	if ( month.length() < 2 ) month.prepend( "0" );
	if ( day.length() < 2 ) day.prepend( "0" );

	strRep_ = year + '-' + month + '-' + day;

	if ( hasTime_ ) {
	    QCString hour;
	    QCString minute;
	    QCString second;

	    hour.setNum( hour_ );
	    minute.setNum( minute_ );
	    second.setNum( second_ );

	    if ( hour.length() < 2 ) hour.prepend( "0" );
	    if ( minute.length() < 2 ) minute.prepend( "0" );
	    if ( second.length() < 2 ) second.prepend( "0" );

	    strRep_ += 'T' + hour + ':' + minute + ':' + second + 'Z';
	}
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
	parse();
	QDate d(year_, month_, day_);
	return d;
}

	QTime
DateValue::qtime()
{
	parse();
	QTime t(hour_, minute_, second_);
//	t.setMs(1 / secFrac_);
	return t;
}

	QDateTime
DateValue::qdt()
{
	parse();
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

