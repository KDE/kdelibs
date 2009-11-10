/*
* This file is part of the Nepomuk KDE project.
* Copyright (c) 2009 Adam Kidder <thekidder@gmail.com>
* Copyright (c) 2009 Sebastian Trueg <trueg@kde.org>
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

#include "dateparser_p.h"

#include <kdebug.h>

#include <QtCore/QLocale>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QVector>


namespace {
    //represents a time difference from the current time, to represent relative dates
    struct time_difference
    {
        time_difference() : seconds(0), minutes(0), hours(0), days(0), weeks(0), months(0), years(0) {}
        int seconds, minutes, hours, days, weeks, months, years;
    };

    struct format
    {
        format() : pos(0) {}
        format(const QRegExp& r, const QStringList& f) : regex(r), pos(0), useRelativeDate(false), formats(f) {}
        format(const QRegExp& r, const time_difference& d, bool dr = false) : regex(r), pos(0), useRelativeDate(true), difference(d), dynamicRelative(dr) {}
        QRegExp regex;
        int pos;

        bool useRelativeDate;
        time_difference difference;
        bool dynamicRelative;
        QStringList formats;
    };

    struct date_string
    {
        QDate date;
        unsigned int pos;
        unsigned int length;
    };
}

class Nepomuk::Search::DateParser::Private
{
public:
    Private(const QString& text, unsigned int flags) :
        m_text(text), m_locale(QLocale::English), m_flags(flags) {
        //TODO: we are english-only here!
        QStringList longMonthNames;
        QStringList shortMonthNames;
        for ( int i = 1; i <= 12; ++i ) {
            longMonthNames << m_locale.monthName( i, QLocale::LongFormat );
            shortMonthNames << m_locale.monthName( i, QLocale::ShortFormat );
        }

        // DD.MM.YYYY
        format date1( QRegExp( "\\b\\d{1,2}\\.\\d{1,2}\\.\\d{4,4}\\b" ), QStringList("d.M.yyyy") );

        // DD.MM.YY
        format date2( QRegExp( "\\b\\d{1,2}\\.\\d{1,2}\\.\\d{2,2}\\b" ), QStringList("d.M.yy") );

        // MM/DD/YYYY
        format date3( QRegExp( "\\b\\d{1,2}/\\d{1,2}/\\d{4,4}\\b" ), QStringList("M/d/yyyy") );

        // YYYY-MM-DD
        format date13(QRegExp( "\\b\\d{4,4}-\\d{1,2}-\\d{1,2}\\b" ), QStringList("yyyy-M-d") );

        // MM/DD/YY
        format date4( QRegExp( "\\b\\d{1,2}/\\d{1,2}/\\d{2,2}\\b" ), QStringList("M/d/yy") );

        // January MM [YYYY] (no word boundry at the end for 'st' or 'nd' or 'th') (also excluding ranges)
        format date5( QRegExp( QString( "\\b(%1)\\s\\d{1,2}(?!(\\d|\\s?-\\s?\\d))(\\s\\d{4,4})?" ).arg( longMonthNames.join( "|" ) ) ),
            QStringList("MMMM d") << QString("MMMM d yyyy") );

        // January, MM [YYYY] (no word boundry at the end for 'st' or 'nd' or 'th') (also excluding ranges)
        format date6( QRegExp( QString( "\\b(%1),\\s?\\d{1,2}(?!(\\d|\\s?-\\s?\\d))(\\s\\d{4,4})?" ).arg( longMonthNames.join( "|" ) ) ),
            QStringList("MMMM, d") << QString("MMMM,d") << QString("MMMM, d yyyy") << QString("MMMM,d yyyy") );

        // FIXME: trueg: IMHO something like "yesterday" should result in a range if not used with < or >.

        //TODO: english only again!
        time_difference days;
        days.days = -1;
        format date7( QRegExp( QString( "\\b(yesterday)\\b" ) ), days );

        format date8( QRegExp( QString( "\\b(\\d{1,3}) (day)s? ago\\b" ) ), days, true );

        time_difference weeks;
        weeks.weeks = -1;
        format date9( QRegExp( QString( "\\ba week ago\\b" ) ), weeks );

        format date10( QRegExp( QString( "\\b(\\d{1,3}) (week)s? ago\\b" ) ), weeks, true );

        time_difference months;
        months.months = -1;
        format date11( QRegExp( QString( "\\ba month ago\\b" ) ), months );

        format date12( QRegExp( QString( "\\b(\\d{1,3}) (month)s? ago\\b" ) ), months, true );

        m_regexes.push_back( date1 );
        m_regexes.push_back( date2 );
        m_regexes.push_back( date3 );
        m_regexes.push_back( date4 );
        m_regexes.push_back( date5 );
        m_regexes.push_back( date6 );
        m_regexes.push_back( date7 );
        m_regexes.push_back( date8 );
        m_regexes.push_back( date9 );
        m_regexes.push_back( date10);
        m_regexes.push_back( date11);
        m_regexes.push_back( date12);
        m_regexes.push_back( date13);
    }


    bool hasDate() {
        if(!m_dates.empty()) return true;

        while(m_dates.empty() && !finishedParsing())
        {
            parseAllRegexes();
        }

        if(!m_dates.empty()) return true;
        return false;
    }


    QDate getDate() {
        if( !m_dates.isEmpty() )
            return m_dates.first().date;
        else
            return QDate();
    }

    void next() {
        m_dates.pop_front();
    }

    unsigned int length() const {
        if( !m_dates.isEmpty() )
            return m_dates.first().length;
        else
            return 0;
    }

    unsigned int pos() const {
        if( !m_dates.isEmpty() )
            return m_dates.first().pos;
        else
            return 0;
    }
    int dateObject;

private:
    bool finishedParsing() {
        foreach(format r, m_regexes) {
            if(r.pos != -1) return false;
        }
        return true;
    }


    void parseAllRegexes() {
        QVector<format>::iterator it ;
        for(it = m_regexes.begin(); it != m_regexes.end(); ++it) {
            it->pos = it->regex.indexIn(m_text, it->pos);
            if( it->pos == -1 )
                continue;
            if( !it->useRelativeDate && (m_flags & AbsoluteDates) ) {
                foreach(QString format, it->formats) {
                    QDate date = m_locale.toDate( it->regex.cap( 0 ), format );
                    if(date.isValid()) {
                        if(!format.contains( "yy" ) )
                            date.setDate( QDate::currentDate().year(), date.month(), date.day() );
                        kDebug() << "Found absolute date:" << date;
                        date_string dateObject;
                        dateObject.date = date;
                        dateObject.pos = it->pos;
                        dateObject.length = it->regex.matchedLength();
                        m_dates.append( dateObject );
                        break;
                    }
                }
            }
            else if( m_flags & RelativeDates) {
                int amount = 1;
                if( it->dynamicRelative ) {
                    amount = it->regex.cap( 1 ).toInt();
                    kDebug() << "dynamic relative date, amount is" << amount << it->regex.cap( 1 );
                }
                QDate current( QDate::currentDate() );
                current = current.addDays( it->difference.days * amount );
                current = current.addDays( it->difference.weeks * 7 * amount );
                current = current.addMonths( it->difference.months * amount );
                current = current.addYears( it->difference.years * amount );

                kDebug() << "Found relative date:" << current << it->regex.pattern();
                date_string dateObject;
                dateObject.date = current;
                dateObject.pos = it->pos;
                dateObject.length = it->regex.matchedLength();
                m_dates.append( dateObject );
            }
        }
    }


    const QString& m_text;
    QLocale m_locale;
    QVector<format> m_regexes;
    QList<date_string> m_dates;
    unsigned int m_flags;
};



Nepomuk::Search::DateParser::DateParser(const QString& text, unsigned int flags) :
    d( new Private(text, flags) ) {
}


Nepomuk::Search::DateParser::~DateParser() {
    delete d;
}

bool Nepomuk::Search::DateParser::hasDate() {
    return d->hasDate();
}

QDate Nepomuk::Search::DateParser::getDate() {
    return d->getDate();
}

void Nepomuk::Search::DateParser::next() {
    d->next();
}

unsigned int Nepomuk::Search::DateParser::pos() const {
    return d->pos();
}

unsigned int Nepomuk::Search::DateParser::length() const {
    return d->length();
}

class Nepomuk::Search::TimeParser::Private
{
public:
    Private(const QString& text) : m_text(text), m_locale(QLocale::English) {
        // hh:mm[pm|am]
        format time1( QRegExp( "\\b\\d{1,2}\\:\\d{2,2}\\s?(pm|am|AM|PM)?\\b" ), QStringList("h:map") << QString("h:m ap") );

        // hh:mm
        format time2( QRegExp( "\\b\\d{1,2}\\:\\d{2,2}\\b(?!\\s?(pm|am|AM|PM))\\b" ), QStringList("h:m") );

        m_regexes.push_back( time1 );
        m_regexes.push_back( time2 );
    }


    bool hasTime() {
        if(!m_times.empty()) return true;

        while(m_times.empty() && !finishedParsing())
        {
            parseAllRegexes();
        }

        if(!m_times.empty()) return true;
        return false;
    }


    QTime next() {
        return m_times.takeFirst();
    }
private:
    bool finishedParsing() {
        foreach(format r, m_regexes) {
            if(r.pos != -1) return false;
        }
        return true;
    }


    void parseAllRegexes() {
        QVector<format>::iterator it ;
        for(it = m_regexes.begin(); it != m_regexes.end(); ++it) {
            it->pos = it->regex.indexIn(m_text, it->pos);
            if( !it->useRelativeDate ) {
                foreach(QString format, it->formats) {
                    QTime time = m_locale.toTime( it->regex.cap( 0 ), format );
                    if(time.isValid()) {
                        kDebug() << "Found time:" << time;
                        m_times.append( time );
                        break;
                    }
                }
            }
            else {
                QTime current( QTime::currentTime() );
                current.addSecs( it->difference.seconds );
                current.addSecs( it->difference.minutes * 60 );
                current.addSecs( it->difference.hours * 60 * 60 );

                kDebug() << "Found time:" << current;
                m_times.append( current );
                break;
            }
        }
    }


    const QString& m_text;
    QLocale m_locale;
    QVector<format> m_regexes;
    QList<QTime> m_times;
};



Nepomuk::Search::TimeParser::TimeParser(const QString& text) : d( new Private(text) ) {
}


Nepomuk::Search::TimeParser::~TimeParser() {
    delete d;
}

bool Nepomuk::Search::TimeParser::hasTime() {
    return d->hasTime();
}

QTime Nepomuk::Search::TimeParser::next() {
    return d->next();
}
