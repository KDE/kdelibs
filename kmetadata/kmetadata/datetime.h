/* 
 *
 * $Id: sourceheader 511311 2006-02-19 14:51:05Z trueg $
 *
 * This file is part of the Nepomuk KDE project.
 * Copyright (C) 2006 Sebastian Trueg <trueg@kde.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * See the file "COPYING" for the exact licensing terms.
 */

#ifndef _KMETADATA_DATE_TIME_H_
#define _KMETADATA_DATE_TIME_H_

#include <QString>
#include <QDateTime>

#include <kmetadata/kmetadata_export.h>

namespace Nepomuk {
    namespace KMetaData {
	class KMETADATA_EXPORT DateTime
	    {
	    public:
		/**
		 * Parse a time string according to format hh:mm:ss.szzzzzz as defined in 
		 * XML Schema Part 2: Datatypes Second Edition: http://www.w3.org/TR/xmlschema-2
		 *
		 * Be aware that QTime has no notion of timezones and the returned value is 
		 * always in UTC.
		 */
		static QTime fromTimeString( const QString& );
		static QDate fromDateString( const QString& );
		static QDateTime fromDateTimeString( const QString& );

		static QString toString( const QTime& );
		static QString toString( const QDate& );
		static QString toString( const QDateTime& );

	    private:
		DateTime();
		~DateTime();

		class Private;
		Private* d;
	    };
    }
}

#endif
