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
 * the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 *
 */


#include <kaboutdata.h>
#include <klocale.h>
#include <kstddirs.h>
#include <qfile.h>
#include <qtextstream.h>

QString
KAboutPerson::name() const
{
   return QString::fromLatin1(mName);
}

QString
KAboutPerson::task() const
{
   if (mTask)
      return i18n(mTask);
   else
      return QString::null;
}

QString
KAboutPerson::emailAddress() const
{
   return QString::fromLatin1(mEmailAddress);
}


QString
KAboutPerson::webAddress() const
{
   return QString::fromLatin1(mWebAddress);
}

KAboutData::KAboutData( const char *appName,
                        const char *programName,
			const char *version,
                        const char *shortDescription,
			int licenseType,
			const char *copyrightStatement,
			const char *text,
			const char *homePageAddress,
			const char *bugsEmailAddress
			) :
  mProgramName( programName ),
  mVersion( version ),
  mShortDescription( shortDescription ),
  mLicenseKey( licenseType ),
  mCopyrightStatement( copyrightStatement ),
  mOtherText( text ),
  mHomepageAddress( homePageAddress ),
  mBugEmailAddress( bugsEmailAddress )
{
   if( appName ) {
     const char *p = strrchr(appName, '/');
     if( p )
	 mAppName = p+1;
     else
	 mAppName = appName;
   } else
     mAppName = 0;
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


const char *
KAboutData::appName() const
{
   return mAppName;
}

QString
KAboutData::programName() const
{
   if (mProgramName && *mProgramName)
      return i18n(mProgramName);
   else
      return QString::null;
}

QString
KAboutData::version() const
{
   return QString::fromLatin1(mVersion);
}

QString
KAboutData::shortDescription() const
{
   if (mShortDescription)
      return i18n(mShortDescription);
   else
      return QString::null;
}

QString
KAboutData::homepage() const
{
   return QString::fromLatin1(mHomepageAddress);
}

QString
KAboutData::bugAddress() const
{
   return QString::fromLatin1(mBugEmailAddress);
}

const QValueList<KAboutPerson>
KAboutData::authors() const
{
   return mAuthorList;
}

const QValueList<KAboutPerson>
KAboutData::credits() const
{
   return mCreditList;
}

QString
KAboutData::otherText() const
{
   if (mOtherText)
      return i18n(mOtherText);
   else
      return QString::null;
}


QString
KAboutData::license() const
{
  QString l;
  QString f;
  switch ( mLicenseKey ) 
  {
    case License_File:
       f = QFile::decodeName(mLicenseText);
       break;
    case License_Custom:
       return( i18n(mLicenseText) );
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
    default: 
       return i18n("No licensing terms for this program have been specified.\n"
                   "Please check the documentation or the source for any\n"
                   "licensing terms.\n");
      }
 
  QString result;
  if (!l.isEmpty())
     result = i18n("This program is distributed under the terms of the %1.").arg( l );

  if (!f.isEmpty())
  {
     QFile file(f);
     if (file.open(IO_ReadOnly))
     {
        result += '\n';
        result += '\n';
        QTextStream str(&file);
        result += str.read();
     }
  }

  return result;
}

