/****************************************************************************
**
** Copyright (C) 1992-2005 Trolltech AS. All rights reserved.
**
** This file is part of the QtCore module of the Qt Toolkit.
**
** Licensees holding a valid Qt License Agreement may use this file in
** accordance with the rights, responsibilities and obligations
** contained therein.  Please consult your licensing agreement or
** contact sales@trolltech.com if any conditions of this licensing
** agreement are not clear to you.
**
** Further information about Qt licensing is available at:
** http://www.trolltech.com/products/qt/licensing.html or by
** contacting info@trolltech.com.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QURL_H
#define QURL_H

#include <QtCore/qobjectdefs.h>
#include <QtCore/qpair.h>
#include <QtCore/qstring.h>

QT_MODULE(Core)

class QByteArray;
class QUrlPrivate;
class QDataStream;

class Q_CORE_EXPORT QUrl
{
public:
    enum ParsingMode {
        TolerantMode,
        StrictMode
    };

    // encoding / toString values
    enum FormattingOption {
        None = 0x0,
        RemoveScheme = 0x1,
        RemovePassword = 0x2,
        RemoveUserInfo = RemovePassword | 0x4,
        RemovePort = 0x8,
        RemoveAuthority = RemoveUserInfo | RemovePort | 0x10,
        RemovePath = 0x20,
        RemoveQuery = 0x40,
        RemoveFragment = 0x80,

        StripTrailingSlash = 0x10000
    };
    Q_DECLARE_FLAGS(FormattingOptions, FormattingOption)

    QUrl();
    QUrl(const QString &url);
    QUrl(const QString &url, ParsingMode mode);
    // ### Qt 5: merge the two constructors, with mode = TolerantMode
    QUrl(const QUrl &copy);
    QUrl &operator =(const QUrl &copy);
    QUrl &operator =(const QString &url);
    ~QUrl();

    void setUrl(const QString &url);
    void setUrl(const QString &url, ParsingMode mode);
    // ### Qt 5: merge the two setUrl() functions, with mode = TolerantMode
    void setEncodedUrl(const QByteArray &url);
    void setEncodedUrl(const QByteArray &url, ParsingMode mode);
    // ### Qt 5: merge the two setEncodedUrl() functions, with mode = TolerantMode

    bool isValid() const;

    bool isEmpty() const;

    void clear();

    void setScheme(const QString &scheme);
    QString scheme() const;

    void setAuthority(const QString &authority);
    QString authority() const;

    void setUserInfo(const QString &userInfo);
    QString userInfo() const;

    void setUserName(const QString &userName);
    QString userName() const;

    void setPassword(const QString &password);
    QString password() const;

    void setHost(const QString &host);
    QString host() const;

    void setPort(int port);
    int port() const;
    int port(int defaultPort) const;
    // ### Qt 5: merge the two port() functions, with defaultPort = -1

    void setPath(const QString &path);
    QString path() const;

    void setEncodedQuery(const QByteArray &query);
    QByteArray encodedQuery() const;

    void setQueryDelimiters(char valueDelimiter, char pairDelimiter);
    char queryValueDelimiter() const;
    char queryPairDelimiter() const;

    void setQueryItems(const QList<QPair<QString, QString> > &query);
    void addQueryItem(const QString &key, const QString &value);
    QList<QPair<QString, QString> > queryItems() const;
    bool hasQueryItem(const QString &key) const;
    QString queryItemValue(const QString &key) const;
    QStringList allQueryItemValues(const QString &key) const;
    void removeQueryItem(const QString &key);
    void removeAllQueryItems(const QString &key);

    void setFragment(const QString &fragment);
    QString fragment() const;

    QUrl resolved(const QUrl &relative) const;

    bool isRelative() const;
    bool isParentOf(const QUrl &url) const;

    static QUrl fromLocalFile(const QString &localfile);
    QString toLocalFile() const;

    QString toString(FormattingOptions options = None) const;

    QByteArray toEncoded(FormattingOptions options = None) const;
    static QUrl fromEncoded(const QByteArray &url);
    static QUrl fromEncoded(const QByteArray &url, ParsingMode mode);
    // ### Qt 5: merge the two fromEncoded() functions, with mode = TolerantMode

    void detach();
    bool isDetached() const;

    bool operator <(const QUrl &url) const;
    bool operator ==(const QUrl &url) const;
    bool operator !=(const QUrl &url) const;

    static QString fromPercentEncoding(const QByteArray &);
    static QByteArray toPercentEncoding(const QString &,
                                        const QByteArray &exclude = QByteArray(),
                                        const QByteArray &include = QByteArray());
    static QString fromPunycode(const QByteArray &);
    static QByteArray toPunycode(const QString &);

#if defined QT3_SUPPORT
    inline QT3_SUPPORT QString protocol() const { return scheme(); }
    inline QT3_SUPPORT void setProtocol(const QString &s) { setScheme(s); }
    inline QT3_SUPPORT void setUser(const QString &s) { setUserName(s); }
    inline QT3_SUPPORT QString user() const { return userName(); }
    inline QT3_SUPPORT bool hasUser() const { return !userName().isEmpty(); }
    inline QT3_SUPPORT bool hasPassword() const { return !password().isEmpty(); }
    inline QT3_SUPPORT bool hasHost() const { return !host().isEmpty(); }
    inline QT3_SUPPORT bool hasPort() const { return port() != -1; }
    inline QT3_SUPPORT bool hasPath() const { return !path().isEmpty(); }
    inline QT3_SUPPORT void setQuery(const QString &txt)
    {
        setEncodedQuery(txt.toLatin1());
    }
    inline QT3_SUPPORT QString query() const
    {
        return QString::fromLatin1(encodedQuery());
    }
    inline QT3_SUPPORT QString ref() const { return fragment(); }
    inline QT3_SUPPORT void setRef(const QString &txt) { setFragment(txt); }
    inline QT3_SUPPORT bool hasRef() const { return !fragment().isEmpty(); }
    inline QT3_SUPPORT void addPath(const QString &p) { setPath(path() + QLatin1String("/") + p); }
    QT3_SUPPORT void setFileName(const QString &txt);
    QT3_SUPPORT QString fileName() const;
    QT3_SUPPORT QString dirPath() const;
    static inline QT3_SUPPORT void decode(QString &url)
    {
        url = QUrl::fromPercentEncoding(url.toLatin1());
    }
    static inline QT3_SUPPORT void encode(QString &url)
    {
        url = QString::fromLatin1(QUrl::toPercentEncoding(url));
    }
    inline QT3_SUPPORT operator QString() const { return toString(); }
    inline QT3_SUPPORT bool cdUp()
    {
        *this = resolved(QUrl(QLatin1String("..")));
        return true;
    }
    static inline QT3_SUPPORT bool isRelativeUrl(const QString &url)
    {
        return QUrl(url).isRelative();
    }
#endif

protected:
#if defined (QT3_SUPPORT)
    inline QT3_SUPPORT void reset() { clear(); }
#endif

private:
    QUrlPrivate *d;
};

Q_DECLARE_TYPEINFO(QUrl, Q_MOVABLE_TYPE);
Q_DECLARE_SHARED(QUrl)
Q_DECLARE_OPERATORS_FOR_FLAGS(QUrl::FormattingOptions)

#ifndef QT_NO_DATASTREAM
Q_CORE_EXPORT QDataStream &operator<<(QDataStream &, const QUrl &);
Q_CORE_EXPORT QDataStream &operator>>(QDataStream &, QUrl &);
#endif

#ifndef QT_NO_DEBUG_STREAM
Q_CORE_EXPORT QDebug operator<<(QDebug, const QUrl &);
#endif

#endif // QURL_H
