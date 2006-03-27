#include <QDebug>


#include <QUrl_bind.h>

using namespace KJSEmbed;

QUrlBinding::QUrl( KJS::ExecState *exec, const QUrl &value )
   : ValueBinding(exec, value)
{
   StaticBinding::publish(exec, this, QUrl::methods() );
   StaticBinding::publish(exec, this, ValueFactory::methods() );
}

namespace QUrlNS
{

START_CTOR(QUrl, QUrl, 0 )
   return new KJSEmbed::QUrlBinding(exec, QUrl())
END_CTOR

START_CTOR(QUrl, QUrl, 1 )
   const QString & url = KJSEmbed::extractObject<const QString &>(exec, args, 0, 0);
   return new KJSEmbed::QUrlBinding(exec, QUrl(url))
END_CTOR

START_CTOR(QUrl, QUrl, 2 )
   const QString & url = KJSEmbed::extractObject<const QString &>(exec, args, 0, 0);
   ParsingMode mode = KJSEmbed::extractObject<ParsingMode>(exec, args, 1, 0);
   return new KJSEmbed::QUrlBinding(exec, QUrl(url, mode))
END_CTOR

START_CTOR(QUrl, QUrl, 1 )
   const QUrl & copy = KJSEmbed::extractObject<const QUrl &>(exec, args, 0, 0);
   return new KJSEmbed::QUrlBinding(exec, QUrl(copy))
END_CTOR

START_CTOR(QUrl, QUrl, 0 )
   return new KJSEmbed::QUrlBinding(exec, QUrl())
END_CTOR

// void setUrl(const QString &url)
START_VARIANT_METHOD( setUrl, QUrl )
   const QString & url = KJSEmbed::extractObject<const QString &>(exec, args, 0, 0);
END_VARIANT_METHOD

// void setUrl(const QString &url, ParsingMode mode)
START_VARIANT_METHOD( setUrl, QUrl )
   const QString & url = KJSEmbed::extractObject<const QString &>(exec, args, 0, 0);
   ParsingMode mode = KJSEmbed::extractObject<ParsingMode>(exec, args, 1, 0);
END_VARIANT_METHOD

// void setEncodedUrl(const QByteArray &url)
START_VARIANT_METHOD( setEncodedUrl, QUrl )
   const QByteArray & url = KJSEmbed::extractObject<const QByteArray &>(exec, args, 0, 0);
END_VARIANT_METHOD

// void setEncodedUrl(const QByteArray &url, ParsingMode mode)
START_VARIANT_METHOD( setEncodedUrl, QUrl )
   const QByteArray & url = KJSEmbed::extractObject<const QByteArray &>(exec, args, 0, 0);
   ParsingMode mode = KJSEmbed::extractObject<ParsingMode>(exec, args, 1, 0);
END_VARIANT_METHOD

// bool isValid() const 
START_VARIANT_METHOD( isValid, QUrl )
END_VARIANT_METHOD

// bool isEmpty() const 
START_VARIANT_METHOD( isEmpty, QUrl )
END_VARIANT_METHOD

// void clear()
START_VARIANT_METHOD( clear, QUrl )
END_VARIANT_METHOD

// void setScheme(const QString &scheme)
START_VARIANT_METHOD( setScheme, QUrl )
   const QString & scheme = KJSEmbed::extractObject<const QString &>(exec, args, 0, 0);
END_VARIANT_METHOD

// QString scheme() const 
START_VARIANT_METHOD( scheme, QUrl )
END_VARIANT_METHOD

// void setAuthority(const QString &authority)
START_VARIANT_METHOD( setAuthority, QUrl )
   const QString & authority = KJSEmbed::extractObject<const QString &>(exec, args, 0, 0);
END_VARIANT_METHOD

// QString authority() const 
START_VARIANT_METHOD( authority, QUrl )
END_VARIANT_METHOD

// void setUserInfo(const QString &userInfo)
START_VARIANT_METHOD( setUserInfo, QUrl )
   const QString & userInfo = KJSEmbed::extractObject<const QString &>(exec, args, 0, 0);
END_VARIANT_METHOD

// QString userInfo() const 
START_VARIANT_METHOD( userInfo, QUrl )
END_VARIANT_METHOD

// void setUserName(const QString &userName)
START_VARIANT_METHOD( setUserName, QUrl )
   const QString & userName = KJSEmbed::extractObject<const QString &>(exec, args, 0, 0);
END_VARIANT_METHOD

// QString userName() const 
START_VARIANT_METHOD( userName, QUrl )
END_VARIANT_METHOD

// void setPassword(const QString &password)
START_VARIANT_METHOD( setPassword, QUrl )
   const QString & password = KJSEmbed::extractObject<const QString &>(exec, args, 0, 0);
END_VARIANT_METHOD

// QString password() const 
START_VARIANT_METHOD( password, QUrl )
END_VARIANT_METHOD

// void setHost(const QString &host)
START_VARIANT_METHOD( setHost, QUrl )
   const QString & host = KJSEmbed::extractObject<const QString &>(exec, args, 0, 0);
END_VARIANT_METHOD

// QString host() const 
START_VARIANT_METHOD( host, QUrl )
END_VARIANT_METHOD

// void setPort(int port)
START_VARIANT_METHOD( setPort, QUrl )
   int port = KJSEmbed::extractObject<int>(exec, args, 0, 0);
END_VARIANT_METHOD

// int port() const 
START_VARIANT_METHOD( port, QUrl )
END_VARIANT_METHOD

// int port(int defaultPort) const 
START_VARIANT_METHOD( port, QUrl )
   int defaultPort = KJSEmbed::extractObject<int>(exec, args, 0, 0);
END_VARIANT_METHOD

// void setPath(const QString &path)
START_VARIANT_METHOD( setPath, QUrl )
   const QString & path = KJSEmbed::extractObject<const QString &>(exec, args, 0, 0);
END_VARIANT_METHOD

// QString path() const 
START_VARIANT_METHOD( path, QUrl )
END_VARIANT_METHOD

// void setEncodedQuery(const QByteArray &query)
START_VARIANT_METHOD( setEncodedQuery, QUrl )
   const QByteArray & query = KJSEmbed::extractObject<const QByteArray &>(exec, args, 0, 0);
END_VARIANT_METHOD

// QByteArray encodedQuery() const 
START_VARIANT_METHOD( encodedQuery, QUrl )
END_VARIANT_METHOD

// void setQueryDelimiters(char valueDelimiter, char pairDelimiter)
START_VARIANT_METHOD( setQueryDelimiters, QUrl )
   char valueDelimiter = KJSEmbed::extractObject<char>(exec, args, 0, 0);
   char pairDelimiter = KJSEmbed::extractObject<char>(exec, args, 1, 0);
END_VARIANT_METHOD

// char queryValueDelimiter() const 
START_VARIANT_METHOD( queryValueDelimiter, QUrl )
END_VARIANT_METHOD

// char queryPairDelimiter() const 
START_VARIANT_METHOD( queryPairDelimiter, QUrl )
END_VARIANT_METHOD

// void setQueryItems(const QList< QPair< QString, QString > > &query)
START_VARIANT_METHOD( setQueryItems, QUrl )
   const QList< QPair< QString, QString > > & query = KJSEmbed::extractObject<const QList< QPair< QString, QString > > &>(exec, args, 0, 0);
END_VARIANT_METHOD

// void addQueryItem(const QString &key, const QString &value)
START_VARIANT_METHOD( addQueryItem, QUrl )
   const QString & key = KJSEmbed::extractObject<const QString &>(exec, args, 0, 0);
   const QString & value = KJSEmbed::extractObject<const QString &>(exec, args, 1, 0);
END_VARIANT_METHOD

// QList< QPair< QString, QString > > queryItems() const 
START_VARIANT_METHOD( queryItems, QUrl )
END_VARIANT_METHOD

// bool hasQueryItem(const QString &key) const 
START_VARIANT_METHOD( hasQueryItem, QUrl )
   const QString & key = KJSEmbed::extractObject<const QString &>(exec, args, 0, 0);
END_VARIANT_METHOD

// QString queryItemValue(const QString &key) const 
START_VARIANT_METHOD( queryItemValue, QUrl )
   const QString & key = KJSEmbed::extractObject<const QString &>(exec, args, 0, 0);
END_VARIANT_METHOD

// QStringList allQueryItemValues(const QString &key) const 
START_VARIANT_METHOD( allQueryItemValues, QUrl )
   const QString & key = KJSEmbed::extractObject<const QString &>(exec, args, 0, 0);
END_VARIANT_METHOD

// void removeQueryItem(const QString &key)
START_VARIANT_METHOD( removeQueryItem, QUrl )
   const QString & key = KJSEmbed::extractObject<const QString &>(exec, args, 0, 0);
END_VARIANT_METHOD

// void removeAllQueryItems(const QString &key)
START_VARIANT_METHOD( removeAllQueryItems, QUrl )
   const QString & key = KJSEmbed::extractObject<const QString &>(exec, args, 0, 0);
END_VARIANT_METHOD

// void setFragment(const QString &fragment)
START_VARIANT_METHOD( setFragment, QUrl )
   const QString & fragment = KJSEmbed::extractObject<const QString &>(exec, args, 0, 0);
END_VARIANT_METHOD

// QString fragment() const 
START_VARIANT_METHOD( fragment, QUrl )
END_VARIANT_METHOD

// QUrl resolved(const QUrl &relative) const 
START_VARIANT_METHOD( resolved, QUrl )
   const QUrl & relative = KJSEmbed::extractObject<const QUrl &>(exec, args, 0, 0);
END_VARIANT_METHOD

// bool isRelative() const 
START_VARIANT_METHOD( isRelative, QUrl )
END_VARIANT_METHOD

// bool isParentOf(const QUrl &url) const 
START_VARIANT_METHOD( isParentOf, QUrl )
   const QUrl & url = KJSEmbed::extractObject<const QUrl &>(exec, args, 0, 0);
END_VARIANT_METHOD

// QString toLocalFile() const 
START_VARIANT_METHOD( toLocalFile, QUrl )
END_VARIANT_METHOD

// QString toString(FormattingOptions options=None) const 
START_VARIANT_METHOD( toString, QUrl )
   FormattingOptions options = KJSEmbed::extractObject<FormattingOptions>(exec, args, 0, None);
END_VARIANT_METHOD

// QByteArray toEncoded(FormattingOptions options=None) const 
START_VARIANT_METHOD( toEncoded, QUrl )
   FormattingOptions options = KJSEmbed::extractObject<FormattingOptions>(exec, args, 0, None);
END_VARIANT_METHOD

// void detach()
START_VARIANT_METHOD( detach, QUrl )
END_VARIANT_METHOD

// bool isDetached() const 
START_VARIANT_METHOD( isDetached, QUrl )
END_VARIANT_METHOD

// QUrl fromLocalFile(const QString &localfile)
START_VARIANT_METHOD( fromLocalFile, QUrl )
   const QString & localfile = KJSEmbed::extractObject<const QString &>(exec, args, 0, 0);
END_VARIANT_METHOD

// QUrl fromEncoded(const QByteArray &url)
START_VARIANT_METHOD( fromEncoded, QUrl )
   const QByteArray & url = KJSEmbed::extractObject<const QByteArray &>(exec, args, 0, 0);
END_VARIANT_METHOD

// QUrl fromEncoded(const QByteArray &url, ParsingMode mode)
START_VARIANT_METHOD( fromEncoded, QUrl )
   const QByteArray & url = KJSEmbed::extractObject<const QByteArray &>(exec, args, 0, 0);
   ParsingMode mode = KJSEmbed::extractObject<ParsingMode>(exec, args, 1, 0);
END_VARIANT_METHOD

// QString fromPercentEncoding(const QByteArray &)
START_VARIANT_METHOD( fromPercentEncoding, QUrl )
   const QByteArray &  = KJSEmbed::extractObject<const QByteArray &>(exec, args, 0, 0);
END_VARIANT_METHOD

// QByteArray toPercentEncoding(const QString &, const QByteArray &exclude=QByteArray(), const QByteArray &include=QByteArray())
START_VARIANT_METHOD( toPercentEncoding, QUrl )
   const QString &  = KJSEmbed::extractObject<const QString &>(exec, args, 0, 0);
   const QByteArray & exclude = KJSEmbed::extractObject<const QByteArray &>(exec, args, 1, QByteArray());
   const QByteArray & include = KJSEmbed::extractObject<const QByteArray &>(exec, args, 2, QByteArray());
END_VARIANT_METHOD

// QString fromPunycode(const QByteArray &)
START_VARIANT_METHOD( fromPunycode, QUrl )
   const QByteArray &  = KJSEmbed::extractObject<const QByteArray &>(exec, args, 0, 0);
END_VARIANT_METHOD

// QByteArray toPunycode(const QString &)
START_VARIANT_METHOD( toPunycode, QUrl )
   const QString &  = KJSEmbed::extractObject<const QString &>(exec, args, 0, 0);
END_VARIANT_METHOD
}


START_METHOD_LUT( QUrl)
{QUrl, 0, KJS::DontDelete|KJS::ReadOnly, &QUrlNS::QUrl },
{QUrl, 1, KJS::DontDelete|KJS::ReadOnly, &QUrlNS::QUrl },
{QUrl, 2, KJS::DontDelete|KJS::ReadOnly, &QUrlNS::QUrl },
{QUrl, 1, KJS::DontDelete|KJS::ReadOnly, &QUrlNS::QUrl },
{operator=, 1, KJS::DontDelete|KJS::ReadOnly, &QUrlNS::operator= },
{operator=, 1, KJS::DontDelete|KJS::ReadOnly, &QUrlNS::operator= },
{~QUrl, 0, KJS::DontDelete|KJS::ReadOnly, &QUrlNS::~QUrl },
{setUrl, 1, KJS::DontDelete|KJS::ReadOnly, &QUrlNS::setUrl },
{setUrl, 2, KJS::DontDelete|KJS::ReadOnly, &QUrlNS::setUrl },
{setEncodedUrl, 1, KJS::DontDelete|KJS::ReadOnly, &QUrlNS::setEncodedUrl },
{setEncodedUrl, 2, KJS::DontDelete|KJS::ReadOnly, &QUrlNS::setEncodedUrl },
{isValid, 0, KJS::DontDelete|KJS::ReadOnly, &QUrlNS::isValid },
{isEmpty, 0, KJS::DontDelete|KJS::ReadOnly, &QUrlNS::isEmpty },
{clear, 0, KJS::DontDelete|KJS::ReadOnly, &QUrlNS::clear },
{setScheme, 1, KJS::DontDelete|KJS::ReadOnly, &QUrlNS::setScheme },
{scheme, 0, KJS::DontDelete|KJS::ReadOnly, &QUrlNS::scheme },
{setAuthority, 1, KJS::DontDelete|KJS::ReadOnly, &QUrlNS::setAuthority },
{authority, 0, KJS::DontDelete|KJS::ReadOnly, &QUrlNS::authority },
{setUserInfo, 1, KJS::DontDelete|KJS::ReadOnly, &QUrlNS::setUserInfo },
{userInfo, 0, KJS::DontDelete|KJS::ReadOnly, &QUrlNS::userInfo },
{setUserName, 1, KJS::DontDelete|KJS::ReadOnly, &QUrlNS::setUserName },
{userName, 0, KJS::DontDelete|KJS::ReadOnly, &QUrlNS::userName },
{setPassword, 1, KJS::DontDelete|KJS::ReadOnly, &QUrlNS::setPassword },
{password, 0, KJS::DontDelete|KJS::ReadOnly, &QUrlNS::password },
{setHost, 1, KJS::DontDelete|KJS::ReadOnly, &QUrlNS::setHost },
{host, 0, KJS::DontDelete|KJS::ReadOnly, &QUrlNS::host },
{setPort, 1, KJS::DontDelete|KJS::ReadOnly, &QUrlNS::setPort },
{port, 0, KJS::DontDelete|KJS::ReadOnly, &QUrlNS::port },
{port, 1, KJS::DontDelete|KJS::ReadOnly, &QUrlNS::port },
{setPath, 1, KJS::DontDelete|KJS::ReadOnly, &QUrlNS::setPath },
{path, 0, KJS::DontDelete|KJS::ReadOnly, &QUrlNS::path },
{setEncodedQuery, 1, KJS::DontDelete|KJS::ReadOnly, &QUrlNS::setEncodedQuery },
{encodedQuery, 0, KJS::DontDelete|KJS::ReadOnly, &QUrlNS::encodedQuery },
{setQueryDelimiters, 2, KJS::DontDelete|KJS::ReadOnly, &QUrlNS::setQueryDelimiters },
{queryValueDelimiter, 0, KJS::DontDelete|KJS::ReadOnly, &QUrlNS::queryValueDelimiter },
{queryPairDelimiter, 0, KJS::DontDelete|KJS::ReadOnly, &QUrlNS::queryPairDelimiter },
{setQueryItems, 1, KJS::DontDelete|KJS::ReadOnly, &QUrlNS::setQueryItems },
{addQueryItem, 2, KJS::DontDelete|KJS::ReadOnly, &QUrlNS::addQueryItem },
{queryItems, 0, KJS::DontDelete|KJS::ReadOnly, &QUrlNS::queryItems },
{hasQueryItem, 1, KJS::DontDelete|KJS::ReadOnly, &QUrlNS::hasQueryItem },
{queryItemValue, 1, KJS::DontDelete|KJS::ReadOnly, &QUrlNS::queryItemValue },
{allQueryItemValues, 1, KJS::DontDelete|KJS::ReadOnly, &QUrlNS::allQueryItemValues },
{removeQueryItem, 1, KJS::DontDelete|KJS::ReadOnly, &QUrlNS::removeQueryItem },
{removeAllQueryItems, 1, KJS::DontDelete|KJS::ReadOnly, &QUrlNS::removeAllQueryItems },
{setFragment, 1, KJS::DontDelete|KJS::ReadOnly, &QUrlNS::setFragment },
{fragment, 0, KJS::DontDelete|KJS::ReadOnly, &QUrlNS::fragment },
{resolved, 1, KJS::DontDelete|KJS::ReadOnly, &QUrlNS::resolved },
{isRelative, 0, KJS::DontDelete|KJS::ReadOnly, &QUrlNS::isRelative },
{isParentOf, 1, KJS::DontDelete|KJS::ReadOnly, &QUrlNS::isParentOf },
{toLocalFile, 0, KJS::DontDelete|KJS::ReadOnly, &QUrlNS::toLocalFile },
{toString, 1, KJS::DontDelete|KJS::ReadOnly, &QUrlNS::toString },
{toEncoded, 1, KJS::DontDelete|KJS::ReadOnly, &QUrlNS::toEncoded },
{detach, 0, KJS::DontDelete|KJS::ReadOnly, &QUrlNS::detach },
{isDetached, 0, KJS::DontDelete|KJS::ReadOnly, &QUrlNS::isDetached },
{operator<, 1, KJS::DontDelete|KJS::ReadOnly, &QUrlNS::operator< },
{operator==, 1, KJS::DontDelete|KJS::ReadOnly, &QUrlNS::operator== },
{operator!=, 1, KJS::DontDelete|KJS::ReadOnly, &QUrlNS::operator!= },
{fromLocalFile, 1, KJS::DontDelete|KJS::ReadOnly, &QUrlNS::fromLocalFile },
{fromEncoded, 1, KJS::DontDelete|KJS::ReadOnly, &QUrlNS::fromEncoded },
{fromEncoded, 2, KJS::DontDelete|KJS::ReadOnly, &QUrlNS::fromEncoded },
{fromPercentEncoding, 1, KJS::DontDelete|KJS::ReadOnly, &QUrlNS::fromPercentEncoding },
{toPercentEncoding, 3, KJS::DontDelete|KJS::ReadOnly, &QUrlNS::toPercentEncoding },
{fromPunycode, 1, KJS::DontDelete|KJS::ReadOnly, &QUrlNS::fromPunycode },
{toPunycode, 1, KJS::DontDelete|KJS::ReadOnly, &QUrlNS::toPunycode },
END_METHOD_LUT

