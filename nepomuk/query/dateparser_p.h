#ifndef DATEPARSER_P_H
#define DATEPARSER_P_H

/*
* This file is part of the Nepomuk KDE project.
* Copyright (c) 2009 Adam Kidder <thekidder@gmail.com>
*
* This library is free software; you can redistribute it and/or
* modify it under the terms of the GNU Library General Public
* License as published by the Free Software Foundation; either
* version 2 of the License, or (at your option) any later version.
*
* This library is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
* Library General Public License for more details.
*
* You should have received a copy of the GNU Library General Public License
* along with this library; see the file COPYING.LIB.  If not, write to
* the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
* Boston, MA 02110-1301, USA.
*/

#include <QtCore/QDate>
#include <QtCore/QTime>


namespace Nepomuk {
    namespace Search {
        /*DateTimeParser parses a string to extract QDateTime object(s). It is (will be) capable of extracting:
          * fully qualified date times
          * times qualified using a specified date
          * dates qualified with a specified time
        */
        class DateTimeParser
        {
        public:
            DateTimeParser(const QString& dateTimeString);


        private:
            const QString& text;
        };


        class DateParser
        {
        public:
            //flags
            const static unsigned int AbsoluteDates = 0x1;
            const static unsigned int RelativeDates = 0x2;

            DateParser(const QString& dateString, unsigned int flags = AbsoluteDates | RelativeDates);
            ~DateParser();

            //true if another date has been found
            bool hasDate();
            //advances to the next date
            void next();

            //next 3 functions only valid if hasDate()
            QDate getDate();
            //position of current extracted date
            unsigned int pos() const;
            //length of current extracted date string
            unsigned int length() const;
        private:
            class Private;
            Private* const d;
        };

        //FIXME: code duplication in DateParser and TimeParser
        class TimeParser
        {
        public:
            TimeParser(const QString& timeString);
            ~TimeParser();

            bool hasTime();
            QTime next();
        private:
            class Private;
            Private* const d;
        };
    }
}
#endif
