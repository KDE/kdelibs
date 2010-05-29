/*
    This file is part of the KDE File Manager

    Copyright (C) 1998 Waldo Bastian (bastian@kde.org)

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License
    version 2 as published by the Free Software Foundation.

    This software is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this library; see the file COPYING. If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/
//----------------------------------------------------------------------------
//
// KDE File Manager -- HTTP Cookies
// $Id: kcookiejar.h 465272 2005-09-29 09:47:40Z mueller $

#ifndef KCOOKIEJAR_H
#define KCOOKIEJAR_H

#include <qstring.h>
#include <qstringlist.h>
#include <qdict.h>
#include <qptrlist.h>
#include <time.h>

class KConfig;
class KCookieJar;
class KHttpCookie;
class KHttpCookieList;

typedef KHttpCookie *KHttpCookiePtr;

enum KCookieAdvice
{
    KCookieDunno=0,
    KCookieAccept,
    KCookieReject,
    KCookieAsk
};

class KHttpCookie
{
    friend class KCookieJar;
    friend class KHttpCookieList;

protected:
    QString mHost;
    QString mDomain;
    QString mPath;
    QString mName;
    QString mValue;
    time_t  mExpireDate;
    int     mProtocolVersion;
    bool    mSecure;
    bool    mCrossDomain;
    bool    mHttpOnly;
    bool    mExplicitPath;
    QValueList<long> mWindowIds;

    QString cookieStr(bool useDOMFormat);

public:
    KHttpCookie(const QString &_host=QString::null,
                const QString &_domain=QString::null,
                const QString &_path=QString::null,
                const QString &_name=QString::null,
                const QString &_value=QString::null,
                time_t _expireDate=0,
                int _protocolVersion=0,
                bool _secure = false,
                bool _httpOnly = false,
                bool _explicitPath = false);

    QString domain(void) { return mDomain; }
    QString host(void) { return mHost; }
    QString path(void) { return mPath; }
    QString name(void) { return mName; }
    QString value(void) { return mValue; }
    QValueList<long> &windowIds(void) { return mWindowIds; }
    void    fixDomain(const QString &domain) { mDomain = domain; }
    time_t  expireDate(void) { return mExpireDate; }
    int     protocolVersion(void) { return mProtocolVersion; }
    bool    isSecure(void) { return mSecure; }
    bool    isExpired(time_t currentDate);
    bool    isCrossDomain(void) { return mCrossDomain; }
    bool    isHttpOnly(void) { return mHttpOnly; }
    bool    hasExplicitPath(void) { return mExplicitPath; }
    bool    match(const QString &fqdn, const QStringList &domainList, const QString &path);
};

class KHttpCookieList : public QPtrList<KHttpCookie>
{
public:
    KHttpCookieList() : QPtrList<KHttpCookie>(), advice( KCookieDunno )
    { }
    virtual ~KHttpCookieList() { }

    virtual int compareItems( void * item1, void * item2);
    KCookieAdvice getAdvice(void) { return advice; }
    void setAdvice(KCookieAdvice _advice) { advice = _advice; }

private:
    KCookieAdvice advice;
};

class KCookieJar
{
public:
    /**
     * Constructs a new cookie jar
     *
     * One jar should be enough for all cookies.
     */
    KCookieJar();

    /**
     * Destructs the cookie jar
     *
     * Poor little cookies, they will all be eaten by the cookie monster!
     */
    ~KCookieJar();

    /**
     * Returns whether the cookiejar has been changed
     */
    bool changed() const { return m_cookiesChanged || m_configChanged; }

    /**
     * Store all the cookies in a safe(?) place
     */
    bool saveCookies(const QString &_filename);

    /**
     * Load all the cookies from file and add them to the cookie jar.
     */
    bool loadCookies(const QString &_filename);

    /**
     * Save the cookie configuration
     */
    void saveConfig(KConfig *_config);

    /**
     * Load the cookie configuration
     */
    void loadConfig(KConfig *_config, bool reparse = false);

    /**
     * Looks for cookies in the cookie jar which are appropriate for _url.
     * Returned is a string containing all appropriate cookies in a format
     * which can be added to a HTTP-header without any additional processing.
     *
     * If @p useDOMFormat is true, the string is formatted in a format
     * in compliance with the DOM standard.
     * @p pendingCookies contains a list of cookies that have not been
     * approved yet by the user but that will be included in the result
     * none the less.
     */
    QString findCookies(const QString &_url, bool useDOMFormat, long windowId, KHttpCookieList *pendingCookies=0);

    /**
     * This function parses cookie_headers and returns a linked list of
     * valid KHttpCookie objects for all cookies found in cookie_headers.
     * If no cookies could be found 0 is returned.
     *
     * cookie_headers should be a concatenation of all lines of a HTTP-header
     * which start with "Set-Cookie". The lines should be separated by '\n's.
     */
    KHttpCookieList makeCookies(const QString &_url, const QCString &cookie_headers, long windowId);

    /**
     * This function parses cookie_headers and returns a linked list of
     * valid KHttpCookie objects for all cookies found in cookie_headers.
     * If no cookies could be found 0 is returned.
     *
     * cookie_domstr should be a concatenation of "name=value" pairs, separated
     * by a semicolon ';'.
     */
    KHttpCookieList makeDOMCookies(const QString &_url, const QCString &cookie_domstr, long windowId);

    /**
     * This function hands a KHttpCookie object over to the cookie jar.
     *
     * On return cookiePtr is set to 0.
     */
    void addCookie(KHttpCookiePtr &cookiePtr);

    /**
     * This function advices whether a single KHttpCookie object should
     * be added to the cookie jar.
     *
     * Possible return values are:
     *     - KCookieAccept, the cookie should be added
     *     - KCookieReject, the cookie should not be added
     *     - KCookieAsk, the user should decide what to do
     */
    KCookieAdvice cookieAdvice(KHttpCookiePtr cookiePtr);

    /**
     * This function gets the advice for all cookies originating from
     * _domain.
     *
     *     - KCookieDunno, no specific advice for _domain
     *     - KCookieAccept, accept all cookies for _domain
     *     - KCookieReject, reject all cookies for _domain
     *     - KCookieAsk, the user decides what to do with cookies for _domain
     */
    KCookieAdvice getDomainAdvice(const QString &_domain);

    /**
     * This function sets the advice for all cookies originating from
     * _domain.
     *
     * _advice can have the following values:
     *     - KCookieDunno, no specific advice for _domain
     *     - KCookieAccept, accept all cookies for _domain
     *     - KCookieReject, reject all cookies for _domain
     *     - KCookieAsk, the user decides what to do with cookies for _domain
     */
    void setDomainAdvice(const QString &_domain, KCookieAdvice _advice);

    /**
     * This function sets the advice for all cookies originating from
     * the same domain as _cookie
     *
     * _advice can have the following values:
     *     - KCookieDunno, no specific advice for _domain
     *     - KCookieAccept, accept all cookies for _domain
     *     - KCookieReject, reject all cookies for _domain
     *     - KCookieAsk, the user decides what to do with cookies for _domain
     */
    void setDomainAdvice(KHttpCookiePtr _cookie, KCookieAdvice _advice);

    /**
     * Get the global advice for cookies
     *
     * The returned advice can have the following values:
     *     - KCookieAccept, accept cookies
     *     - KCookieReject, reject cookies
     *     - KCookieAsk, the user decides what to do with cookies
     *
     * The global advice is used if the domain has no advice set.
     */
    KCookieAdvice getGlobalAdvice() { return m_globalAdvice; }

    /**
     * This function sets the global advice for cookies
     *
     * _advice can have the following values:
     *     - KCookieAccept, accept cookies
     *     - KCookieReject, reject cookies
     *     - KCookieAsk, the user decides what to do with cookies
     *
     * The global advice is used if the domain has no advice set.
     */
    void setGlobalAdvice(KCookieAdvice _advice);

    /**
     * Get a list of all domains known to the cookie jar.
     * A domain is known to the cookie jar if:
     *     - It has a cookie originating from the domain
     *     - It has a specific advice set for the domain
     */
    const QStringList& getDomainList();

    /**
     * Get a list of all cookies in the cookie jar originating from _domain.
     */
    const KHttpCookieList *getCookieList(const QString & _domain,
                                         const QString& _fqdn );

    /**
     * Remove & delete a cookie from the jar.
     *
     * cookiePtr should be one of the entries in a KHttpCookieList.
     * Update your KHttpCookieList by calling getCookieList after
     * calling this function.
     */
    void eatCookie(KHttpCookiePtr cookiePtr);

    /**
     * Remove & delete all cookies for @p domain.
     */
    void eatCookiesForDomain(const QString &domain);

    /**
     * Remove & delete all cookies
     */
    void eatAllCookies();

    /**
     * Removes all end of session cookies set by the
     * session @p windId.
     */
    void eatSessionCookies( long windowId );

    /**
     * Removes all end of session cookies set by the
     * session @p windId.
     */
    void eatSessionCookies( const QString& fqdn, long windowId, bool isFQDN = true );

    /**
     * Parses _url and returns the FQDN (_fqdn) and path (_path).
     */
    static bool parseURL(const QString &_url,
                         QString &_fqdn,
                         QString &_path);

    /**
     * Returns a list of domains in @p _domainList relevant for this host.
     * The list is sorted with the FQDN listed first and the top-most
     * domain listed last
     */
    void extractDomains(const QString &_fqdn,
                        QStringList &_domainList);

    static QString adviceToStr(KCookieAdvice _advice);
    static KCookieAdvice strToAdvice(const QString &_str);

    /** Returns the */
    int preferredDefaultPolicy() const { return m_preferredPolicy; }

    /** Returns the */
    bool showCookieDetails () const { return m_showCookieDetails; }

     /**
      * Sets the user's default preference cookie policy.
      */
     void setPreferredDefaultPolicy (int value) { m_preferredPolicy = value; }

     /**
      * Sets the user's preference of level of detail displayed
      * by the cookie dialog.
      */
     void setShowCookieDetails (bool value) { m_showCookieDetails = value; }

protected:
     void stripDomain(const QString &_fqdn, QString &_domain);
     QString stripDomain( KHttpCookiePtr cookiePtr);

protected:
    QStringList m_domainList;
    KCookieAdvice m_globalAdvice;
    QDict<KHttpCookieList> m_cookieDomains;
    QDict<int> m_twoLevelTLD;

    bool m_configChanged;
    bool m_cookiesChanged;
    bool m_showCookieDetails;
    bool m_rejectCrossDomainCookies;
    bool m_autoAcceptSessionCookies;
    bool m_ignoreCookieExpirationDate;

    int m_preferredPolicy;
};
#endif
