/*
    This file is part of libkabc.
    Copyright (c) 2001 Cornelius Schumacher <schumacher@kde.org>

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
    the Free Software Foundation, Inc., 51 Franklin Steet, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include <qapplication.h>
#include <qpair.h>
#include <q3valuelist.h>

#include <ksimpleconfig.h>
#include <kstandarddirs.h>
#include <kdebug.h>

#include "distributionlist.h"

using namespace KABC;

DistributionList::DistributionList( DistributionListManager *manager,
                                    const QString &name ) :
  mManager( manager ), mName( name )
{
  mManager->insert( this );
}

DistributionList::~DistributionList()
{
  mManager->remove( this );
}

void DistributionList::setName( const QString &name )
{
  mName = name;
}

QString DistributionList::name() const
{
  return mName;
}

void DistributionList::insertEntry( const Addressee &a, const QString &email )
{
  Entry e( a, email );

  Q3ValueList<Entry>::Iterator it;
  for( it = mEntries.begin(); it != mEntries.end(); ++it ) {
    if ( (*it).addressee.uid() == a.uid() ) {
      /**
        We have to check if both email addresses contains no data,
        a simple 'email1 == email2' wont work here
       */
      if ( ( (*it).email.isNull() && email.isEmpty() ) ||
           ( (*it).email.isEmpty() && email.isNull() ) ||
           ( (*it).email == email ) ) {
        *it = e;
        return;
      }
    }
  }
  mEntries.append( e );
}

void DistributionList::removeEntry( const Addressee &a, const QString &email )
{
  Q3ValueList<Entry>::Iterator it;
  for( it = mEntries.begin(); it != mEntries.end(); ++it ) {
    if ( (*it).addressee.uid() == a.uid() && (*it).email == email ) {
      mEntries.remove( it );
      return;
    }
  }
}

QStringList DistributionList::emails() const
{
  QStringList emails;

  Entry::List::ConstIterator it;
  for( it = mEntries.begin(); it != mEntries.end(); ++it ) {
    Addressee a = (*it).addressee;
    QString email = (*it).email.isEmpty() ? a.fullEmail() :
                                            a.fullEmail( (*it).email );

    if ( !email.isEmpty() ) {
      emails.append( email );
    }
  }

  return emails;
}

DistributionList::Entry::List DistributionList::entries() const
{
  return mEntries;
}

typedef Q3ValueList< QPair<QString, QString> > MissingEntryList;

class DistributionListManager::DistributionListManagerPrivate
{
  public:
    AddressBook *mAddressBook;
    QMap< QString, MissingEntryList > mMissingEntries;
};

DistributionListManager::DistributionListManager( AddressBook *ab )
  : d( new DistributionListManagerPrivate )
{
  d->mAddressBook = ab;
  mLists.setAutoDelete( true );
}

DistributionListManager::~DistributionListManager()
{
  mLists.clear();

  delete d;
  d = 0;
}

DistributionList *DistributionListManager::list( const QString &name )
{
  DistributionList *list;
  for( list = mLists.first(); list; list = mLists.next() ) {
    if ( list->name() == name ) return list;
  }

  return 0;
}

void DistributionListManager::insert( DistributionList *l )
{
  if ( !l )
    return;

  DistributionList *list;
  for( list = mLists.first(); list; list = mLists.next() ) {
    if ( list->name() == l->name() ) {
      mLists.remove( list );
      break;
    }
  }
  mLists.append( l );
}

void DistributionListManager::remove( DistributionList *l )
{
  if ( !l )
    return;

  DistributionList *list;
  for( list = mLists.first(); list; list = mLists.next() ) {
    if ( list->name() == l->name() ) {
      mLists.remove( list );
      return;
    }
  }
}

QStringList DistributionListManager::listNames()
{
  QStringList names;

  DistributionList *list;
  for( list = mLists.first(); list; list = mLists.next() ) {
    names.append( list->name() );
  }

  return names;
}

bool DistributionListManager::load()
{
  KSimpleConfig cfg( locateLocal( "data", "kabc/distlists" ) );

  QMap<QString,QString> entryMap = cfg.entryMap( "DistributionLists" );
  cfg.setGroup( "DistributionLists" );

  // clear old lists
  mLists.clear();
  d->mMissingEntries.clear();

  QMap<QString,QString>::ConstIterator it;
  for( it = entryMap.constBegin(); it != entryMap.constEnd(); ++it ) {
    QString name = it.key();
    QStringList value = cfg.readListEntry( name );

    kdDebug(5700) << "DLM::load(): " << name << ": " << value.join(",") << endl;

    DistributionList *list = new DistributionList( this, name );

    MissingEntryList missingEntries;
    QStringList::ConstIterator entryIt = value.constBegin();
    while( entryIt != value.constEnd() ) {
      QString id = *entryIt++;
      QString email = *entryIt;

      kdDebug(5700) << "----- Entry " << id << endl;

      Addressee a = d->mAddressBook->findByUid( id );
      if ( !a.isEmpty() ) {
        list->insertEntry( a, email );
      } else {
        missingEntries.append( qMakePair( id, email ) );
      }

      if ( entryIt == value.end() )
        break;
      ++entryIt;
    }

    d->mMissingEntries.insert( name, missingEntries );
  }

  return true;
}

bool DistributionListManager::save()
{
  kdDebug(5700) << "DistListManager::save()" << endl;

  KSimpleConfig cfg( locateLocal( "data", "kabc/distlists" ) );

  cfg.deleteGroup( "DistributionLists" );
  cfg.setGroup( "DistributionLists" );

  DistributionList *list;
  for( list = mLists.first(); list; list = mLists.next() ) {
    kdDebug(5700) << "  Saving '" << list->name() << "'" << endl;

    QStringList value;
    const DistributionList::Entry::List entries = list->entries();
    DistributionList::Entry::List::ConstIterator it;
    for( it = entries.begin(); it != entries.end(); ++it ) {
      value.append( (*it).addressee.uid() );
      value.append( (*it).email );
    }

    if ( d->mMissingEntries.find( list->name() ) != d->mMissingEntries.end() ) {
      const MissingEntryList missList = d->mMissingEntries[ list->name() ];
      MissingEntryList::ConstIterator missIt;
      for ( missIt = missList.begin(); missIt != missList.end(); ++missIt ) {
        value.append( (*missIt).first );
        value.append( (*missIt).second );
      }
    }

    cfg.writeEntry( list->name(), value );
  }

  cfg.sync();

  return true;
}

DistributionListWatcher* DistributionListWatcher::mSelf = 0;

DistributionListWatcher::DistributionListWatcher()
 : QObject( qApp )
{
    setObjectName( "DistributionListWatcher" );
  mDirWatch = new KDirWatch;
  mDirWatch->addFile( locateLocal( "data", "kabc/distlists" ) );

  connect( mDirWatch, SIGNAL( dirty( const QString& ) ), SIGNAL( changed() ) );
  mDirWatch->startScan();
}

DistributionListWatcher::~DistributionListWatcher()
{
  delete mDirWatch;
  mDirWatch = 0;
}

DistributionListWatcher *DistributionListWatcher::self()
{
  kdWarning( !qApp ) << "No QApplication object available, you'll get a memleak!" << endl;

  if ( !mSelf )
    mSelf = new DistributionListWatcher();

  return mSelf;
}

#include "distributionlist.moc"
