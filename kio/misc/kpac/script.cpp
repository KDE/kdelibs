/*
   Copyright (c) 2003 Malte Starostik <malte@kde.org>

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


#include <cstdlib>
#include <vector>
#include <algorithm>
#include <ctime>
#include <cstring>

#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#include <qregexp.h>
#include <qstring.h>

#include <kbufferedsocket.h>
#include <ksocketaddress.h>
#include <kurl.h>
#include <kjs/object.h>
#include <kresolver.h>

#include "script.h"

using namespace KJS;
using namespace KNetwork;

QString UString::qstring() const
{
    return QString( reinterpret_cast< const QChar* >( data() ), size() );
}

UString::UString( const QString &s )
{
    UChar* data = new UChar[ s.length() ];
    std::memcpy( data, s.unicode(), s.length() * sizeof( UChar ) );
    rep = Rep::create( data, s.length() );
}

namespace
{
    class Address
    {
    public:
        struct Error {};
        static Address resolve( const UString& host )
            { return Address( host.qstring(), false ); }
        static Address parse( const UString& ip )
            { return Address( ip.qstring(), true ); }

        operator KInetSocketAddress() const { return m_address; }
        operator String() const { return String( m_address.ipAddress().toString() ); }
        operator const in_addr_t() const {
          const sockaddr_in* sin = m_address;
          return sin->sin_addr.s_addr;
        }

    private:
        Address( const QString& host, bool numeric )
        {
            int flags = 0;

            if ( numeric )
              flags = KNetwork::KResolver::NoResolve;

            KNetwork::KResolverResults addresses =
               KNetwork::KResolver::resolve( host, QString::null, flags,
                                             KNetwork::KResolver::IPv4Family );

            if ( addresses.error() || addresses.isEmpty() ) 
	      throw Error();

            m_address = addresses.first().address().asInet();
        }
         
        KInetSocketAddress m_address;
    };

    struct Function : public ObjectImp
    {
        struct ResolveError {};

        virtual bool implementsCall() const { return true; }

        static int findString( const UString& s, const char* const* values )
        {
            int index = 0;
            UString lower = s.toLower();
            for ( const char* const* p = values; *p; ++p, ++index )
                if ( lower == *p ) return index;
            return -1;
        }

        static const tm* getTime( ExecState* exec, const List& args )
        {
            time_t now = std::time( 0 );
            if ( args[ args.size() - 1 ].toString( exec ).toLower() == "gmt" )
                return std::gmtime( &now );
            else return std::localtime( &now );
        }

        Boolean checkRange( int value, int min, int max )
        {
            return ( min <= max && value >= min && value <= max ) ||
                   ( min > max && ( value <= min || value >= max ) );
        }
    };

    // isPlainHostName( host )
    // @returns true if @p host doesn't contains a domain part
    struct IsPlainHostName : public Function
    {
        virtual Value call( ExecState* exec, Object&, const List& args )
        {
            if ( args.size() != 1 ) return Undefined();
            return Boolean( args[ 0 ].toString( exec ).find( "." ) == -1 );
        }
    };

    // dnsDomainIs( host, domain )
    // @returns true if the domain part of @p host matches @p domain
    struct DNSDomainIs : public Function
    {
        virtual Value call( ExecState* exec, Object&, const List& args )
        {
            if ( args.size() != 2 ) return Undefined();
            QString host = args[ 0 ].toString( exec ).qstring().toLower();
            QString domain = args[ 1 ].toString( exec ).qstring().toLower();
            return Boolean( host.endsWith( domain ) );
        }
    };

    // localHostOrDomainIs( host, fqdn )
    // @returns true if @p host is unqualified or equals @p fqdn
    struct LocalHostOrDomainIs : public Function
    {
        virtual Value call( ExecState* exec, Object&, const List& args )
        {
            if ( args.size() != 2 ) return Undefined();
            UString host = args[ 0 ].toString( exec ).toLower();
            if ( host.find( "." ) == -1 ) return Boolean( true );
            UString fqdn = args[ 1 ].toString( exec ).toLower();
            return Boolean( host == fqdn );
        }
    };

    // isResolvable( host )
    // @returns true if host can be resolved via DNS
    struct IsResolvable : public Function
    {
        virtual Value call( ExecState* exec, Object&, const List& args )
        {
            if ( args.size() != 1 ) return Undefined();
            try { Address::resolve( args[ 0 ].toString( exec ) ); }
            catch ( const Address::Error& ) { return Boolean( false ); }
            return Boolean( true );
        }
    };

    // isInNet( host, subnet, mask )
    // @returns true if @p host is within the IP subnet
    //          specified via @p subnet and @p mask
    struct IsInNet : public Function
    {
        virtual Value call( ExecState* exec, Object&, const List& args )
        {
            if ( args.size() != 3 ) return Undefined();
            try
            {
                KInetSocketAddress host = Address::resolve( args[ 0 ].toString( exec ) );
                KInetSocketAddress subnet = Address::parse( args[ 1 ].toString( exec ) );
                KInetSocketAddress mask = Address::parse( args[ 2 ].toString( exec ) );

#warning "Write a method to do netmask comparison in the KNetwork library"
                return Boolean( ( host.ipAddress().IPv4Addr() & mask.ipAddress().IPv4Addr() ) == ( subnet.ipAddress().IPv4Addr() & mask.ipAddress().IPv4Addr() ) );
            }
            catch ( const Address::Error& )
            {
                return Undefined();
            }
        }
    };

    // dnsResolve( host )
    // @returns the IP address of @p host in dotted quad notation
    struct DNSResolve : public Function
    {
        virtual Value call( ExecState* exec, Object&, const List& args )
        {
            if ( args.size() != 1 ) return Undefined();
            try { return Address::resolve( args[ 0 ].toString( exec ) ); }
            catch ( const Address::Error& ) { return Undefined(); }
        }
    };

    // myIpAddress()
    // @returns the local machine's IP address in dotted quad notation
    struct MyIpAddress : public Function
    {
        virtual Value call( ExecState*, Object&, const List& args )
        {
            if ( args.size() ) return Undefined();
            char hostname[ 256 ];
            gethostname( hostname, 255 );
            hostname[ 255 ] = 0;
            try { return Address::resolve( hostname ); }
            catch ( const Address::Error& ) { return Undefined(); }
        }
    };

    // dnsDomainLevels( host )
    // @returns the number of dots ('.') in @p host
    struct DNSDomainLevels : public Function
    {
        virtual Value call( ExecState* exec, Object&, const List& args )
        {
            if ( args.size() != 1 ) return Undefined();
            UString host = args[ 0 ].toString( exec );
            if ( host.isNull() ) return Number( 0 );
            return Number( std::count(
                host.data(), host.data() + host.size(), '.' ) );
        }
    };

    // shExpMatch( str, pattern )
    // @returns true if @p str matches the shell @p pattern
    struct ShExpMatch : public Function
    {
        virtual Value call( ExecState* exec, Object&, const List& args )
        {
            if ( args.size() != 2 ) return Undefined();
            QRegExp pattern( args[ 1 ].toString( exec ).qstring(), true, true );
            return Boolean( pattern.exactMatch(args[ 0 ].toString( exec ).qstring()) );
        }
    };

    // weekdayRange( day [, "GMT" ] )
    // weekdayRange( day1, day2 [, "GMT" ] )
    // @returns true if the current day equals day or between day1 and day2 resp.
    // If the last argument is "GMT", GMT timezone is used, otherwise local time
    struct WeekdayRange : public Function
    {
        virtual Value call( ExecState* exec, Object&, const List& args )
        {
            if ( args.size() < 1 || args.size() > 3 ) return Undefined();
            static const char* const days[] =
                { "sun", "mon", "tue", "wed", "thu", "fri", "sat", 0 };
            int d1 = findString( args[ 0 ].toString( exec ), days );
            if ( d1 == -1 ) return Undefined();

            int d2 = findString( args[ 1 ].toString( exec ), days );
            if ( d2 == -1 ) d2 = d1;
            return checkRange( getTime( exec, args )->tm_wday, d1, d2 );
        }
    };

    // dateRange( day [, "GMT" ] )
    // dateRange( day1, day2 [, "GMT" ] )
    // dateRange( month [, "GMT" ] )
    // dateRange( month1, month2 [, "GMT" ] )
    // dateRange( year [, "GMT" ] )
    // dateRange( year1, year2 [, "GMT" ] )
    // dateRange( day1, month1, day2, month2 [, "GMT" ] )
    // dateRange( month1, year1, month2, year2 [, "GMT" ] )
    // dateRange( day1, month1, year1, day2, month2, year2 [, "GMT" ] )
    // @returns true if the current date (GMT or local time according to
    // presence of "GMT" as last argument) is within the given range
    struct DateRange : public Function
    {
        virtual Value call( ExecState* exec, Object&, const List& args )
        {
            if ( args.size() < 1 || args.size() > 7 ) return Undefined();
            static const char* const months[] =
                { "jan", "feb", "mar", "apr", "may", "jun", "jul", "aug", "nov", "dec", 0 };

            std::vector< int > values;
            for ( int i = 0; i < args.size(); ++i )
            {
                int value = -1;
                if ( args[ i ].isA( NumberType ) )
                    value = args[ i ].toInteger( exec );
                else value = findString( args[ i ].toString( exec ), months );
                if ( value >= 0 ) values.push_back( value );
                else break;
            }

            const tm* now = getTime( exec, args );

            // day1, month1, year1, day2, month2, year2
            if ( values.size() == 6 )
                return checkRange( ( now->tm_year + 1900 ) * 372 + now->tm_mon * 31 + now->tm_mday,
                                   values[ 2 ] * 372 + values[ 1 ] * 31 + values[ 0 ],
                                   values[ 5 ] * 372 + values[ 4 ] * 31 + values[ 3 ] );

            // day1, month1, day2, month2
            else if ( values.size() == 4 &&
                      values[ 1 ] < 12 &&
                      values[ 3 ] < 12 )
                return checkRange( now->tm_mon * 31 + now->tm_mday,
                                   values[ 1 ] * 31 + values[ 0 ],
                                   values[ 3 ] * 31 + values[ 2 ] );

            // month1, year1, month2, year2
            else if ( values.size() == 4 )
                return checkRange( ( now->tm_year + 1900 ) * 12 + now->tm_mon,
                                   values[ 1 ] * 12 + values[ 0 ],
                                   values[ 3 ] * 12 + values[ 2 ] );

            // year1, year2
            else if ( values.size() == 2 &&
                      values[ 0 ] >= 1000 &&
                      values[ 1 ] >= 1000 )
                return checkRange( now->tm_year + 1900, values[ 0 ], values[ 1 ] );

            // day1, day2
            else if ( values.size() == 2 &&
                      args[ 0 ].isA( NumberType ) &&
                      args[ 1 ].isA( NumberType ) )
                return checkRange( now->tm_mday, values[ 0 ], values[ 1 ] );

            // month1, month2
            else if ( values.size() == 2 )
                return checkRange( now->tm_mon, values[ 0 ], values[ 1 ] );

            // year
            else if ( values.size() == 1 && values[ 0 ] >= 1000 )
                return checkRange( now->tm_year + 1900, values[ 0 ], values[ 0 ] );

            // day
            else if ( values.size() == 1 && args[ 0 ].isA( NumberType ) )
                return checkRange( now->tm_mday, values[ 0 ], values[ 0 ] );

            // month
            else if ( values.size() == 1 )
                return checkRange( now->tm_mon, values[ 0 ], values[ 0 ] );

            else return Undefined();
        }
    };

    // timeRange( hour [, "GMT" ] )
    // timeRange( hour1, hour2 [, "GMT" ] )
    // timeRange( hour1, min1, hour2, min2 [, "GMT" ] )
    // timeRange( hour1, min1, sec1, hour2, min2, sec2 [, "GMT" ] )
    // @returns true if the current time (GMT or local based on presence
    // of "GMT" argument) is within the given range
    struct TimeRange : public Function
    {
        virtual Value call( ExecState* exec, Object&, const List& args )
        {
            if ( args.size() < 1 || args.size() > 7 ) return Undefined();

            std::vector< int > values;
            for ( int i = 0; i < args.size(); ++i )
                if ( args[ i ].isA( NumberType ) )
                    values.push_back( args[ i ].toInteger( exec ) );
                else break;

            const tm* now = getTime( exec, args );

            // hour1, min1, sec1, hour2, min2, sec2
            if ( values.size() == 6 )
                return checkRange( now->tm_hour * 3600 + now->tm_min * 60 + now->tm_sec,
                                   values[ 0 ] * 3600 + values[ 1 ] * 60 + values[ 2 ],
                                   values[ 3 ] * 3600 + values[ 4 ] * 60 + values[ 5 ] );

            // hour1, min1, hour2, min2
            else if ( values.size() == 4 )
                return checkRange( now->tm_hour * 60 + now->tm_min,
                                   values[ 0 ] * 60 + values[ 1 ],
                                   values[ 2 ] * 60 + values[ 3 ] );

            // hour1, hour2
            else if ( values.size() == 2 )
                return checkRange( now->tm_hour, values[ 0 ], values[ 1 ] );

            // hour
            else if ( values.size() == 1 )
                return checkRange( now->tm_hour, values[ 0 ], values[ 0 ] );

            else return Undefined();
        }
    };

    void registerFunctions( ExecState* exec, Object& global )
    {
        global.put( exec, "isPlainHostName",
                    Object( new IsPlainHostName ) );
        global.put( exec, "dnsDomainIs",
                    Object( new DNSDomainIs ) );
        global.put( exec, "localHostOrDomainIs",
                    Object( new LocalHostOrDomainIs ) );
        global.put( exec, "isResolvable",
                    Object( new IsResolvable ) );
        global.put( exec, "isInNet",
                    Object( new IsInNet ) );
        global.put( exec, "dnsResolve",
                    Object( new DNSResolve ) );
        global.put( exec, "myIpAddress",
                    Object( new MyIpAddress ) );
        global.put( exec, "dnsDomainLevels",
                    Object( new DNSDomainLevels ) );
        global.put( exec, "shExpMatch",
                    Object( new ShExpMatch ) );
        global.put( exec, "weekdayRange",
                    Object( new WeekdayRange ) );
        global.put( exec, "dateRange",
                    Object( new DateRange ) );
        global.put( exec, "timeRange",
                    Object( new TimeRange ) );
    }
}

namespace KPAC
{
    Script::Script( const QString& code )
    {
        ExecState* exec = m_interpreter.globalExec();
        Object global = m_interpreter.globalObject();
        registerFunctions( exec, global );

        Completion result = m_interpreter.evaluate( code );
        if ( result.complType() == Throw )
            throw Error( result.value().toString( exec ).qstring() );
    }

    QString Script::evaluate( const KURL& url )
    {
        ExecState *exec = m_interpreter.globalExec();
        Value findFunc = m_interpreter.globalObject().get( exec, "FindProxyForURL" );
        Object findObj = Object::dynamicCast( findFunc );
        if (!findObj.isValid() || !findObj.implementsCall())
            throw Error( "No such function FindProxyForURL" );
    
        Object thisObj;
        List args;
        args.append(String(url.url()));
        args.append(String(url.host()));
        Value retval = findObj.call( exec, thisObj, args );
        
        if ( exec->hadException() ) {
            Value ex = exec->exception();
            exec->clearException();
            throw Error( ex.toString( exec ).qstring() );
        }

        return retval.toString( exec ).qstring();
    }
}

// vim: ts=4 sw=4 et
