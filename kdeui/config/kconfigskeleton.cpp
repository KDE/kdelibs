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

class KConfigSkeleton::Private
{
public:
  Private()
    : mCurrentGroup( "No Group" ), mUseDefaults( false )
  {}
  ~Private()
  {
    KConfigSkeletonItem::List::ConstIterator it;
    for( it = mItems.begin(); it != mItems.end(); ++it )
    {
      delete *it;
    }
  }
  QString mCurrentGroup;

  KSharedConfig::Ptr mConfig; // pointer to KConfig object

  KConfigSkeletonItem::List mItems;
  KConfigSkeletonItem::Dict mItemDict;

  bool mUseDefaults;
};

class KConfigSkeletonItemPrivate
{
public:
    KConfigSkeletonItemPrivate()
        : mIsImmutable(true)
    {}
    bool mIsImmutable; ///< Indicates this item is immutable

    QString mLabel; ///< The label for this item
    QString mWhatsThis; ///< The What's This text for this item
};

KConfigSkeletonItem::KConfigSkeletonItem(const QString & _group,
                                         const QString & _key)
    : mGroup(_group)
    , mKey(_key)
    , d( new KConfigSkeletonItemPrivate )
{
}

KConfigSkeletonItem::~KConfigSkeletonItem()
{
    delete d;
}

void KConfigSkeletonItem::setGroup( const QString &_group )
{
    mGroup = _group;
}

QString KConfigSkeletonItem::group() const
{
    return mGroup;
}

void KConfigSkeletonItem::setKey( const QString &_key )
{
    mKey = _key;
}

QString KConfigSkeletonItem::key() const
{
    return mKey;
}

void KConfigSkeletonItem::setName(const QString &_name)
{
    mName = _name;
}

QString KConfigSkeletonItem::name() const
{
    return mName;
}

void KConfigSkeletonItem::setLabel( const QString &l )
{
    d->mLabel = l;
}

QString KConfigSkeletonItem::label() const
{
    return d->mLabel;
}

void KConfigSkeletonItem::setWhatsThis( const QString &w )
{
    d->mWhatsThis = w;
}

QString KConfigSkeletonItem::whatsThis() const
{
    return d->mWhatsThis;
}

QVariant KConfigSkeletonItem::minValue() const
{
    return QVariant();
}

QVariant KConfigSkeletonItem::maxValue() const
{
    return QVariant();
}

bool KConfigSkeletonItem::isImmutable() const
{
    return d->mIsImmutable;
}

void KConfigSkeletonItem::readImmutability( const KConfigGroup &group )
{
  d->mIsImmutable = group.isEntryImmutable( mKey );
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
    KConfigGroup cg(config, mGroup );
    if ((mDefault == mReference) && !cg.hasDefault( mKey))
      cg.revertToDefault( mKey );
    else if ( mType == Path )
      cg.writePathEntry( mKey, mReference );
    else if ( mType == Password )
      cg.writeEntry( mKey, KStringHandler::obscure( mReference ) );
    else
      cg.writeEntry( mKey, mReference );
  }
}


void KConfigSkeleton::ItemString::readConfig( KConfig *config )
{
  KConfigGroup cg(config, mGroup );

  if ( mType == Path )
  {
    mReference = cg.readPathEntry( mKey, mDefault );
  }
  else if ( mType == Password )
  {
    QString val = cg.readEntry( mKey, KStringHandler::obscure( mDefault ) );
    mReference = KStringHandler::obscure( val );
  }
  else
  {
    mReference = cg.readEntry( mKey, mDefault );
  }

  mLoadedValue = mReference;

  readImmutability( cg );
}

void KConfigSkeleton::ItemString::setProperty(const QVariant & p)
{
  mReference = p.toString();
}

bool KConfigSkeleton::ItemString::isEqual(const QVariant &v) const
{
    return mReference == v.toString();
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

KConfigSkeleton::ItemUrl::ItemUrl( const QString &_group, const QString &_key,
                                    KUrl &reference,
                                    const KUrl &defaultValue )
  : KConfigSkeletonGenericItem<KUrl>( _group, _key, reference, defaultValue )
{
}

void KConfigSkeleton::ItemUrl::writeConfig( KConfig *config )
{
    if ( mReference != mLoadedValue ) // WABA: Is this test needed?
    {
        KConfigGroup cg(config, mGroup );
        if ((mDefault == mReference) && !cg.hasDefault( mKey))
            cg.revertToDefault( mKey );
        else
            cg.writeEntry<QString>( mKey, mReference.url() );
    }
}

void KConfigSkeleton::ItemUrl::readConfig( KConfig *config )
{
    KConfigGroup cg(config, mGroup );

    mReference = KUrl( cg.readEntry<QString>( mKey, mDefault.url() ) );
    mLoadedValue = mReference;

    readImmutability( cg );
}

void KConfigSkeleton::ItemUrl::setProperty(const QVariant & p)
{
    mReference = qvariant_cast<KUrl>(p);
}

bool KConfigSkeleton::ItemUrl::isEqual(const QVariant &v) const
{
    return mReference == qvariant_cast<KUrl>(v);
}

QVariant KConfigSkeleton::ItemUrl::property() const
{
    return qVariantFromValue<KUrl>(mReference);
}

KConfigSkeleton::ItemProperty::ItemProperty( const QString &_group,
                                        const QString &_key,
                                        QVariant &reference,
                                        const QVariant &defaultValue )
  : KConfigSkeletonGenericItem<QVariant>( _group, _key, reference, defaultValue )
{
}

void KConfigSkeleton::ItemProperty::readConfig( KConfig *config )
{
  KConfigGroup cg(config, mGroup );
  mReference = cg.readEntry( mKey, mDefault );
  mLoadedValue = mReference;

  readImmutability( cg );
}

void KConfigSkeleton::ItemProperty::setProperty(const QVariant & p)
{
  mReference = p;
}

bool KConfigSkeleton::ItemProperty::isEqual(const QVariant &v) const
{
    //this might cause problems if the QVariants are not of default types
    return mReference == v;
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
  KConfigGroup cg(config, mGroup );
  mReference = cg.readEntry( mKey, mDefault );
  mLoadedValue = mReference;

  readImmutability( cg );
}

void KConfigSkeleton::ItemBool::setProperty(const QVariant & p)
{
  mReference = p.toBool();
}

bool KConfigSkeleton::ItemBool::isEqual(const QVariant &v) const
{
    return mReference == v.toBool();
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
  KConfigGroup cg(config, mGroup );
  mReference = cg.readEntry( mKey, mDefault );
  if (mHasMin)
    mReference = qMax(mReference, mMin);
  if (mHasMax)
    mReference = qMin(mReference, mMax);
  mLoadedValue = mReference;

  readImmutability( cg );
}

void KConfigSkeleton::ItemInt::setProperty(const QVariant & p)
{
  mReference = p.toInt();
}

bool KConfigSkeleton::ItemInt::isEqual(const QVariant &v) const
{
    return mReference == v.toInt();
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
  KConfigGroup cg(config, mGroup );
  mReference = cg.readEntry( mKey, mDefault );
  if (mHasMin)
    mReference = qMax(mReference, mMin);
  if (mHasMax)
    mReference = qMin(mReference, mMax);
  mLoadedValue = mReference;

  readImmutability( cg );
}

void KConfigSkeleton::ItemLongLong::setProperty(const QVariant & p)
{
  mReference = p.toLongLong();
}

bool KConfigSkeleton::ItemLongLong::isEqual(const QVariant &v) const
{
    return mReference == v.toLongLong();
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
                                     qint32 &reference,
                                     const QList<Choice> &choices,
                                     qint32 defaultValue )
  : ItemInt( _group, _key, reference, defaultValue ), mChoices( choices )
{
}

void KConfigSkeleton::ItemEnum::readConfig( KConfig *config )
{
  KConfigGroup cg(config, mGroup );
  if (!cg.hasKey(mKey))
  {
    mReference = mDefault;
  }
  else
  {
    int i = 0;
    mReference = -1;
    QString tmp = cg.readEntry( mKey, QString() ).toLower();
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
       mReference = cg.readEntry( mKey, mDefault );
  }
  mLoadedValue = mReference;

  readImmutability( cg );
}

void KConfigSkeleton::ItemEnum::writeConfig( KConfig *config )
{
  if ( mReference != mLoadedValue ) // WABA: Is this test needed?
  {
    KConfigGroup cg(config, mGroup );
    if ((mDefault == mReference) && !cg.hasDefault( mKey))
      cg.revertToDefault( mKey );
    else if ((mReference >= 0) && (mReference < (int) mChoices.count()))
      cg.writeEntry( mKey, mChoices[mReference].name );
    else
      cg.writeEntry( mKey, mReference );
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
  KConfigGroup cg(config, mGroup );
  mReference = cg.readEntry( mKey, mDefault );
  if (mHasMin)
    mReference = qMax(mReference, mMin);
  if (mHasMax)
    mReference = qMin(mReference, mMax);
  mLoadedValue = mReference;

  readImmutability( cg );
}

void KConfigSkeleton::ItemUInt::setProperty(const QVariant & p)
{
  mReference = p.toUInt();
}

bool KConfigSkeleton::ItemUInt::isEqual(const QVariant &v) const
{
    return mReference == v.toUInt();
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
  KConfigGroup cg(config, mGroup );
  mReference = cg.readEntry( mKey, mDefault );
  if (mHasMin)
    mReference = qMax(mReference, mMin);
  if (mHasMax)
    mReference = qMin(mReference, mMax);
  mLoadedValue = mReference;

  readImmutability( cg );
}

void KConfigSkeleton::ItemULongLong::setProperty(const QVariant & p)
{
  mReference = p.toULongLong();
}

bool KConfigSkeleton::ItemULongLong::isEqual(const QVariant &v) const
{
    return mReference == v.toULongLong();
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
  KConfigGroup cg(config, mGroup );
  mReference = cg.readEntry( mKey, mDefault );
  if (mHasMin)
    mReference = qMax(mReference, mMin);
  if (mHasMax)
    mReference = qMin(mReference, mMax);
  mLoadedValue = mReference;

  readImmutability( cg );
}

void KConfigSkeleton::ItemDouble::setProperty(const QVariant & p)
{
  mReference = p.toDouble();
}

bool KConfigSkeleton::ItemDouble::isEqual(const QVariant &v) const
{
    return mReference == v.toDouble();
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
  KConfigGroup cg(config, mGroup );
  mReference = cg.readEntry( mKey, mDefault );
  mLoadedValue = mReference;

  readImmutability( cg );
}

void KConfigSkeleton::ItemColor::setProperty(const QVariant & p)
{
  mReference = qvariant_cast<QColor>(p);
}

bool KConfigSkeleton::ItemColor::isEqual(const QVariant &v) const
{
    return mReference == qvariant_cast<QColor>(v);
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
  KConfigGroup cg(config, mGroup );
  mReference = cg.readEntry( mKey, mDefault );
  mLoadedValue = mReference;

  readImmutability( cg );
}

void KConfigSkeleton::ItemFont::setProperty(const QVariant & p)
{
  mReference = qvariant_cast<QFont>(p);
}

bool KConfigSkeleton::ItemFont::isEqual(const QVariant &v) const
{
    return mReference == qvariant_cast<QFont>(v);
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
  KConfigGroup cg(config, mGroup );
  mReference = cg.readEntry( mKey, mDefault );
  mLoadedValue = mReference;

  readImmutability( cg );
}

void KConfigSkeleton::ItemRect::setProperty(const QVariant & p)
{
  mReference = p.toRect();
}

bool KConfigSkeleton::ItemRect::isEqual(const QVariant &v) const
{
    return mReference == v.toRect();
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
  KConfigGroup cg(config, mGroup );
  mReference = cg.readEntry( mKey, mDefault );
  mLoadedValue = mReference;

  readImmutability( cg );
}

void KConfigSkeleton::ItemPoint::setProperty(const QVariant & p)
{
  mReference = p.toPoint();
}

bool KConfigSkeleton::ItemPoint::isEqual(const QVariant &v) const
{
    return mReference == v.toPoint();
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
  KConfigGroup cg(config, mGroup );
  mReference = cg.readEntry( mKey, mDefault );
  mLoadedValue = mReference;

  readImmutability( cg );
}

void KConfigSkeleton::ItemSize::setProperty(const QVariant & p)
{
  mReference = p.toSize();
}

bool KConfigSkeleton::ItemSize::isEqual(const QVariant &v) const
{
    return mReference == v.toSize();
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
  KConfigGroup cg(config, mGroup );
  mReference = cg.readEntry( mKey, mDefault );
  mLoadedValue = mReference;

  readImmutability( cg );
}

void KConfigSkeleton::ItemDateTime::setProperty(const QVariant & p)
{
  mReference = p.toDateTime();
}

bool KConfigSkeleton::ItemDateTime::isEqual(const QVariant &v) const
{
    return mReference == v.toDateTime();
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
  KConfigGroup cg(config, mGroup );
  if ( !cg.hasKey( mKey ) )
    mReference = mDefault;
  else
    mReference = cg.readEntry( mKey, mDefault );
  mLoadedValue = mReference;

  readImmutability( cg );
}

void KConfigSkeleton::ItemStringList::setProperty(const QVariant & p)
{
  mReference = p.toStringList();
}

bool KConfigSkeleton::ItemStringList::isEqual(const QVariant &v) const
{
    return mReference == v.toStringList();
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
  KConfigGroup cg(config, mGroup );
  if ( !cg.hasKey( mKey ) )
    mReference = mDefault;
  else
    mReference = cg.readPathEntry( mKey, QStringList() );
  mLoadedValue = mReference;

  readImmutability( cg );
}

void KConfigSkeleton::ItemPathList::writeConfig( KConfig *config )
{
  if ( mReference != mLoadedValue ) // WABA: Is this test needed?
  {
    KConfigGroup cg(config, mGroup );
    if ((mDefault == mReference) && !cg.hasDefault( mKey))
      cg.revertToDefault( mKey );
    else {
      QStringList sl = mReference;
      cg.writePathEntry( mKey, sl );
    }
  }
}

KConfigSkeleton::ItemUrlList::ItemUrlList( const QString &_group, const QString &_key,
                                            KUrl::List &reference,
                                            const KUrl::List &defaultValue )
  : KConfigSkeletonGenericItem<KUrl::List>( _group, _key, reference, defaultValue )
{
}

void KConfigSkeleton::ItemUrlList::readConfig( KConfig *config )
{
    KConfigGroup cg(config, mGroup );
    if ( !cg.hasKey( mKey ) )
        mReference = mDefault;
    else
        mReference = KUrl::List( cg.readEntry<QStringList>( mKey, mDefault.toStringList() ) );
    mLoadedValue = mReference;

    readImmutability( cg );
}

void KConfigSkeleton::ItemUrlList::writeConfig( KConfig *config )
{
    if ( mReference != mLoadedValue ) // WABA: Is this test needed?
    {
        KConfigGroup cg(config, mGroup );
        if ((mDefault == mReference) && !cg.hasDefault( mKey))
            cg.revertToDefault( mKey );
        else
            cg.writeEntry<QStringList>( mKey, mReference.toStringList() );
    }
}

void KConfigSkeleton::ItemUrlList::setProperty(const QVariant & p)
{
    mReference = qvariant_cast<KUrl::List>(p);
}

bool KConfigSkeleton::ItemUrlList::isEqual(const QVariant &v) const
{
    return mReference == qvariant_cast<KUrl::List>(v);
}

QVariant KConfigSkeleton::ItemUrlList::property() const
{
    return qVariantFromValue<KUrl::List>(mReference);
}

#ifndef KDE_USE_FINAL
Q_DECLARE_METATYPE( QList<int> )
#endif

KConfigSkeleton::ItemIntList::ItemIntList( const QString &_group, const QString &_key,
                                      QList<int> &reference,
                                      const QList<int> &defaultValue )
  : KConfigSkeletonGenericItem<QList<int> >( _group, _key, reference, defaultValue )
{
}

void KConfigSkeleton::ItemIntList::readConfig( KConfig *config )
{
  KConfigGroup cg(config, mGroup );
  if ( !cg.hasKey( mKey ) )
    mReference = mDefault;
  else
    mReference = cg.readEntry( mKey , mDefault );
  mLoadedValue = mReference;

  readImmutability( cg );
}

void KConfigSkeleton::ItemIntList::setProperty(const QVariant &p)
{
    mReference = qvariant_cast< QList<int> >(p);
}

bool KConfigSkeleton::ItemIntList::isEqual(const QVariant &v) const
{
    return mReference == qvariant_cast< QList<int> >(v);
}

QVariant KConfigSkeleton::ItemIntList::property() const
{
    return qVariantFromValue< QList<int> >(mReference);
}



KConfigSkeleton::KConfigSkeleton(const QString &configname, QObject* parent)
  : QObject(parent),
    d( new Private )
{
  kDebug(177) << "Creating KConfigSkeleton (" << (void *)this << ")";

  if ( !configname.isEmpty() )
  {
    d->mConfig = KSharedConfig::openConfig( configname );
  }
  else
  {
    d->mConfig = KGlobal::config();
  }
}

KConfigSkeleton::KConfigSkeleton(KSharedConfig::Ptr pConfig, QObject* parent)
  : QObject(parent),
    d( new Private )
{
  kDebug(177) << "Creating KConfigSkeleton (" << (void *)this << ")";
  d->mConfig = pConfig;
}


KConfigSkeleton::~KConfigSkeleton()
{
  delete d;
}

void KConfigSkeleton::setCurrentGroup( const QString &group )
{
  d->mCurrentGroup = group;
}

QString KConfigSkeleton::currentGroup() const
{
    return d->mCurrentGroup;
}

KConfig *KConfigSkeleton::config()
{
  return d->mConfig.data();
}

const KConfig *KConfigSkeleton::config() const
{
  return d->mConfig.data();
}

void KConfigSkeleton::setSharedConfig(KSharedConfig::Ptr pConfig)
{
    d->mConfig = pConfig;
}

KConfigSkeletonItem::List KConfigSkeleton::items() const
{
    return d->mItems;
}

bool KConfigSkeleton::useDefaults(bool b)
{
  if (b == d->mUseDefaults)
    return d->mUseDefaults;

  d->mUseDefaults = b;
  KConfigSkeletonItem::List::ConstIterator it;
  for( it = d->mItems.begin(); it != d->mItems.end(); ++it )
  {
    (*it)->swapDefault();
  }
  usrUseDefaults(b);
  return !d->mUseDefaults;
}

void KConfigSkeleton::setDefaults()
{
  KConfigSkeletonItem::List::ConstIterator it;
  for( it = d->mItems.begin(); it != d->mItems.end(); ++it ) {
    (*it)->setDefault();
  }
  usrSetDefaults();
}

void KConfigSkeleton::readConfig()
{
  kDebug(177) << "KConfigSkeleton::readConfig()";
  d->mConfig->reparseConfiguration();
  KConfigSkeletonItem::List::ConstIterator it;
  for( it = d->mItems.begin(); it != d->mItems.end(); ++it )
  {
    (*it)->readConfig( d->mConfig.data() );
  }
  usrReadConfig();
}

void KConfigSkeleton::writeConfig()
{
  kDebug(177) << "KConfigSkeleton::writeConfig()";
  KConfigSkeletonItem::List::ConstIterator it;
  for( it = d->mItems.begin(); it != d->mItems.end(); ++it )
  {
    (*it)->writeConfig( d->mConfig.data() );
  }
  usrWriteConfig();

  d->mConfig->sync();

  readConfig();

  emit configChanged();
}

bool KConfigSkeleton::usrUseDefaults(bool)
{
  return false;
}

void KConfigSkeleton::usrSetDefaults()
{
}

void KConfigSkeleton::usrReadConfig()
{
}

void KConfigSkeleton::usrWriteConfig()
{
}

void KConfigSkeleton::addItem( KConfigSkeletonItem *item, const QString &name )
{
  item->setName( name.isEmpty() ? item->key() : name );
  d->mItems.append( item );
  d->mItemDict.insert( item->name(), item );
  item->readDefault( d->mConfig.data() );
  item->readConfig( d->mConfig.data() );
}

KConfigSkeleton::ItemString *KConfigSkeleton::addItemString( const QString &name, QString &reference,
                                     const QString &defaultValue, const QString &key )
{
  KConfigSkeleton::ItemString *item;
  item = new KConfigSkeleton::ItemString( d->mCurrentGroup, key.isEmpty() ? name : key,
                                          reference, defaultValue,
                                          KConfigSkeleton::ItemString::Normal );
  addItem( item, name );
  return item;
}

KConfigSkeleton::ItemPassword *KConfigSkeleton::addItemPassword( const QString &name, QString &reference,
                                       const QString &defaultValue, const QString &key )
{
  KConfigSkeleton::ItemPassword *item;
  item = new KConfigSkeleton::ItemPassword( d->mCurrentGroup, key.isNull() ? name : key,
                                          reference, defaultValue );
  addItem( item, name );
  return item;
}

KConfigSkeleton::ItemPath *KConfigSkeleton::addItemPath( const QString &name, QString &reference,
                                   const QString &defaultValue, const QString &key )
{
  KConfigSkeleton::ItemPath *item;
  item = new KConfigSkeleton::ItemPath( d->mCurrentGroup, key.isNull() ? name : key,
                                        reference, defaultValue );
  addItem( item, name );
  return item;
}

KConfigSkeleton::ItemProperty *KConfigSkeleton::addItemProperty( const QString &name, QVariant &reference,
                                       const QVariant &defaultValue, const QString &key )
{
  KConfigSkeleton::ItemProperty *item;
  item = new KConfigSkeleton::ItemProperty( d->mCurrentGroup, key.isNull() ? name : key,
                                            reference, defaultValue );
  addItem( item, name );
  return item;
}

KConfigSkeleton::ItemBool *KConfigSkeleton::addItemBool( const QString &name, bool &reference,
                                   bool defaultValue, const QString &key )
{
  KConfigSkeleton::ItemBool *item;
  item = new KConfigSkeleton::ItemBool( d->mCurrentGroup, key.isNull() ? name : key,
                                        reference, defaultValue );
  addItem( item, name );
  return item;
}

KConfigSkeleton::ItemInt *KConfigSkeleton::addItemInt( const QString &name, qint32 &reference,
                                  qint32 defaultValue, const QString &key )
{
  KConfigSkeleton::ItemInt *item;
  item = new KConfigSkeleton::ItemInt( d->mCurrentGroup, key.isNull() ? name : key,
                                       reference, defaultValue );
  addItem( item, name );
  return item;
}

KConfigSkeleton::ItemUInt *KConfigSkeleton::addItemUInt( const QString &name, quint32 &reference,
                                   quint32 defaultValue, const QString &key )
{
  KConfigSkeleton::ItemUInt *item;
  item = new KConfigSkeleton::ItemUInt( d->mCurrentGroup, key.isNull() ? name : key,
                                        reference, defaultValue );
  addItem( item, name );
  return item;
}

KConfigSkeleton::ItemLongLong *KConfigSkeleton::addItemLongLong( const QString &name, qint64 &reference,
                                    qint64 defaultValue, const QString &key )
{
  KConfigSkeleton::ItemLongLong *item;
  item = new KConfigSkeleton::ItemLongLong( d->mCurrentGroup, key.isNull() ? name : key,
                                         reference, defaultValue );
  addItem( item, name );
  return item;
}

KConfigSkeleton::ItemLongLong *KConfigSkeleton::addItemInt64(
        const QString& name,
        qint64 &reference,
        qint64 defaultValue,
        const QString & key)
{
    return addItemLongLong(name, reference, defaultValue, key);
}

KConfigSkeleton::ItemULongLong *KConfigSkeleton::addItemULongLong( const QString &name, quint64 &reference,
                                     quint64 defaultValue, const QString &key )
{
  KConfigSkeleton::ItemULongLong *item;
  item = new KConfigSkeleton::ItemULongLong( d->mCurrentGroup, key.isNull() ? name : key,
                                          reference, defaultValue );
  addItem( item, name );
  return item;
}

KConfigSkeleton::ItemULongLong *KConfigSkeleton::addItemUInt64(
        const QString & name,
        quint64 &reference,
        quint64 defaultValue,
        const QString & key)
{
    return addItemULongLong(name, reference, defaultValue, key);
}

KConfigSkeleton::ItemDouble *KConfigSkeleton::addItemDouble( const QString &name, double &reference,
                                     double defaultValue, const QString &key )
{
  KConfigSkeleton::ItemDouble *item;
  item = new KConfigSkeleton::ItemDouble( d->mCurrentGroup, key.isNull() ? name : key,
                                          reference, defaultValue );
  addItem( item, name );
  return item;
}

KConfigSkeleton::ItemColor *KConfigSkeleton::addItemColor( const QString &name, QColor &reference,
                                    const QColor &defaultValue, const QString &key )
{
  KConfigSkeleton::ItemColor *item;
  item = new KConfigSkeleton::ItemColor( d->mCurrentGroup, key.isNull() ? name : key,
                                         reference, defaultValue );
  addItem( item, name );
  return item;
}

KConfigSkeleton::ItemFont *KConfigSkeleton::addItemFont( const QString &name, QFont &reference,
                                   const QFont &defaultValue, const QString &key )
{
  KConfigSkeleton::ItemFont *item;
  item = new KConfigSkeleton::ItemFont( d->mCurrentGroup, key.isNull() ? name : key,
                                        reference, defaultValue );
  addItem( item, name );
  return item;
}

KConfigSkeleton::ItemRect *KConfigSkeleton::addItemRect( const QString &name, QRect &reference,
                                   const QRect &defaultValue, const QString &key )
{
  KConfigSkeleton::ItemRect *item;
  item = new KConfigSkeleton::ItemRect( d->mCurrentGroup, key.isNull() ? name : key,
                                        reference, defaultValue );
  addItem( item, name );
  return item;
}

KConfigSkeleton::ItemPoint *KConfigSkeleton::addItemPoint( const QString &name, QPoint &reference,
                                    const QPoint &defaultValue, const QString &key )
{
  KConfigSkeleton::ItemPoint *item;
  item = new KConfigSkeleton::ItemPoint( d->mCurrentGroup, key.isNull() ? name : key,
                                         reference, defaultValue );
  addItem( item, name );
  return item;
}

KConfigSkeleton::ItemSize *KConfigSkeleton::addItemSize( const QString &name, QSize &reference,
                                   const QSize &defaultValue, const QString &key )
{
  KConfigSkeleton::ItemSize *item;
  item = new KConfigSkeleton::ItemSize( d->mCurrentGroup, key.isNull() ? name : key,
                                        reference, defaultValue );
  addItem( item, name );
  return item;
}

KConfigSkeleton::ItemDateTime *KConfigSkeleton::addItemDateTime( const QString &name, QDateTime &reference,
                                       const QDateTime &defaultValue, const QString &key )
{
  KConfigSkeleton::ItemDateTime *item;
  item = new KConfigSkeleton::ItemDateTime( d->mCurrentGroup, key.isNull() ? name : key,
                                            reference, defaultValue );
  addItem( item, name );
  return item;
}

KConfigSkeleton::ItemStringList *KConfigSkeleton::addItemStringList( const QString &name, QStringList &reference,
                                         const QStringList &defaultValue, const QString &key )
{
  KConfigSkeleton::ItemStringList *item;
  item = new KConfigSkeleton::ItemStringList( d->mCurrentGroup, key.isNull() ? name : key,
                                              reference, defaultValue );
  addItem( item, name );
  return item;
}

KConfigSkeleton::ItemIntList *KConfigSkeleton::addItemIntList( const QString &name, QList<int> &reference,
                                      const QList<int> &defaultValue, const QString &key )
{
  KConfigSkeleton::ItemIntList *item;
  item = new KConfigSkeleton::ItemIntList( d->mCurrentGroup, key.isNull() ? name : key,
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
  return d->mItemDict.value(name);
}

#include "kconfigskeleton.moc"

