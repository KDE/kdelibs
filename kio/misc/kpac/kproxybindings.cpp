/*
 *  $Id$
 *  KJS-Bindings for Proxy Auto Configuration
 *
 *  Copyright (C) 2000 Malte Starostik <malte@kde.org>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 *  Boston, MA 02111-1307, USA.
 */

#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>

#include <netinet/in.h>
#include <arpa/inet.h>

#include <netdb.h>
#include <time.h>

#include <qstring.h>
#include <qregexp.h>

#include "kproxybindings.h"

// e.g. Solaris doesn't have that, but uses -1 for inet_addr() error-handling
#ifndef INADDR_NONE
#define INADDR_NONE -1
#endif

using namespace KJS;

/// @internal
QString UString::qstring() const
{
    return QString((QChar*) data(), size());
}

KProxyFunc::KProxyFunc(int id)
{
    m_id = id;
}

bool KProxyFunc::implementsCall() const
{
  return true;
}

Value KProxyFunc::call(ExecState *exec, Object &/*thisObj*/, const List &args)
{
    Value result = Undefined();
    switch (m_id)
    {
        case IsPlainHostName:
            // isPlainHostName(host)
            // true if host does not contain a domain.
            if (args.size() == 1)
                result = Boolean(args[0].toString(exec).find(".") == -1);
            break;
        case DNSDomainIs:
            // dnsDomainIs(host, domain)
            // true is host ends in domain
            if (args.size() == 2)
            {
                QString host = args[0].toString(exec).qstring().lower();
                QString domain = args[1].toString(exec).qstring().lower();
                int p = host.find(domain);
                if (p >= 0)
                    result = Boolean(host.mid(p) == domain);
                else
                    result = Boolean(false);
            }
            break;
        case LocalHostOrDomainIs:
            // localHostOrDomainIs(host, domain)
            // true if host ends in domain or does not contain any dots.
            if (args.size() == 2)
            {
                QString host = args[0].toString(exec).qstring().lower();
                if (host.find(".") == -1)
                    result = Boolean(true);
                else
                {
                    QString domain = args[1].toString(exec).qstring().lower();
                    int p = host.find(domain);
                    if (p >= 0)
                        result = Boolean(host.mid(p) == domain);
                    else
                        result = Boolean(false);
                }
            }
            break;
        case IsResolvable:
            // isResolvable(host)
            // true if host can be resolved to an IP
            if (args.size() == 1)
                result = Boolean(!dnsResolve(args[0].toString(exec)).isNull());
            break;
        case IsInNet:
            // isInNet(host, pattern, mask)
            // true if host (after DNS lookup) is inside the subnet
            // given by pattern/mask, both dotted quads
            if (args.size() == 3)
            {
                UString host = dnsResolve(args[0].toString(exec));
                if (host.isNull())
                    result = Boolean(false);
                else
                {
                    unsigned long ip, pattern = 0, mask = 0;
                    // unfortunately inet_aton is not available on Solaris (malte)
                    if ((ip = inet_addr(host.ascii())) == INADDR_NONE
                        || (pattern = inet_addr(args[1].toString(exec).ascii())) == INADDR_NONE
                        || (mask = inet_addr(args[2].toString(exec).ascii())) == INADDR_NONE)
                        result = Boolean(false);
                    else
                        result = Boolean((ip & mask) == (pattern & mask));
                }
            }
            break;
        case DNSResolve:
            // dnsResolve(host)
            // returns the IP of host
            if (args.size() == 1)
            {
                UString addr = dnsResolve(args[0].toString(exec));
                if (addr.isNull())
                    result = Undefined();
                else
                    result = String(addr);
            }
            break;
        case MyIPAddress:
            // myIpAddress()
            // returns the IP of the client
            if (args.size() == 0)
            {
                char hostname[256];
                gethostname(hostname, 255);
                UString addr = dnsResolve(hostname);
                if (addr.isNull())
                    result = Undefined();
                else
                    result = String(addr);
            }
            break;
        case DNSDomainLevels:
            // dnsDomainLevels(host)
            // counts the dots in host
            if (args.size() == 1)
            {
                UString host = args[0].toString(exec);
                int p = -1, count = 0;
                while ((p = host.find(".", p+1)) != -1)
                    count++;
                result = Number(count);
            }
            break;
        case ShExpMatch:
            // shExpMatch(string, glob)
            // true if string matches the shell-glob-like pattern glob
            if (args.size() == 2)
            {
                QRegExp rex(args[1].toString(exec).qstring(), true, true);
                result = Boolean(rex.search(args[0].toString(exec).qstring(), 0) != -1);
            }
            break;
        case WeekdayRange:
            // weekdayRange(day [, "GMT"])
            // weekdayRange(day1, day2 [, "GMT"])
            // true if the current day is day or between day1 and day2
            // if the additional parameter "GMT" is given, compares
            // to the current time in GMT, otherwise to localtime
            if (args.size() >= 1 && args.size() <= 3)
            {
                static const char *weekdays[] = {"son", "mon", "tue", "wed", "thu", "fri", "sat", 0};
                int day1 = findString(args[0].toString(exec).qstring().lower(), weekdays);
                if (day1 == -1)
                    break;
                int day2 = args.size() > 1 ?
                    findString(args[1].toString(exec).qstring().lower(), weekdays) : -1;
                if (day2 == -1)
                    day2 = day1;
                const struct tm *now = getTime(exec,args);
                result = Boolean(checkRange(now->tm_wday, day1, day2));
            }
            break;
        case DateRange:
            // dateRange(day [, "GMT"])
            // dateRange(day1, day2 [, "GMT"])
            // dateRange(month [, "GMT"])
            // dateRange(month1, month2 [, "GMT"])
            // dateRange(year [, "GMT"])
            // dateRange(year1, year2 [, "GMT"])
            // dateRange(day1, month1, day2, month2 [, "GMT"])
            // dateRange(month1, year1, month2, year2 [, "GMT"])
            // dateRange(day1, month1, year1, day2, month2, year2 [, "GMT"])
            // true if the current is day or between day1 and day2
            // in the month month or between month1 and month2
            // in the year year or between year1 and year2
            // days are given as numbers from 1 to 31
            // months are given as three-letter-names
            // years are given with four digits
            if (args.size() >= 1 && args.size() <= 7)
            {
                static const char *months[] = {"jan", "feb", "mar", "apr", "may", "jun", "jul", "aug", "nov", "dec", 0};
                int values[6] = {-1, -1, -1, -1, -1, -1};
                for (int i = 0; i < 6 && i < args.size(); ++i)
                {
                    if (args[i].isA(NumberType))
                        values[i] = (args[i].toInteger(exec));
                    else
                        values[i] = findString(args[i].toString(exec).qstring().lower(), months);
                }
                int min, max, current;
                const struct tm *now = getTime(exec,args);
                if (values[5] != -1) // day1, month1, year1, day2, month2, year2
                {
                    min = values[2] * 372 + values[1] * 31 + values[0];
                    max = values[5] * 372 + values[4] * 31 + values[3];
                    current = now->tm_year * 372 + now->tm_mon * 31 + now->tm_mday;
                }
                else if (values[3] != -1 && args[0].isA(NumberType)) // day1, month1, day2, month2
                {
                    min = values[1] * 31 + values[0];
                    max = values[3] * 31 + values[2];
                    current = now->tm_mon * 31 + now->tm_mday;
                }
                else if (values[3] != -1) // month1, year1, month2, year2
                {
                    min = values[1] * 12 + values[0];
                    max = values[3] * 12 + values[2];
                    current = now->tm_year * 12 + now->tm_mon;
                }
                else
                {
                    min = values[0];
                    max = values[1] != -1 ? values[1] : values[0];
                    if (!args[0].isA(NumberType)) // month[1, month2]
                        current = now->tm_mon;
                    else if (values[0] <= 31) // day[1, day2]
                        current = now->tm_mday;
                    else // year[1, year2]
                        current = now->tm_year;
                }
                result = Boolean(checkRange(current, min, max));
            }
            break;
        case TimeRange:
            // timeRange(hour [, "GMT"])
            // timeRange(hour1, hour2 [, "GMT"])
            // timeRange(hour1, min1, hour2, min2 [, "GMT"])
            // timeRange(hour1, min1, sec1, hour2, min2, sec2 [, "GMT"])
            // true if the current time is in the given range
            if (args.size() >= 1 && args.size() <= 7)
            {
                int values[6] = {-1, -1, -1, -1, -1, -1};
                for (int i = 0; i < args.size(); ++i)
                {
                    if (!args[i].isA(NumberType))
                        break;
                    values[i] = (args[i].toInteger(exec));
                }
                if (values[0] == -1)
                    break;
                int min, max;
                if (values[5] != -1) // hour1, min1, sec1, hour2, min2, sec2
                {
                    min = values[0] * 3600 + values[1] * 60 + values[2];
                    max = values[3] * 3600 + values[4] * 60 + values[5];
                }
                else if (values[3] != -1) // hour1, min1, hour2, min2
                {
                    min = values[0] * 3600 + values[1] * 60;
                    max = values[2] * 3600 + values[3] * 60 + 59;
                }
                else if (values[1] != -1) // hour1, hour2
                {
                    min = values[0] * 3600;
                    max = values[1] * 3600 + 3559;
                }
                else // hour1
                {
                    min = values[0] * 3600;
                    max = values[0] * 3600 + 3559;
                }
                const struct tm *now = getTime(exec,args);
                result = Boolean(checkRange(now->tm_hour * 3600 + now->tm_min * 60 + now->tm_sec, min, max));
            }
            break;
    }
    return result;
}

const UString KProxyFunc::dnsResolve(const UString &host) const
{
    struct hostent *info = gethostbyname(host.ascii());
    if (!info)
        return UString();
    // FIXME is this cast okay (malte)?
    return UString(inet_ntoa(*((struct in_addr *) info->h_addr_list[0])));
}

const struct tm *KProxyFunc::getTime(ExecState *exec, const List &args) const
{
    time_t now = time(0);
    return args[args.size() -1].toString(exec).qstring().lower() == "gmt" ?
        gmtime(&now) : localtime(&now);
}

int KProxyFunc::findString(const QString &str, const char **list) const
{
    for (int i = 0; list[i]; ++i)
    {
        if (list[i] == str)
            return i;
    }
    return -1;
}

bool KProxyFunc::checkRange(int value, int min, int max) const
{
    return (min <= max && value >= min && value <= max)
        || (min > max && (value >= min || value <= max));
}


void KProxyFunc::init(ExecState *exec, Object &global)
{
    global.put(exec, "ProxyConfig", global);
    global.put(exec, "isPlainHostName", Object(new KProxyFunc(KProxyFunc::IsPlainHostName)));
    global.put(exec, "dnsDomainIs", Object(new KProxyFunc(KProxyFunc::DNSDomainIs)));
    global.put(exec, "localHostOrDomainIs", Object(new KProxyFunc(KProxyFunc::LocalHostOrDomainIs)));
    global.put(exec, "isResolvable", Object(new KProxyFunc(KProxyFunc::IsResolvable)));
    global.put(exec, "isInNet", Object(new KProxyFunc(KProxyFunc::IsInNet)));
    global.put(exec, "dnsResolve", Object(new KProxyFunc(KProxyFunc::DNSResolve)));
    global.put(exec, "myIpAddress", Object(new KProxyFunc(KProxyFunc::MyIPAddress)));
    global.put(exec, "dnsDomainLevels", Object(new KProxyFunc(KProxyFunc::DNSDomainLevels)));
    global.put(exec, "shExpMatch", Object(new KProxyFunc(KProxyFunc::ShExpMatch)));
    global.put(exec, "weekdayRange", Object(new KProxyFunc(KProxyFunc::WeekdayRange)));
    global.put(exec, "dateRange", Object(new KProxyFunc(KProxyFunc::DateRange)));
    global.put(exec, "timeRange", Object(new KProxyFunc(KProxyFunc::TimeRange)));
}

// vim: ts=4 sw=4 et
