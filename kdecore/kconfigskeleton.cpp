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
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "kconfigskeleton.h"
#include "kstandarddirs.h"
#include "kglobal.h"
#include "kdebug.h"
#include "kstringhandler.h"

void KConfigSkeletonItem::readImmutability( KConfig *config )
{
  mIsImmutable = config->entryIsImmutable( mKey );
}


KConfigSkeleton::ItemString::ItemString( const QString &_group, const QString &_key,
                                    QString &reference,
                                    const QString &defaultValue,
                                    Type type )
  : KConfigSkeletonGenericItem<QString>( _group, _key, reference, defaultValue ),
    mType( type )
{
}

void KConfigSkeleton::ItemString::writeConfig( KConfig *config )
{
  if ( mReference != mLoadedValue ) // WABA: Is this test needed?
  {
    config->setGroup( mGroup );
    if ((mDefault == mReference) && !config->hasDefault( mKey))
      config->revertToDefault( mKey );
    else if ( mType == Path )
      config->writePathEntry( mKey, mReference );
    else if ( mType == Password )
      config->writeEntry( mKey, KStringHandler::obscure( mReference ) );
    else
      config->writeEntry( mKey, mReference );
  }
}


void KConfigSkeleton::ItemString::readConfig( KConfig *config )
{
  config->setGroup( mGroup );

  if ( mType == Path )
  {
    mReference = config->readPathEntry( mKey, mDefault );
  }
  else if ( mType == Password ) 
  {
    QString val = config->readEntry( mKey,
                                       KStringHandler::obscure( mDefault ) );
    mReference = KStringHandler::obscure( val );
  }
  else
  {
    mReference = config->readEntry( mKey, mDefault );
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

KConfigSkeleton::ItemPassword::ItemPassword( const QString &_group, const QString &_key,
                                    QString &reference,
                                    const QString &defaultValue)
  : ItemString( _group, _key, reference, defaultValue, Password )
{
}

KConfigSkeleton::ItemPath::ItemPath( const QString &_group, const QString &_key,
                                    QString &reference,
                                    const QString &defaultValue)
  : ItemString( _group, _key, reference, defaultValue, Path )
{
}

KConfigSkeleton::ItemProperty::ItemProperty( const QString &_group,
                                        const QString &_key,
                                        QVariant &reference,
                                        QVariant defaultValue )
  : KConfigSkeletonGenericItem<QVariant>( _group, _key, reference, defaultValue )
{
}

void KConfigSkeleton::ItemProperty::readConfig( KConfig *config )
{
  config->setGroup( mGroup );
  mReference = config->readEntry( mKey, mDefault );
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

KConfigSkeleton::ItemBool::ItemBool( const QString &_group, const QString &_key,
                                bool &reference, bool defaultValue )
  : KConfigSkeletonGenericItem<bool>( _group, _key, reference, defaultValue )
{
}

void KConfigSkeleton::ItemBool::readConfig( KConfig *config )
{
  config->setGroup( mGroup );
  mReference = config->readEntry( mKey, mDefault );
  mLoadedValue = mReference;

  readImmutability( config );
}

void KConfigSkeleton::ItemBool::setProperty(const QVariant & p)
{
  mReference = p.toBool();
}

QVariant KConfigSkeleton::ItemBool::property() const
{
  return QVariant( mReference );
}


KConfigSkeleton::ItemInt::ItemInt( const QString &_group, const QString &_key,
                              qint32 &reference, qint32 defaultValue )
  : KConfigSkeletonGenericItem<qint32>( _group, _key, reference, defaultValue )
  ,mHasMin(false), mHasMax(false)
{
}

void KConfigSkeleton::ItemInt::readConfig( KConfig *config )
{
  config->setGroup( mGroup );
  mReference = config->readEntry( mKey, mDefault );
  if (mHasMin)
    mReference = qMax(mReference, mMin);
  if (mHasMax)
    mReference = qMin(mReference, mMax);
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

QVariant KConfigSkeleton::ItemInt::minValue() const
{
  if (mHasMin)
    return QVariant(mMin);
  return QVariant();
}

QVariant KConfigSkeleton::ItemInt::maxValue() const
{
  if (mHasMax)
    return QVariant(mMax);
  return QVariant();
}

void KConfigSkeleton::ItemInt::setMinValue(qint32 v)
{
  mHasMin = true;
  mMin = v;
}

void KConfigSkeleton::ItemInt::setMaxValue(qint32 v)
{
  mHasMax = true;
  mMax = v;
}


KConfigSkeleton::ItemLongLong::ItemLongLong( const QString &_group, const QString &_key,
                              qint64 &reference, qint64 defaultValue )
  : KConfigSkeletonGenericItem<qint64>( _group, _key, reference, defaultValue )
  ,mHasMin(false), mHasMax(false)
{
}

void KConfigSkeleton::ItemLongLong::readConfig( KConfig *config )
{
  config->setGroup( mGroup );
  mReference = config->readEntry( mKey, mDefault );
  if (mHasMin)
    mReference = qMax(mReference, mMin);
  if (mHasMax)
    mReference = qMin(mReference, mMax);
  mLoadedValue = mReference;

  readImmutability( config );
}

void KConfigSkeleton::ItemLongLong::setProperty(const QVariant & p)
{
  mReference = p.toLongLong();
}

QVariant KConfigSkeleton::ItemLongLong::property() const
{
  return QVariant(mReference);
}

QVariant KConfigSkeleton::ItemLongLong::minValue() const
{
  if (mHasMin)
    return QVariant(mMin);
  return QVariant();
}

QVariant KConfigSkeleton::ItemLongLong::maxValue() const
{
  if (mHasMax)
    return QVariant(mMax);
  return QVariant();
}

void KConfigSkeleton::ItemLongLong::setMinValue(qint64 v)
{
  mHasMin = true;
  mMin = v;
}

void KConfigSkeleton::ItemLongLong::setMaxValue(qint64 v)
{
  mHasMax = true;
  mMax = v;
}

KConfigSkeleton::ItemEnum::ItemEnum( const QString &_group, const QString &_key,
                                     int &reference,
                                     const QList<Choice> &choicesList,
                                     int defaultValue )
  : ItemInt( _group, _key, reference, defaultValue ), mChoices( choicesList )
{
}

void KConfigSkeleton::ItemEnum::readConfig( KConfig *config )
{
  config->setGroup( mGroup );
  if (!config->hasKey(mKey))
  {
    mReference = mDefault;
  }
  else
  {
    int i = 0;
    mReference = -1;
    QString tmp = config->readEntry( mKey, QString() ).toLower();
    for(QList<Choice>::ConstIterator it = mChoices.begin();
        it != mChoices.end(); ++it, ++i)
    {
      if ((*it).name.toLower() == tmp)
      {
        mReference = i;
        break;
      }
    }
    if (mReference == -1)
       mReference = config->readEntry( mKey, mDefault );
  }
  mLoadedValue = mReference;

  readImmutability( config );
}

void KConfigSkeleton::ItemEnum::writeConfig( KConfig *config )
{
  if ( mReference != mLoadedValue ) // WABA: Is this test needed?
  {
    config->setGroup( mGroup );
    if ((mDefault == mReference) && !config->hasDefault( mKey))
      config->revertToDefault( mKey );
    else if ((mReference >= 0) && (mReference < (int) mChoices.count()))
      config->writeEntry( mKey, mChoices[mReference].name );
    else
      config->writeEntry( mKey, mReference );
  }
}

QList<KConfigSkeleton::ItemEnum::Choice> KConfigSkeleton::ItemEnum::choices() const
{
  return mChoices;
}


KConfigSkeleton::ItemUInt::ItemUInt( const QString &_group, const QString &_key,
                                quint32 &reference,
                                quint32 defaultValue )
  : KConfigSkeletonGenericItem<quint32>( _group, _key, reference, defaultValue )
  ,mHasMin(false), mHasMax(false)
{
}

void KConfigSkeleton::ItemUInt::readConfig( KConfig *config )
{
  config->setGroup( mGroup );
  mReference = config->readEntry( mKey, mDefault );
  if (mHasMin)
    mReference = qMax(mReference, mMin);
  if (mHasMax)
    mReference = qMin(mReference, mMax);
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

QVariant KConfigSkeleton::ItemUInt::minValue() const
{
  if (mHasMin)
    return QVariant(mMin);
  return QVariant();
}

QVariant KConfigSkeleton::ItemUInt::maxValue() const
{
  if (mHasMax)
    return QVariant(mMax);
  return QVariant();
}

void KConfigSkeleton::ItemUInt::setMinValue(quint32 v)
{
  mHasMin = true;
  mMin = v;
}

void KConfigSkeleton::ItemUInt::setMaxValue(quint32 v)
{
  mHasMax = true;
  mMax = v;
}


KConfigSkeleton::ItemULongLong::ItemULongLong( const QString &_group, const QString &_key,
                              quint64 &reference, quint64 defaultValue )
  : KConfigSkeletonGenericItem<quint64>( _group, _key, reference, defaultValue )
  ,mHasMin(false), mHasMax(false)
{
}

void KConfigSkeleton::ItemULongLong::readConfig( KConfig *config )
{
  config->setGroup( mGroup );
  mReference = config->readEntry( mKey, mDefault );
  if (mHasMin)
    mReference = qMax(mReference, mMin);
  if (mHasMax)
    mReference = qMin(mReference, mMax);
  mLoadedValue = mReference;

  readImmutability( config );
}

void KConfigSkeleton::ItemULongLong::setProperty(const QVariant & p)
{
  mReference = p.toULongLong();
}

QVariant KConfigSkeleton::ItemULongLong::property() const
{
  return QVariant(mReference);
}

QVariant KConfigSkeleton::ItemULongLong::minValue() const
{
  if (mHasMin)
    return QVariant(mMin);
  return QVariant();
}

QVariant KConfigSkeleton::ItemULongLong::maxValue() const
{
  if (mHasMax)
    return QVariant(mMax);
  return QVariant();
}

void KConfigSkeleton::ItemULongLong::setMinValue(quint64 v)
{
  mHasMin = true;
  mMin = v;
}

void KConfigSkeleton::ItemULongLong::setMaxValue(quint64 v)
{
  mHasMax = true;
  mMax = v;
}

KConfigSkeleton::ItemDouble::ItemDouble( const QString &_group, const QString &_key,
                                    double &reference, double defaultValue )
  : KConfigSkeletonGenericItem<double>( _group, _key, reference, defaultValue )
  ,mHasMin(false), mHasMax(false)
{
}

void KConfigSkeleton::ItemDouble::readConfig( KConfig *config )
{
  config->setGroup( mGroup );
  mReference = config->readEntry( mKey, mDefault );
  if (mHasMin)
    mReference = qMax(mReference, mMin);
  if (mHasMax)
    mReference = qMin(mReference, mMax);
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

QVariant KConfigSkeleton::ItemDouble::minValue() const
{
  if (mHasMin)
    return QVariant(mMin);
  return QVariant();
}

QVariant KConfigSkeleton::ItemDouble::maxValue() const
{
  if (mHasMax)
    return QVariant(mMax);
  return QVariant();
}

void KConfigSkeleton::ItemDouble::setMinValue(double v)
{
  mHasMin = true;
  mMin = v;
}

void KConfigSkeleton::ItemDouble::setMaxValue(double v)
{
  mHasMax = true;
  mMax = v;
}


KConfigSkeleton::ItemColor::ItemColor( const QString &_group, const QString &_key,
                                  QColor &reference,
                                  const QColor &defaultValue )
  : KConfigSkeletonGenericItem<QColor>( _group, _key, reference, defaultValue )
{
}

void KConfigSkeleton::ItemColor::readConfig( KConfig *config )
{
  config->setGroup( mGroup );
  mReference = config->readEntry( mKey, mDefault );
  mLoadedValue = mReference;

  readImmutability( config );
}

void KConfigSkeleton::ItemColor::setProperty(const QVariant & p)
{
  mReference = qvariant_cast<QColor>(p);
}

QVariant KConfigSkeleton::ItemColor::property() const
{
  return QVariant(mReference);
}


KConfigSkeleton::ItemFont::ItemFont( const QString &_group, const QString &_key,
                                QFont &reference,
                                const QFont &defaultValue )
  : KConfigSkeletonGenericItem<QFont>( _group, _key, reference, defaultValue )
{
}

void KConfigSkeleton::ItemFont::readConfig( KConfig *config )
{
  config->setGroup( mGroup );
  mReference = config->readEntry( mKey, mDefault );
  mLoadedValue = mReference;

  readImmutability( config );
}

void KConfigSkeleton::ItemFont::setProperty(const QVariant & p)
{
  mReference = qvariant_cast<QFont>(p);
}

QVariant KConfigSkeleton::ItemFont::property() const
{
  return QVariant(mReference);
}


KConfigSkeleton::ItemRect::ItemRect( const QString &_group, const QString &_key,
                                QRect &reference,
                                const QRect &defaultValue )
  : KConfigSkeletonGenericItem<QRect>( _group, _key, reference, defaultValue )
{
}

void KConfigSkeleton::ItemRect::readConfig( KConfig *config )
{
  config->setGroup( mGroup );
  mReference = config->readEntry( mKey, mDefault );
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


KConfigSkeleton::ItemPoint::ItemPoint( const QString &_group, const QString &_key,
                                  QPoint &reference,
                                  const QPoint &defaultValue )
  : KConfigSkeletonGenericItem<QPoint>( _group, _key, reference, defaultValue )
{
}

void KConfigSkeleton::ItemPoint::readConfig( KConfig *config )
{
  config->setGroup( mGroup );
  mReference = config->readEntry( mKey, mDefault );
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


KConfigSkeleton::ItemSize::ItemSize( const QString &_group, const QString &_key,
                                QSize &reference,
                                const QSize &defaultValue )
  : KConfigSkeletonGenericItem<QSize>( _group, _key, reference, defaultValue )
{
}

void KConfigSkeleton::ItemSize::readConfig( KConfig *config )
{
  config->setGroup( mGroup );
  mReference = config->readEntry( mKey, mDefault );
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


KConfigSkeleton::ItemDateTime::ItemDateTime( const QString &_group, const QString &_key,
                                        QDateTime &reference,
                                        const QDateTime &defaultValue )
  : KConfigSkeletonGenericItem<QDateTime>( _group, _key, reference, defaultValue )
{
}

void KConfigSkeleton::ItemDateTime::readConfig( KConfig *config )
{
  config->setGroup( mGroup );
  mReference = config->readEntry( mKey, mDefault );
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


KConfigSkeleton::ItemStringList::ItemStringList( const QString &_group, const QString &_key,
                                            QStringList &reference,
                                            const QStringList &defaultValue )
  : KConfigSkeletonGenericItem<QStringList>( _group, _key, reference, defaultValue )
{
}

void KConfigSkeleton::ItemStringList::readConfig( KConfig *config )
{
  config->setGroup( mGroup );
  if ( !config->hasKey( mKey ) )
    mReference = mDefault;
  else
    mReference = config->readEntry( mKey, mDefault );
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


KConfigSkeleton::ItemPathList::ItemPathList( const QString &_group, const QString &_key,
                                            QStringList &reference,
                                            const QStringList &defaultValue )
  : ItemStringList( _group, _key, reference, defaultValue )
{
}

void KConfigSkeleton::ItemPathList::readConfig( KConfig *config )
{
  config->setGroup( mGroup );
  if ( !config->hasKey( mKey ) )
    mReference = mDefault;
  else
    mReference = config->readPathListEntry( mKey );
  mLoadedValue = mReference;

  readImmutability( config );
}

void KConfigSkeleton::ItemPathList::writeConfig( KConfig *config )
{
  if ( mReference != mLoadedValue ) // WABA: Is this test needed?
  {
    config->setGroup( mGroup );
    if ((mDefault == mReference) && !config->hasDefault( mKey))
      config->revertToDefault( mKey );
    else {
      QStringList sl = mReference;
      config->writePathEntry( mKey, sl );
    }
  }
}


KConfigSkeleton::ItemIntList::ItemIntList( const QString &_group, const QString &_key,
                                      QList<int> &reference,
                                      const QList<int> &defaultValue )
  : KConfigSkeletonGenericItem<QList<int> >( _group, _key, reference, defaultValue )
{
}

void KConfigSkeleton::ItemIntList::readConfig( KConfig *config )
{
  config->setGroup( mGroup );
  if ( !config->hasKey( mKey ) )
    mReference = mDefault;
  else
    mReference = config->readEntry( mKey , mDefault );
  mLoadedValue = mReference;

  readImmutability( config );
}

void KConfigSkeleton::ItemIntList::setProperty(const QVariant &)
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
  kDebug(177) << "Creating KConfigSkeleton (" << (void *)this << ")" << endl;

  if ( !configname.isEmpty() )
  {
    mConfig = KSharedConfig::openConfig( configname );
  }
  else
  {
    mConfig = KGlobal::sharedConfig();
  }
}

KConfigSkeleton::KConfigSkeleton(KSharedConfig::Ptr pConfig)
  : QObject(0), mCurrentGroup( "No Group" ), mUseDefaults(false)
{
  kDebug(177) << "Creating KConfigSkeleton (" << (void *)this << ")" << endl;
  mConfig = pConfig;
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

KConfig *KConfigSkeleton::config()
{
  return mConfig.data();
}

const KConfig *KConfigSkeleton::config() const
{
  return mConfig.data();
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
  kDebug(177) << "KConfigSkeleton::readConfig()" << endl;
  
  QString origGroup = mConfig->group();

  mConfig->reparseConfiguration();
  KConfigSkeletonItem::List::ConstIterator it;
  for( it = mItems.begin(); it != mItems.end(); ++it )
  {
    (*it)->readConfig( mConfig.data() );
  }

  usrReadConfig();
  
  mConfig->setGroup(origGroup);
}

void KConfigSkeleton::writeConfig()
{
  kDebug(177) << "KConfigSkeleton::writeConfig()" << endl;

  QString origGroup = mConfig->group();

  KConfigSkeletonItem::List::ConstIterator it;
  for( it = mItems.begin(); it != mItems.end(); ++it )
  {
    (*it)->writeConfig( mConfig.data() );
  }

  usrWriteConfig();

  mConfig->sync();

  readConfig();

  mConfig->setGroup(origGroup);

  emit configChanged();
}

void KConfigSkeleton::addItem( KConfigSkeletonItem *item, const QString &name )
{
  item->setName( name.isEmpty() ? item->key() : name );
  mItems.append( item );
  mItemDict.insert( item->name(), item );
  item->readDefault( mConfig.data() );
}

KConfigSkeleton::ItemString *KConfigSkeleton::addItemString( const QString &name, QString &reference,
                                     const QString &defaultValue, const QString &key )
{
  KConfigSkeleton::ItemString *item;
  item = new KConfigSkeleton::ItemString( mCurrentGroup, key.isEmpty() ? name : key,
                                          reference, defaultValue,
                                          KConfigSkeleton::ItemString::Normal );
  addItem( item, name );
  return item;
}

KConfigSkeleton::ItemPassword *KConfigSkeleton::addItemPassword( const QString &name, QString &reference,
                                       const QString &defaultValue, const QString &key )
{
  KConfigSkeleton::ItemPassword *item;
  item = new KConfigSkeleton::ItemPassword( mCurrentGroup, key.isNull() ? name : key,
                                          reference, defaultValue );
  addItem( item, name );
  return item;
}

KConfigSkeleton::ItemPath *KConfigSkeleton::addItemPath( const QString &name, QString &reference,
                                   const QString &defaultValue, const QString &key )
{
  KConfigSkeleton::ItemPath *item;
  item = new KConfigSkeleton::ItemPath( mCurrentGroup, key.isNull() ? name : key,
                                        reference, defaultValue );
  addItem( item, name );
  return item;
}

KConfigSkeleton::ItemProperty *KConfigSkeleton::addItemProperty( const QString &name, QVariant &reference,
                                       const QVariant &defaultValue, const QString &key )
{
  KConfigSkeleton::ItemProperty *item;
  item = new KConfigSkeleton::ItemProperty( mCurrentGroup, key.isNull() ? name : key,
                                            reference, defaultValue );
  addItem( item, name );
  return item;
}

KConfigSkeleton::ItemBool *KConfigSkeleton::addItemBool( const QString &name, bool &reference,
                                   bool defaultValue, const QString &key )
{
  KConfigSkeleton::ItemBool *item;
  item = new KConfigSkeleton::ItemBool( mCurrentGroup, key.isNull() ? name : key,
                                        reference, defaultValue );
  addItem( item, name );
  return item;
}

KConfigSkeleton::ItemInt *KConfigSkeleton::addItemInt( const QString &name, qint32 &reference,
                                  qint32 defaultValue, const QString &key )
{
  KConfigSkeleton::ItemInt *item;
  item = new KConfigSkeleton::ItemInt( mCurrentGroup, key.isNull() ? name : key,
                                       reference, defaultValue );
  addItem( item, name );
  return item;
}

KConfigSkeleton::ItemUInt *KConfigSkeleton::addItemUInt( const QString &name, quint32 &reference,
                                   quint32 defaultValue, const QString &key )
{
  KConfigSkeleton::ItemUInt *item;
  item = new KConfigSkeleton::ItemUInt( mCurrentGroup, key.isNull() ? name : key,
                                        reference, defaultValue );
  addItem( item, name );
  return item;
}

KConfigSkeleton::ItemLongLong *KConfigSkeleton::addItemLongLong( const QString &name, qint64 &reference,
                                    qint64 defaultValue, const QString &key )
{
  KConfigSkeleton::ItemLongLong *item;
  item = new KConfigSkeleton::ItemLongLong( mCurrentGroup, key.isNull() ? name : key,
                                         reference, defaultValue );
  addItem( item, name );
  return item;
}

KConfigSkeleton::ItemULongLong *KConfigSkeleton::addItemULongLong( const QString &name, quint64 &reference,
                                     quint64 defaultValue, const QString &key )
{
  KConfigSkeleton::ItemULongLong *item;
  item = new KConfigSkeleton::ItemULongLong( mCurrentGroup, key.isNull() ? name : key,
                                          reference, defaultValue );
  addItem( item, name );
  return item;
}

KConfigSkeleton::ItemDouble *KConfigSkeleton::addItemDouble( const QString &name, double &reference,
                                     double defaultValue, const QString &key )
{
  KConfigSkeleton::ItemDouble *item;
  item = new KConfigSkeleton::ItemDouble( mCurrentGroup, key.isNull() ? name : key,
                                          reference, defaultValue );
  addItem( item, name );
  return item;
}

KConfigSkeleton::ItemColor *KConfigSkeleton::addItemColor( const QString &name, QColor &reference,
                                    const QColor &defaultValue, const QString &key )
{
  KConfigSkeleton::ItemColor *item;
  item = new KConfigSkeleton::ItemColor( mCurrentGroup, key.isNull() ? name : key,
                                         reference, defaultValue );
  addItem( item, name );
  return item;
}

KConfigSkeleton::ItemFont *KConfigSkeleton::addItemFont( const QString &name, QFont &reference,
                                   const QFont &defaultValue, const QString &key )
{
  KConfigSkeleton::ItemFont *item;
  item = new KConfigSkeleton::ItemFont( mCurrentGroup, key.isNull() ? name : key,
                                        reference, defaultValue );
  addItem( item, name );
  return item;
}

KConfigSkeleton::ItemRect *KConfigSkeleton::addItemRect( const QString &name, QRect &reference,
                                   const QRect &defaultValue, const QString &key )
{
  KConfigSkeleton::ItemRect *item;
  item = new KConfigSkeleton::ItemRect( mCurrentGroup, key.isNull() ? name : key,
                                        reference, defaultValue );
  addItem( item, name );
  return item;
}

KConfigSkeleton::ItemPoint *KConfigSkeleton::addItemPoint( const QString &name, QPoint &reference,
                                    const QPoint &defaultValue, const QString &key )
{
  KConfigSkeleton::ItemPoint *item;
  item = new KConfigSkeleton::ItemPoint( mCurrentGroup, key.isNull() ? name : key,
                                         reference, defaultValue );
  addItem( item, name );
  return item;
}

KConfigSkeleton::ItemSize *KConfigSkeleton::addItemSize( const QString &name, QSize &reference,
                                   const QSize &defaultValue, const QString &key )
{
  KConfigSkeleton::ItemSize *item;
  item = new KConfigSkeleton::ItemSize( mCurrentGroup, key.isNull() ? name : key,
                                        reference, defaultValue );
  addItem( item, name );
  return item;
}

KConfigSkeleton::ItemDateTime *KConfigSkeleton::addItemDateTime( const QString &name, QDateTime &reference,
                                       const QDateTime &defaultValue, const QString &key )
{
  KConfigSkeleton::ItemDateTime *item;
  item = new KConfigSkeleton::ItemDateTime( mCurrentGroup, key.isNull() ? name : key,
                                            reference, defaultValue );
  addItem( item, name );
  return item;
}

KConfigSkeleton::ItemStringList *KConfigSkeleton::addItemStringList( const QString &name, QStringList &reference,
                                         const QStringList &defaultValue, const QString &key )
{
  KConfigSkeleton::ItemStringList *item;
  item = new KConfigSkeleton::ItemStringList( mCurrentGroup, key.isNull() ? name : key,
                                              reference, defaultValue );
  addItem( item, name );
  return item;
}

KConfigSkeleton::ItemIntList *KConfigSkeleton::addItemIntList( const QString &name, QList<int> &reference,
                                      const QList<int> &defaultValue, const QString &key )
{
  KConfigSkeleton::ItemIntList *item;
  item = new KConfigSkeleton::ItemIntList( mCurrentGroup, key.isNull() ? name : key,
                                           reference, defaultValue );
  addItem( item, name );
  return item;
}

bool KConfigSkeleton::isImmutable(const QString &name)
{
  KConfigSkeletonItem *item = findItem(name);
  return !item || item->isImmutable();
}

KConfigSkeletonItem *KConfigSkeleton::findItem(const QString &name)
{
  return mItemDict.value(name);
}

#include "kconfigskeleton.moc"

