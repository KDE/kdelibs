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


#include <iostream.h>
#include <kaboutdata.h>
#include <klocale.h>

QString
KAboutPerson::name() const
{
   return QString::fromLatin1(mName);
}

QString
KAboutPerson::task() const
{
   return i18n(mTask);
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
     char *p = strrchr(appName, '/');
     if( p )
       mAppName = p+1;
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
}

const char * 
KAboutData::appName() const 
{ 
   return mAppName; 
}

QString 
KAboutData::programName() const 
{ 
   return i18n(mProgramName); 
}

QString 
KAboutData::version() const 
{ 
   return QString::fromLatin1(mVersion); 
}

QString 
KAboutData::shortDescription() const 
{ 
   return i18n(mShortDescription); 
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
   return i18n(mOtherText); 
}


QString 
KAboutData::license() const
{
  if( mLicenseKey != 0 )
  {
    // TODO put standard license texts in some files under $KDEDIR
    return( QString("Key based license text not yet implemented." ) );
  }
  else
  {
    return( QString::fromLatin1(mLicenseText) );
  }
}

