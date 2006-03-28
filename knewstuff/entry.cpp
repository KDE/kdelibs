/*
    This file is part of KNewStuff.
    Copyright (c) 2002 Cornelius Schumacher <schumacher@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "entry.h"

#include <qhash.h>
#include <qcoreapplication.h>

#include <kglobal.h>
#include <klocale.h>

using namespace KNS;

// BCI for KDE 3.5 only

class EntryPrivate
{
  public:
  EntryPrivate(){}
  QString mEmail;
};

static QHash<void*, EntryPrivate*> *d_ptr = 0;

static void cleanup_d_ptr()
{
  delete d_ptr;
  d_ptr = 0; // not in BIC guide - add there
}

static EntryPrivate *d(const Entry *e)
{
  if(!d_ptr)
  {
    d_ptr = new QHash<void*, EntryPrivate*>();
    qAddPostRoutine(cleanup_d_ptr);
  }
  EntryPrivate *ret = d_ptr->value((void*)e);
  if(!ret)
  {
    ret = new EntryPrivate();
    d_ptr->insert((void*)e, ret);
  }
  return ret;
}

QString Entry::authorEmail() const
{
  return d(this)->mEmail;
}

void Entry::setAuthorEmail( const QString& email )
{
  d(this)->mEmail = email;
}

// BCI part ends here

Entry::Entry() :
  mRelease( 0 ), mReleaseDate( QDate::currentDate() ), mRating( 0 ),
  mDownloads( 0 ), mCompatibility( false )
{
}

Entry::Entry( const QDomElement &e ) :
  mRelease( 0 ), mReleaseDate( QDate::currentDate() ), mRating( 0 ),
  mDownloads( 0 ), mCompatibility( false )
{
  parseDomElement( e );
}

Entry::~Entry()
{
}


void Entry::setName( const QString &name, const QString &lang )
{
  mNameMap.insert( lang, name );

  if ( !mLangs.contains( lang ) )
    mLangs.append( lang );
}

QString Entry::name( const QString &lang ) const
{
  if ( mNameMap.isEmpty() ) return QString();

  if ( !mNameMap[ lang ].isEmpty() ) return mNameMap[ lang ];
  else {
    QStringList langs = KGlobal::locale()->languageList();
    for(QStringList::Iterator it = langs.begin(); it != langs.end(); ++it)
      if( !mNameMap[ *it ].isEmpty() ) return mNameMap[ *it ];
  }
  if ( !mNameMap[ QString() ].isEmpty() ) return mNameMap[ QString() ];
  else return *(mNameMap.begin());
}


void Entry::setCategory( const QString &category )
{
  mCategory = category;
}

QString Entry::category() const
{
  return mCategory;
}


void Entry::setAuthor( const QString &author )
{
  mAuthor = author;
}

QString Entry::author() const
{
  return mAuthor;
}


void Entry::setLicense( const QString &license )
{
  mLicense = license;
}

QString Entry::license() const
{
  return mLicense;
}


void Entry::setSummary( const QString &text, const QString &lang )
{
  mSummaryMap.insert( lang, text );

  if ( !mLangs.contains( lang ) )
    mLangs.append( lang );
}

QString Entry::summary( const QString &lang ) const
{
  if ( mSummaryMap.isEmpty() ) return QString();

  if ( !mSummaryMap[ lang ].isEmpty() ) return mSummaryMap[ lang ];
  else {
    QStringList langs = KGlobal::locale()->languageList();
    for(QStringList::Iterator it = langs.begin(); it != langs.end(); ++it)
      if( !mSummaryMap[ *it ].isEmpty() ) return mSummaryMap[ *it ];
  }
  if ( !mSummaryMap[ QString() ].isEmpty() ) return mSummaryMap[ QString() ];
  else return *(mSummaryMap.begin());
}


void Entry::setVersion( const QString &version )
{
  mVersion = version;
}

QString Entry::version() const
{
  return mVersion;
}


void Entry::setRelease( int release )
{
  mRelease = release;
}

int Entry::release() const
{
  return mRelease;
}


void Entry::setReleaseDate( const QDate &d )
{
  mReleaseDate = d;
}

QDate Entry::releaseDate() const
{
  return mReleaseDate;
}


void Entry::setPayload( const KUrl &url, const QString &lang )
{
  mPayloadMap.insert( lang, url );

  if ( !mLangs.contains( lang ) )
    mLangs.append( lang );
}

KUrl Entry::payload( const QString &lang ) const
{
  KUrl payload = mPayloadMap[ lang ];
  if ( payload.isEmpty() ) {
    QStringList langs = KGlobal::locale()->languageList();
    for(QStringList::Iterator it = langs.begin(); it != langs.end(); ++it)
      if( !mPayloadMap[ *it ].isEmpty() ) return mPayloadMap[ *it ];
  }
  if ( payload.isEmpty() ) payload = mPayloadMap [ QString() ];
  if ( payload.isEmpty() && !mPayloadMap.isEmpty() ) {
    payload = *(mPayloadMap.begin());
  }
  return payload;
}


void Entry::setPreview( const KUrl &url, const QString &lang )
{
  mPreviewMap.insert( lang, url );

  if ( !mLangs.contains( lang ) )
    mLangs.append( lang );
}

KUrl Entry::preview( const QString &lang ) const
{
  KUrl preview = mPreviewMap[ lang ];
  if ( preview.isEmpty() ) {
    QStringList langs = KGlobal::locale()->languageList();
    for(QStringList::Iterator it = langs.begin(); it != langs.end(); ++it)
      if( !mPreviewMap[ *it ].isEmpty() ) return mPreviewMap[ *it ];
  }
  if ( preview.isEmpty() ) preview = mPreviewMap [ QString() ];
  if ( preview.isEmpty() && !mPreviewMap.isEmpty() ) {
    preview = *(mPreviewMap.begin());
  }
  return preview;
}


void Entry::setRating( int rating )
{
  mRating = rating;
}

int Entry::rating()
{
  return mRating;
}


void Entry::setDownloads( int downloads )
{
  mDownloads = downloads;
}

int Entry::downloads()
{
  return mDownloads;
}

QString Entry::fullName()
{
  return name() + "-" + version() + "-" + QString::number( release() );
}

QStringList Entry::langs()
{
  return mLangs;
}

void Entry::parseDomElement( const QDomElement &element )
{
  if ( element.tagName() != "stuff" ) return;
  mCategory = element.attribute("category");
  if ( mCategory.isEmpty()) {
    mCategory = element.attribute("type");
    if ( !mCategory.isEmpty()) {
      mCompatibility = true;
    }
  }

  QDomNode n;
  for( n = element.firstChild(); !n.isNull(); n = n.nextSibling() ) {
    QDomElement e = n.toElement();
    if ( e.tagName() == "name" ) {
      QString lang = e.attribute( "lang" );
      setName( e.text().trimmed(), lang );
    }
    if ( e.tagName() == "author" ) {
      setAuthor( e.text().trimmed() );
      QString email = e.attribute( "email" );
      setAuthorEmail( email );
    }
    if ( e.tagName() == "email" ) setAuthorEmail( e.text().trimmed() ); /* kde-look; change on server! */
    if ( e.tagName() == "licence" ) setLicense( e.text().trimmed() );
    if ( e.tagName() == "summary" ) {
      QString lang = e.attribute( "lang" );
      setSummary( e.text().trimmed(), lang );
    }
    if ( e.tagName() == "version" ) setVersion( e.text().trimmed() );
    if ( e.tagName() == "release" ) setRelease( e.text().toInt() );
    if ( e.tagName() == "releasedate" ) {
      QDate date = QDate::fromString( e.text().trimmed(), Qt::ISODate );
      setReleaseDate( date );
    }
    if ( e.tagName() == "preview" ) {
      QString lang = e.attribute( "lang" );
      setPreview( KUrl( e.text().trimmed() ), lang );
    }
    if ( e.tagName() == "payload" ) {
      QString lang = e.attribute( "lang" );
      setPayload( KUrl( e.text().trimmed() ), lang );
    }
    if ( e.tagName() == "rating" ) setRating( e.text().toInt() );
    if ( e.tagName() == "downloads" ) setDownloads( e.text().toInt() );
  }
}

QDomElement Entry::createDomElement( QDomDocument &doc,
                                     QDomElement &parent )
{
  QDomElement entry = doc.createElement( "stuff" );
  parent.appendChild( entry );

  if (mCompatibility) entry.setAttribute("type", mCategory);
  else entry.setAttribute("category", mCategory);

  addElement( doc, entry, "author", author() );
  addElement( doc, entry, "licence", license() );
  addElement( doc, entry, "version", version() );
  addElement( doc, entry, "release", QString::number( release() ) );
  addElement( doc, entry, "rating", QString::number( rating() ) );
  addElement( doc, entry, "downloads", QString::number( downloads() ) );

  addElement( doc, entry, "releasedate",
              releaseDate().toString( Qt::ISODate ) );

  QStringList ls = langs();
  QStringList::ConstIterator it;
  QDomElement e;
  for( it = ls.begin(); it != ls.end(); ++it ) {
    e = addElement( doc, entry, "name", name( *it ) );
    e.setAttribute( "lang", *it );
    e = addElement( doc, entry, "summary", summary( *it ) );
    e.setAttribute( "lang", *it );
    e = addElement( doc, entry, "preview", preview( *it ).url() );
    e.setAttribute( "lang", *it );
    e = addElement( doc, entry, "payload", payload( *it ).url() );
    e.setAttribute( "lang", *it );
  }

  return entry;
}

QDomElement Entry::addElement( QDomDocument &doc, QDomElement &parent,
                               const QString &tag, const QString &value )
{
  QDomElement n = doc.createElement( tag );
  n.appendChild( doc.createTextNode( value ) );
  parent.appendChild( n );

  return n;
}
