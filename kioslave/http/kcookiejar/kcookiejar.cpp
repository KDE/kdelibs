/* This file is part of the KDE File Manager

   Copyright (C) 1998-2000 Waldo Bastian (bastian@kde.org)
   Copyright (C) 2000,2001 Dawit Alemayehu (adawit@kde.org)

   Permission is hereby granted, free of charge, to any person obtaining a copy
   of this software and associated documentation files (the "Software"), to deal
   in the Software without restriction, including without limitation the rights
   to use, copy, modify, merge, publish, distribute, and/or sell copies of the
   Software, and to permit persons to whom the Software is furnished to do so,
   subject to the following conditions:

   The above copyright notice and this permission notice shall be included in
   all copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
   AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
   ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
   WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/
//----------------------------------------------------------------------------
//
// KDE File Manager -- HTTP Cookies

//
// The cookie protocol is a mess. RFC2109 is a joke since nobody seems to
// use it. Apart from that it is badly written.
// We try to implement Netscape Cookies and try to behave us according to
// RFC2109 as much as we can.
//
// We assume cookies do not contain any spaces (Netscape spec.)
// According to RFC2109 this is allowed though.
//

#include "kcookiejar.h"

#include <config.h>
#include <sys/types.h>
#include <sys/stat.h>
#ifdef HAVE_SYS_PARAM_H
#include <sys/param.h>
#endif
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>

#ifdef USE_SOLARIS
#include <strings.h>
#endif

#include <stdlib.h>

//#include <netinet/in.h>
//#include <arpa/inet.h>

#include <QtCore/QString>
#include <QtCore/QFile>
#include <QtCore/QDir>
#include <QtCore/QRegExp>
#include <QtCore/QTextStream>

#include <kurl.h>
#include <kdatetime.h>
#include <ksystemtimezone.h>
#include <kconfig.h>
#include <kconfiggroup.h>
#include <ksavefile.h>
#include <kdebug.h>

#include <algorithm>

// BR87227
// Waba: Should the number of cookies be limited?
// I am not convinced of the need of such limit
// Mozilla seems to limit to 20 cookies / domain
// but it is unclear which policy it uses to expire
// cookies when it exceeds that amount
#undef MAX_COOKIE_LIMIT

#define MAX_COOKIES_PER_HOST 25
#define READ_BUFFER_SIZE 8192
#define IP_ADDRESS_EXPRESSION "(?:(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\.){3}(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)"

// Note with respect to QString::fromLatin1( )
// Cookies are stored as 8 bit data and passed to kio_http as
// latin1 regardless of their actual encoding.

// L1 is used to indicate latin1 constants
#define QL1S(x) QLatin1String(x)

QString KCookieJar::adviceToStr(KCookieAdvice _advice)
{
    switch( _advice )
    {
    case KCookieAccept: return QL1S("Accept");
    case KCookieReject: return QL1S("Reject");
    case KCookieAsk: return QL1S("Ask");
    default: return QL1S("Dunno");
    }
}

KCookieAdvice KCookieJar::strToAdvice(const QString &_str)
{
    if (_str.isEmpty())
        return KCookieDunno;

    QString advice = _str.toLower();

    if (advice == QL1S("accept"))
        return KCookieAccept;
    else if (advice == QL1S("reject"))
        return KCookieReject;
    else if (advice == QL1S("ask"))
        return KCookieAsk;

    return KCookieDunno;
}

// KHttpCookie
///////////////////////////////////////////////////////////////////////////

//
// Cookie constructor
//
KHttpCookie::KHttpCookie(const QString &_host,
                 const QString &_domain,
                 const QString &_path,
                 const QString &_name,
                 const QString &_value,
                 qint64 _expireDate,
                 int _protocolVersion,
                 bool _secure,
                 bool _httpOnly,
                 bool _explicitPath) :
       mHost(_host),
       mDomain(_domain),
       mPath(_path.isEmpty() ? QString() : _path),
       mName(_name),
       mValue(_value),
       mExpireDate(_expireDate),
       mProtocolVersion(_protocolVersion),
       mSecure(_secure),
       mHttpOnly(_httpOnly),
       mExplicitPath(_explicitPath)
{
}

//
// Checks if a cookie has been expired
//
bool    KHttpCookie::isExpired(qint64 currentDate) const
{
    if (currentDate == -1) {
        KDateTime epoch;
        epoch.setTime_t(0);
        currentDate = epoch.secsTo_long(KDateTime::currentUtcDateTime());
    }

    return (mExpireDate != 0) && (mExpireDate < currentDate);
}

//
// Returns a string for a HTTP-header
//
QString KHttpCookie::cookieStr(bool useDOMFormat) const
{
    QString result;

    if (useDOMFormat || (mProtocolVersion == 0))
    {
        if ( !mName.isEmpty() )
           result = mName + '=';
        result += mValue;
    }
    else
    {
        result = mName + '=' + mValue;
        if (mExplicitPath)
            result += QL1S("; $Path=\"") + mPath + QL1S("\"");
        if (!mDomain.isEmpty())
            result += QL1S("; $Domain=\"") + mDomain + QL1S("\"");
    }
    return result;
}

//
// Returns whether this cookie should be send to this location.
bool KHttpCookie::match(const QString &fqdn, const QStringList &domains,
                        const QString &path) const
{
    // Cookie domain match check
    if (mDomain.isEmpty())
    {
       if (fqdn != mHost)
          return false;
    }
    else if (!domains.contains(mDomain))
    {
        if (mDomain[0] == '.')
            return false;

        // Maybe the domain needs an extra dot.
        QString domain = '.' + mDomain;
        if ( !domains.contains( domain ) )
          if ( fqdn != mDomain )
            return false;
    }

    // Cookie path match check
    if (mPath.isEmpty())
        return true;

    // According to the netscape spec both http://www.acme.com/foobar,
    // http://www.acme.com/foo.bar and http://www.acme.com/foo/bar
    // match http://www.acme.com/foo.
    // We only match http://www.acme.com/foo/bar

    if( path.startsWith(mPath) &&
        (
         (path.length() == mPath.length() ) || 	// Paths are exact match
          mPath.endsWith('/') || 	        // mPath ended with a slash
         (path[mPath.length()] == '/')		// A slash follows
         ))
        return true; // Path of URL starts with cookie-path

    return false;
}

// KCookieJar
///////////////////////////////////////////////////////////////////////////

//
// Constructs a new cookie jar
//
// One jar should be enough for all cookies.
//
KCookieJar::KCookieJar()
{
    m_globalAdvice = KCookieDunno;
    m_configChanged = false;
    m_cookiesChanged = false;

    KConfig cfg( "khtml/domain_info", KConfig::NoGlobals, "data" );
    KConfigGroup group( &cfg, QString() );
    QStringList countries = group.readEntry( "twoLevelTLD", QStringList() );
    Q_FOREACH ( const QString& country, countries ) {
       m_twoLevelTLD.insert( country, 1 );
    }
}

//
// Destructs the cookie jar
//
// Poor little cookies, they will all be eaten by the cookie monster!
//
KCookieJar::~KCookieJar()
{
    qDeleteAll(m_cookieDomains);
    // Not much to do here
}

// cookiePtr is modified: the window ids of the existing cookie in the list are added to it
static void removeDuplicateFromList(KHttpCookieList *list, KHttpCookie& cookiePtr, bool nameMatchOnly=false, bool updateWindowId=false)
{
    QString domain1 = cookiePtr.domain();
    if (domain1.isEmpty())
       domain1 = cookiePtr.host();

    QMutableListIterator<KHttpCookie> cookieIterator(*list);
    while (cookieIterator.hasNext()) {
        const KHttpCookie& cookie = cookieIterator.next();
        QString domain2 = cookie.domain();
        if (domain2.isEmpty())
            domain2 = cookie.host();

        if (
            (cookiePtr.name() == cookie.name()) &&
            (
              nameMatchOnly ||
              ( (domain1 == domain2) && (cookiePtr.path() == cookie.path()) )
            )
          ) {
            if (updateWindowId) {
                Q_FOREACH(long windowId, cookie.windowIds()) {
                    if (windowId && (!cookiePtr.windowIds().contains(windowId))) {
                        cookiePtr.windowIds().append(windowId);
                    }
                }
            }
            cookieIterator.remove();
            break;
        }
    }
}


//
// Looks for cookies in the cookie jar which are appropriate for _url.
// Returned is a string containing all appropriate cookies in a format
// which can be added to a HTTP-header without any additional processing.
//
QString KCookieJar::findCookies(const QString &_url, bool useDOMFormat, long windowId, KHttpCookieList *pendingCookies)
{
    QString cookieStr;
    QStringList domains;
    QString fqdn;
    QString path;
    KCookieAdvice advice = m_globalAdvice;

    if (!parseUrl(_url, fqdn, path))
        return cookieStr;

    bool secureRequest = _url.startsWith( QL1S("https://"), Qt::CaseInsensitive ) ||
                         _url.startsWith( QL1S("webdavs://"), Qt::CaseInsensitive );

    extractDomains(fqdn, domains);

    KHttpCookieList allCookies;

    for(QStringList::ConstIterator it = domains.constBegin();
        true;
        ++it)
    {
       KHttpCookieList *cookieList;
       if (it == domains.constEnd())
       {
          cookieList = pendingCookies; // Add pending cookies
          pendingCookies = 0;
          if (!cookieList)
             break;
       }
       else
       {
          QString key = (*it).isNull() ? QL1S("") : (*it);
          cookieList = m_cookieDomains.value(key);
          if (!cookieList)
             continue; // No cookies for this domain
       }

       if (cookieList->getAdvice() != KCookieDunno)
          advice = cookieList->getAdvice();

        for (KHttpCookieList::iterator cookieIterator = cookieList->begin();
             cookieIterator != cookieList->end();
             ++cookieIterator ) {
            KHttpCookie& cookie = *cookieIterator;
            // If the we are setup to automatically accept all session cookies and to
            // treat all cookies as session cookies or the current cookie is a session
            // cookie, then send the cookie back regardless of domain policy.
            if (advice == KCookieReject &&
                !(m_autoAcceptSessionCookies &&
                  (m_ignoreCookieExpirationDate || cookie.expireDate() == 0))) {
                continue;
            }

          if (!cookie.match(fqdn, domains, path))
             continue;

          if( cookie.isSecure() && !secureRequest ) {
             continue;
          }

          if( cookie.isHttpOnly() && useDOMFormat ) {
             continue;
          }

          // Do not send expired cookies.
          if ( cookie.isExpired())
          {
             // Note there is no need to actually delete the cookie here
             // since the cookieserver will invoke ::saveCookieJar because
             // of the state change below. This will then do the job of
             // deleting the cookie for us.
             m_cookiesChanged = true;
             continue;
          }

          if (windowId && (cookie.windowIds().indexOf(windowId) == -1))
          {
             cookie.windowIds().append(windowId);
          }

          if (it == domains.constEnd()) // Only needed when processing pending cookies
             removeDuplicateFromList(&allCookies, cookie);

          allCookies.append(cookie);
       }
       if (it == domains.constEnd())
          break; // Finished.
    }

    int cookieCount = 0;

    int protVersion=0;
    Q_FOREACH(const KHttpCookie& cookie, allCookies) {
        if (cookie.protocolVersion() > protVersion)
            protVersion = cookie.protocolVersion();
    }

    Q_FOREACH(const KHttpCookie& cookie, allCookies) {
        if (useDOMFormat) {
            if (cookieCount > 0)
                cookieStr += QL1S("; ");
            cookieStr += cookie.cookieStr(true);
        } else {
            if (cookieCount == 0) {
                cookieStr += QL1S("Cookie: ");
                if (protVersion > 0) {
                    QString version;
                    version.sprintf("$Version=%d; ", protVersion); // Without quotes
                    cookieStr += version;
                }
            } else {
                cookieStr += QL1S("; ");
            }
            cookieStr += cookie.cookieStr(false);
        }
        cookieCount++;
    }

    return cookieStr;
}

//
// This function parses a string like 'my_name="my_value";' and returns
// 'my_name' in Name and 'my_value' in Value.
//
// A pointer to the end of the parsed part is returned.
// This pointer points either to:
// '\0' - The end of the string has reached.
// ';'  - Another my_name="my_value" pair follows
// ','  - Another cookie follows
// '\n' - Another header follows
static const char * parseNameValue(const char *header,
                                  QString &Name,
                                  QString &Value,
                                  bool keepQuotes=false,
                                  bool rfcQuotes=false)
{
    const char *s = header;
    // Parse 'my_name' part
    for(; (*s != '='); s++)
    {
        if ((*s=='\0') || (*s==';') || (*s=='\n'))
        {
            // No '=' sign -> use string as the value, name is empty
            // (behavior found in Mozilla and IE)
            Name = "";
            Value = QString::fromLatin1(header);
            Value.truncate( s - header );
            Value = Value.trimmed();
            return (s);
        }
    }

    Name = header;
    Name.truncate( s - header );
    Name = Name.trimmed();

    // *s == '='
    s++;

    // Skip any whitespace
    for(; (*s == ' ') || (*s == '\t'); s++)
    {
        if ((*s=='\0') || (*s==';') || (*s=='\n'))
        {
            // End of Name
            Value = "";
            return (s);
        }
    }

    if ((rfcQuotes || !keepQuotes) && (*s == '\"'))
    {
        // Parse '"my_value"' part (quoted value)
        if (keepQuotes)
           header = s++;
        else
           header = ++s; // skip "
        for(;(*s != '\"');s++)
        {
            if ((*s=='\0') || (*s=='\n'))
            {
                // End of Name
                Value = QString::fromLatin1(header);
                Value.truncate(s - header);
                return (s);
            }
        }
        Value = QString::fromLatin1(header);
        // *s == '\"';
        if (keepQuotes)
           Value.truncate( ++s - header );
        else
           Value.truncate( s++ - header );

        // Skip any remaining garbage
        for(;; s++)
        {
            if ((*s=='\0') || (*s==';') || (*s=='\n'))
                break;
        }
    }
    else
    {
        // Parse 'my_value' part (unquoted value)
        header = s;
        while ((*s != '\0') && (*s != ';') && (*s != '\n'))
            s++;
        // End of Name
        Value = QString::fromLatin1(header);
        Value.truncate( s - header );
        Value = Value.trimmed();
    }
    return (s);

}

void KCookieJar::stripDomain(const QString &_fqdn, QString &_domain)
{
   QStringList domains;
   extractDomains(_fqdn, domains);
   if (domains.count() > 3)
      _domain = domains[3];
   else if ( domains.count() > 0 )
      _domain = domains[0];
   else
      _domain = QL1S("");
}

QString KCookieJar::stripDomain(const KHttpCookie& cookie)
{
    QString domain; // We file the cookie under this domain.
    if (cookie.domain().isEmpty())
       stripDomain( cookie.host(), domain);
    else
       stripDomain( cookie.domain(), domain);
    return domain;
}

bool KCookieJar::parseUrl(const QString &_url,
                          QString &_fqdn,
                          QString &_path)
{
    KUrl kurl(_url);
    if (!kurl.isValid() || kurl.protocol().isEmpty())
       return false;

    _fqdn = kurl.host().toLower();
    if (kurl.port() > 0)
    {
       if (((kurl.protocol() == QL1S("http")) && (kurl.port() != 80)) ||
           ((kurl.protocol() == QL1S("https")) && (kurl.port() != 443)))
       {
          // It's <port>:<host> so that the sorting works as expected
          _fqdn = QString::fromLatin1("%1:%2").arg(kurl.port()).arg(_fqdn);
       }
    }

    // Cookie spoofing protection.  Since there is no way a path separator
    // or escape encoded character is allowed in the hostname according
    // to RFC 2396, reject attempts to include such things there!
    if(_fqdn.contains('/') || _fqdn.contains('%'))
    {
        return false;  // deny everything!!
    }

    _path = kurl.path();
    if (_path.isEmpty())
       _path = QL1S("/");

    QRegExp exp(QL1S("[\\\\/]\\.\\.[\\\\/]"));
    // Weird path, cookie stealing attempt?
    if (exp.indexIn(_path) != -1)
       return false; // Deny everything!!

    return true;
}

// not static because it uses m_twoLevelTLD
void KCookieJar::extractDomains(const QString &_fqdn,
                                QStringList &_domains) const
{
    if (_fqdn.isEmpty()) {
        _domains.append( QL1S("localhost") );
        return;
    }

    // Return numeric IPv6 addresses as is...
    if (_fqdn[0] == '[')
    {
       _domains.append( _fqdn );
       return;
    }
    // Return numeric IPv4 addresses as is...
    if ((_fqdn[0] >= '0') && (_fqdn[0] <= '9'))
    {
       if (_fqdn.indexOf(QRegExp(IP_ADDRESS_EXPRESSION)) > -1)
       {
          _domains.append( _fqdn );
          return;
       }
    }

    QStringList partList = _fqdn.split('.', QString::SkipEmptyParts);

    if (partList.count())
        partList.erase(partList.begin()); // Remove hostname

    while(partList.count())
    {

       if (partList.count() == 1)
         break; // We only have a TLD left.

       if ((partList.count() == 2) && (m_twoLevelTLD.value(partList[1].toLower(), 0) == 1))
       {
          // This domain uses two-level TLDs in the form xxxx.yy
          break;
       }

       if ((partList.count() == 2) && (partList[1].length() == 2))
       {
          // If this is a TLD, we should stop. (e.g. co.uk)
          // We assume this is a TLD if it ends with .xx.yy or .x.yy
          if (partList[0].length() <= 2)
             break; // This is a TLD.

          // Catch some TLDs that we miss with the previous check
          // e.g. com.au, org.uk, mil.co
          const QString t = partList[0].toLower();
          if ((t == "com") || (t == "net") || (t == "org") || (t == "gov") || (t == "edu") || (t == "mil") || (t == "int"))
              break;
       }

       QString domain = partList.join(QL1S("."));
       _domains.append(domain);
       _domains.append('.' + domain);
       partList.erase(partList.begin()); // Remove part
    }

    // Always add the FQDN at the start of the list for
    // hostname == cookie-domainname checks!
    _domains.prepend( '.' + _fqdn );
    _domains.prepend( _fqdn );
}

//
// This function parses cookie_headers and returns a linked list of
// KHttpCookie objects for all cookies found in cookie_headers.
// If no cookies could be found 0 is returned.
//
// cookie_headers should be a concatenation of all lines of a HTTP-header
// which start with "Set-Cookie". The lines should be separated by '\n's.
//
KHttpCookieList KCookieJar::makeCookies(const QString &_url,
                                       const QByteArray &cookie_headers,
                                       long windowId)
{
    KHttpCookieList cookieList;
    KHttpCookieList cookieList2;
    KHttpCookieList::iterator lastCookie = cookieList.end();
    const char *cookieStr = cookie_headers.data();
    QString Name;
    QString Value;
    QString fqdn;
    QString path;
    bool crossDomain = false;

    if (!parseUrl(_url, fqdn, path))
    {
        // Error parsing _url
        return KHttpCookieList();
    }
    QString defaultPath;
    int i = path.lastIndexOf('/');
    if (i > 0)
       defaultPath = path.left(i);

    KDateTime epoch;
    epoch.setTime_t(0);

    //  The hard stuff :)
    for(;;)
    {
        // check for "Set-Cookie"
        if (strncmp(cookieStr, "Cross-Domain\n", 13) == 0)
        {
            cookieStr += 13;
            crossDomain = true;
        }
        else if (strncasecmp(cookieStr, "Set-Cookie:", 11) == 0)
        {
            cookieStr = parseNameValue(cookieStr+11, Name, Value, true);

            // Host = FQDN
            // Default domain = ""
            // Default path according to rfc2109

            KHttpCookie cookie(fqdn, QL1S(""), defaultPath, Name, Value);
            if (windowId)
               cookie.mWindowIds.append(windowId);
            cookie.mCrossDomain = crossDomain;

            // Insert cookie in chain
            cookieList.append(cookie);
            lastCookie = cookieList.end(); --lastCookie;            
        }
        else if (strncasecmp(cookieStr, "Set-Cookie2:", 12) == 0)
        {
            // Attempt to follow rfc2965
            cookieStr = parseNameValue(cookieStr+12, Name, Value, true, true);

            // Host = FQDN
            // Default domain = ""
            // Default path according to rfc2965

            KHttpCookie cookie(fqdn, QL1S(""), defaultPath, Name, Value);
            if (windowId)
               cookie.mWindowIds.append(windowId);
            cookie.mCrossDomain = crossDomain;

            // Insert cookie in chain
            cookieList2.append(cookie);
            lastCookie = cookieList2.end(); --lastCookie;
        }
        else
        {
            // This is not the start of a cookie header, skip till next line.
            while (*cookieStr && *cookieStr != '\n')
                cookieStr++;

            if (*cookieStr == '\n')
                cookieStr++;

            if (!*cookieStr)
                break; // End of cookie_headers
            else
                continue; // end of this header, continue with next.
        }

        while ((*cookieStr == ';') || (*cookieStr == ' '))
        {
            cookieStr++;

            // Name-Value pair follows
            cookieStr = parseNameValue(cookieStr, Name, Value);

            QString cName = Name.toLower();
            if (cName == "domain")
            {
                QString dom = Value.toLower();
                // RFC2965 3.2.2: If an explicitly specified value does not
                // start with a dot, the user agent supplies a leading dot
                if(dom.length() && dom[0] != '.')
                    dom.prepend(".");
                // remove a trailing dot
                if(dom.length() > 2 && dom[dom.length()-1] == '.')
                    dom = dom.left(dom.length()-1);

                if(dom.count('.') > 1 || dom == ".local")
                    lastCookie->mDomain = dom;
            }
            else if (cName == "max-age")
            {
                int max_age = Value.toInt();
                if (max_age == 0)
                    lastCookie->mExpireDate = 1;
                else
                    lastCookie->mExpireDate = epoch.secsTo_long(KDateTime::currentUtcDateTime().addSecs(max_age));
            }
            else if (cName == "expires")
            {
                KTimeZones *zones = KSystemTimeZones::timeZones();
                // Check for the most common cookie expire date format: Thu, 01-Jan-1970 00:00:00 GMT
                KDateTime dt = KDateTime::fromString(Value, QL1S("%:A,%t%d-%:B-%Y%t%H:%M:%S%t%Z"), zones);
                if (!dt.isValid()) {
                    // Check for a variation of the above format: Thu, 01 Jan 1970 00:00:00 GMT
                    dt = KDateTime::fromString(Value, QL1S("%:A,%t%d%t%:B%t%Y%t%H:%M:%S%t%Z"), zones);
                    if (!dt.isValid()) {
                        // Check for incorrect formats (amazon.com): Thu Jan 01 1970 00:00:00 GMT
                        dt = KDateTime::fromString(Value, QL1S("%:A%t%:B%t%d%t%Y%t%H:%M:%S%t%Z"), zones);
                        if (!dt.isValid()) {
                            // Check for a variation of the above format: Thu Jan 01 00:00:00 1970 GMT (BR# 145244)
                            dt = KDateTime::fromString(Value, QL1S("%:A%t%:B%t%d%t%H:%M:%S%t%Y%t%Z"), zones);
                            if (!dt.isValid()) {
                                // Finally we try the RFC date formats as last resort
                                dt = KDateTime::fromString(Value, KDateTime::RFCDate);
                            }
                        }
                    }
                }

                if (dt.isValid()) {
                    lastCookie->mExpireDate = epoch.secsTo_long(dt);
                    if (lastCookie->mExpireDate == 0)
                        lastCookie->mExpireDate = 1;
                }
            }
            else if (cName == "path")
            {
                if (Value.isEmpty())
                   lastCookie->mPath.clear(); // Catch "" <> QString()
                else
                   lastCookie->mPath = QUrl::fromPercentEncoding(Value.toLatin1());
                lastCookie->mExplicitPath = true;
            }
            else if (cName == "version")
            {
                lastCookie->mProtocolVersion = Value.toInt();
            }
            else if ((cName == "secure") ||
                     (cName.isEmpty() && Value.toLower() == QL1S("secure")))
            {
                lastCookie->mSecure = true;
            }
            else if ((cName == "httponly") ||
                     (cName.isEmpty() && Value.toLower() == QL1S("httponly")))
            {
                lastCookie->mHttpOnly = true;
            }
        }

        if (*cookieStr == '\0')
            break; // End of header

        // Skip ';' or '\n'
        cookieStr++;
    }

    // RFC2965 cookies come last so that they override netscape cookies.
    while(!cookieList2.isEmpty()) {
        lastCookie = cookieList2.begin();
        removeDuplicateFromList(&cookieList, *lastCookie, true);
        cookieList.append(*lastCookie);
        cookieList2.removeFirst();
    }

    return cookieList;
}

/**
* Parses cookie_domstr and returns a linked list of KHttpCookie objects.
* cookie_domstr should be a semicolon-delimited list of "name=value"
* pairs. Any whitespace before "name" or around '=' is discarded.
* If no cookies are found, 0 is returned.
*/
KHttpCookieList KCookieJar::makeDOMCookies(const QString &_url,
                                           const QByteArray &cookie_domstring,
                                           long windowId)
{
    // A lot copied from above
    KHttpCookieList cookieList;

    const char *cookieStr = cookie_domstring.data();
    QString fqdn;
    QString path;

    if (!parseUrl(_url, fqdn, path))
    {
        // Error parsing _url
        return KHttpCookieList();
    }

    QString Name;
    QString Value;
    //  This time it's easy
    while(*cookieStr)
    {
        cookieStr = parseNameValue(cookieStr, Name, Value);

        // Host = FQDN
        // Default domain = ""
        // Default path = ""
        KHttpCookie cookie(fqdn, QString(), QString(),
                           Name, Value );
        if (windowId)
            cookie.mWindowIds.append(windowId);

        cookieList.append(cookie);

        if (*cookieStr != '\0')
            cookieStr++;         // Skip ';' or '\n'
    }

    return cookieList;
}

// KHttpCookieList sorting
///////////////////////////////////////////////////////////////////////////

// We want the longest path first
static bool compareCookies(const KHttpCookie& item1, const KHttpCookie& item2)
{
    return item1.path().length() > item2.path().length();
}


#ifdef MAX_COOKIE_LIMIT
static void makeRoom(KHttpCookieList *cookieList, KHttpCookiePtr &cookiePtr)
{
     // Too many cookies: throw one away, try to be somewhat clever
     KHttpCookiePtr lastCookie = 0;
     for(KHttpCookiePtr cookie = cookieList->first(); cookie; cookie = cookieList->next())
     {
         if (compareCookies(cookie, cookiePtr))
            break;
         lastCookie = cookie;
     }
     if (!lastCookie)
         lastCookie = cookieList->first();
     cookieList->removeRef(lastCookie);
}
#endif

//
// This function hands a KHttpCookie object over to the cookie jar.
//
void KCookieJar::addCookie(KHttpCookie &cookie)
{
    QStringList domains;
    KHttpCookieList *cookieList = 0L;

    // We always need to do this to make sure that the
    // that cookies of type hostname == cookie-domainname
    // are properly removed and/or updated as necessary!
    extractDomains( cookie.host(), domains );
    for ( QStringList::ConstIterator it = domains.constBegin();
          (it != domains.constEnd() && !cookieList);
          ++it )
    {
        QString key = (*it).isNull() ? QString::fromLatin1("") : (*it);
        KHttpCookieList *list= m_cookieDomains.value(key);
        if ( !list ) continue;

        removeDuplicateFromList(list, cookie, false, true);
    }

    QString domain = stripDomain( cookie );
    QString key = domain.isNull() ? QString::fromLatin1("") : domain;
    cookieList = m_cookieDomains.value(key);
    if (!cookieList)
    {
        // Make a new cookie list
        cookieList = new KHttpCookieList();

        // All cookies whose domain is not already
        // known to us should be added with KCookieDunno.
        // KCookieDunno means that we use the global policy.
        cookieList->setAdvice( KCookieDunno );

        m_cookieDomains.insert( domain, cookieList);

        // Update the list of domains
        m_domainList.append(domain);
    }

    // Add the cookie to the cookie list
    // The cookie list is sorted 'longest path first'
    if (!cookie.isExpired())
    {
#ifdef MAX_COOKIE_LIMIT
        if (cookieList->count() >= MAX_COOKIES_PER_HOST)
           makeRoom(cookieList, cookie); // Delete a cookie
#endif
        cookieList->push_back(cookie);
        // Use a stable sort so that unit tests are reliable.
        // In practice it doesn't matter though.
        qStableSort(cookieList->begin(), cookieList->end(), compareCookies);

        m_cookiesChanged = true;
    }
}

//
// This function advices whether a single KHttpCookie object should
// be added to the cookie jar.
//
KCookieAdvice KCookieJar::cookieAdvice(KHttpCookie& cookie)
{
    if (m_rejectCrossDomainCookies && cookie.isCrossDomain())
       return KCookieReject;

    QStringList domains;
    extractDomains(cookie.host(), domains);

    // If the cookie specifies a domain, check whether it is valid. Otherwise,
    // accept the cookie anyways but removes the domain="" value to prevent
    // cross-site cookie injection.
    if (!cookie.domain().isEmpty())
    {
      if (!domains.contains(cookie.domain()) &&
          !cookie.domain().endsWith('.'+cookie.host()))
         cookie.fixDomain(QString());
    }

    if (m_autoAcceptSessionCookies && (cookie.expireDate() == 0 ||
        m_ignoreCookieExpirationDate))
       return KCookieAccept;

    KCookieAdvice advice = KCookieDunno;
    bool isFQDN = true; // First is FQDN
    QStringList::Iterator it = domains.begin(); // Start with FQDN which first in the list.
    while( (advice == KCookieDunno) && (it != domains.end()))
    {
       QString domain = *it;
       // Check if a policy for the FQDN/domain is set.
       if ( domain.startsWith('.') || isFQDN )
       {
          isFQDN = false;
          KHttpCookieList *cookieList = m_cookieDomains.value(domain);
          if (cookieList)
             advice = cookieList->getAdvice();
       }
       domains.erase(it);
       it = domains.begin(); // Continue from begin of remaining list
    }

    if (advice == KCookieDunno)
        advice = m_globalAdvice;

    return advice;
}

//
// This function gets the advice for all cookies originating from
// _domain.
//
KCookieAdvice KCookieJar::getDomainAdvice(const QString &_domain)
{
    KHttpCookieList *cookieList = m_cookieDomains.value(_domain);
    KCookieAdvice advice;

    if (cookieList)
    {
        advice = cookieList->getAdvice();
    }
    else
    {
        advice = KCookieDunno;
    }

    return advice;
}

//
// This function sets the advice for all cookies originating from
// _domain.
//
void KCookieJar::setDomainAdvice(const QString &_domain, KCookieAdvice _advice)
{
    QString domain(_domain);
    KHttpCookieList *cookieList = m_cookieDomains.value(domain);

    if (cookieList)
    {
        if (cookieList->getAdvice() != _advice)
        {
           m_configChanged = true;
           // domain is already known
           cookieList->setAdvice( _advice);
        }

        if ((cookieList->isEmpty()) &&
            (_advice == KCookieDunno))
        {
            // This deletes cookieList!
            delete m_cookieDomains.take(domain);
            m_domainList.removeAll(domain);
        }
    }
    else
    {
        // domain is not yet known
        if (_advice != KCookieDunno)
        {
            // We should create a domain entry
            m_configChanged = true;
            // Make a new cookie list
            cookieList = new KHttpCookieList();
            cookieList->setAdvice(_advice);
            m_cookieDomains.insert(domain, cookieList);
            // Update the list of domains
            m_domainList.append( domain);
        }
    }
}

//
// This function sets the advice for all cookies originating from
// the same domain as _cookie
//
void KCookieJar::setDomainAdvice(const KHttpCookie& cookie, KCookieAdvice _advice)
{
    QString domain;
    stripDomain(cookie.host(), domain); // We file the cookie under this domain.

    setDomainAdvice(domain, _advice);
}

//
// This function sets the global advice for cookies
//
void KCookieJar::setGlobalAdvice(KCookieAdvice _advice)
{
    if (m_globalAdvice != _advice)
       m_configChanged = true;
    m_globalAdvice = _advice;
}

//
// Get a list of all domains known to the cookie jar.
//
const QStringList& KCookieJar::getDomainList()
{
    return m_domainList;
}

//
// Get a list of all cookies in the cookie jar originating from _domain.
//
KHttpCookieList *KCookieJar::getCookieList(const QString & _domain,
                                           const QString & _fqdn )
{
    QString domain;

    if (_domain.isEmpty())
        stripDomain(_fqdn, domain);
    else
        stripDomain (_domain, domain);

    return m_cookieDomains.value(domain);
}

//
// Eat a cookie out of the jar.
// cookieIterator should be one of the cookies returned by getCookieList()
//
void KCookieJar::eatCookie(KHttpCookieList::iterator cookieIterator)
{
    const KHttpCookie& cookie = *cookieIterator;
    QString domain = stripDomain(cookie); // We file the cookie under this domain.
    KHttpCookieList *cookieList = m_cookieDomains.value(domain);

    if (cookieList) {
        // This deletes cookie!
        cookieList->erase(cookieIterator);

        if ((cookieList->isEmpty()) &&
            (cookieList->getAdvice() == KCookieDunno))
        {
            // This deletes cookieList!
            delete m_cookieDomains.take(domain);

            m_domainList.removeAll(domain);
        }
    }
}

void KCookieJar::eatCookiesForDomain(const QString &domain)
{
   KHttpCookieList *cookieList = m_cookieDomains.value(domain);
   if (!cookieList || cookieList->isEmpty()) return;

   cookieList->clear();
   if (cookieList->getAdvice() == KCookieDunno)
   {
       // This deletes cookieList!
       delete m_cookieDomains.take(domain);
       m_domainList.removeAll(domain);
   }
   m_cookiesChanged = true;
}

void KCookieJar::eatSessionCookies( long windowId )
{
    if (!windowId)
        return;

    QStringList::const_iterator it=m_domainList.constBegin();
    for ( ; it != m_domainList.constEnd(); ++it )
        eatSessionCookies( *it, windowId, false );
}

void KCookieJar::eatAllCookies()
{
    Q_FOREACH(const QString& domain, m_domainList) {
        // This might remove domain from m_domainList!
        eatCookiesForDomain(domain);
    }
}

void KCookieJar::eatSessionCookies( const QString& fqdn, long windowId,
                                    bool isFQDN )
{
    KHttpCookieList* cookieList;
    if ( !isFQDN )
        cookieList = m_cookieDomains.value(fqdn);
    else {
        QString domain;
        stripDomain( fqdn, domain );
        cookieList = m_cookieDomains.value(domain);
    }

    if (cookieList) {
        QMutableListIterator<KHttpCookie> cookieIterator(*cookieList);
        while (cookieIterator.hasNext()) {
            KHttpCookie& cookie = cookieIterator.next();
            if ((cookie.expireDate() != 0) && !m_ignoreCookieExpirationDate) {
               continue;
            }

            QList<long> &ids = cookie.windowIds();

#ifndef NDEBUG
            if (ids.contains(windowId)) {
                if (ids.count() > 1)
                    kDebug() << "removing window id" << windowId << "from session cookie";
                else
                    kDebug() << "deleting session cookie";
            }
#endif
            if (!ids.removeAll(windowId) || !ids.isEmpty()) {
               continue;
            }
            cookieIterator.remove();
        }
    }
}

//
// Saves all cookies to the file '_filename'.
// On succes 'true' is returned.
// On failure 'false' is returned.
bool KCookieJar::saveCookies(const QString &_filename)
{
    KSaveFile saveFile(_filename);

    if (!saveFile.open())
       return false;
    saveFile.setPermissions(QFile::ReadUser|QFile::WriteUser);

    QTextStream ts(&saveFile);

    ts << "# KDE Cookie File v2\n#\n";

    QString s;
    s.sprintf("%-20s %-20s %-12s %-10s %-4s %-20s %-4s %s\n",
              "# Host", "Domain", "Path", "Exp.date", "Prot",
              "Name", "Sec", "Value");
    ts << s.toLatin1().constData();

    for ( QStringList::const_iterator it=m_domainList.constBegin(); it != m_domainList.constEnd();
          it++ )
    {
        const QString &domain = *it;
        bool domainPrinted = false;

        KHttpCookieList *cookieList = m_cookieDomains.value(domain);
        QMutableListIterator<KHttpCookie> cookieIterator(*cookieList);
        while (cookieIterator.hasNext()) {
            const KHttpCookie& cookie = cookieIterator.next();
            if (cookie.isExpired()) {
                // Delete expired cookies
                cookieIterator.remove();
            } else if (cookie.expireDate() != 0 && !m_ignoreCookieExpirationDate) {
                // Only save cookies that are not "session-only cookies"
                if (!domainPrinted) {
                    domainPrinted = true;
                    ts << '[' << domain.toLocal8Bit().data() << "]\n";
                }
                // Store persistent cookies
                QString path = QL1S("\"");
                path += cookie.path();
                path += '"';
                QString domain = QL1S("\"");
                domain += cookie.domain();
                domain += '"';
                // TODO: replace with direct QTextStream output ?
                s.sprintf("%-20s %-20s %-12s %10lld  %3d %-20s %-4i %s\n",
                        cookie.host().toLatin1().constData(), domain.toLatin1().constData(),
                        path.toLatin1().constData(), cookie.expireDate(),
                        cookie.protocolVersion(),
                        cookie.name().isEmpty() ? cookie.value().toLatin1().constData() : cookie.name().toLatin1().constData(),
                        (cookie.isSecure() ? 1 : 0) + (cookie.isHttpOnly() ? 2 : 0) +
                        (cookie.hasExplicitPath() ? 4 : 0) + (cookie.name().isEmpty() ? 8 : 0),
                        cookie.value().toLatin1().constData());
                ts << s.toLatin1().constData();
            }
        }
    }

    return saveFile.finalize();
}

static const char *parseField(char* &buffer, bool keepQuotes=false)
{
    char *result;
    if (!keepQuotes && (*buffer == '\"'))
    {
        // Find terminating "
        buffer++;
        result = buffer;
        while((*buffer != '\"') && (*buffer))
            buffer++;
    }
    else
    {
        // Find first white space
        result = buffer;
        while((*buffer != ' ') && (*buffer != '\t') && (*buffer != '\n') && (*buffer))
            buffer++;
    }

    if (!*buffer)
        return result; //
    *buffer++ = '\0';

    // Skip white-space
    while((*buffer == ' ') || (*buffer == '\t') || (*buffer == '\n'))
        buffer++;

    return result;
}


//
// Reloads all cookies from the file '_filename'.
// On succes 'true' is returned.
// On failure 'false' is returned.
bool KCookieJar::loadCookies(const QString &_filename)
{
    FILE *fStream = fopen( QFile::encodeName(_filename), "r");
    if (fStream == 0)
    {
        return false;
    }

    qint64 curTime = time(0);

    char *buffer = new char[READ_BUFFER_SIZE];

    bool err = false;
    err = (fgets(buffer, READ_BUFFER_SIZE, fStream) == 0);

    int version = 1;
    if (!err)
    {
        if (strcmp(buffer, "# KDE Cookie File\n") == 0)
        {
            // version 1
        }
        else if (sscanf(buffer, "# KDE Cookie File v%d\n", &version) != 1)
        {
            err = true;
        }
    }

    if (!err)
    {
        while(fgets(buffer, READ_BUFFER_SIZE, fStream) != 0)
        {
            char *line = buffer;
            // Skip lines which begin with '#' or '['
            if ((line[0] == '#') || (line[0] == '['))
                continue;

            const QString host = QString::fromLatin1( parseField(line) );
            const QString domain = QString::fromLatin1( parseField(line) );
            if (host.isEmpty() && domain.isEmpty())
                continue;
            const QString path = QString::fromLatin1( parseField(line) );
            const QString expStr = QString::fromLatin1( parseField(line) );
            if (expStr.isEmpty()) continue;
            const qint64 expDate = expStr.toLongLong();
            const QString verStr = QString::fromLatin1( parseField(line) );
            if (verStr.isEmpty()) continue;
            int protVer  = verStr.toInt();
            QString name = QString::fromLatin1( parseField(line) );
            bool keepQuotes = false;
            bool secure = false;
            bool httpOnly = false;
            bool explicitPath = false;
            const char *value = 0;
            if ((version == 2) || (protVer >= 200))
            {
                if (protVer >= 200)
                    protVer -= 200;
                int i = atoi( parseField(line) );
                secure = i & 1;
                httpOnly = i & 2;
                explicitPath = i & 4;
                if (i & 8)
                    name = "";
                line[strlen(line)-1] = '\0'; // Strip LF.
                value = line;
            }
            else
            {
                if (protVer >= 100)
                {
                    protVer -= 100;
                    keepQuotes = true;
                }
                value = parseField(line, keepQuotes);
                secure = atoi( parseField(line) );
            }

            // Parse error
            if (!value) continue;

            // Expired or parse error
            if ((expDate == 0) || (expDate < curTime))
                continue;

            KHttpCookie cookie(host,
                               domain,
                               path,
                               name,
                               value,
                               expDate, protVer,
                               secure, httpOnly, explicitPath);
            addCookie(cookie);
        }
    }
    delete [] buffer;
    m_cookiesChanged = false;

    fclose( fStream);
    return err;
}

//
// Save the cookie configuration
//

void KCookieJar::saveConfig(KConfig *_config)
{
    if (!m_configChanged)
        return;

    KConfigGroup dlgGroup(_config, "Cookie Dialog");
    dlgGroup.writeEntry("PreferredPolicy", static_cast<int>(m_preferredPolicy));
    dlgGroup.writeEntry("ShowCookieDetails", m_showCookieDetails );
    KConfigGroup policyGroup(_config,"Cookie Policy");
    policyGroup.writeEntry("CookieGlobalAdvice", adviceToStr( m_globalAdvice));

    QStringList domainSettings;
    for ( QStringList::const_iterator it=m_domainList.constBegin();
          it != m_domainList.constEnd();
          ++it )
    {
         const QString &domain = *it;
         KCookieAdvice advice = getDomainAdvice( domain);
         if (advice != KCookieDunno)
         {
             QString value(domain);
             value += ':';
             value += adviceToStr(advice);
             domainSettings.append(value);
         }
    }
    policyGroup.writeEntry("CookieDomainAdvice", domainSettings);
    _config->sync();
    m_configChanged = false;
}


//
// Load the cookie configuration
//

void KCookieJar::loadConfig(KConfig *_config, bool reparse )
{
    if ( reparse )
        _config->reparseConfiguration();

    KConfigGroup dlgGroup(_config, "Cookie Dialog");
    m_showCookieDetails = dlgGroup.readEntry( "ShowCookieDetails" , false );
    m_preferredPolicy = static_cast<KCookieDefaultPolicy>(dlgGroup.readEntry("PreferredPolicy", 0));

    KConfigGroup policyGroup(_config,"Cookie Policy");
    const QStringList domainSettings = policyGroup.readEntry("CookieDomainAdvice", QStringList());
    // Warning: those default values are duplicated in the kcm (kio/kcookiespolicies.cpp)
    m_rejectCrossDomainCookies = policyGroup.readEntry("RejectCrossDomainCookies", true);
    m_autoAcceptSessionCookies = policyGroup.readEntry("AcceptSessionCookies", true);
    m_ignoreCookieExpirationDate = policyGroup.readEntry("IgnoreExpirationDate", false);
    QString value = policyGroup.readEntry("CookieGlobalAdvice", QString::fromLatin1("Accept"));
    m_globalAdvice = strToAdvice(value);

    // Reset current domain settings first.
    //  (must make a copy because setDomainAdvice() might delete the domain from m_domainList inside the for loop)
    const QStringList domains = m_domainList;
    Q_FOREACH( const QString &domain, domains )
    {
         setDomainAdvice(domain, KCookieDunno);
    }

    // Now apply the domain settings read from config file...
    for ( QStringList::const_iterator it=domainSettings.begin();
          it != domainSettings.end(); )
    {
        const QString &value = *it++;

        int sepPos = value.lastIndexOf(':');

        if (sepPos <= 0)
          continue;

        QString domain(value.left(sepPos));
        KCookieAdvice advice = strToAdvice( value.mid(sepPos + 1) );
        setDomainAdvice(domain, advice);
    }
}

QDebug operator<<(QDebug dbg, const KHttpCookie& cookie)
{
    dbg.nospace() << cookie.cookieStr(false);
    return dbg.space();
}

QDebug operator<<(QDebug dbg, const KHttpCookieList& list)
{
    Q_FOREACH(const KHttpCookie& cookie, list)
        dbg << cookie;
    return dbg;
}
