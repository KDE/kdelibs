/*
    This file is part of KOrganizer.
    Copyright (c) 2000,2001 Cornelius Schumacher <schumacher@kde.org>
    Copyright (c) 2003 Waldo Bastian <bastian@kde.org>

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
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#include <qcolor.h>
#include <qvariant.h>

#include <kconfig.h>
#include <kstandarddirs.h>
#include <kglobal.h>
#include <kglobalsettings.h>
#include <kdebug.h>

#include "kstringhandler.h"

#include "kconfigskeleton.h"

void KConfigSkeletonItem::readImmutability( KConfig *config )
{
  mIsImmutable = config->entryIsImmutable( mName );
}


KConfigSkeleton::ItemString::ItemString( const QString &group, const QString &name,
                                    QString &reference,
                                    const QString &defaultValue,
                                    Type type )
  : KConfigSkeletonGenericItem<QString>( group, name, reference, defaultValue ),
    mType( type )
{
}

void KConfigSkeleton::ItemString::writeConfig( KConfig *config )
{
  if ( mReference != mLoadedValue ) // WABA: Is this test needed?
  {
    config->setGroup( mGroup );
    if ((mDefault == mReference) && !config->hasDefault( mName))
      config->revertToDefault( mName );
    else if ( mType == Path )
      config->writePathEntry( mName, mReference );
    else if ( mType == Password )
      config->writeEntry( mName, KStringHandler::obscure( mReference ) );
    else
      config->writeEntry( mName, mReference );
  }
}


void KConfigSkeleton::ItemString::readConfig( KConfig *config )
{
  config->setGroup( mGroup );

  if ( mType == Path )
  {
    mReference = config->readPathEntry( mName, mDefault );
  }
  else if ( mType == Password ) 
  {
    QString value = config->readEntry( mName,
                                       KStringHandler::obscure( mDefault ) );
    mReference = KStringHandler::obscure( value );
  }
  else
  {
    mReference = config->readEntry( mName, mDefault );
  }

  mLoadedValue = mReference;

  readImmutability( config );
}

void KConfigSkeleton::ItemString::setProperty(const QVariant & p)
{
  mReference = p.toString();
}

QVariant KConfigSkeleton::ItemString::property() const
{
  return QVariant(mReference);
}


KConfigSkeleton::ItemProperty::ItemProperty( const QString &group,
                                        const QString &name,
                                        QVariant &reference,
                                        QVariant defaultValue )
  : KConfigSkeletonGenericItem<QVariant>( group, name, reference, defaultValue )
{
}

void KConfigSkeleton::ItemProperty::readConfig( KConfig *config )
{
  config->setGroup( mGroup );
  mReference = config->readPropertyEntry( mName, mDefault );
  mLoadedValue = mReference;

  readImmutability( config );
}

void KConfigSkeleton::ItemProperty::setProperty(const QVariant & p)
{
  mReference = p;
}

QVariant KConfigSkeleton::ItemProperty::property() const
{
  return mReference;
}

KConfigSkeleton::ItemBool::ItemBool( const QString &group, const QString &name,
                                bool &reference, bool defaultValue )
  : KConfigSkeletonGenericItem<bool>( group, name, reference, defaultValue )
{
}

void KConfigSkeleton::ItemBool::readConfig( KConfig *config )
{
  config->setGroup( mGroup );
  mReference = config->readBoolEntry( mName, mDefault );
  mLoadedValue = mReference;

  readImmutability( config );
}

void KConfigSkeleton::ItemBool::setProperty(const QVariant & p)
{
  mReference = p.toBool();
}

QVariant KConfigSkeleton::ItemBool::property() const
{
  return QVariant(mReference);
}


KConfigSkeleton::ItemInt::ItemInt( const QString &group, const QString &name,
                              int &reference, int defaultValue )
  : KConfigSkeletonGenericItem<int>( group, name, reference, defaultValue )
{
}

void KConfigSkeleton::ItemInt::readConfig( KConfig *config )
{
  config->setGroup( mGroup );
  mReference = config->readNumEntry( mName, mDefault );
  mLoadedValue = mReference;

  readImmutability( config );
}

void KConfigSkeleton::ItemInt::setProperty(const QVariant & p)
{
  mReference = p.toInt();
}

QVariant KConfigSkeleton::ItemInt::property() const
{
  return QVariant(mReference);
}

KConfigSkeleton::ItemInt64::ItemInt64( const QString &group, const QString &name,
                              Q_INT64 &reference, Q_INT64 defaultValue )
  : KConfigSkeletonGenericItem<Q_INT64>( group, name, reference, defaultValue )
{
}

void KConfigSkeleton::ItemInt64::readConfig( KConfig *config )
{
  config->setGroup( mGroup );
  mReference = config->readNum64Entry( mName, mDefault );
  mLoadedValue = mReference;

  readImmutability( config );
}

void KConfigSkeleton::ItemInt64::setProperty(const QVariant & p)
{
  mReference = p.toLongLong();
}

QVariant KConfigSkeleton::ItemInt64::property() const
{
  return QVariant(mReference);
}

KConfigSkeleton::ItemEnum::ItemEnum( const QString &group, const QString &name,
                              int &reference, const QStringList &choices, 
                              int defaultValue )
  : ItemInt( group, name, reference, defaultValue ), mChoices(choices)
{
}

void KConfigSkeleton::ItemEnum::readConfig( KConfig *config )
{
  config->setGroup( mGroup );
  if (!config->hasKey(mName))
  {
    mReference = mDefault;
  }
  else
  {
    int i = 0;
    mReference = -1;
    QString tmp = config->readEntry( mName ).lower();
    for(QStringList::ConstIterator it = mChoices.begin();
        it != mChoices.end(); ++it, ++i)
    {
      if ((*it).lower() == tmp)
      {
        mReference = i;
        break;
      }
    }
    if (mReference == -1)
       mReference = config->readNumEntry( mName, mDefault );
  }
  mLoadedValue = mReference;

  readImmutability( config );
}

void KConfigSkeleton::ItemEnum::writeConfig( KConfig *config )
{
  if ( mReference != mLoadedValue ) // WABA: Is this test needed?
  {
    config->setGroup( mGroup );
    if ((mDefault == mReference) && !config->hasDefault( mName))
      config->revertToDefault( mName );
    else if ((mReference >= 0) && (mReference < (int) mChoices.count()))
      config->writeEntry( mName, mChoices[mReference] );
    else
      config->writeEntry( mName, mReference );
  }
}

QStringList KConfigSkeleton::ItemEnum::choices() const
{
  return mChoices;
}


KConfigSkeleton::ItemUInt::ItemUInt( const QString &group, const QString &name,
                                unsigned int &reference,
                                unsigned int defaultValue )
  : KConfigSkeletonGenericItem<unsigned int>( group, name, reference, defaultValue )
{
}

void KConfigSkeleton::ItemUInt::readConfig( KConfig *config )
{
  config->setGroup( mGroup );
  mReference = config->readUnsignedNumEntry( mName, mDefault );
  mLoadedValue = mReference;

  readImmutability( config );
}

void KConfigSkeleton::ItemUInt::setProperty(const QVariant & p)
{
  mReference = p.toUInt();
}

QVariant KConfigSkeleton::ItemUInt::property() const
{
  return QVariant(mReference);
}

KConfigSkeleton::ItemUInt64::ItemUInt64( const QString &group, const QString &name,
                              Q_UINT64 &reference, Q_UINT64 defaultValue )
  : KConfigSkeletonGenericItem<Q_UINT64>( group, name, reference, defaultValue )
{
}

void KConfigSkeleton::ItemUInt64::readConfig( KConfig *config )
{
  config->setGroup( mGroup );
  mReference = config->readUnsignedNum64Entry( mName, mDefault );
  mLoadedValue = mReference;

  readImmutability( config );
}

void KConfigSkeleton::ItemUInt64::setProperty(const QVariant & p)
{
  mReference = p.toULongLong();
}

QVariant KConfigSkeleton::ItemUInt64::property() const
{
  return QVariant(mReference);
}

KConfigSkeleton::ItemLong::ItemLong( const QString &group, const QString &name,
                                long &reference, long defaultValue )
  : KConfigSkeletonGenericItem<long>( group, name, reference, defaultValue )
{
}

void KConfigSkeleton::ItemLong::readConfig( KConfig *config )
{
  config->setGroup( mGroup );
  mReference = config->readLongNumEntry( mName, mDefault );
  mLoadedValue = mReference;

  readImmutability( config );
}

void KConfigSkeleton::ItemLong::setProperty(const QVariant & p)
{
  mReference = p.toLongLong();
}

QVariant KConfigSkeleton::ItemLong::property() const
{
  return QVariant((Q_LLONG) mReference);
}


KConfigSkeleton::ItemULong::ItemULong( const QString &group, const QString &name,
                                  unsigned long &reference,
                                  unsigned long defaultValue )
  : KConfigSkeletonGenericItem<unsigned long>( group, name, reference, defaultValue )
{
}

void KConfigSkeleton::ItemULong::readConfig( KConfig *config )
{
  config->setGroup( mGroup );
  mReference = config->readUnsignedLongNumEntry( mName, mDefault );
  mLoadedValue = mReference;

  readImmutability( config );
}

void KConfigSkeleton::ItemULong::setProperty(const QVariant & p)
{
  mReference = p.toULongLong();
}

QVariant KConfigSkeleton::ItemULong::property() const
{
  return QVariant((Q_ULLONG) mReference);
}


KConfigSkeleton::ItemDouble::ItemDouble( const QString &group, const QString &name,
                                    double &reference, double defaultValue )
  : KConfigSkeletonGenericItem<double>( group, name, reference, defaultValue )
{
}

void KConfigSkeleton::ItemDouble::readConfig( KConfig *config )
{
  config->setGroup( mGroup );
  mReference = config->readDoubleNumEntry( mName, mDefault );
  mLoadedValue = mReference;

  readImmutability( config );
}

void KConfigSkeleton::ItemDouble::setProperty(const QVariant & p)
{
  mReference = p.toDouble();
}

QVariant KConfigSkeleton::ItemDouble::property() const
{
  return QVariant(mReference);
}


KConfigSkeleton::ItemColor::ItemColor( const QString &group, const QString &name,
                                  QColor &reference,
                                  const QColor &defaultValue )
  : KConfigSkeletonGenericItem<QColor>( group, name, reference, defaultValue )
{
}

void KConfigSkeleton::ItemColor::readConfig( KConfig *config )
{
  config->setGroup( mGroup );
  mReference = config->readColorEntry( mName, &mDefault );
  mLoadedValue = mReference;

  readImmutability( config );
}

void KConfigSkeleton::ItemColor::setProperty(const QVariant & p)
{
  mReference = p.toColor();
}

QVariant KConfigSkeleton::ItemColor::property() const
{
  return QVariant(mReference);
}


KConfigSkeleton::ItemFont::ItemFont( const QString &group, const QString &name,
                                QFont &reference,
                                const QFont &defaultValue )
  : KConfigSkeletonGenericItem<QFont>( group, name, reference, defaultValue )
{
}

void KConfigSkeleton::ItemFont::readConfig( KConfig *config )
{
  config->setGroup( mGroup );
  mReference = config->readFontEntry( mName, &mDefault );
  mLoadedValue = mReference;

  readImmutability( config );
}

void KConfigSkeleton::ItemFont::setProperty(const QVariant & p)
{
  mReference = p.toFont();
}

QVariant KConfigSkeleton::ItemFont::property() const
{
  return QVariant(mReference);
}


KConfigSkeleton::ItemRect::ItemRect( const QString &group, const QString &name,
                                QRect &reference,
                                const QRect &defaultValue )
  : KConfigSkeletonGenericItem<QRect>( group, name, reference, defaultValue )
{
}

void KConfigSkeleton::ItemRect::readConfig( KConfig *config )
{
  config->setGroup( mGroup );
  mReference = config->readRectEntry( mName, &mDefault );
  mLoadedValue = mReference;

  readImmutability( config );
}

void KConfigSkeleton::ItemRect::setProperty(const QVariant & p)
{
  mReference = p.toRect();
}

QVariant KConfigSkeleton::ItemRect::property() const
{
  return QVariant(mReference);
}


KConfigSkeleton::ItemPoint::ItemPoint( const QString &group, const QString &name,
                                  QPoint &reference,
                                  const QPoint &defaultValue )
  : KConfigSkeletonGenericItem<QPoint>( group, name, reference, defaultValue )
{
}

void KConfigSkeleton::ItemPoint::readConfig( KConfig *config )
{
  config->setGroup( mGroup );
  mReference = config->readPointEntry( mName, &mDefault );
  mLoadedValue = mReference;

  readImmutability( config );
}

void KConfigSkeleton::ItemPoint::setProperty(const QVariant & p)
{
  mReference = p.toPoint();
}

QVariant KConfigSkeleton::ItemPoint::property() const
{
  return QVariant(mReference);
}


KConfigSkeleton::ItemSize::ItemSize( const QString &group, const QString &name,
                                QSize &reference,
                                const QSize &defaultValue )
  : KConfigSkeletonGenericItem<QSize>( group, name, reference, defaultValue )
{
}

void KConfigSkeleton::ItemSize::readConfig( KConfig *config )
{
  config->setGroup( mGroup );
  mReference = config->readSizeEntry( mName, &mDefault );
  mLoadedValue = mReference;

  readImmutability( config );
}

void KConfigSkeleton::ItemSize::setProperty(const QVariant & p)
{
  mReference = p.toSize();
}

QVariant KConfigSkeleton::ItemSize::property() const
{
  return QVariant(mReference);
}


KConfigSkeleton::ItemDateTime::ItemDateTime( const QString &group, const QString &name,
                                        QDateTime &reference,
                                        const QDateTime &defaultValue )
  : KConfigSkeletonGenericItem<QDateTime>( group, name, reference, defaultValue )
{
}

void KConfigSkeleton::ItemDateTime::readConfig( KConfig *config )
{
  config->setGroup( mGroup );
  mReference = config->readDateTimeEntry( mName, &mDefault );
  mLoadedValue = mReference;

  readImmutability( config );
}

void KConfigSkeleton::ItemDateTime::setProperty(const QVariant & p)
{
  mReference = p.toDateTime();
}

QVariant KConfigSkeleton::ItemDateTime::property() const
{
  return QVariant(mReference);
}


KConfigSkeleton::ItemStringList::ItemStringList( const QString &group, const QString &name,
                                            QStringList &reference,
                                            const QStringList &defaultValue )
  : KConfigSkeletonGenericItem<QStringList>( group, name, reference, defaultValue )
{
}

void KConfigSkeleton::ItemStringList::readConfig( KConfig *config )
{
  config->setGroup( mGroup );
  if ( !config->hasKey( mName ) )
    mReference = mDefault;
  else
    mReference = config->readListEntry( mName );
  mLoadedValue = mReference;

  readImmutability( config );
}

void KConfigSkeleton::ItemStringList::setProperty(const QVariant & p)
{
  mReference = p.toStringList();
}

QVariant KConfigSkeleton::ItemStringList::property() const
{
  return QVariant(mReference);
}


KConfigSkeleton::ItemIntList::ItemIntList( const QString &group, const QString &name,
                                      QValueList<int> &reference,
                                      const QValueList<int> &defaultValue )
  : KConfigSkeletonGenericItem<QValueList<int> >( group, name, reference, defaultValue )
{
}

void KConfigSkeleton::ItemIntList::readConfig( KConfig *config )
{
  config->setGroup( mGroup );
  if ( !config->hasKey( mName ) )
    mReference = mDefault;
  else
    mReference = config->readIntListEntry( mName );
  mLoadedValue = mReference;

  readImmutability( config );
}

void KConfigSkeleton::ItemIntList::setProperty(const QVariant & p)
{
  // TODO: Not yet supported
}

QVariant KConfigSkeleton::ItemIntList::property() const
{
  // TODO: Not yet supported
  return QVariant();  
}


KConfigSkeleton::KConfigSkeleton( const QString &configname )
  : mCurrentGroup( "No Group" ), mUseDefaults(false)
{
  kdDebug(177) << "Creating KConfigSkeleton (" << int(this) << ")" << endl;

  if ( !configname.isEmpty() )
  {
    mConfig = KSharedConfig::openConfig( configname );
  }
  else
  {
    mConfig = KGlobal::sharedConfig();
  }
}

KConfigSkeleton::~KConfigSkeleton()
{
  KConfigSkeletonItem::List::ConstIterator it;
  for( it = mItems.begin(); it != mItems.end(); ++it )
  {
    delete *it;
  }
}

void KConfigSkeleton::setCurrentGroup( const QString &group )
{
  mCurrentGroup = group;
}

KConfig *KConfigSkeleton::config() const
{
  return mConfig;
}

bool KConfigSkeleton::useDefaults(bool b)
{
  if (b == mUseDefaults)
     return mUseDefaults;

  mUseDefaults = b;
  KConfigSkeletonItem::List::ConstIterator it;
  for( it = mItems.begin(); it != mItems.end(); ++it )
  {
    (*it)->swapDefault();
  }

  usrUseDefaults(b);
  return !mUseDefaults;
}

void KConfigSkeleton::setDefaults()
{
  KConfigSkeletonItem::List::ConstIterator it;
  for( it = mItems.begin(); it != mItems.end(); ++it ) {
    (*it)->setDefault();
  }

  usrSetDefaults();
}

void KConfigSkeleton::readConfig()
{
  kdDebug(177) << "KConfigSkeleton::readConfig()" << endl;

  KConfigSkeletonItem::List::ConstIterator it;
  for( it = mItems.begin(); it != mItems.end(); ++it )
  {
    (*it)->readConfig( mConfig );
  }

  usrReadConfig();
}

void KConfigSkeleton::writeConfig()
{
  kdDebug(177) << "KConfigSkeleton::writeConfig()" << endl;

  KConfigSkeletonItem::List::ConstIterator it;
  for( it = mItems.begin(); it != mItems.end(); ++it )
  {
    (*it)->writeConfig( mConfig );
  }

  usrWriteConfig();

  mConfig->sync();

  readConfig();
}

void KConfigSkeleton::addItem( const QString &name, KConfigSkeletonItem *item )
{
  mItems.append( item );
  mItemDict.insert( name, item );
  item->readDefault( mConfig );
}

KConfigSkeleton::ItemString *KConfigSkeleton::addItemString( const QString &name, QString &reference,
                                     const QString &defaultValue, const QString &key )
{
  KConfigSkeleton::ItemString *item;
  item = new KConfigSkeleton::ItemString( mCurrentGroup, key.isNull() ? name : key,
                                          reference, defaultValue,
                                          KConfigSkeleton::ItemString::Normal );
  addItem( name, item );
  return item;
}

KConfigSkeleton::ItemString *KConfigSkeleton::addItemPassword( const QString &name, QString &reference,
                                       const QString &defaultValue, const QString &key )
{
  KConfigSkeleton::ItemString *item;
  item = new KConfigSkeleton::ItemString( mCurrentGroup, key.isNull() ? name : key,
                                          reference, defaultValue,
                                          KConfigSkeleton::ItemString::Password );
  addItem( name, item );
  return item;
}

KConfigSkeleton::ItemString *KConfigSkeleton::addItemPath( const QString &name, QString &reference,
                                   const QString &defaultValue, const QString &key )
{
  KConfigSkeleton::ItemString *item;
  item = new KConfigSkeleton::ItemString( mCurrentGroup, key.isNull() ? name : key,
                                          reference, defaultValue,
                                          KConfigSkeleton::ItemString::Path );
  addItem( name, item );
  return item;
}

KConfigSkeleton::ItemProperty *KConfigSkeleton::addItemProperty( const QString &name, QVariant &reference,
                                       const QVariant &defaultValue, const QString &key )
{
  KConfigSkeleton::ItemProperty *item;
  item = new KConfigSkeleton::ItemProperty( mCurrentGroup, key.isNull() ? name : key,
                                            reference, defaultValue );
  addItem( name, item );
  return item;
}

KConfigSkeleton::ItemBool *KConfigSkeleton::addItemBool( const QString &name, bool &reference,
                                   bool defaultValue, const QString &key )
{
  KConfigSkeleton::ItemBool *item;
  item = new KConfigSkeleton::ItemBool( mCurrentGroup, key.isNull() ? name : key,
                                        reference, defaultValue );
  addItem( name, item );
  return item;
}

KConfigSkeleton::ItemInt *KConfigSkeleton::addItemInt( const QString &name, int &reference,
                                  int defaultValue, const QString &key )
{
  KConfigSkeleton::ItemInt *item;
  item = new KConfigSkeleton::ItemInt( mCurrentGroup, key.isNull() ? name : key,
                                       reference, defaultValue );
  addItem( name, item );
  return item;
}

KConfigSkeleton::ItemUInt *KConfigSkeleton::addItemUInt( const QString &name, unsigned int &reference,
                                   unsigned int defaultValue, const QString &key )
{
  KConfigSkeleton::ItemUInt *item;
  item = new KConfigSkeleton::ItemUInt( mCurrentGroup, key.isNull() ? name : key,
                                        reference, defaultValue );
  addItem( name, item );
  return item;
}

KConfigSkeleton::ItemInt64 *KConfigSkeleton::addItemInt64( const QString &name, Q_INT64 &reference,
                                    Q_INT64 defaultValue, const QString &key )
{
  KConfigSkeleton::ItemInt64 *item;
  item = new KConfigSkeleton::ItemInt64( mCurrentGroup, key.isNull() ? name : key,
                                         reference, defaultValue );
  addItem( name, item );
  return item;
}

KConfigSkeleton::ItemUInt64 *KConfigSkeleton::addItemUInt64( const QString &name, Q_UINT64 &reference,
                                     Q_UINT64 defaultValue, const QString &key )
{
  KConfigSkeleton::ItemUInt64 *item;
  item = new KConfigSkeleton::ItemUInt64( mCurrentGroup, key.isNull() ? name : key,
                                          reference, defaultValue );
  addItem( name, item );
  return item;
}

KConfigSkeleton::ItemLong *KConfigSkeleton::addItemLong( const QString &name, long &reference,
                                   long defaultValue, const QString &key )
{
  KConfigSkeleton::ItemLong *item;
  item = new KConfigSkeleton::ItemLong( mCurrentGroup, key.isNull() ? name : key,
                                        reference, defaultValue );
  addItem( name, item );
  return item;
}

KConfigSkeleton::ItemULong *KConfigSkeleton::addItemULong( const QString &name, unsigned long &reference,
                                    unsigned long defaultValue, const QString &key )
{
  KConfigSkeleton::ItemULong *item;
  item = new KConfigSkeleton::ItemULong( mCurrentGroup, key.isNull() ? name : key,
                                         reference, defaultValue );
  addItem( name, item );
  return item;
}

KConfigSkeleton::ItemDouble *KConfigSkeleton::addItemDouble( const QString &name, double &reference,
                                     double defaultValue, const QString &key )
{
  KConfigSkeleton::ItemDouble *item;
  item = new KConfigSkeleton::ItemDouble( mCurrentGroup, key.isNull() ? name : key,
                                          reference, defaultValue );
  addItem( name, item );
  return item;
}

KConfigSkeleton::ItemColor *KConfigSkeleton::addItemColor( const QString &name, QColor &reference,
                                    const QColor &defaultValue, const QString &key )
{
  KConfigSkeleton::ItemColor *item;
  item = new KConfigSkeleton::ItemColor( mCurrentGroup, key.isNull() ? name : key,
                                         reference, defaultValue );
  addItem( name, item );
  return item;
}

KConfigSkeleton::ItemFont *KConfigSkeleton::addItemFont( const QString &name, QFont &reference,
                                   const QFont &defaultValue, const QString &key )
{
  KConfigSkeleton::ItemFont *item;
  item = new KConfigSkeleton::ItemFont( mCurrentGroup, key.isNull() ? name : key,
                                        reference, defaultValue );
  addItem( name, item );
  return item;
}

KConfigSkeleton::ItemRect *KConfigSkeleton::addItemRect( const QString &name, QRect &reference,
                                   const QRect &defaultValue, const QString &key )
{
  KConfigSkeleton::ItemRect *item;
  item = new KConfigSkeleton::ItemRect( mCurrentGroup, key.isNull() ? name : key,
                                        reference, defaultValue );
  addItem( name, item );
  return item;
}

KConfigSkeleton::ItemPoint *KConfigSkeleton::addItemPoint( const QString &name, QPoint &reference,
                                    const QPoint &defaultValue, const QString &key )
{
  KConfigSkeleton::ItemPoint *item;
  item = new KConfigSkeleton::ItemPoint( mCurrentGroup, key.isNull() ? name : key,
                                         reference, defaultValue );
  addItem( name, item );
  return item;
}

KConfigSkeleton::ItemSize *KConfigSkeleton::addItemSize( const QString &name, QSize &reference,
                                   const QSize &defaultValue, const QString &key )
{
  KConfigSkeleton::ItemSize *item;
  item = new KConfigSkeleton::ItemSize( mCurrentGroup, key.isNull() ? name : key,
                                        reference, defaultValue );
  addItem( name, item );
  return item;
}

KConfigSkeleton::ItemDateTime *KConfigSkeleton::addItemDateTime( const QString &name, QDateTime &reference,
                                       const QDateTime &defaultValue, const QString &key )
{
  KConfigSkeleton::ItemDateTime *item;
  item = new KConfigSkeleton::ItemDateTime( mCurrentGroup, key.isNull() ? name : key,
                                            reference, defaultValue );
  addItem( name, item );
  return item;
}

KConfigSkeleton::ItemStringList *KConfigSkeleton::addItemStringList( const QString &name, QStringList &reference,
                                         const QStringList &defaultValue, const QString &key )
{
  KConfigSkeleton::ItemStringList *item;
  item = new KConfigSkeleton::ItemStringList( mCurrentGroup, key.isNull() ? name : key,
                                              reference, defaultValue );
  addItem( name, item );
  return item;
}

KConfigSkeleton::ItemIntList *KConfigSkeleton::addItemIntList( const QString &name, QValueList<int> &reference,
                                      const QValueList<int> &defaultValue, const QString &key )
{
  KConfigSkeleton::ItemIntList *item;
  item = new KConfigSkeleton::ItemIntList( mCurrentGroup, key.isNull() ? name : key,
                                           reference, defaultValue );
  addItem( name, item );
  return item;
}

bool KConfigSkeleton::isImmutable(const QString &name)
{
  KConfigSkeletonItem *item = findItem(name);
  return !item || item->isImmutable();
}

KConfigSkeletonItem *KConfigSkeleton::findItem(const QString &name)
{
  return mItemDict.find(name);
}
