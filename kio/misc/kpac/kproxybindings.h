/*
 *  $Id$
 *  KJS-Bindings for Proxy Auto Configuration
 *  
 *  Copyright (C) 2000 Malte Starostik <malte.starostik@t-online.de>
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

#ifndef _KPROXYBINDINGS_H_
#define _KPROXYBINDINGS_H_

#include <kjs/object.h>

struct tm;

namespace KJS
{
    class KProxyFunc : public ObjectImp
    {
    public:
        enum { IsPlainHostName, DNSDomainIs, LocalHostOrDomainIs,
               IsResolvable, IsInNet,
               DNSResolve, MyIPAddress, DNSDomainLevels,
               ShExpMatch,
               WeekdayRange, DateRange, TimeRange };
        KProxyFunc(int id);
        virtual bool implementsCall() const;
        virtual Value call(ExecState *exec, Object &thisObj,
                           const List &args);
        /**
         * Initializes the global object with all the necessary
         * KProxyFunc instances
         */
        static void init(ExecState *exec, Object &global);
    protected:
        /**
         *  Helper for all DNS operations
         **/
        const UString dnsResolve(const UString &) const;
        /**
         * Helper for the *Range functions,
         * returns the current time in GMT or local time depending
         * on whether the last argument in args is "GMT"
         **/
        const struct tm *getTime(ExecState *exec, const List &args) const;
        /**
         * Helper for weekday/month name calculations
         * returns the index of the element in list
         * that matches str or -1 if no match was found
         **/
        int findString(const QString &str, const char **list) const;
        /**
         * Help for the *Range functions
         * return true if min <= value <= max and can deal with
         * max < min for ranges like 18-12 o' clock
         **/
        bool checkRange(int value, int min, int max) const;
    private:
        int m_id;
    };
};

#endif

