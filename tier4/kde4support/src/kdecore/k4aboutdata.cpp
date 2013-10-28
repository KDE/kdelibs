/*
 * This file is part of the KDE Libraries
 * Copyright (C) 2000 Espen Sand (espen@kde.org)
 * Copyright (C) 2006 Nicolas GOUTTE <goutte@kde.org>
 * Copyright (C) 2008 Friedrich W. H. Kossebau <kossebau@kde.org>
 * Copyright (C) 2010 Teo Mrnjavac <teo@kde.org>
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
 *
 */

#include "k4aboutdata.h"
#include "kaboutdata.h"

#include <QStandardPaths>
#include <QtCore/QFile>
#include <QtCore/QTextStream>
#include <QtCore/QSharedData>
#include <QtCore/QVariant>
#include <QtCore/QList>
#include <QHash>

// -----------------------------------------------------------------------------
// Design notes:
//
// These classes deal with a lot of text, some of which needs to be
// marked for translation. Since at the time when these object and calls are
// made the translation catalogs are usually still not initialized, the
// translation has to be delayed. This is achieved by using KLocalizedString
// for translatable strings. KLocalizedStrings are produced by ki18n* calls,
// instead of the more usuall i18n* calls which produce QString by trying to
// translate immediately.
//
// All the non-translatable string arguments to methods are taken QByteArray,
// all the translatable are KLocalizedString. The getter methods always return
// proper QString: the non-translatable strings supplied by the code are
// treated with QString::fromUtf8(), those coming from the outside with
// QTextCodec::toUnicode(), and translatable strings are finalized to QStrings
// at the point of getter calls (i.e. delayed translation).
// -----------------------------------------------------------------------------

class K4AboutPerson::Private
{
public:
   KLocalizedString _name;
   KLocalizedString _task;
   QString _emailAddress;
   QString _webAddress;
   QString _ocsUsername;

   QString _nameNoop;
};

K4AboutPerson::K4AboutPerson( const KLocalizedString &_name,
                            const KLocalizedString &_task,
                            const QByteArray &_emailAddress,
                            const QByteArray &_webAddress )
  : d(new Private)
{
   d->_name = _name;
   d->_task = _task;
   d->_emailAddress = QString::fromUtf8(_emailAddress.data());
   d->_webAddress = QString::fromUtf8(_webAddress.data());
}

K4AboutPerson::K4AboutPerson( const KLocalizedString &_name,
                            const KLocalizedString &_task,
                            const QByteArray &_emailAddress,
                            const QByteArray &_webAddress,
                            const QByteArray &_ocsUsername )
  : d(new Private)
{
   d->_name = _name;
   d->_task = _task;
   d->_emailAddress = QString::fromUtf8(_emailAddress.data());
   d->_webAddress = QString::fromUtf8(_webAddress.data());
   d->_ocsUsername = QString::fromUtf8( _ocsUsername.data() );
}

K4AboutPerson::K4AboutPerson( const QString &_name, const QString &_email )
  : d(new Private)
{
   d->_nameNoop = _name;
   d->_emailAddress = _email;
}

K4AboutPerson::K4AboutPerson(const K4AboutPerson& other): d(new Private)
{
    *d = *other.d;
}

K4AboutPerson::~K4AboutPerson()
{
   delete d;
}

QString K4AboutPerson::name() const
{
   if (!d->_nameNoop.isEmpty())
      return d->_nameNoop;
   return d->_name.toString();
}

QString K4AboutPerson::task() const
{
   if (!d->_task.isEmpty())
      return d->_task.toString();
   return QString();
}

QString K4AboutPerson::emailAddress() const
{
   return d->_emailAddress;
}


QString K4AboutPerson::webAddress() const
{
   return d->_webAddress;
}

QString K4AboutPerson::ocsUsername() const
{
    return d->_ocsUsername;
}

K4AboutPerson &K4AboutPerson::operator=(const K4AboutPerson& other)
{
   *d = *other.d;
   return *this;
}



class K4AboutLicense::Private : public QSharedData
{
public:
    Private( enum K4AboutData::LicenseKey licenseType, const K4AboutData *aboutData );
    Private( const QString &pathToFile, const K4AboutData *aboutData );
    Private( const KLocalizedString &licenseText, const K4AboutData *aboutData );
    Private( const Private& other);
public:
    enum K4AboutData::LicenseKey  _licenseKey;
    KLocalizedString             _licenseText;
    QString                      _pathToLicenseTextFile;
    // needed for access to the possibly changing copyrightStatement()
    const K4AboutData *           _aboutData;
};

K4AboutLicense::Private::Private( enum K4AboutData::LicenseKey licenseType, const K4AboutData *aboutData )
  : QSharedData(),
    _licenseKey( licenseType ),
    _aboutData( aboutData )
{
}

K4AboutLicense::Private::Private( const QString &pathToFile, const K4AboutData *aboutData )
  : QSharedData(),
    _licenseKey( K4AboutData::License_File ),
    _pathToLicenseTextFile( pathToFile ),
    _aboutData( aboutData )
{
}

K4AboutLicense::Private::Private( const KLocalizedString &licenseText, const K4AboutData *aboutData )
  : QSharedData(),
    _licenseKey( K4AboutData::License_Custom ),
    _licenseText( licenseText ),
    _aboutData( aboutData )
{
}

K4AboutLicense::Private::Private(const K4AboutLicense::Private& other)
  : QSharedData(other),
    _licenseKey( other._licenseKey ),
    _licenseText( other._licenseText ),
    _pathToLicenseTextFile( other._pathToLicenseTextFile ),
    _aboutData( other._aboutData )
{}


K4AboutLicense::K4AboutLicense( enum K4AboutData::LicenseKey licenseType, const K4AboutData *aboutData )
  : d(new Private(licenseType,aboutData))
{
}

K4AboutLicense::K4AboutLicense( const QString &pathToFile, const K4AboutData *aboutData )
  : d(new Private(pathToFile,aboutData))
{
}

K4AboutLicense::K4AboutLicense( const KLocalizedString &licenseText, const K4AboutData *aboutData )
  : d(new Private(licenseText,aboutData))
{
}

K4AboutLicense::K4AboutLicense(const K4AboutLicense& other)
  : d(other.d)
{
}

K4AboutLicense::~K4AboutLicense()
{}

QString K4AboutLicense::text() const
{
    QString result;

    const QString lineFeed = QString::fromLatin1( "\n\n" );

    if (d->_aboutData && !d->_aboutData->copyrightStatement().isEmpty()) {
        result = d->_aboutData->copyrightStatement() + lineFeed;
    }

    bool knownLicense = false;
    QString pathToFile;
    switch ( d->_licenseKey )
    {
    case K4AboutData::License_File:
        pathToFile = d->_pathToLicenseTextFile;
        break;
    case K4AboutData::License_GPL_V2:
        knownLicense = true;
        pathToFile = QStandardPaths::locate(QStandardPaths::GenericDataLocation, QString::fromLatin1("LICENSES/GPL_V2"));
        break;
    case K4AboutData::License_LGPL_V2:
        knownLicense = true;
        pathToFile = QStandardPaths::locate(QStandardPaths::GenericDataLocation, QString::fromLatin1("LICENSES/LGPL_V2"));
        break;
    case K4AboutData::License_BSD:
        knownLicense = true;
        pathToFile = QStandardPaths::locate(QStandardPaths::GenericDataLocation, QString::fromLatin1("LICENSES/BSD"));
        break;
    case K4AboutData::License_Artistic:
        knownLicense = true;
        pathToFile = QStandardPaths::locate(QStandardPaths::GenericDataLocation, QString::fromLatin1("LICENSES/ARTISTIC"));
        break;
    case K4AboutData::License_QPL_V1_0:
        knownLicense = true;
        pathToFile = QStandardPaths::locate(QStandardPaths::GenericDataLocation, QString::fromLatin1("LICENSES/QPL_V1.0"));
        break;
    case K4AboutData::License_GPL_V3:
        knownLicense = true;
        pathToFile = QStandardPaths::locate(QStandardPaths::GenericDataLocation, QString::fromLatin1("LICENSES/GPL_V3"));
        break;
    case K4AboutData::License_LGPL_V3:
        knownLicense = true;
        pathToFile = QStandardPaths::locate(QStandardPaths::GenericDataLocation, QString::fromLatin1("LICENSES/LGPL_V3"));
        break;
    case K4AboutData::License_Custom:
        if (!d->_licenseText.isEmpty()) {
            result = d->_licenseText.toString();
            break;
        }
        // fall through
    default:
        result += QObject::tr("No licensing terms for this program have been specified.\n"
                       "Please check the documentation or the source for any\n"
                       "licensing terms.\n");
    }

    if (knownLicense) {
        result += QObject::tr("This program is distributed under the terms of the %1.").arg(name(K4AboutData::ShortName));
        if (!pathToFile.isEmpty()) {
            result += lineFeed;
        }
    }

    if (!pathToFile.isEmpty()) {
        QFile file(pathToFile);
        if (file.open(QIODevice::ReadOnly)) {
            QTextStream str(&file);
            result += str.readAll();
        }
    }

    return result;
}


QString K4AboutLicense::name(K4AboutData::NameFormat formatName) const
{
    QString licenseShort;
    QString licenseFull;

    switch (d->_licenseKey) {
    case K4AboutData::License_GPL_V2:
        licenseShort = i18nc("@item license (short name)","GPL v2");
        licenseFull = i18nc("@item license","GNU General Public License Version 2");
        break;
    case K4AboutData::License_LGPL_V2:
        licenseShort = i18nc("@item license (short name)","LGPL v2");
        licenseFull = i18nc("@item license","GNU Lesser General Public License Version 2");
        break;
    case K4AboutData::License_BSD:
        licenseShort = i18nc("@item license (short name)","BSD License");
        licenseFull = i18nc("@item license","BSD License");
        break;
    case K4AboutData::License_Artistic:
        licenseShort = i18nc("@item license (short name)","Artistic License");
        licenseFull = i18nc("@item license","Artistic License");
        break;
    case K4AboutData::License_QPL_V1_0:
        licenseShort = i18nc("@item license (short name)","QPL v1.0");
        licenseFull = i18nc("@item license","Q Public License");
        break;
    case K4AboutData::License_GPL_V3:
        licenseShort = i18nc("@item license (short name)","GPL v3");
        licenseFull = i18nc("@item license","GNU General Public License Version 3");
        break;
    case K4AboutData::License_LGPL_V3:
        licenseShort = i18nc("@item license (short name)","LGPL v3");
        licenseFull = i18nc("@item license","GNU Lesser General Public License Version 3");
        break;
    case K4AboutData::License_Custom:
    case K4AboutData::License_File:
        licenseShort = licenseFull = i18nc("@item license","Custom");
        break;
    default:
        licenseShort = licenseFull = i18nc("@item license","Not specified");
    }

    const QString result =
        (formatName == K4AboutData::ShortName ) ? licenseShort :
        (formatName == K4AboutData::FullName ) ?  licenseFull :
                                                 QString();

    return result;
}


K4AboutLicense &K4AboutLicense::operator=(const K4AboutLicense& other)
{
   d = other.d;
   return *this;
}

K4AboutData::LicenseKey K4AboutLicense::key() const
{
    return d->_licenseKey;
}

K4AboutLicense K4AboutLicense::byKeyword(const QString &rawKeyword)
{
    // Setup keyword->enum dictionary on first call.
    // Use normalized keywords, by the algorithm below.
    static QHash<QByteArray, K4AboutData::LicenseKey> ldict;
    if (ldict.isEmpty()) {
        ldict.insert("gpl", K4AboutData::License_GPL);
        ldict.insert("gplv2", K4AboutData::License_GPL_V2);
        ldict.insert("gplv2+", K4AboutData::License_GPL_V2);
        ldict.insert("lgpl", K4AboutData::License_LGPL);
        ldict.insert("lgplv2", K4AboutData::License_LGPL_V2);
        ldict.insert("lgplv2+", K4AboutData::License_LGPL_V2);
        ldict.insert("bsd", K4AboutData::License_BSD);
        ldict.insert("artistic", K4AboutData::License_Artistic);
        ldict.insert("qpl", K4AboutData::License_QPL);
        ldict.insert("qplv1", K4AboutData::License_QPL_V1_0);
        ldict.insert("qplv10", K4AboutData::License_QPL_V1_0);
        ldict.insert("gplv3", K4AboutData::License_GPL_V3);
        ldict.insert("gplv3+", K4AboutData::License_GPL_V3);
        ldict.insert("lgplv3", K4AboutData::License_LGPL_V3);
        ldict.insert("lgplv3+", K4AboutData::License_LGPL_V3);
    }

    // Normalize keyword.
    QString keyword = rawKeyword;
    keyword = keyword.toLower();
    keyword.remove(QLatin1Char(' '));
    keyword.remove(QLatin1Char('.'));

    K4AboutData::LicenseKey license = ldict.value(keyword.toLatin1(),
                                                 K4AboutData::License_Custom);
    return K4AboutLicense(license, 0);
}


class K4AboutData::Private
{
public:
    Private()
        : customAuthorTextEnabled(false)
        {}
    QByteArray _appName;
    KLocalizedString _programName;
    KLocalizedString _shortDescription;
    QByteArray _catalogName;
    KLocalizedString _copyrightStatement;
    KLocalizedString _otherText;
    QString _homepageAddress;
    QList<K4AboutPerson> _authorList;
    QList<K4AboutPerson> _creditList;
    QList<K4AboutLicense> _licenseList;
    KLocalizedString translatorName;
    KLocalizedString translatorEmail;
    QString productName;
    QString programIconName;
    QVariant programLogo;
    KLocalizedString customAuthorPlainText, customAuthorRichText;
    bool customAuthorTextEnabled;

    QString organizationDomain;
    QByteArray _ocsProviderUrl;

    // Everything dr.konqi needs, we store as utf-8, so we
    // can just give it a pointer, w/o any allocations.
    QByteArray _translatedProgramName; // ### I don't see it ever being translated, and I did not change that
    QByteArray _version;
    QByteArray _bugEmailAddress;
};


K4AboutData::K4AboutData( const QByteArray &_appName,
                        const QByteArray &_catalogName,
                        const KLocalizedString &_programName,
                        const QByteArray &_version,
                        const KLocalizedString &_shortDescription,
                        enum LicenseKey licenseType,
                        const KLocalizedString &_copyrightStatement,
                        const KLocalizedString &text,
                        const QByteArray &homePageAddress,
                        const QByteArray &bugsEmailAddress
                      )
  : d(new Private)
{
    d->_appName = _appName;
    int p = d->_appName.indexOf('/');
    if (p >= 0) {
        d->_appName = d->_appName.mid(p + 1);
    }

    d->_catalogName = _catalogName;
    d->_programName = _programName;
    if (!d->_programName.isEmpty()) // KComponentData("klauncher") gives empty program name
        d->_translatedProgramName = _programName.toString().toUtf8();
    d->_version = _version;
    d->_shortDescription = _shortDescription;
    d->_licenseList.append(K4AboutLicense(licenseType,this));
    d->_copyrightStatement = _copyrightStatement;
    d->_otherText = text;
    d->_homepageAddress = QString::fromLatin1(homePageAddress.data());
    d->_bugEmailAddress = bugsEmailAddress;

    if (d->_homepageAddress.contains(QLatin1String("http://"))) {
        const int dot = d->_homepageAddress.indexOf(QLatin1Char('.'));
        if (dot >= 0) {
            d->organizationDomain = d->_homepageAddress.mid(dot + 1);
            const int slash = d->organizationDomain.indexOf(QLatin1Char('/'));
            if (slash >= 0)
                d->organizationDomain.truncate(slash);
        }
        else {
            d->organizationDomain = QString::fromLatin1("kde.org");
        }
    }
    else {
        d->organizationDomain = QString::fromLatin1("kde.org");
    }
}

K4AboutData::~K4AboutData()
{
    delete d;
}

K4AboutData::K4AboutData(const K4AboutData& other): d(new Private)
{
    *d = *other.d;
    QList<K4AboutLicense>::iterator it = d->_licenseList.begin(), itEnd = d->_licenseList.end();
    for ( ; it != itEnd; ++it) {
        K4AboutLicense& al = *it;
        al.d.detach();
        al.d->_aboutData = this;
    }
}

K4AboutData &K4AboutData::operator=(const K4AboutData& other)
{
    if (this != &other) {
        *d = *other.d;
        QList<K4AboutLicense>::iterator it = d->_licenseList.begin(), itEnd = d->_licenseList.end();
        for ( ; it != itEnd; ++it) {
            K4AboutLicense& al = *it;
            al.d.detach();
            al.d->_aboutData = this;
        }
    }
    return *this;
}

K4AboutData::operator KAboutData() const
{
    KAboutData aboutData(appName(), catalogName(), programName(), version(), shortDescription(),
                         KAboutData::License_Unknown, copyrightStatement(),
                         otherText(), d->_homepageAddress, bugAddress());
    // TODO K4AboutLicense -> KAboutLicense conversion, using licenses()
    return aboutData;
}

K4AboutData &K4AboutData::addAuthor( const KLocalizedString &name,
                                   const KLocalizedString &task,
                                   const QByteArray &emailAddress,
                                   const QByteArray &webAddress )
{
  d->_authorList.append(K4AboutPerson(name,task,emailAddress,webAddress));
  return *this;
}

K4AboutData &K4AboutData::addAuthor( const KLocalizedString &name,
                                   const KLocalizedString &task,
                                   const QByteArray &emailAddress,
                                   const QByteArray &webAddress,
                                   const QByteArray &ocsUsername )
{
  d->_authorList.append(K4AboutPerson(name,task,emailAddress,webAddress,ocsUsername));
  return *this;
}

K4AboutData &K4AboutData::addCredit( const KLocalizedString &name,
                                   const KLocalizedString &task,
                                   const QByteArray &emailAddress,
                                   const QByteArray &webAddress )
{
  d->_creditList.append(K4AboutPerson(name,task,emailAddress,webAddress));
  return *this;
}

K4AboutData &K4AboutData::addCredit( const KLocalizedString &name,
                                   const KLocalizedString &task,
                                   const QByteArray &emailAddress,
                                   const QByteArray &webAddress,
                                   const QByteArray &ocsUsername )
{
  d->_creditList.append(K4AboutPerson(name,task,emailAddress,webAddress,ocsUsername));
  return *this;
}

K4AboutData &K4AboutData::setTranslator( const KLocalizedString& name,
                                       const KLocalizedString& emailAddress )
{
  d->translatorName = name;
  d->translatorEmail = emailAddress;
  return *this;
}

K4AboutData &K4AboutData::setLicenseText( const KLocalizedString &licenseText )
{
    d->_licenseList[0] = K4AboutLicense(licenseText,this);
    return *this;
}

K4AboutData &K4AboutData::addLicenseText( const KLocalizedString &licenseText )
{
    // if the default license is unknown, overwrite instead of append
    K4AboutLicense &firstLicense = d->_licenseList[0];
    if (d->_licenseList.count() == 1 && firstLicense.d->_licenseKey == License_Unknown) {
        firstLicense = K4AboutLicense(licenseText,this);
    } else {
        d->_licenseList.append(K4AboutLicense(licenseText,this));
    }
    return *this;
}

K4AboutData &K4AboutData::setLicenseTextFile( const QString &pathToFile )
{
    d->_licenseList[0] = K4AboutLicense(pathToFile,this);
    return *this;
}

K4AboutData &K4AboutData::addLicenseTextFile( const QString &pathToFile )
{
    // if the default license is unknown, overwrite instead of append
    K4AboutLicense &firstLicense = d->_licenseList[0];
    if (d->_licenseList.count() == 1 && firstLicense.d->_licenseKey == License_Unknown) {
        firstLicense = K4AboutLicense(pathToFile,this);
    } else {
        d->_licenseList.append(K4AboutLicense(pathToFile,this));
    }
    return *this;
}

K4AboutData &K4AboutData::setAppName( const QByteArray &_appName )
{
  d->_appName = _appName;
  return *this;
}

K4AboutData &K4AboutData::setProgramName( const KLocalizedString &_programName )
{
  d->_programName = _programName;
  translateInternalProgramName();
  return *this;
}

K4AboutData &K4AboutData::setOcsProvider(const QByteArray &_ocsProviderUrl )
{
    d->_ocsProviderUrl = _ocsProviderUrl;
    return *this;
}

K4AboutData &K4AboutData::setVersion( const QByteArray &_version )
{
  d->_version = _version;
  return *this;
}

K4AboutData &K4AboutData::setShortDescription( const KLocalizedString &_shortDescription )
{
  d->_shortDescription = _shortDescription;
  return *this;
}

K4AboutData &K4AboutData::setCatalogName( const QByteArray &_catalogName )
{
  d->_catalogName = _catalogName;
  return *this;
}

K4AboutData &K4AboutData::setLicense( LicenseKey licenseKey)
{
    d->_licenseList[0] = K4AboutLicense(licenseKey,this);
    return *this;
}

K4AboutData &K4AboutData::addLicense( LicenseKey licenseKey)
{
    // if the default license is unknown, overwrite instead of append
    K4AboutLicense &firstLicense = d->_licenseList[0];
    if (d->_licenseList.count() == 1 && firstLicense.d->_licenseKey == License_Unknown) {
        firstLicense = K4AboutLicense(licenseKey,this);
    } else {
        d->_licenseList.append(K4AboutLicense(licenseKey,this));
    }
    return *this;
}

K4AboutData &K4AboutData::setCopyrightStatement( const KLocalizedString &_copyrightStatement )
{
  d->_copyrightStatement = _copyrightStatement;
  return *this;
}

K4AboutData &K4AboutData::setOtherText( const KLocalizedString &_otherText )
{
  d->_otherText = _otherText;
  return *this;
}

K4AboutData &K4AboutData::setHomepage( const QByteArray &_homepage )
{
  d->_homepageAddress = QString::fromLatin1(_homepage.data());
  return *this;
}

K4AboutData &K4AboutData::setBugAddress( const QByteArray &_bugAddress )
{
  d->_bugEmailAddress = _bugAddress;
  return *this;
}

K4AboutData &K4AboutData::setOrganizationDomain( const QByteArray &domain )
{
  d->organizationDomain = QString::fromLatin1(domain.data());
  return *this;
}

K4AboutData &K4AboutData::setProductName( const QByteArray &_productName )
{
  d->productName = QString::fromUtf8(_productName.data());
  return *this;
}

QString K4AboutData::appName() const
{
  return QString::fromUtf8(d->_appName.data());
}

QString K4AboutData::productName() const
{
   if (!d->productName.isEmpty())
      return d->productName;
   return appName();
}

QString K4AboutData::programName() const
{
   if (!d->_programName.isEmpty())
      return d->_programName.toString();
   return QString();
}

/// @internal
/// Return the program name. It is always pre-allocated.
/// Needed for KCrash in particular.
const char* K4AboutData::internalProgramName() const
{
   return d->_translatedProgramName.constData();
}

/// @internal
/// KCrash should call as few things as possible and should avoid e.g. malloc()
/// because it may deadlock. Since i18n() needs it, when KLocale is available
/// the i18n() call will be done here in advance.
void K4AboutData::translateInternalProgramName() const
{
  d->_translatedProgramName.clear();
#pragma message("KDE5 FIXME: This code must be replaced by something with KLocalizedString")
#if 0
  if( KLocale::global())
      d->_translatedProgramName = programName().toUtf8();
#endif
}

QString K4AboutData::programIconName() const
{
    return d->programIconName.isEmpty() ? appName() : d->programIconName;
}

K4AboutData &K4AboutData::setProgramIconName( const QString &iconName )
{
    d->programIconName = iconName;
    return *this;
}

QVariant K4AboutData::programLogo() const
{
    return d->programLogo;
}

K4AboutData &K4AboutData::setProgramLogo(const QVariant& image)
{
    d->programLogo = image ;
    return *this;
}

QString K4AboutData::ocsProviderUrl() const
{
    if( !d->_ocsProviderUrl.isEmpty() )
      return QString::fromUtf8( d->_ocsProviderUrl.data() );
    return QString();
}

QString K4AboutData::version() const
{
    return QString::fromUtf8(d->_version.data());
}

/// @internal
/// Return the untranslated and uninterpreted (to UTF8) string
/// for the version information. Used in particular for KCrash.
const char* K4AboutData::internalVersion() const
{
   return d->_version.constData();
}

QString K4AboutData::shortDescription() const
{
   if (!d->_shortDescription.isEmpty())
      return d->_shortDescription.toString();
   return QString();
}

QString K4AboutData::catalogName() const
{
   if (!d->_catalogName.isEmpty())
     return QString::fromUtf8(d->_catalogName.data());
   // Fallback to appname for catalog name if empty.
   return QString::fromUtf8(d->_appName.data());
}

QString K4AboutData::homepage() const
{
   return d->_homepageAddress;
}

QString K4AboutData::bugAddress() const
{
   return QString::fromUtf8(d->_bugEmailAddress.data());
}

QString K4AboutData::organizationDomain() const
{
    return d->organizationDomain;
}


/// @internal
/// Return the untranslated and uninterpreted (to UTF8) string
/// for the bug mail address. Used in particular for KCrash.
const char* K4AboutData::internalBugAddress() const
{
   if (d->_bugEmailAddress.isEmpty())
      return 0;
   return d->_bugEmailAddress.constData();
}

QList<K4AboutPerson> K4AboutData::authors() const
{
   return d->_authorList;
}

QList<K4AboutPerson> K4AboutData::credits() const
{
   return d->_creditList;
}

#define NAME_OF_TRANSLATORS "Your names"
#define EMAIL_OF_TRANSLATORS "Your emails"
QList<K4AboutPerson> K4AboutData::translators() const
{
    QList<K4AboutPerson> personList;
#pragma message("KDE5 TODO: What about this code ?")
#if 0
    KLocale *tmpLocale = NULL;
    if (KLocale::global()) {
        // There could be many catalogs loaded into the global locale,
        // e.g. in systemsettings. The tmp locale is needed to make sure we
        // use the translators name from this aboutdata's catalog, rather than
        // from any other loaded catalog.
        tmpLocale = new KLocale(*KLocale::global());
        tmpLocale->setActiveCatalog(catalogName());
    }
#endif
    QString translatorName;
    if (!d->translatorName.isEmpty()) {
        translatorName = d->translatorName.toString();
    }
    else {
        translatorName = ki18nc("NAME OF TRANSLATORS", NAME_OF_TRANSLATORS).toString(); //toString(tmpLocale);
    }

    QString translatorEmail;
    if (!d->translatorEmail.isEmpty()) {
        translatorEmail = d->translatorEmail.toString();
    }
    else {
        translatorEmail = ki18nc("EMAIL OF TRANSLATORS", EMAIL_OF_TRANSLATORS).toString(); //toString(tmpLocale);
    }
#if 0
    delete tmpLocale;
#endif
    if ( translatorName.isEmpty() || translatorName == QString::fromUtf8( NAME_OF_TRANSLATORS ) )
        return personList;

    const QStringList nameList(translatorName.split(QString(QLatin1Char(','))));

    QStringList emailList;
    if( !translatorEmail.isEmpty() && translatorEmail != QString::fromUtf8( EMAIL_OF_TRANSLATORS ) )
    {
       emailList = translatorEmail.split(QString(QLatin1Char(',')), QString::KeepEmptyParts);
    }

    QStringList::const_iterator nit;
    QStringList::const_iterator eit = emailList.constBegin();

    for( nit = nameList.constBegin(); nit != nameList.constEnd(); ++nit )
    {
        QString email;
        if ( eit != emailList.constEnd() )
        {
            email = *eit;
            ++eit;
        }

        personList.append( K4AboutPerson( (*nit).trimmed(), email.trimmed() ) );
    }

    return personList;
}

QString K4AboutData::aboutTranslationTeam()
{
    return i18nc("replace this with information about your translation team",
            "<p>KDE is translated into many languages thanks to the work "
            "of the translation teams all over the world.</p>"
            "<p>For more information on KDE internationalization "
            "visit <a href=\"http://l10n.kde.org\">http://l10n.kde.org</a></p>"
            );
}

QString K4AboutData::otherText() const
{
   if (!d->_otherText.isEmpty())
      return d->_otherText.toString();
   return QString();
}

QString K4AboutData::license() const
{
    return d->_licenseList.at(0).text();
}

QString K4AboutData::licenseName( NameFormat formatName ) const
{
    return d->_licenseList.at(0).name(formatName);
}

QList<K4AboutLicense> K4AboutData::licenses() const
{
    return d->_licenseList;
}

QString K4AboutData::copyrightStatement() const
{
  if (!d->_copyrightStatement.isEmpty())
    return d->_copyrightStatement.toString();
  return QString();
}

QString K4AboutData::customAuthorPlainText() const
{
  if (!d->customAuthorPlainText.isEmpty())
    return d->customAuthorPlainText.toString();
  return QString();
}

QString K4AboutData::customAuthorRichText() const
{
  if (!d->customAuthorRichText.isEmpty())
    return d->customAuthorRichText.toString();
  return QString();
}

bool K4AboutData::customAuthorTextEnabled() const
{
  return d->customAuthorTextEnabled;
}

K4AboutData &K4AboutData::setCustomAuthorText( const KLocalizedString &plainText,
                                             const KLocalizedString &richText )
{
  d->customAuthorPlainText = plainText;
  d->customAuthorRichText = richText;

  d->customAuthorTextEnabled = true;

  return *this;
}

K4AboutData &K4AboutData::unsetCustomAuthorText()
{
  d->customAuthorPlainText = KLocalizedString();
  d->customAuthorRichText = KLocalizedString();

  d->customAuthorTextEnabled = false;

  return *this;
}

