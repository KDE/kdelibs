/*
 * This file is part of the KDE Libraries
 * Copyright (C) 2000 Espen Sand (espen@kde.org)
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

#undef QT3_SUPPORT
#include "kaboutdata.h"
#include "kstandarddirs.h"
#include "klocale.h"
#include <qfile.h>
#include <qtextstream.h>
#include <qimage.h>
#include <QList>

class KAboutPerson::Private
{
public:
   const char *_name;
#define mName d->_name
   const char *_task;
#define mTask d->_task
   const char *_emailAddress;
#define mEmailAddress d->_emailAddress
   const char *_webAddress;
#define mWebAddress d->_webAddress
};

KAboutPerson::KAboutPerson( const char* _name, const char* _task,
                  const char* _emailAddress, const char* _webAddress )
  : d(new Private)
{
   mName = _name;
   mTask = _task;
   mEmailAddress = _emailAddress;
   mWebAddress = _webAddress;
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
   return QString::fromUtf8(mName);
}

QString
KAboutPerson::task() const
{
   if (mTask && *mTask)
      return i18n(mTask);
   return QString();
}

QString
KAboutPerson::emailAddress() const
{
   return QString::fromUtf8(mEmailAddress);
}


QString
KAboutPerson::webAddress() const
{
   return QString::fromUtf8(mWebAddress);
}

#undef mTask
#undef mEmailAddress
#undef mWebAddress

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
#define mEmail d->_email
};

KAboutTranslator::KAboutTranslator(const QString & _name,
                const QString & _emailAddress)
  : d(new Private)
{
    mName = _name;
    mEmail = _emailAddress;
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
    return mName;
}

QString KAboutTranslator::emailAddress() const
{
    return mEmail;
}

#undef mName
#undef mEmail

class KAboutData::Private
{
public:
    Private()
        : translatorName("_: NAME OF TRANSLATORS\nYour names")
        , translatorEmail("_: EMAIL OF TRANSLATORS\nYour emails")
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
#define mAppName d->_appName
    const char *_programName;
#define mProgramName d->_programName
    const char *_version;
#define mVersion d->_version
    const char *_shortDescription;
#define mShortDescription d->_shortDescription
    int _licenseKey;
#define mLicenseKey d->_licenseKey
    const char *_copyrightStatement;
#define mCopyrightStatement d->_copyrightStatement
    const char *_otherText;
#define mOtherText d->_otherText
    const char *_homepageAddress;
#define mHomepageAddress d->_homepageAddress
    const char *_bugEmailAddress;
#define mBugEmailAddress d->_bugEmailAddress
    QList<KAboutPerson> _authorList;
#define mAuthorList d->_authorList
    QList<KAboutPerson> _creditList;
#define mCreditList d->_creditList
    const char *_licenseText;
#define mLicenseText d->_licenseText
    const char *translatorName;
    const char *translatorEmail;
    const char *productName;
    QImage* programLogo;
    QString customAuthorPlainText, customAuthorRichText;
    bool customAuthorTextEnabled;
    const char *mTranslatedProgramName;
};



KAboutData::KAboutData( const char* _appName,
                        const char* _programName,
                        const char* _version,
                        const char* _shortDescription,
			int licenseType,
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
	 mAppName = p+1;
     else
	 mAppName = _appName;
   } else
     mAppName = 0;
   mProgramName = _programName;
   mVersion = _version;
   mShortDescription = _shortDescription;
   mLicenseKey = licenseType;
   mCopyrightStatement = _copyrightStatement;
   mOtherText = text;
   mHomepageAddress = homePageAddress;
   mBugEmailAddress = bugsEmailAddress;
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
  mAuthorList.append(KAboutPerson(name,task,emailAddress,webAddress));
}

void
KAboutData::addCredit( const char *name, const char *task,
		    const char *emailAddress, const char *webAddress )
{
  mCreditList.append(KAboutPerson(name,task,emailAddress,webAddress));
}

void
KAboutData::setTranslator( const char *name, const char *emailAddress)
{
  d->translatorName=name;
  d->translatorEmail=emailAddress;
}

void
KAboutData::setLicenseText( const char *licenseText )
{
  mLicenseText = licenseText;
  mLicenseKey = License_Custom;
}

void
KAboutData::setLicenseTextFile( const QString &file )
{
  mLicenseText = qstrdup(QFile::encodeName(file));
  mLicenseKey = License_File;
}

void
KAboutData::setAppName( const char* _appName )
{
  mAppName = _appName;
}

void
KAboutData::setProgramName( const char* _programName )
{
  mProgramName = _programName;
  translateInternalProgramName();
}

void
KAboutData::setVersion( const char* _version )
{
  mVersion = _version;
}

void
KAboutData::setShortDescription( const char* _shortDescription )
{
  mShortDescription = _shortDescription;
}

void
KAboutData::setLicense( LicenseKey licenseKey)
{
  mLicenseKey = licenseKey;
}

void
KAboutData::setCopyrightStatement( const char* _copyrightStatement )
{
  mCopyrightStatement = _copyrightStatement;
}

void
KAboutData::setOtherText( const char* _otherText )
{
  mOtherText = _otherText;
}

void
KAboutData::setHomepage( const char* _homepage )
{
  mHomepageAddress = _homepage;
}

void
KAboutData::setBugAddress( const char* _bugAddress )
{
  mBugEmailAddress = _bugAddress;
}

void
KAboutData::setProductName( const char* _productName )
{
  d->productName = _productName;
}

const char *
KAboutData::appName() const
{
   return mAppName;
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
   if (mProgramName && *mProgramName)
      return i18n(mProgramName);
   return QString();
}

const char*
KAboutData::internalProgramName() const
{
   if (d->mTranslatedProgramName)
      return d->mTranslatedProgramName;
   else
      return mProgramName;
}

// KCrash should call as few things as possible and should avoid e.g. malloc()
// because it may deadlock. Since i18n() needs it, when KLocale is available
// the i18n() call will be done here in advance.
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
   return QLatin1String(mVersion);
}

const char*
KAboutData::internalVersion() const
{
   return mVersion;
}

QString
KAboutData::shortDescription() const
{
   if (mShortDescription && *mShortDescription)
      return i18n(mShortDescription);
   return QString();
}

QString
KAboutData::homepage() const
{
   return QLatin1String(mHomepageAddress);
}

QString
KAboutData::bugAddress() const
{
   return QLatin1String(mBugEmailAddress);
}

const char*
KAboutData::internalBugAddress() const
{
   return mBugEmailAddress;
}

const QList<KAboutPerson>
KAboutData::authors() const
{
   return mAuthorList;
}

const QList<KAboutPerson>
KAboutData::credits() const
{
   return mCreditList;
}

const QList<KAboutTranslator>
KAboutData::translators() const
{
    QList<KAboutTranslator> personList;

    if(d->translatorName == 0)
        return personList;

    QStringList nameList;
    QStringList emailList;

    QString names = i18n(d->translatorName);
    if(names != QString::fromUtf8(d->translatorName))
    {
        nameList = names.split( ',');
    }


    if(d->translatorEmail)
    {
        QString emails = i18n(d->translatorEmail);

        if(emails != QString::fromUtf8(d->translatorEmail))
        {
            emailList = emails.split( ',', QString::KeepEmptyParts);
        }
    }


    QStringList::Iterator nit;
    QStringList::Iterator eit=emailList.begin();

    for(nit = nameList.begin(); nit != nameList.end(); ++nit)
    {
        QString email;
        if(eit != emailList.end())
        {
            email=*eit;
            ++eit;
        }

        QString name=*nit;

        personList.append(KAboutTranslator(name.trimmed(), email.trimmed()));
    }

    return personList;
}

QString
KAboutData::aboutTranslationTeam()
{
    return i18n("replace this with information about your translation team",
            "<p>KDE is translated into many languages thanks to the work "
            "of the translation teams all over the world.</p>"
            "<p>For more information on KDE internationalization "
            "visit <a href=\"http://l10n.kde.org\">http://l10n.kde.org</a></p>"
            );
}

QString
KAboutData::otherText() const
{
   if (mOtherText && *mOtherText)
      return i18n(mOtherText);
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
  switch ( mLicenseKey )
  {
    case License_File:
       f = QFile::decodeName(mLicenseText);
       break;
    case License_GPL_V2:
       l = "GPL v2";
       f = locate("data", "LICENSES/GPL_V2");
       break;
    case License_LGPL_V2:
       l = "LGPL v2";
       f = locate("data", "LICENSES/LGPL_V2");
       break;
    case License_BSD:
       l = "BSD License";
       f = locate("data", "LICENSES/BSD");
       break;
    case License_Artistic:
       l = "Artistic License";
       f = locate("data", "LICENSES/ARTISTIC");
       break;
    case License_QPL_V1_0:
       l = "QPL v1.0";
       f = locate("data", "LICENSES/QPL_V1.0");
       break;
    case License_Custom:
       if (mLicenseText && *mLicenseText)
          return( i18n(mLicenseText) );
       // fall through
    default:
       result += i18n("No licensing terms for this program have been specified.\n"
                   "Please check the documentation or the source for any\n"
                   "licensing terms.\n");
       return result;
      }

  if (!l.isEmpty())
     result += i18n("This program is distributed under the terms of the %1.").arg( l );

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
  if (mCopyrightStatement && *mCopyrightStatement)
     return i18n(mCopyrightStatement);
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

#undef mAppName
#undef mProgramName
#undef mVersion
#undef mShortDescription
#undef mLicenseKey
#undef mCopyrightStatement
#undef mOtherText
#undef mHomepageAddress
#undef mBugEmailAddress
#undef mAuthorList
#undef mCreditList
#undef mLicenseText
