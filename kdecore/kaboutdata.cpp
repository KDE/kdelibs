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

#include <qfile.h>
#include <qtextstream.h>
#include <qimage.h>
#include <QList>

class KAboutPerson::Private
{
public:
   const char *_name;
   const char *_task;
   const char *_emailAddress;
   const char *_webAddress;
};

KAboutPerson::KAboutPerson( const char* _name, const char* _task,
                  const char* _emailAddress, const char* _webAddress )
  : d(new Private)
{
   d->_name = _name;
   d->_task = _task;
   d->_emailAddress = _emailAddress;
   d->_webAddress = _webAddress;
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
   return QString::fromUtf8(d->_name);
}

QString
KAboutPerson::task() const
{
   if (d->_task && *d->_task)
      return i18n(d->_task);
   return QString();
}

QString
KAboutPerson::emailAddress() const
{
   return QString::fromUtf8(d->_emailAddress);
}


QString
KAboutPerson::webAddress() const
{
   return QString::fromUtf8(d->_webAddress);
}


KAboutPerson&
KAboutPerson::operator=(const KAboutPerson& other)
{
   *d = *other.d;
   return *this;
}

class KAboutTranslator::Private
{
public:
   QString _name;
   QString _email;

};

KAboutTranslator::KAboutTranslator(const QString & _name,
                const QString & _emailAddress)
  : d(new Private)
{
    d->_name = _name;
    d->_email = _emailAddress;
}

KAboutTranslator::~KAboutTranslator()
{
    delete d;
}

KAboutTranslator&
KAboutTranslator::operator=(const KAboutTranslator& other)
{
    *d = *other.d;
    return *this;
}

KAboutTranslator::KAboutTranslator(const KAboutTranslator& other): d(new Private)
{
    *d = *other.d;
}

QString KAboutTranslator::name() const
{
    return d->_name;
}

QString KAboutTranslator::emailAddress() const
{
    return d->_email;
}

class KAboutData::Private
{
public:
    Private()
        : mTranslatorName( i18nc("NAME OF TRANSLATORS", "Your names") )
        , mTranslatorEmail( i18nc("EMAIL OF TRANSLATORS", "Your emails") )
        , productName(0)
        , programLogo(0)
        , customAuthorTextEnabled(false)
        , mTranslatedProgramName( 0 )
        {}
    ~Private()
        {
             if (_licenseKey == License_File)
                 delete [] _licenseText;
             delete programLogo;
             delete[] mTranslatedProgramName;
        }
    const char *_appName;
    const char *_programName;
    const char *_version;
    const char *_shortDescription;
    enum KAboutData::LicenseKey  _licenseKey;
    const char *_copyrightStatement;
    const char *_otherText;
    const char *_homepageAddress;
    const char *_bugEmailAddress;
    QList<KAboutPerson> _authorList;
    QList<KAboutPerson> _creditList;
    const char *_licenseText;
    QString mTranslatorName;
    QString mTranslatorEmail;
    const char *productName;
    QImage* programLogo;
    QString customAuthorPlainText, customAuthorRichText;
    bool customAuthorTextEnabled;
    const char *mTranslatedProgramName;
    QString organizationDomain;
};



KAboutData::KAboutData( const char* _appName,
                        const char* _programName,
                        const char* _version,
                        const char* _shortDescription,
			enum LicenseKey licenseType,
			const char* _copyrightStatement,
			const char* text,
			const char* homePageAddress,
			const char* bugsEmailAddress
			)
  : d(new Private)
{

   if( _appName ) {
     const char *p = strrchr(_appName, '/');
     if( p )
	 d->_appName = p+1;
     else
	 d->_appName = _appName;
   } else
     d->_appName = 0;
   d->_programName = _programName;
   d->_version = _version;
   d->_shortDescription = _shortDescription;
   d->_licenseKey = licenseType;
   d->_copyrightStatement = _copyrightStatement;
   d->_otherText = text;
   d->_homepageAddress = homePageAddress;
   d->_bugEmailAddress = bugsEmailAddress;
   if ( homePageAddress && strncmp( homePageAddress, "http://", 7 ) == 0 ) {
       const QByteArray addr = homePageAddress;
       const int dot = addr.indexOf( '.' );
       if ( dot > -1 ) {
           d->organizationDomain = homePageAddress + dot + 1;
           const int slash = d->organizationDomain.indexOf( '/' );
           if ( slash > -1 )
               d->organizationDomain.truncate( slash );
       }
       else
           d->organizationDomain = "kde.org";
   } else
       d->organizationDomain = "kde.org";
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

void
KAboutData::addAuthor( const char *name, const char *task,
		    const char *emailAddress, const char *webAddress )
{
  d->_authorList.append(KAboutPerson(name,task,emailAddress,webAddress));
}

void
KAboutData::addCredit( const char *name, const char *task,
		    const char *emailAddress, const char *webAddress )
{
  d->_creditList.append(KAboutPerson(name,task,emailAddress,webAddress));
}

// deprecated
void
KAboutData::setTranslator( const char *name, const char *emailAddress )
{
  d->mTranslatorName = i18nc( "NAME OF TRANSLATORS", name );
  d->mTranslatorEmail = i18nc( "EMAILS OF TRANSLATORS", emailAddress );
}

void
KAboutData::setTranslator( const KLocalizedString& name, const KLocalizedString& emailAddress )
{
  d->mTranslatorName = name.toString();
  d->mTranslatorEmail = emailAddress.toString();
}

void
KAboutData::setLicenseText( const char *licenseText )
{
  d->_licenseText = licenseText;
  d->_licenseKey = License_Custom;
}

void
KAboutData::setLicenseTextFile( const QString &file )
{
  d->_licenseText = qstrdup(QFile::encodeName(file));
  d->_licenseKey = License_File;
}

void
KAboutData::setAppName( const char* _appName )
{
  d->_appName = _appName;
}

void
KAboutData::setProgramName( const char* _programName )
{
  d->_programName = _programName;
  translateInternalProgramName();
}

void
KAboutData::setVersion( const char* _version )
{
  d->_version = _version;
}

void
KAboutData::setShortDescription( const char* _shortDescription )
{
  d->_shortDescription = _shortDescription;
}

void
KAboutData::setLicense( LicenseKey licenseKey)
{
  d->_licenseKey = licenseKey;
}

void
KAboutData::setCopyrightStatement( const char* _copyrightStatement )
{
  d->_copyrightStatement = _copyrightStatement;
}

void
KAboutData::setOtherText( const char* _otherText )
{
  d->_otherText = _otherText;
}

void
KAboutData::setHomepage( const char* _homepage )
{
  d->_homepageAddress = _homepage;
}

void
KAboutData::setBugAddress( const char* _bugAddress )
{
  d->_bugEmailAddress = _bugAddress;
}

void
KAboutData::setOrganizationDomain( const char *domain )
{
  d->organizationDomain = domain;
}

void
KAboutData::setProductName( const char* _productName )
{
  d->productName = _productName;
}

const char *
KAboutData::appName() const
{
   return d->_appName;
}

const char *
KAboutData::productName() const
{
   if (d->productName)
      return d->productName;
   return appName();
}

QString
KAboutData::programName() const
{
   if (d->_programName && *d->_programName)
      return i18n(d->_programName);
   return QString();
}

/// @internal
/// Gracefully handle cases where the program name is not
/// translated (yet) and return the untranslated name then.
/// Needed for KCrash in particular.
const char*
KAboutData::internalProgramName() const
{
   if (d->mTranslatedProgramName)
      return d->mTranslatedProgramName;
   else
      return d->_programName;
}

/// @internal
/// KCrash should call as few things as possible and should avoid e.g. malloc()
/// because it may deadlock. Since i18n() needs it, when KLocale is available
/// the i18n() call will be done here in advance.
void
KAboutData::translateInternalProgramName() const
{
  delete[] d->mTranslatedProgramName;
  d->mTranslatedProgramName = 0;
  if( KGlobal::locale())
      d->mTranslatedProgramName = qstrdup( programName().toUtf8() );
}

QImage
KAboutData::programLogo() const
{
    return d->programLogo ? (*d->programLogo) : QImage();
}

void
KAboutData::setProgramLogo(const QImage& image)
{
    if (!d->programLogo)
       d->programLogo = new QImage( image );
    else
       *d->programLogo = image;
}

QString
KAboutData::version() const
{
   return QLatin1String(d->_version);
}

/// @internal
/// Return the untranslated and uninterpreted (to UTF8) string
/// for the version information. Used in particular for KCrash.
const char*
KAboutData::internalVersion() const
{
   return d->_version;
}

QString
KAboutData::shortDescription() const
{
   if (d->_shortDescription && *d->_shortDescription)
      return i18n(d->_shortDescription);
   return QString();
}

QString
KAboutData::homepage() const
{
   return QLatin1String(d->_homepageAddress);
}

QString
KAboutData::bugAddress() const
{
   return QLatin1String(d->_bugEmailAddress);
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
   return d->_bugEmailAddress;
}

const QList<KAboutPerson>
KAboutData::authors() const
{
   return d->_authorList;
}

const QList<KAboutPerson>
KAboutData::credits() const
{
   return d->_creditList;
}

const QList<KAboutTranslator>
KAboutData::translators() const
{
    QList<KAboutTranslator> personList;

    if ( d->mTranslatorName.isEmpty() )
        return personList;

    const QStringList nameList ( d->mTranslatorName.split( ',' ) );

    QStringList emailList;
    if( !d->mTranslatorEmail.isEmpty() )
    {
       emailList = d->mTranslatorName.split( ',', QString::KeepEmptyParts );
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

        personList.append( KAboutTranslator( (*nit).trimmed(), email.trimmed() ) );
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
   if (d->_otherText && *d->_otherText)
      return i18n(d->_otherText);
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
       f = QFile::decodeName(d->_licenseText);
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
       if (d->_licenseText && *d->_licenseText)
          return( i18n(d->_licenseText) );
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

QString
KAboutData::copyrightStatement() const
{
  if (d->_copyrightStatement && *d->_copyrightStatement)
     return i18n(d->_copyrightStatement);
  return QString();
}

QString
KAboutData::customAuthorPlainText() const
{
  return d->customAuthorPlainText;
}

QString
KAboutData::customAuthorRichText() const
{
  return d->customAuthorRichText;
}

bool
KAboutData::customAuthorTextEnabled() const
{
  return d->customAuthorTextEnabled;
}

void
KAboutData::setCustomAuthorText(const QString &plainText, const QString &richText)
{
  d->customAuthorPlainText = plainText;
  d->customAuthorRichText = richText;

  d->customAuthorTextEnabled = true;
}

void
KAboutData::unsetCustomAuthorText()
{
  d->customAuthorPlainText.clear();
  d->customAuthorRichText.clear();

  d->customAuthorTextEnabled = false;
}

