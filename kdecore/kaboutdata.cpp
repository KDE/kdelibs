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

KAboutData::KAboutData( const QString &programName,
			const QString &version,
			int licenseType,
			const QString &copyrightStatement,
			const QString &text,
			const QString &homePageAddress,
			const QString &bugsEmailAddress
			) :
  mProgramName( programName ),
  mVersion( version ),
  mLicenseKey( licenseType ),
  mCopyrightStatement( copyrightStatement ),
  mOtherText( text ),
  mHomepageAddress( homePageAddress ),
  mBugEmailAddress( bugsEmailAddress )
{
}

void KAboutData::addAuthor( const QString &name, const QString &task,
		    const QString &emailAddress, const QString &webAddress )
{
  mAuthorList.append(KAboutPerson(name,task,emailAddress,webAddress));
}

void KAboutData::addCredit( const QString &name, const QString &task,
		    const QString &emailAddress, const QString &webAddress )
{
  mCreditList.append(KAboutPerson(name,task,emailAddress,webAddress));
}

void KAboutData::setLicenseText( const QString &licenseText )
{
  mLicenseText = licenseText;
}

QString KAboutData::license() const
{
  if( mLicenseKey != 0 )
  {
    // TODO put standard license texts in some files under $KDEDIR
    return( QString("Key based license text not yet implemented." ) );
  }
  else
  {
    return( mLicenseText );
  }
}

