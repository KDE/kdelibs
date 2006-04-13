<QtGui>

#include <QUrl_bind.h>

using namespace KJSEmbed;

const KJS::ClassInfo QUrlBinding::info = { "QUrlBinding", &ValueBinding::info, 0, 0 };
QUrlBinding::QUrl( KJS::ExecState *exec, const QUrl &value )
   : ValueBinding(exec, value)
{
   StaticBinding::publish(exec, this, QUrl::methods() );
   StaticBinding::publish(exec, this, ValueFactory::methods() );
}


namespace QUrlNS
{

// void setUrl(const QString &url)
START_VARIANT_METHOD( setUrl, QUrl )
   const QString & url = KJSEmbed::extractValue<const QString &>(exec, args, 0);
    value.setUrl(url);
END_VARIANT_METHOD

// void setUrl(const QString &url, ParsingMode mode)
START_VARIANT_METHOD( setUrl, QUrl )
   const QString & url = KJSEmbed::extractValue<const QString &>(exec, args, 0);
   ParsingMode mode = KJSEmbed::extractObject<ParsingMode>(exec, args, 1, 0);
    value.setUrl(mode);
END_VARIANT_METHOD

// void setEncodedUrl(const QByteArray &url)
START_VARIANT_METHOD( setEncodedUrl, QUrl )
   const QByteArray & url = KJSEmbed::extractValue<const QByteArray &>(exec, args, 0);
    value.setEncodedUrl(url);
END_VARIANT_METHOD

// void setEncodedUrl(const QByteArray &url, ParsingMode mode)
START_VARIANT_METHOD( setEncodedUrl, QUrl )
   const QByteArray & url = KJSEmbed::extractValue<const QByteArray &>(exec, args, 0);
   ParsingMode mode = KJSEmbed::extractObject<ParsingMode>(exec, args, 1, 0);
    value.setEncodedUrl(mode);
END_VARIANT_METHOD

// bool isValid() const 
START_VARIANT_METHOD( isValid, QUrl )
   bool tmp = value.isValid();
   result = KJSEmbed::createValue( exec, "bool", tmp );
END_VARIANT_METHOD

// bool isEmpty() const 
START_VARIANT_METHOD( isEmpty, QUrl )
   bool tmp = value.isEmpty();
   result = KJSEmbed::createValue( exec, "bool", tmp );
END_VARIANT_METHOD

// void clear()
START_VARIANT_METHOD( clear, QUrl )
   void tmp = value.clear();
   result = KJSEmbed::createValue( exec, "void", tmp );
END_VARIANT_METHOD

// void setScheme(const QString &scheme)
START_VARIANT_METHOD( setScheme, QUrl )
   const QString & scheme = KJSEmbed::extractValue<const QString &>(exec, args, 0);
    value.setScheme(scheme);
END_VARIANT_METHOD

// QString scheme() const 
START_VARIANT_METHOD( scheme, QUrl )
   QString tmp = value.scheme();
   result = KJSEmbed::createValue( exec, "QString", tmp );
END_VARIANT_METHOD

// void setAuthority(const QString &authority)
START_VARIANT_METHOD( setAuthority, QUrl )
   const QString & authority = KJSEmbed::extractValue<const QString &>(exec, args, 0);
    value.setAuthority(authority);
END_VARIANT_METHOD

// QString authority() const 
START_VARIANT_METHOD( authority, QUrl )
   QString tmp = value.authority();
   result = KJSEmbed::createValue( exec, "QString", tmp );
END_VARIANT_METHOD

// void setUserInfo(const QString &userInfo)
START_VARIANT_METHOD( setUserInfo, QUrl )
   const QString & userInfo = KJSEmbed::extractValue<const QString &>(exec, args, 0);
    value.setUserInfo(userInfo);
END_VARIANT_METHOD

// QString userInfo() const 
START_VARIANT_METHOD( userInfo, QUrl )
   QString tmp = value.userInfo();
   result = KJSEmbed::createValue( exec, "QString", tmp );
END_VARIANT_METHOD

// void setUserName(const QString &userName)
START_VARIANT_METHOD( setUserName, QUrl )
   const QString & userName = KJSEmbed::extractValue<const QString &>(exec, args, 0);
    value.setUserName(userName);
END_VARIANT_METHOD

// QString userName() const 
START_VARIANT_METHOD( userName, QUrl )
   QString tmp = value.userName();
   result = KJSEmbed::createValue( exec, "QString", tmp );
END_VARIANT_METHOD

// void setPassword(const QString &password)
START_VARIANT_METHOD( setPassword, QUrl )
   const QString & password = KJSEmbed::extractValue<const QString &>(exec, args, 0);
    value.setPassword(password);
END_VARIANT_METHOD

// QString password() const 
START_VARIANT_METHOD( password, QUrl )
   QString tmp = value.password();
   result = KJSEmbed::createValue( exec, "QString", tmp );
END_VARIANT_METHOD

// void setHost(const QString &host)
START_VARIANT_METHOD( setHost, QUrl )
   const QString & host = KJSEmbed::extractValue<const QString &>(exec, args, 0);
    value.setHost(host);
END_VARIANT_METHOD

// QString host() const 
START_VARIANT_METHOD( host, QUrl )
   QString tmp = value.host();
   result = KJSEmbed::createValue( exec, "QString", tmp );
END_VARIANT_METHOD

// void setPort(int port)
START_VARIANT_METHOD( setPort, QUrl )
   int port = KJSEmbed::extractValue<int>(exec, args, 0);
    value.setPort(port);
END_VARIANT_METHOD

// int port() const 
START_VARIANT_METHOD( port, QUrl )
   int tmp = value.port();
   result = KJSEmbed::createValue( exec, "int", tmp );
END_VARIANT_METHOD

// int port(int defaultPort) const 
START_VARIANT_METHOD( port, QUrl )
   int defaultPort = KJSEmbed::extractValue<int>(exec, args, 0);
END_VARIANT_METHOD

// void setPath(const QString &path)
START_VARIANT_METHOD( setPath, QUrl )
   const QString & path = KJSEmbed::extractValue<const QString &>(exec, args, 0);
    value.setPath(path);
END_VARIANT_METHOD

// QString path() const 
START_VARIANT_METHOD( path, QUrl )
   QString tmp = value.path();
   result = KJSEmbed::createValue( exec, "QString", tmp );
END_VARIANT_METHOD

// void setEncodedQuery(const QByteArray &query)
START_VARIANT_METHOD( setEncodedQuery, QUrl )
   const QByteArray & query = KJSEmbed::extractValue<const QByteArray &>(exec, args, 0);
    value.setEncodedQuery(query);
END_VARIANT_METHOD

// QByteArray encodedQuery() const 
START_VARIANT_METHOD( encodedQuery, QUrl )
   QByteArray tmp = value.encodedQuery();
   result = KJSEmbed::createValue( exec, "QByteArray", tmp );
END_VARIANT_METHOD

// void setQueryDelimiters(char valueDelimiter, char pairDelimiter)
START_VARIANT_METHOD( setQueryDelimiters, QUrl )
   char valueDelimiter = KJSEmbed::extractObject<char>(exec, args, 0, 0);
   char pairDelimiter = KJSEmbed::extractObject<char>(exec, args, 1, 0);
    value.setQueryDelimiters(pairDelimiter);
END_VARIANT_METHOD

// char queryValueDelimiter() const 
START_VARIANT_METHOD( queryValueDelimiter, QUrl )
   char tmp = value.queryValueDelimiter();
   result = KJSEmbed::createValue( exec, "char", tmp );
END_VARIANT_METHOD

// char queryPairDelimiter() const 
START_VARIANT_METHOD( queryPairDelimiter, QUrl )
   char tmp = value.queryPairDelimiter();
   result = KJSEmbed::createValue( exec, "char", tmp );
END_VARIANT_METHOD

// void setQueryItems(const QList< QPair< QString, QString > > &query)
START_VARIANT_METHOD( setQueryItems, QUrl )
   const QList< QPair< QString, QString > > & query = KJSEmbed::extractValue<const QList< QPair< QString, QString > > &>(exec, args, 0);
    value.setQueryItems(query);
END_VARIANT_METHOD

// void addQueryItem(const QString &key, const QString &value)
START_VARIANT_METHOD( addQueryItem, QUrl )
   const QString & key = KJSEmbed::extractValue<const QString &>(exec, args, 0);
   const QString & value = KJSEmbed::extractValue<const QString &>(exec, args, 0);
END_VARIANT_METHOD

// QList< QPair< QString, QString > > queryItems() const 
START_VARIANT_METHOD( queryItems, QUrl )
   QList< QPair< QString, QString > > tmp = value.queryItems();
   result = KJSEmbed::createValue( exec, "QList< QPair< QString, QString > >", tmp );
END_VARIANT_METHOD

// bool hasQueryItem(const QString &key) const 
START_VARIANT_METHOD( hasQueryItem, QUrl )
   const QString & key = KJSEmbed::extractValue<const QString &>(exec, args, 0);
END_VARIANT_METHOD

// QString queryItemValue(const QString &key) const 
START_VARIANT_METHOD( queryItemValue, QUrl )
   const QString & key = KJSEmbed::extractValue<const QString &>(exec, args, 0);
END_VARIANT_METHOD

// QStringList allQueryItemValues(const QString &key) const 
START_VARIANT_METHOD( allQueryItemValues, QUrl )
   const QString & key = KJSEmbed::extractValue<const QString &>(exec, args, 0);
END_VARIANT_METHOD

// void removeQueryItem(const QString &key)
START_VARIANT_METHOD( removeQueryItem, QUrl )
   const QString & key = KJSEmbed::extractValue<const QString &>(exec, args, 0);
END_VARIANT_METHOD

// void removeAllQueryItems(const QString &key)
START_VARIANT_METHOD( removeAllQueryItems, QUrl )
   const QString & key = KJSEmbed::extractValue<const QString &>(exec, args, 0);
END_VARIANT_METHOD

// void setFragment(const QString &fragment)
START_VARIANT_METHOD( setFragment, QUrl )
   const QString & fragment = KJSEmbed::extractValue<const QString &>(exec, args, 0);
    value.setFragment(fragment);
END_VARIANT_METHOD

// QString fragment() const 
START_VARIANT_METHOD( fragment, QUrl )
   QString tmp = value.fragment();
   result = KJSEmbed::createValue( exec, "QString", tmp );
END_VARIANT_METHOD

// QUrl resolved(const QUrl &relative) const 
START_VARIANT_METHOD( resolved, QUrl )
   const QUrl & relative = KJSEmbed::extractValue<const QUrl &>(exec, args, 0);
END_VARIANT_METHOD

// bool isRelative() const 
START_VARIANT_METHOD( isRelative, QUrl )
   bool tmp = value.isRelative();
   result = KJSEmbed::createValue( exec, "bool", tmp );
END_VARIANT_METHOD

// bool isParentOf(const QUrl &url) const 
START_VARIANT_METHOD( isParentOf, QUrl )
   const QUrl & url = KJSEmbed::extractValue<const QUrl &>(exec, args, 0);
END_VARIANT_METHOD

// QString toLocalFile() const 
START_VARIANT_METHOD( toLocalFile, QUrl )
   QString tmp = value.toLocalFile();
   result = KJSEmbed::createValue( exec, "QString", tmp );
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
   void tmp = value.detach();
   result = KJSEmbed::createValue( exec, "void", tmp );
END_VARIANT_METHOD

// bool isDetached() const 
START_VARIANT_METHOD( isDetached, QUrl )
   bool tmp = value.isDetached();
   result = KJSEmbed::createValue( exec, "bool", tmp );
END_VARIANT_METHOD

// QUrl fromLocalFile(const QString &localfile)
START_VARIANT_METHOD( fromLocalFile, QUrl )
   const QString & localfile = KJSEmbed::extractValue<const QString &>(exec, args, 0);
END_VARIANT_METHOD

// QUrl fromEncoded(const QByteArray &url)
START_VARIANT_METHOD( fromEncoded, QUrl )
   const QByteArray & url = KJSEmbed::extractValue<const QByteArray &>(exec, args, 0);
END_VARIANT_METHOD

// QUrl fromEncoded(const QByteArray &url, ParsingMode mode)
START_VARIANT_METHOD( fromEncoded, QUrl )
   const QByteArray & url = KJSEmbed::extractValue<const QByteArray &>(exec, args, 0);
   ParsingMode mode = KJSEmbed::extractObject<ParsingMode>(exec, args, 1, 0);
END_VARIANT_METHOD

// QString fromPercentEncoding(const QByteArray &)
START_VARIANT_METHOD( fromPercentEncoding, QUrl )
   const QByteArray & arg0 = KJSEmbed::extractValue<const QByteArray &>(exec, args, 0);
END_VARIANT_METHOD

// QByteArray toPercentEncoding(const QString &, const QByteArray &exclude=QByteArray(), const QByteArray &include=QByteArray())
START_VARIANT_METHOD( toPercentEncoding, QUrl )
   const QString & arg0 = KJSEmbed::extractValue<const QString &>(exec, args, 0);
   const QByteArray & exclude = KJSEmbed::extractValue<const QByteArray &>(exec, args, QByteArray());
   const QByteArray & include = KJSEmbed::extractValue<const QByteArray &>(exec, args, QByteArray());
END_VARIANT_METHOD

// QString fromPunycode(const QByteArray &)
START_VARIANT_METHOD( fromPunycode, QUrl )
   const QByteArray & arg0 = KJSEmbed::extractValue<const QByteArray &>(exec, args, 0);
END_VARIANT_METHOD

// QByteArray toPunycode(const QString &)
START_VARIANT_METHOD( toPunycode, QUrl )
   const QString & arg0 = KJSEmbed::extractValue<const QString &>(exec, args, 0);
END_VARIANT_METHOD
}

NO_ENUMS( QUrl ) 
NO_STATICS( QUrl ) 



START_CTOR( QUrl, QUrl, 0 )
   if (args.size() == 0 )
   {
       return new KJSEmbed::QUrlBinding(exec, QUrl())
   }
   if (args.size() == 1 )
   {
   const QString & url = KJSEmbed::extractValue<const QString &>(exec, args, 0);
       return new KJSEmbed::QUrlBinding(exec, QUrl(url))
   }
   if (args.size() == 2 )
   {
   const QString & url = KJSEmbed::extractValue<const QString &>(exec, args, 0);
   ParsingMode mode = KJSEmbed::extractObject<ParsingMode>(exec, args, 1, 0);
       return new KJSEmbed::QUrlBinding(exec, QUrl(url, mode))
   }
   if (args.size() == 1 )
   {
   const QUrl & copy = KJSEmbed::extractValue<const QUrl &>(exec, args, 0);
       return new KJSEmbed::QUrlBinding(exec, QUrl(copy))
   }
END_CTOR


START_METHOD_LUT( QUrl )
    { setUrl, 1, KJS::DontDelete|KJS::ReadOnly, &QUrlNS::setUrl },
    { setUrl, 2, KJS::DontDelete|KJS::ReadOnly, &QUrlNS::setUrl },
    { setEncodedUrl, 1, KJS::DontDelete|KJS::ReadOnly, &QUrlNS::setEncodedUrl },
    { setEncodedUrl, 2, KJS::DontDelete|KJS::ReadOnly, &QUrlNS::setEncodedUrl },
    { isValid, 0, KJS::DontDelete|KJS::ReadOnly, &QUrlNS::isValid },
    { isEmpty, 0, KJS::DontDelete|KJS::ReadOnly, &QUrlNS::isEmpty },
    { clear, 0, KJS::DontDelete|KJS::ReadOnly, &QUrlNS::clear },
    { setScheme, 1, KJS::DontDelete|KJS::ReadOnly, &QUrlNS::setScheme },
    { scheme, 0, KJS::DontDelete|KJS::ReadOnly, &QUrlNS::scheme },
    { setAuthority, 1, KJS::DontDelete|KJS::ReadOnly, &QUrlNS::setAuthority },
    { authority, 0, KJS::DontDelete|KJS::ReadOnly, &QUrlNS::authority },
    { setUserInfo, 1, KJS::DontDelete|KJS::ReadOnly, &QUrlNS::setUserInfo },
    { userInfo, 0, KJS::DontDelete|KJS::ReadOnly, &QUrlNS::userInfo },
    { setUserName, 1, KJS::DontDelete|KJS::ReadOnly, &QUrlNS::setUserName },
    { userName, 0, KJS::DontDelete|KJS::ReadOnly, &QUrlNS::userName },
    { setPassword, 1, KJS::DontDelete|KJS::ReadOnly, &QUrlNS::setPassword },
    { password, 0, KJS::DontDelete|KJS::ReadOnly, &QUrlNS::password },
    { setHost, 1, KJS::DontDelete|KJS::ReadOnly, &QUrlNS::setHost },
    { host, 0, KJS::DontDelete|KJS::ReadOnly, &QUrlNS::host },
    { setPort, 1, KJS::DontDelete|KJS::ReadOnly, &QUrlNS::setPort },
    { port, 0, KJS::DontDelete|KJS::ReadOnly, &QUrlNS::port },
    { port, 1, KJS::DontDelete|KJS::ReadOnly, &QUrlNS::port },
    { setPath, 1, KJS::DontDelete|KJS::ReadOnly, &QUrlNS::setPath },
    { path, 0, KJS::DontDelete|KJS::ReadOnly, &QUrlNS::path },
    { setEncodedQuery, 1, KJS::DontDelete|KJS::ReadOnly, &QUrlNS::setEncodedQuery },
    { encodedQuery, 0, KJS::DontDelete|KJS::ReadOnly, &QUrlNS::encodedQuery },
    { setQueryDelimiters, 2, KJS::DontDelete|KJS::ReadOnly, &QUrlNS::setQueryDelimiters },
    { queryValueDelimiter, 0, KJS::DontDelete|KJS::ReadOnly, &QUrlNS::queryValueDelimiter },
    { queryPairDelimiter, 0, KJS::DontDelete|KJS::ReadOnly, &QUrlNS::queryPairDelimiter },
    { setQueryItems, 1, KJS::DontDelete|KJS::ReadOnly, &QUrlNS::setQueryItems },
    { addQueryItem, 2, KJS::DontDelete|KJS::ReadOnly, &QUrlNS::addQueryItem },
    { queryItems, 0, KJS::DontDelete|KJS::ReadOnly, &QUrlNS::queryItems },
    { hasQueryItem, 1, KJS::DontDelete|KJS::ReadOnly, &QUrlNS::hasQueryItem },
    { queryItemValue, 1, KJS::DontDelete|KJS::ReadOnly, &QUrlNS::queryItemValue },
    { allQueryItemValues, 1, KJS::DontDelete|KJS::ReadOnly, &QUrlNS::allQueryItemValues },
    { removeQueryItem, 1, KJS::DontDelete|KJS::ReadOnly, &QUrlNS::removeQueryItem },
    { removeAllQueryItems, 1, KJS::DontDelete|KJS::ReadOnly, &QUrlNS::removeAllQueryItems },
    { setFragment, 1, KJS::DontDelete|KJS::ReadOnly, &QUrlNS::setFragment },
    { fragment, 0, KJS::DontDelete|KJS::ReadOnly, &QUrlNS::fragment },
    { resolved, 1, KJS::DontDelete|KJS::ReadOnly, &QUrlNS::resolved },
    { isRelative, 0, KJS::DontDelete|KJS::ReadOnly, &QUrlNS::isRelative },
    { isParentOf, 1, KJS::DontDelete|KJS::ReadOnly, &QUrlNS::isParentOf },
    { toLocalFile, 0, KJS::DontDelete|KJS::ReadOnly, &QUrlNS::toLocalFile },
    { toString, 1, KJS::DontDelete|KJS::ReadOnly, &QUrlNS::toString },
    { toEncoded, 1, KJS::DontDelete|KJS::ReadOnly, &QUrlNS::toEncoded },
    { detach, 0, KJS::DontDelete|KJS::ReadOnly, &QUrlNS::detach },
    { isDetached, 0, KJS::DontDelete|KJS::ReadOnly, &QUrlNS::isDetached },
    { fromLocalFile, 1, KJS::DontDelete|KJS::ReadOnly, &QUrlNS::fromLocalFile },
    { fromEncoded, 1, KJS::DontDelete|KJS::ReadOnly, &QUrlNS::fromEncoded },
    { fromEncoded, 2, KJS::DontDelete|KJS::ReadOnly, &QUrlNS::fromEncoded },
    { fromPercentEncoding, 1, KJS::DontDelete|KJS::ReadOnly, &QUrlNS::fromPercentEncoding },
    { toPercentEncoding, 3, KJS::DontDelete|KJS::ReadOnly, &QUrlNS::toPercentEncoding },
    { fromPunycode, 1, KJS::DontDelete|KJS::ReadOnly, &QUrlNS::fromPunycode },
    { toPunycode, 1, KJS::DontDelete|KJS::ReadOnly, &QUrlNS::toPunycode },
END_METHOD_LUT

