/*
	libvcard - vCard parsing library for vCard version 3.0
	
	Copyright (C) 1999 Rik Hemsley rik@kde.org
	
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

#ifndef  DATEVALUE_H
#define  DATEVALUE_H

#include <qcstring.h>
#include <qdatetime.h>

#include <VCardValue.h>

namespace VCARD
{

class DateValue : public Value
{
#include "DateValue-generated.h"
	
	DateValue(
		unsigned int	year,
		unsigned int	month,
		unsigned int	day,
		unsigned int	hour = 0,
		unsigned int	minute = 0,
		unsigned int	second = 0,
		double			secFrac = 0,
		bool			zonePositive = true,
		unsigned int	zoneHour = 0,
		unsigned int	zoneMinute = 0);

	DateValue(const QDate &);
	DateValue(const QDateTime &);
	
	bool hasTime();
	
	unsigned int	year();
	unsigned int	month();
	unsigned int	day();
	unsigned int	hour();
	unsigned int	minute();
	unsigned int	second();
	double			secondFraction();
	bool			zonePositive();
	unsigned int	zoneHour();
	unsigned int	zoneMinute();
	
	void setYear			(unsigned int);
	void setMonth			(unsigned int);
	void setDay				(unsigned int);
	void setHour			(unsigned int);
	void setMinute			(unsigned int);
	void setSecond			(unsigned int);
	void setSecondFraction	(double);
	void setZonePositive	(bool);
	void setZoneHour		(unsigned int);
	void setZoneMinute		(unsigned int);
	
	QDate qdate();
	QTime qtime();
	QDateTime qdt();
	
	private:
		
		unsigned int	year_, month_, day_,
						hour_, minute_, second_,
						zoneHour_, zoneMinute_;
						
		double secFrac_;

		bool zonePositive_;
		
		bool hasTime_;
};

}

#endif
