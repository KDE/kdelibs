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

#include "kconfigskeleton.h"

void KConfigSkeleton::Item::readImmutability( KConfig *config )
{
  mIsImmutable = config->entryIsImmutable( mName );
}


KConfigSkeleton::ItemString::ItemString( const QString &group, const QString &name,
                                    QString &reference,
                                    const QString &defaultValue,
                                    Type type )
  : GenericItem<QString>( group, name, reference, defaultValue ),
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
      config->writeEntry( mName, endecryptStr( mReference ) );
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
    QString value = config->readEntry( mName, endecryptStr( mDefault ) );
    mReference = endecryptStr( value );
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

QString KConfigSkeleton::ItemString::endecryptStr( const QString &aStr )
{
  QString result;
  for ( uint i = 0; i < aStr.length(); i++ )
  {
    result += (aStr[i].unicode() < 0x20) ? aStr[i] : QChar(0x1001F - aStr[i].unicode());
  }
  return result;
}


KConfigSkeleton::ItemProperty::ItemProperty( const QString &group,
                                        const QString &name,
                                        QVariant &reference,
                                        QVariant defaultValue )
  : GenericItem<QVariant>( group, name, reference, defaultValue )
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
  : GenericItem<bool>( group, name, reference, defaultValue )
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
  : GenericItem<int>( group, name, reference, defaultValue )
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
  : GenericItem<unsigned int>( group, name, reference, defaultValue )
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

KConfigSkeleton::ItemLong::ItemLong( const QString &group, const QString &name,
                                long &reference, long defaultValue )
  : GenericItem<long>( group, name, reference, defaultValue )
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
  : GenericItem<unsigned long>( group, name, reference, defaultValue )
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
  : GenericItem<double>( group, name, reference, defaultValue )
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
  : GenericItem<QColor>( group, name, reference, defaultValue )
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
  : GenericItem<QFont>( group, name, reference, defaultValue )
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
  : GenericItem<QRect>( group, name, reference, defaultValue )
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
  : GenericItem<QPoint>( group, name, reference, defaultValue )
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
  : GenericItem<QSize>( group, name, reference, defaultValue )
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
  : GenericItem<QDateTime>( group, name, reference, defaultValue )
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
  : GenericItem<QStringList>( group, name, reference, defaultValue )
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
  : GenericItem<QValueList<int> >( group, name, reference, defaultValue )
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
  Item::List::ConstIterator it;
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
  Item::List::ConstIterator it;
  for( it = mItems.begin(); it != mItems.end(); ++it )
  {
    (*it)->swapDefault();
  }

  usrUseDefaults(b);
  return !mUseDefaults;
}

void KConfigSkeleton::readConfig()
{
  kdDebug(177) << "KConfigSkeleton::readConfig()" << endl;

  Item::List::ConstIterator it;
  for( it = mItems.begin(); it != mItems.end(); ++it )
  {
    (*it)->readConfig( mConfig );
  }

  usrReadConfig();
}

void KConfigSkeleton::writeConfig()
{
  kdDebug(177) << "KConfigSkeleton::writeConfig()" << endl;

  Item::List::ConstIterator it;
  for( it = mItems.begin(); it != mItems.end(); ++it )
  {
    (*it)->writeConfig( mConfig );
  }

  usrWriteConfig();

  mConfig->sync();

  readConfig();
}

void KConfigSkeleton::addItem( const QString &name, Item *item )
{
  mItems.append( item );
  mItemDict.insert( name, item );
  item->readDefault( mConfig );
}

void KConfigSkeleton::addItemString( const QString &name, const QString &key, QString &reference, const QString &defaultValue )
{
  addItem( name, new KConfigSkeleton::ItemString( mCurrentGroup, key, reference, defaultValue, KConfigSkeleton::ItemString::Normal ) );
}

void KConfigSkeleton::addItemPassword( const QString &name, const QString &key, QString &reference, const QString &defaultValue )
{
  addItem( name, new KConfigSkeleton::ItemString( mCurrentGroup, key, reference, defaultValue, KConfigSkeleton::ItemString::Password ) );
}

void KConfigSkeleton::addItemPath( const QString &name, const QString &key, QString &reference, const QString &defaultValue )
{
  addItem( name, new KConfigSkeleton::ItemString( mCurrentGroup, key, reference, defaultValue, KConfigSkeleton::ItemString::Path ) );
}

void KConfigSkeleton::addItemProperty( const QString &name,
                              const QString &key, QVariant &reference,
                              const QVariant &defaultValue )
{
  addItem( name, new KConfigSkeleton::ItemProperty( mCurrentGroup, key, reference, defaultValue ) );
}

void KConfigSkeleton::addItemBool( const QString &name, const QString &key, bool &reference, bool defaultValue )
{
  addItem( name, new KConfigSkeleton::ItemBool( mCurrentGroup, key, reference, defaultValue ) );
}

void KConfigSkeleton::addItemInt( const QString &name, const QString &key, int &reference, int defaultValue )
{
  addItem( name, new KConfigSkeleton::ItemInt( mCurrentGroup, key, reference, defaultValue ) );
}

void KConfigSkeleton::addItemUInt( const QString &name, const QString &key, unsigned int &reference,
                          unsigned int defaultValue )
{
  addItem( name, new KConfigSkeleton::ItemUInt( mCurrentGroup, key, reference, defaultValue ) );
}

void KConfigSkeleton::addItemLong( const QString &name, const QString &key, long &reference, long defaultValue )
{
  addItem( name, new KConfigSkeleton::ItemLong( mCurrentGroup, key, reference, defaultValue ) );
}

void KConfigSkeleton::addItemULong( const QString &name, const QString &key, unsigned long &reference,
                           unsigned long defaultValue )
{
  addItem( name, new KConfigSkeleton::ItemULong( mCurrentGroup, key, reference, defaultValue ) );
}

void KConfigSkeleton::addItemDouble( const QString &name, const QString &key, double &reference,
                            double defaultValue )
{
  addItem( name, new KConfigSkeleton::ItemDouble( mCurrentGroup, key, reference, defaultValue ) );
}

void KConfigSkeleton::addItemColor( const QString &name, const QString &key, QColor &reference, const QColor &defaultValue )
{
  addItem( name, new KConfigSkeleton::ItemColor( mCurrentGroup, key, reference, defaultValue ) );
}

void KConfigSkeleton::addItemFont( const QString &name, const QString &key, QFont &reference, const QFont &defaultValue )
{
  addItem( name, new KConfigSkeleton::ItemFont( mCurrentGroup, key, reference, defaultValue ) );
}

void KConfigSkeleton::addItemRect( const QString &name, const QString &key, QRect &reference,
                          const QRect &defaultValue )
{
  addItem( name, new KConfigSkeleton::ItemRect( mCurrentGroup, key, reference, defaultValue ) );
}

void KConfigSkeleton::addItemPoint( const QString &name, 
                           const QString &key, QPoint &reference,
                           const QPoint &defaultValue )
{
  addItem( name, new KConfigSkeleton::ItemPoint( mCurrentGroup, key, reference, defaultValue ) );
}

void KConfigSkeleton::addItemSize( const QString &name,
                          const QString &key, QSize &reference,
                          const QSize &defaultValue )
{
  addItem( name, new KConfigSkeleton::ItemSize( mCurrentGroup, key, reference, defaultValue ) );
}

void KConfigSkeleton::addItemDateTime( const QString &name,
                              const QString &key, QDateTime &reference,
                              const QDateTime &defaultValue )
{
  addItem( name, new KConfigSkeleton::ItemDateTime( mCurrentGroup, key, reference, defaultValue ) );
}

void KConfigSkeleton::addItemStringList( const QString &name,
                                const QString &key, QStringList &reference,
                                const QStringList &defaultValue )
{
  addItem( name, new KConfigSkeleton::ItemStringList( mCurrentGroup, key, reference, defaultValue ) );
}

void KConfigSkeleton::addItemIntList( const QString &name,
                             const QString &key, QValueList<int> &reference,
                             const QValueList<int> &defaultValue )
{
  addItem( name, new KConfigSkeleton::ItemIntList( mCurrentGroup, key, reference, defaultValue ) );
}

bool KConfigSkeleton::isImmutable(const QString &name)
{
  Item *item = findItem(name);
  return !item || item->isImmutable();
}

KConfigSkeleton::Item *KConfigSkeleton::findItem(const QString &name)
{
  return mItemDict.find(name);
}
