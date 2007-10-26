/*
 * This file is part of the KDE Libraries
 * Copyright (C) 2000 Espen Sand (espen@kde.org)
 * Copyright (C) 2006 Nicolas GOUTTE <goutte@kde.org>
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

#include "kaboutdata.h"
#include "kstandarddirs.h"
#include "klocalizedstring.h"

#include <QtCore/QFile>
#include <QtCore/QTextIStream>
#include <QVariant>
#include <QList>

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

class KAboutPerson::Private
{
public:
   KLocalizedString _name;
   KLocalizedString _task;
   QString _emailAddress;
   QString _webAddress;

   QString _nameNoop;
};

KAboutPerson::KAboutPerson( const KLocalizedString &_name,
                            const KLocalizedString &_task,
                            const QByteArray &_emailAddress,
                            const QByteArray &_webAddress )
  : d(new Private)
{
   d->_name = _name;
   d->_task = _task;
   d->_emailAddress = QString::fromUtf8(_emailAddress);
   d->_webAddress = QString::fromUtf8(_webAddress);
}

KAboutPerson::KAboutPerson( const QString &_name, const QString &_email )
  : d(new Private)
{
   d->_nameNoop = _name;
   d->_emailAddress = _email;
}

KAboutPerson::KAboutPerson(const KAboutPerson& other): d(new Private)
{
    *d = *other.d;
}

KAboutPerson::~KAboutPerson()
{
   delete d;
}

QString
KAboutPerson::name() const
{
   if (!d->_nameNoop.isEmpty())
      return d->_nameNoop;
   return d->_name.toString();
}

QString
KAboutPerson::task() const
{
   if (!d->_task.isEmpty())
      return d->_task.toString();
   return QString();
}

QString
KAboutPerson::emailAddress() const
{
   return d->_emailAddress;
}


QString
KAboutPerson::webAddress() const
{
   return d->_webAddress;
}


KAboutPerson&
KAboutPerson::operator=(const KAboutPerson& other)
{
   *d = *other.d;
   return *this;
}

class KAboutData::Private
{
public:
    Private()
        : customAuthorTextEnabled(false)
        {}
    QString _appName;
    KLocalizedString _programName;
    KLocalizedString _shortDescription;
    QString _catalogName;
    enum KAboutData::LicenseKey  _licenseKey;
    KLocalizedString _copyrightStatement;
    KLocalizedString _otherText;
    QString _homepageAddress;
    QList<KAboutPerson> _authorList;
    QList<KAboutPerson> _creditList;
    KLocalizedString _licenseText;
    QString _licenseTextFile;
    KLocalizedString translatorName;
    KLocalizedString translatorEmail;
    QString productName;
    QVariant programLogo;
    KLocalizedString customAuthorPlainText, customAuthorRichText;
    bool customAuthorTextEnabled;

    QString organizationDomain;

    // Everything dr.konqi needs, we store as utf-8, so we 
    // can just give it a pointer, w/o any allocations.
    QByteArray _translatedProgramName; // ### I don't see it ever being translated, and I did not change that
    QByteArray _version;
    QByteArray _bugEmailAddress;
};



KAboutData::KAboutData( const QByteArray &_appName,
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
    d->_appName = QString::fromUtf8(_appName);
    int p = d->_appName.indexOf('/');
    if (p >= 0) {
        d->_appName = d->_appName.mid(p + 1);
    }

    d->_catalogName = _catalogName;
    d->_programName = _programName;
    d->_translatedProgramName = _programName.toString(0).toUtf8();
    d->_version = _version;
    d->_shortDescription = _shortDescription;
    d->_licenseKey = licenseType;
    d->_copyrightStatement = _copyrightStatement;
    d->_otherText = text;
    d->_homepageAddress = homePageAddress;
    d->_bugEmailAddress = bugsEmailAddress;

    if (d->_homepageAddress.contains("http://")) {
        int dot = d->_homepageAddress.indexOf('.');
        if (dot >= 0) {
            d->organizationDomain = d->_homepageAddress.mid(dot + 1);
            int slash = d->organizationDomain.indexOf('/');
            if (slash >= 0)
                d->organizationDomain.truncate(slash);
        }
        else {
            d->organizationDomain = "kde.org";
        }
    }
    else {
        d->organizationDomain = "kde.org";
    }
}

KAboutData::~KAboutData()
{
    delete d;
}

KAboutData::KAboutData(const KAboutData& other): d(new Private)
{
    *d = *other.d;
}

KAboutData&
KAboutData::operator=(const KAboutData& other)
{
    *d = *other.d;
    return *this;
}

KAboutData &
KAboutData::addAuthor( const KLocalizedString &name,
                       const KLocalizedString &task,
                       const QByteArray &emailAddress,
                       const QByteArray &webAddress )
{
  d->_authorList.append(KAboutPerson(name,task,emailAddress,webAddress));
  return *this;
}

KAboutData &
KAboutData::addCredit( const KLocalizedString &name,
                       const KLocalizedString &task,
                       const QByteArray &emailAddress,
                       const QByteArray &webAddress )
{
  d->_creditList.append(KAboutPerson(name,task,emailAddress,webAddress));
  return *this;
}

KAboutData &
KAboutData::setTranslator( const KLocalizedString& name,
                           const KLocalizedString& emailAddress )
{
  d->translatorName = name;
  d->translatorEmail = emailAddress;
  return *this;
}

KAboutData &
KAboutData::setLicenseText( const KLocalizedString &licenseText )
{
  d->_licenseText = licenseText;
  d->_licenseKey = License_Custom;
  return *this;
}

KAboutData &
KAboutData::setLicenseTextFile( const QString &file )
{
  d->_licenseTextFile = file;
  d->_licenseKey = License_File;
  return *this;
}

KAboutData &
KAboutData::setAppName( const QByteArray &_appName )
{
  d->_appName = QString::fromUtf8(_appName);
  return *this;
}

KAboutData &
KAboutData::setProgramName( const KLocalizedString &_programName )
{
  d->_programName = _programName;
  translateInternalProgramName();
  return *this;
}

KAboutData &
KAboutData::setVersion( const QByteArray &_version )
{
  d->_version = _version;
  return *this;
}

KAboutData &
KAboutData::setShortDescription( const KLocalizedString &_shortDescription )
{
  d->_shortDescription = _shortDescription;
  return *this;
}

KAboutData &
KAboutData::setCatalogName( const QByteArray &_catalogName )
{
  d->_catalogName = _catalogName;
  return *this;
}

KAboutData &
KAboutData::setLicense( LicenseKey licenseKey)
{
  d->_licenseKey = licenseKey;
  return *this;
}

KAboutData &
KAboutData::setCopyrightStatement( const KLocalizedString &_copyrightStatement )
{
  d->_copyrightStatement = _copyrightStatement;
  return *this;
}

KAboutData &
KAboutData::setOtherText( const KLocalizedString &_otherText )
{
  d->_otherText = _otherText;
  return *this;
}

KAboutData &
KAboutData::setHomepage( const QByteArray &_homepage )
{
  d->_homepageAddress = QString::fromUtf8(_homepage);
  return *this;
}

KAboutData &
KAboutData::setBugAddress( const QByteArray &_bugAddress )
{
  d->_bugEmailAddress = _bugAddress;
  return *this;
}

KAboutData &
KAboutData::setOrganizationDomain( const QByteArray &domain )
{
  d->organizationDomain = QString::fromUtf8(domain);
  return *this;
}

KAboutData &
KAboutData::setProductName( const QByteArray &_productName )
{
  d->productName = QString::fromUtf8(_productName);
  return *this;
}

QString
KAboutData::appName() const
{
   return d->_appName;
}

QString
KAboutData::productName() const
{
   if (!d->productName.isEmpty())
      return d->productName;
   return appName();
}

QString
KAboutData::programName() const
{
   if (!d->_programName.isEmpty())
      return d->_programName.toString();
   return QString();
}

/// @internal
/// Return the program name. It is always pre-allocated.
/// Needed for KCrash in particular.
const char*
KAboutData::internalProgramName() const
{
   return d->_translatedProgramName.constData();
}

/// @internal
/// KCrash should call as few things as possible and should avoid e.g. malloc()
/// because it may deadlock. Since i18n() needs it, when KLocale is available
/// the i18n() call will be done here in advance.
void
KAboutData::translateInternalProgramName() const
{
  d->_translatedProgramName.clear();
  if( KGlobal::locale())
      d->_translatedProgramName = programName().toUtf8();
}

QVariant 
KAboutData::programLogo() const
{
    return d->programLogo;
}

KAboutData &
KAboutData::setProgramLogo(const QVariant& image)
{
    d->programLogo = image ;
    return *this;
}

QString
KAboutData::version() const
{
   return QString::fromUtf8(d->_version);
}

/// @internal
/// Return the untranslated and uninterpreted (to UTF8) string
/// for the version information. Used in particular for KCrash.
const char*
KAboutData::internalVersion() const
{
   return d->_version.constData();
}

QString
KAboutData::shortDescription() const
{
   if (!d->_shortDescription.isEmpty())
      return d->_shortDescription.toString();
   return QString();
}

QString
KAboutData::catalogName() const
{
   if (!d->_catalogName.isEmpty())
      return d->_catalogName;
   // Fallback to appname for catalog name if empty.
   return d->_appName;
}

QString
KAboutData::homepage() const
{
   return d->_homepageAddress;
}

QString
KAboutData::bugAddress() const
{
   return QString::fromUtf8(d->_bugEmailAddress);
}

QString
KAboutData::organizationDomain() const
{
    return d->organizationDomain;
}


/// @internal
/// Return the untranslated and uninterpreted (to UTF8) string
/// for the bug mail address. Used in particular for KCrash.
const char*
KAboutData::internalBugAddress() const
{
   if (d->_bugEmailAddress.isEmpty())
      return 0;
   return d->_bugEmailAddress.constData();
}

QList<KAboutPerson>
KAboutData::authors() const
{
   return d->_authorList;
}

QList<KAboutPerson>
KAboutData::credits() const
{
   return d->_creditList;
}

#define NAME_OF_TRANSLATORS "Your names"
#define EMAIL_OF_TRANSLATORS "Your emails"
QList<KAboutPerson>
KAboutData::translators() const
{
    QList<KAboutPerson> personList;

    QString translatorName;
    if (!d->translatorName.isEmpty()) {
        translatorName = d->translatorName.toString();
    }
    else {
        translatorName = i18nc("NAME OF TRANSLATORS", NAME_OF_TRANSLATORS);
    }

    QString translatorEmail;
    if (!d->translatorEmail.isEmpty()) {
        translatorEmail = d->translatorEmail.toString();
    }
    else {
        translatorEmail = i18nc("EMAIL OF TRANSLATORS", EMAIL_OF_TRANSLATORS);
    }

    if ( translatorName.isEmpty() || translatorName == QString::fromUtf8( NAME_OF_TRANSLATORS ) )
        return personList;

    const QStringList nameList ( translatorName.split( ',' ) );

    QStringList emailList;
    if( !translatorEmail.isEmpty() && translatorEmail != QString::fromUtf8( EMAIL_OF_TRANSLATORS ) )
    {
       emailList = translatorName.split( ',', QString::KeepEmptyParts );
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

        personList.append( KAboutPerson( (*nit).trimmed(), email.trimmed() ) );
    }

    return personList;
}

QString
KAboutData::aboutTranslationTeam()
{
    return i18nc("replace this with information about your translation team",
            "<p>KDE is translated into many languages thanks to the work "
            "of the translation teams all over the world.</p>"
            "<p>For more information on KDE internationalization "
            "visit <a href=\"http://l10n.kde.org\">http://l10n.kde.org</a></p>"
            );
}

QString
KAboutData::otherText() const
{
   if (!d->_otherText.isEmpty())
      return d->_otherText.toString();
   return QString();
}

QString
KAboutData::license() const
{
  QString result;
  if (!copyrightStatement().isEmpty())
    result = copyrightStatement() + "\n\n";

  QString l;
  QString f;
  switch ( d->_licenseKey )
  {
    case License_File:
       f = d->_licenseTextFile;
       break;
    case License_GPL_V2:
       l = "GPL v2";
       f = KStandardDirs::locate("data", "LICENSES/GPL_V2");
       break;
    case License_LGPL_V2:
       l = "LGPL v2";
       f = KStandardDirs::locate("data", "LICENSES/LGPL_V2");
       break;
    case License_BSD:
       l = "BSD License";
       f = KStandardDirs::locate("data", "LICENSES/BSD");
       break;
    case License_Artistic:
       l = "Artistic License";
       f = KStandardDirs::locate("data", "LICENSES/ARTISTIC");
       break;
    case License_QPL_V1_0:
       l = "QPL v1.0";
       f = KStandardDirs::locate("data", "LICENSES/QPL_V1.0");
       break;
    case License_Custom:
       if (!d->_licenseText.isEmpty())
          return d->_licenseText.toString();
       // fall through
    default:
       result += i18n("No licensing terms for this program have been specified.\n"
                   "Please check the documentation or the source for any\n"
                   "licensing terms.\n");
       return result;
      }

  if (!l.isEmpty())
     result += i18n("This program is distributed under the terms of the %1.",  l );

  if (!f.isEmpty())
  {
     QFile file(f);
     if (file.open(QIODevice::ReadOnly))
     {
        result += '\n';
        result += '\n';
        QTextStream str(&file);
        result += str.readAll();
     }
  }

  return result;
}

QString KAboutData::licenseName(NameFormat formatName) const
{
    QString licenseShort;
    QString licenseFull;

    switch (d->_licenseKey) {
    case License_GPL_V2:
        licenseShort = "GPL v2";
        licenseFull = i18nc("@item license","GNU General Public License Version 2");
        break;
    case License_LGPL_V2:
        licenseShort = "LGPL v2";
        licenseFull = i18nc("@item license","GNU Library General Public License Version 2");
        break;
    case License_BSD:
        licenseShort = "BSD License";
        licenseFull = i18nc("@item license","BSD License");
        break;
    case License_Artistic:
        licenseShort = "Artistic License";
        licenseFull = i18nc("@item license","Artistic License");
        break;
    case License_QPL_V1_0:
        licenseShort = "QPL v1.0";
        licenseFull = i18nc("@item license","Q Public License");
        break;
    case License_Custom:
    case License_File:
        licenseShort = licenseFull = i18nc("@item license","Custom");
        break;
    default:
        licenseShort = licenseFull = i18nc("@item license","Not specified");
    }

    switch (formatName) {
    case ShortName:
        return licenseShort;
        break;
    case FullName:
        return licenseFull;
        break;
    default:
        return QString();
    }

    return QString();
}

QString
KAboutData::copyrightStatement() const
{
  if (!d->_copyrightStatement.isEmpty())
    return d->_copyrightStatement.toString();
  return QString();
}

QString
KAboutData::customAuthorPlainText() const
{
  if (!d->customAuthorPlainText.isEmpty())
    return d->customAuthorPlainText.toString();
  return QString();
}

QString
KAboutData::customAuthorRichText() const
{
  if (!d->customAuthorRichText.isEmpty())
    return d->customAuthorRichText.toString();
  return QString();
}

bool
KAboutData::customAuthorTextEnabled() const
{
  return d->customAuthorTextEnabled;
}

KAboutData &
KAboutData::setCustomAuthorText(const KLocalizedString &plainText,
                                const KLocalizedString &richText)
{
  d->customAuthorPlainText = plainText;
  d->customAuthorRichText = richText;

  d->customAuthorTextEnabled = true;

  return *this;
}

KAboutData &
KAboutData::unsetCustomAuthorText()
{
  d->customAuthorPlainText = KLocalizedString();
  d->customAuthorRichText = KLocalizedString();

  d->customAuthorTextEnabled = false;

  return *this;
}

