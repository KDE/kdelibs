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
// $Id$

//
// The cookie protocol is a mess. RFC2109 is a joke since nobody seems to
// use it. Apart from that it is badly written.
// We try to implement Netscape Cookies and try to behave us according to
// RFC2109 as much as we can.
//
// We assume cookies do not contain any spaces (Netscape spec.)
// According to RFC2109 this is allowed though.
//

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
#include <stdlib.h>

//#include <netinet/in.h>
//#include <arpa/inet.h>

#include <qstring.h>
#include <qstrlist.h>
#include <qptrlist.h>
#include <qptrdict.h>
#include <qfile.h>

#include <kurl.h>
#include <krfcdate.h>
#include <kconfig.h>
#include <ksavefile.h>
#include <kdebug.h>

#include "kcookiejar.h"

#define READ_BUFFER_SIZE 8192

template class QPtrList<KHttpCookie>;
template class QPtrDict<KHttpCookieList>;

QString KCookieJar::adviceToStr(KCookieAdvice _advice)
{
    switch( _advice )
    {
    case KCookieAccept: return "Accept";
    case KCookieReject: return "Reject";
    case KCookieAsk: return "Ask";
    default: return "Dunno";
    }
}

KCookieAdvice KCookieJar::strToAdvice(const QString &_str)
{
    if (_str.isEmpty())
        return KCookieDunno;

    QString advice = _str.lower();

    if (advice == "accept")
        return KCookieAccept;
    else if (advice == "reject")
        return KCookieReject;
    else if (advice == "ask")
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
                 time_t _expireDate,
                 int _protocolVersion,
                 bool _secure) :
       mHost(_host),
       mDomain(_domain),
       mPath(_path),
       mName(_name),
       mValue(_value),
       mExpireDate(_expireDate),
       mProtocolVersion(_protocolVersion),
       mSecure(_secure)
{
}

//
// Checks if a cookie has been expired
//
bool    KHttpCookie::isExpired(time_t currentDate)
{
    return (mExpireDate != 0) && (mExpireDate < currentDate);
}

//
// Returns a string for a HTTP-header
//
QString KHttpCookie::cookieStr(bool useDOMFormat)
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
        result.sprintf("$Version=\"%d\"; ", mProtocolVersion);
        result += mName + "=" + mValue + "";
        if (!mPath.isEmpty())
            result += "; $Path=\""+ mPath + "\"";
        if (!mDomain.isEmpty())
            result += "; $Domain=\""+ mDomain + "\"";
    }
    return result;
}

//
// Returns whether this cookie should be send to this location.
bool KHttpCookie::match(const QString &fqdn, const QStringList &domains,
                        const QString &path)
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
        QString domain = "." + mDomain;
        if ( !domains.contains( domain ) )
          if ( fqdn != mDomain )
            return false;
    }

    // Cookie path match check
    if( !path.isEmpty() && !path.startsWith(mPath) )
        return false; // Path of URL does not start with cookie-path

    return true;
}

// KHttpCookieList
///////////////////////////////////////////////////////////////////////////

int KHttpCookieList::compareItems( void * item1, void * item2)
{
    int pathLen1 = ((KHttpCookie *)item1)->path().length();
    int pathLen2 = ((KHttpCookie *)item2)->path().length();
    if (pathLen1 > pathLen2)
        return 1;
    if (pathLen1 < pathLen2)
        return -1;
    return 0;
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
    m_cookieDomains.setAutoDelete( true );
    m_globalAdvice = KCookieDunno;
    m_configChanged = false;
    m_cookiesChanged = false;
}

//
// Destructs the cookie jar
//
// Poor little cookies, they will all be eaten by the cookie monster!
//
KCookieJar::~KCookieJar()
{
    // Not much to do here
}

//
// Looks for cookies in the cookie jar which are appropriate for _url.
// Returned is a string containing all appropriate cookies in a format
// which can be added to a HTTP-header without any additional processing.
//
QString KCookieJar::findCookies(const QString &_url, bool useDOMFormat, long windowId)
{
    QString cookieStr;
    QStringList domains;
    QString fqdn;
    QString path;
    KHttpCookiePtr cookie;
    int protVersion = 1;
    int cookieCount = 0;
    KCookieAdvice advice = m_globalAdvice;

    if (!parseURL(_url, fqdn, path))
    {
        return cookieStr;
    }

    extractDomains(fqdn, domains);
    bool secureRequest = (_url.find( "https://", 0, false) == 0);
    for(QStringList::ConstIterator it = domains.begin();
        it != domains.end();
        ++it)
    {
       QString key = (*it).isNull() ? "" : (*it);
       KHttpCookieList *cookieList = m_cookieDomains[key];
       
       if (!cookieList)
          continue; // No cookies for this domain

       if (cookieList->getAdvice() != KCookieDunno)
          advice = cookieList->getAdvice();
       
       if (advice == KCookieReject)
          continue; // Do not send cookies for this domain

       for ( cookie=cookieList->first(); cookie != 0; cookie=cookieList->next() )
       {
          if (!cookie->match(fqdn, domains, path))
             continue;

          if( cookie->isSecure() && !secureRequest )
             continue;

          if (windowId && (cookie->windowIds().find(windowId) == cookie->windowIds().end()))
          {
             cookie->windowIds().append(windowId);
          }

          // Use first cookie to determine protocol version
          if (cookieCount == 0)
          {
             protVersion = cookie->protocolVersion();
          }
          if (useDOMFormat)
          {
             if (cookieCount > 0)
                cookieStr += "; ";
             cookieStr += cookie->cookieStr(true);
          }
          else if (protVersion == 0)
          {
             if (cookieCount == 0)
                cookieStr += "Cookie: ";
             else
                cookieStr += "; ";
             cookieStr += cookie->cookieStr(false);
          }
          else
          {
             if (cookieCount > 0)
                cookieStr += "\r\n";
             cookieStr += "Cookie: ";
             cookieStr += cookie->cookieStr(false);
          }
          cookieCount++;
       }
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
                                  bool keepQuotes=false)
{
    const char *s = header;
    // Parse 'my_name' part
    for(; (*s != '='); s++)
    {
        if ((*s=='\0') || (*s==';') || (*s=='\n'))
        {
            // No '=' sign -> use string as the value, name is empty
            // (behaviour found in Mozilla and IE)
            Name = "";
            Value = header;
            Value.truncate( s - header );
            Value = Value.stripWhiteSpace();
            return (s);
        }
    }

    Name = header;
    Name.truncate( s - header );
    Name = Name.stripWhiteSpace();

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

    if (!keepQuotes && (*s == '\"'))
    {
        // Parse '"my_value"' part (quoted value)
        s++;  // skip "
        header = s;
        for(;(*s != '\"');s++)
        {
            if ((*s=='\0') || (*s=='\n'))
            {
                // End of Name
                Value = header;
                Value.truncate(s - header);
                return (s);
            }
        }
        Value = header;
        Value.truncate( s - header );

        // *s == '\"';
        s++;
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
        Value = header;
        Value.truncate( s - header );
        Value = Value.stripWhiteSpace();
    }
    return (s);

}

static void stripDomain(const QString &_fqdn, QString &_domain)
{
   QStringList domains;
   KCookieJar::extractDomains(_fqdn, domains);
   _domain = domains[0];
}

static QString stripDomain( KHttpCookiePtr cookiePtr)
{
    QString domain; // We file the cookie under this domain.
    if (cookiePtr->domain().isEmpty())
       stripDomain( cookiePtr->host(), domain);
    else
       domain = cookiePtr->domain();
    return domain;
}

bool KCookieJar::parseURL(const QString &_url,
                          QString &_fqdn,
                          QString &_path)
{
    KURL kurl(_url);
    if (kurl.isMalformed())
       return false;

    _fqdn = kurl.host().lower();

    // Cookie spoofing protection.  Since there is no way a path separator
    // or escape encoded character is allowed in the hostname according
    // to RFC 2396, reject attempts to include such things there!
    if(_fqdn.find('/') > -1 || _fqdn.find('%') > -1)
    {
        return false;  // deny everything!!
    }

    _path = kurl.path();
    if (_path.isEmpty())
       _path = "/";
    return true;
}

void KCookieJar::extractDomains(const QString &_fqdn,
                                QStringList &_domains)
{
    // Return IPv4 and IPv6 addresses as is...
    if (((_fqdn[0] >= '0') && (_fqdn[0] <= '9')) ||
        _fqdn[0] == '[')
    {
       _domains.append( _fqdn );
       return;
    }

    QStringList partList = QStringList::split('.', _fqdn, false);

    if (partList.count())
        partList.remove(partList.begin()); // Remove hostname

    while(partList.count())
    {
       if (partList.count() == 1)
         break; // We only have a TLD left.
       if (partList.count() == 2)
       {
          // If this is a TLD, we should stop. (e.g. co.uk)
          // We assume this is a TLD if it ends with .xx.yy or .x.yy
          if ((partList[0].length() <= 2) &&
              (partList[1].length() == 2))
             break; // This is a TLD.
       }
       QString domain = partList.join(".");
       _domains.append("." + domain);
       _domains.append(domain);
       partList.remove(partList.begin()); // Remove part
    }

    // Only URLs that would get in here are of type
    // "host.foo" or "host.co.fo" so simply append
    // a '.' on top to make sure they are stored under
    // the proper cookie domain.
    if (_domains.isEmpty())
       _domains.append( "." + _fqdn );

    // Always add the FQDN at the end of the list for
    // hostname == cookie-domainname checks!
    _domains.append( _fqdn );
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
                                       const QCString &cookie_headers,
                                       long windowId)
{
    KHttpCookieList cookieList;
    KHttpCookiePtr lastCookie = 0;
    const char *cookieStr = cookie_headers.data();
    QString Name;
    QString Value;
    QString fqdn;
    QString path;
    bool crossDomain = false;

    if (!parseURL(_url, fqdn, path))
    {
        // Error parsing _url
        return KHttpCookieList();
    }

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
            // Default path = ""
            KHttpCookie *cookie = new KHttpCookie(fqdn, "", "", Name, Value);
            if (windowId)
               cookie->mWindowIds.append(windowId);
            cookie->mCrossDomain = crossDomain;

            // Insert cookie in chain
            cookieList.append(cookie);
            lastCookie = cookie;
        }
        else if (lastCookie && (strncasecmp(cookieStr, "Set-Cookie2:", 12) == 0))
        {
            // What the fuck is this?
            // Does anyone invent his own headers these days?
            // Read the fucking RFC guys! This header is not there!
            cookieStr +=12;
            // Continue with lastCookie
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

            Name = Name.lower();
            if (Name == "domain")
            {
                lastCookie->mDomain = Value.lower();
            }
            else if (Name == "max-age")
            {
                int max_age = Value.toInt();
                if (max_age == 0)
                    lastCookie->mExpireDate = 1;
                else
                    lastCookie->mExpireDate = time(0)+max_age;
            }
            else if (Name == "expires")
            {
                // Parse brain-dead netscape cookie-format
                lastCookie->mExpireDate = KRFCDate::parseDate(Value);
            }
            else if (Name == "path")
            {
                lastCookie->mPath = Value;
            }
            else if (Name == "version")
            {
                lastCookie->mProtocolVersion = Value.toInt();
            }
            else if ((Name == "secure") || 
                     (Name.isEmpty() && Value.lower() == "secure"))
            {
                lastCookie->mSecure = true;
            }
        }

        if (*cookieStr == '\0')
            break; // End of header

        // Skip ';' or '\n'
        cookieStr++;
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
                                          const QCString &cookie_domstring,
                                          long windowId)
{
    // A lot copied from above
    KHttpCookieList cookieList;
    KHttpCookiePtr lastCookie = 0;

    const char *cookieStr = cookie_domstring.data();
    QString Name;
    QString Value;
    QString fqdn;
    QString path;

    if (!parseURL(_url, fqdn, path))
    {
        // Error parsing _url
        return KHttpCookieList();
    }

    //  This time it's easy
    while(*cookieStr)
    {
        cookieStr = parseNameValue(cookieStr, Name, Value);

        // Host = FQDN
        // Default domain = ""
        // Default path = ""
        KHttpCookie *cookie = new KHttpCookie(fqdn, QString::null, QString::null,
                                Name, Value );
        if (windowId)
            cookie->mWindowIds.append(windowId);

        cookieList.append(cookie);
        lastCookie = cookie;

        if (*cookieStr != '\0')
            cookieStr++;         // Skip ';' or '\n'
     }

     return cookieList;
}

//
// This function hands a KHttpCookie object over to the cookie jar.
//
// On return cookiePtr is set to 0.
//
void KCookieJar::addCookie(KHttpCookiePtr &cookiePtr)
{
    QString domain;
    QStringList domains;
    KHttpCookieList *cookieList = 0L;

    // We always need to do this to make sure that the
    // that cookies of type hostname == cookie-domainname
    // are properly removed and/or updated as necessary!
    extractDomains( cookiePtr->host(), domains );
    for ( QStringList::ConstIterator it = domains.begin();
          (it != domains.end() && !cookieList);
          ++it )
    {
        QString key = (*it).isNull() ? "" : (*it);
        KHttpCookieList *list= m_cookieDomains[key];
        if ( !list ) continue;

        for ( KHttpCookiePtr cookie=list->first(); cookie != 0; )
        {
            if ( cookiePtr->name() == cookie->name() &&
                 cookie->match(cookiePtr->host(),domains,cookiePtr->path()) )
            {
                KHttpCookiePtr old_cookie = cookie;
                cookie = list->next();
                list->removeRef( old_cookie );
                break;
            }
            else
            {
                cookie = list->next();
            }
        }
    }

    domain = stripDomain( cookiePtr );
    QString key = domain.isNull() ? "" : domain;
    cookieList = m_cookieDomains[ key ];
    if (!cookieList)
    {
        // Make a new cookie list
        cookieList = new KHttpCookieList();
        cookieList->setAutoDelete(true);

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
    if (!cookiePtr->isExpired(time(0)))
    {
        cookieList->inSort( cookiePtr );
        m_cookiesChanged = true;
    }
    else
    {
        delete cookiePtr;
    }
    cookiePtr = 0;
}

//
// This function advices whether a single KHttpCookie object should
// be added to the cookie jar.
//
KCookieAdvice KCookieJar::cookieAdvice(KHttpCookiePtr cookiePtr)
{
    QStringList domains;
    
    if (m_rejectCrossDomainCookies && cookiePtr->isCrossDomain())
       return KCookieReject;
       
    if (m_autoAcceptSessionCookies && (cookiePtr->expireDate() == 0 || 
        m_ignoreCookieExpirationDate))
       return KCookieAccept;
       
    extractDomains(cookiePtr->host(), domains);

    // If the cookie specifies a domain, check whether it is valid and
    // correct otherwise.
    if (!cookiePtr->domain().isEmpty())
    {
       bool valid = false;

       // This checks whether the cookie is valid based on
       // what ::extractDomains returns
       if (!valid)
       {
          if (domains.contains(cookiePtr->domain()))
             valid = true;
       }

       if (!valid)
       {
          // Maybe the domain doesn't start with a "."
          QString domain = "."+cookiePtr->domain();
          if (domains.contains(domain))
             valid = true;
       }

       if (!valid)
       {
          cookiePtr->fixDomain(QString::null);
       }
    }

    KCookieAdvice advice = KCookieDunno;

    QStringList::Iterator it = domains.fromLast(); // Start with FQDN which is last in the list.
    while( (advice == KCookieDunno) && (it != domains.end()))
    {
       QString domain = *it;
       // Check if a policy for the FQDN/domain is set.
       KHttpCookieList *cookieList = m_cookieDomains[domain];
       if (cookieList)
          advice = cookieList->getAdvice();
       domains.remove(it);
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
    KHttpCookieList *cookieList = m_cookieDomains[_domain];
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
    KHttpCookieList *cookieList = m_cookieDomains[domain];

    if (cookieList)
    {
        if (cookieList->getAdvice() != _advice);
        {
           m_configChanged = true;
           // domain is already known
           cookieList->setAdvice( _advice);
        }

        if ((cookieList->isEmpty()) &&
            (_advice == KCookieDunno))
        {
            // This deletes cookieList!
            m_cookieDomains.remove(domain);
            m_domainList.remove(domain);
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
            cookieList->setAutoDelete(true);
            cookieList->setAdvice( _advice);
            m_cookieDomains.insert( domain, cookieList);
            // Update the list of domains
            m_domainList.append( domain);
        }
    }
}

//
// This function sets the advice for all cookies originating from
// the same domain as _cookie
//
void KCookieJar::setDomainAdvice(KHttpCookiePtr cookiePtr, KCookieAdvice _advice)
{
    QString domain = stripDomain(cookiePtr); // We file the cookie under this domain.

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
const KHttpCookieList *KCookieJar::getCookieList(const QString & _domain,
                                                 const QString & _fqdn )
{
    QString domain;

    if (_domain.isEmpty())
        stripDomain( _fqdn, domain );
    else
        domain = _domain;

    return m_cookieDomains[domain];
}

//
// Eat a cookie out of the jar.
// cookiePtr should be one of the cookies returned by getCookieList()
//
void KCookieJar::eatCookie(KHttpCookiePtr cookiePtr)
{
    QString domain = stripDomain(cookiePtr); // We file the cookie under this domain.
    KHttpCookieList *cookieList = m_cookieDomains[domain];

    if (cookieList)
    {
        // This deletes cookiePtr!
        if (cookieList->removeRef( cookiePtr ))
           m_cookiesChanged = true;

        if ((cookieList->isEmpty()) &&
            (cookieList->getAdvice() == KCookieDunno))
        {
            // This deletes cookieList!
            m_cookieDomains.remove(domain);

            m_domainList.remove(domain);
        }
    }
}

void KCookieJar::eatCookiesForDomain(const QString &domain)
{
   KHttpCookieList *cookieList = m_cookieDomains[domain];
   if (!cookieList || cookieList->isEmpty()) return;

   cookieList->clear();
   if (cookieList->getAdvice() == KCookieDunno)
   {
       // This deletes cookieList!
       m_cookieDomains.remove(domain);
       m_domainList.remove(domain);
   }
   m_cookiesChanged = true;
}

void KCookieJar::eatSessionCookies( long windowId )
{
    if (!windowId)
        return;
        
    QStringList::Iterator it=m_domainList.begin();
    for ( ; it != m_domainList.end(); ++it )
        eatSessionCookies( *it, windowId, false );
}

void KCookieJar::eatAllCookies()
{
    for ( QStringList::Iterator it=m_domainList.begin();
          it != m_domainList.end();)
    {
        QString domain = *it++;
        // This might remove domain from domainList!
        eatCookiesForDomain(domain);
    }
}

void KCookieJar::eatSessionCookies( const QString& fqdn, long windowId,
                                    bool isFQDN )
{
    KHttpCookieList* cookieList;
    if ( !isFQDN )
        cookieList = m_cookieDomains[fqdn];
    else        
    {
        QString domain;
        stripDomain( fqdn, domain );
        cookieList = m_cookieDomains[domain];
    }

    if ( cookieList )
    {
        KHttpCookiePtr cookie=cookieList->first();
        for (; cookie != 0;)
        {
            if ((cookie->expireDate() != 0) && !m_ignoreCookieExpirationDate)
            {
               cookie = cookieList->next();
               continue;
            }
            
            QValueList<long> &ids = cookie->windowIds();
            if (!ids.remove(windowId) || !ids.isEmpty())
            {
               cookie = cookieList->next();
               continue;
            }
            KHttpCookiePtr old_cookie = cookie;
            cookie = cookieList->next();
            cookieList->removeRef( old_cookie );
        }
    }
}

//
// Saves all cookies to the file '_filename'.
// On succes 'true' is returned.
// On failure 'false' is returned.
bool KCookieJar::saveCookies(const QString &_filename)
{
    KSaveFile saveFile(_filename, 0600);

    if (saveFile.status() != 0)
       return false;

    FILE *fStream = saveFile.fstream();

    time_t curTime = time(0);

    fprintf(fStream, "# KDE Cookie File\n#\n");

    fprintf(fStream, "%-20s %-20s %-12s %-10s %-4s %-20s %-4s %s\n",
                     "# Host", "Domain", "Path", "Exp.date", "Prot",
                     "Name", "Sec", "Value");

    for ( QStringList::Iterator it=m_domainList.begin(); it != m_domainList.end();
          it++ )
    {
        const QString &domain = *it;
        bool domainPrinted = false;
        
        KHttpCookieList *cookieList = m_cookieDomains[domain];
        KHttpCookiePtr cookie=cookieList->first();
        
        for (; cookie != 0;)
        {
            if (cookie->isExpired(curTime))
            {
                // Delete expired cookies
                KHttpCookiePtr old_cookie = cookie;
                cookie = cookieList->next();
                cookieList->removeRef( old_cookie );
            }
            else if (cookie->expireDate() != 0 && !m_ignoreCookieExpirationDate)
            {
                if (!domainPrinted)
                {
                    domainPrinted = true;
                    fprintf(fStream, "[%s]\n", domain.local8Bit().data());
                }
                // Store persistent cookies
                QString path("\"");
                path += cookie->path();
                path += "\"";
                QString domain("\"");
                domain += cookie->domain();
                domain += "\"";
                fprintf(fStream, "%-20s %-20s %-12s %10lu  %3d %-20s %-4i %s\n",
                        cookie->host().local8Bit().data(), domain.local8Bit().data(),
                        path.local8Bit().data(), (unsigned long) cookie->expireDate(),
                        cookie->protocolVersion()+200, cookie->name().local8Bit().data(), 
                        cookie->isSecure(), cookie->value().local8Bit().data());
                cookie = cookieList->next();
            }
            else
            {
                // Skip session-only cookies
                cookie = cookieList->next();
            }
        }
    }

    return saveFile.close();
}

typedef char *charPtr;

static const char *parseField(charPtr &buffer, bool keepQuotes=false)
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

    time_t curTime = time(0);

    char *buffer = new char[READ_BUFFER_SIZE];

    bool err = false;
    err = (fgets(buffer, READ_BUFFER_SIZE, fStream) == 0);

    err = err || (strcmp(buffer, "# KDE Cookie File\n") != 0);

    if (!err)
    {
        while(fgets(buffer, READ_BUFFER_SIZE, fStream) != 0)
        {
            char *line = buffer;
            // Skip lines which begin with '#' or '['
            if ((line[0] == '#') || (line[0] == '['))
                continue;

            const char *host( parseField(line) );
            const char *domain( parseField(line) );
            const char *path( parseField(line) );
            const char *expStr( parseField(line) );
            if (!expStr) continue;
            int expDate  = (time_t) strtoul(expStr, 0, 10);
            const char *verStr( parseField(line) );
            if (!verStr) continue;
            int protVer  = (time_t) strtoul(verStr, 0, 10);
            const char *name( parseField(line) );
            bool keepQuotes = false;
            bool secure = false;
            const char *value = 0;
            if (protVer >= 200)
            {
                protVer -= 200;
                secure = atoi( parseField(line) );
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

            KHttpCookie *cookie = new KHttpCookie(host, domain, path, name,
                                                  value, expDate, protVer,
                                                  secure);
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

    _config->setGroup("Cookie Dialog");
    _config->writeEntry("PreferredPolicy", m_preferredPolicy);
    _config->writeEntry("ShowCookieDetails", m_showCookieDetails );
    _config->setGroup("Cookie Policy");
    _config->writeEntry("CookieGlobalAdvice", adviceToStr( m_globalAdvice));
    
    QStringList domainSettings;
    for ( QStringList::Iterator it=m_domainList.begin();
          it != m_domainList.end();
          it++ )
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
    _config->writeEntry("CookieDomainAdvice", domainSettings);
    _config->sync();
}


//
// Load the cookie configuration
//

void KCookieJar::loadConfig(KConfig *_config, bool reparse )
{
    if ( reparse )
        _config->reparseConfiguration();

    _config->setGroup("Cookie Dialog");
    m_showCookieDetails = _config->readBoolEntry( "ShowCookieDetails" );
    m_preferredPolicy = _config->readNumEntry( "PreferredPolicy", 0 );       
    _config->setGroup("Cookie Policy");		
    QStringList domainSettings = _config->readListEntry("CookieDomainAdvice");
    m_rejectCrossDomainCookies = _config->readBoolEntry( "RejectCrossDomainCookies", true );
    m_autoAcceptSessionCookies = _config->readBoolEntry( "AcceptSessionCookies", true );
    m_ignoreCookieExpirationDate = _config->readBoolEntry( "IgnoreExpirationDate", false );
    QString value = _config->readEntry("CookieGlobalAdvice", "Ask");
    m_globalAdvice = strToAdvice(value);

    // Reset current domain settings first.
    for ( QStringList::Iterator it=m_domainList.begin(); it != m_domainList.end(); )
    {
         // Make sure to update iterator before calling setDomainAdvice()
         // setDomainAdvice() might delete the domain from domainList.
         QString domain = *it++;
         setDomainAdvice(domain, KCookieDunno);
    }

    // Now apply the domain settings read from config file...
    for ( QStringList::Iterator it=domainSettings.begin();
          it != domainSettings.end(); )
    {
        const QString &value = *it++;
        
        int sepPos = value.find(':');
        
        if (sepPos <= 0) 
          continue;
          
        QString domain(value.left(sepPos));
        KCookieAdvice advice = strToAdvice( value.mid(sepPos + 1) );
        setDomainAdvice(domain, advice);
    }    
}

