/*
   This file is part of the KDE libraries
   Copyright (c) 1999 Preston Brown <pbrown@kde.org>
   Copyright (c) 1997 Matthias Kalle Dalheimer <kalle@kde.org>

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

#include "kconfigbase.h"
#include "kconfigbackend.h"
#include "kconfigdata.h"
#include "kdebug.h"
#include "kglobal.h"
#include "klocale.h"
#include "kstandarddirs.h"
#include "kstringhandler.h"
#include "kcomponentdata.h"

#include <stdio.h>
#include <stdlib.h>

#include <QtGui/QColor>
#include <QtCore/QDate>
#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtGui/QFont>
#include <QtCore/QPoint>
#include <QtCore/QRect>
#include <QtCore/QMutableStringListIterator>
#include <QtCore/QTextIStream>

class KConfigBase::Private
{
public:
     Private( KComponentData* _componentData, KConfigBase* base )
         : backEnd(0),
           readDefaults( false ),
           bDirty( false ),
           bLocaleInitialized( false ),
           bExpand( false ),
           componentData( _componentData ),
           group( base, QString() )
     {
     }

     ~Private()
     {
         delete backEnd;
         backEnd = 0;

         delete componentData;
         componentData = 0;
     }

     /**
      * A back end for loading/saving to disk in a particular format.
      */
     KConfigBackEnd *backEnd;

     bool readDefaults;

     /**
      * The locale to retrieve keys under if possible, i.e en_US or fr.  */
     QByteArray aLocaleString;

     /**
      * Indicates whether there are any dirty entries in the config object
      * that need to be written back to disk. */
     bool bDirty;
     bool bLocaleInitialized;
     bool bExpand;     // whether dollar expansion is used
     KComponentData *componentData;

     /**
      * The currently selected group. */
     KConfigGroup group;
};

KConfigBase::KConfigBase()
  : d( new Private( new KComponentData(KGlobal::mainComponent()) , this ) )
{
}

KConfigBase::KConfigBase(const KComponentData &componentData)
    : d( new Private( new KComponentData( componentData ) , this ) )
{
}

KConfigBase::~KConfigBase()
{
    delete d;
}

const KComponentData &KConfigBase::componentData() const
{
    return *d->componentData;
}

void KConfigBase::setLocale()
{
    d->bLocaleInitialized = true;

    if ( KGlobal::locale() ) {
        d->aLocaleString = KGlobal::locale()->language().toUtf8();
    } else {
        d->aLocaleString = KLocale::defaultLanguage().toUtf8();
    }

    if ( d->backEnd ) {
        d->backEnd->setLocaleString( d->aLocaleString );
    }
}

QString KConfigBase::locale() const
{
  return QString::fromUtf8( d->aLocaleString );
}

void KConfigBase::setGroup( const QString& _group )
{
    internalSetGroup( _group.toUtf8() );
}

void KConfigBase::setGroup( const char *_group )
{
    internalSetGroup( QByteArray(_group) );
}

void KConfigBase::setGroup( const QByteArray &_group )
{
    internalSetGroup( _group );
}

void KConfigBase::internalSetGroup( const QByteArray &_group )
{
    if ( _group.isEmpty() ) {
        d->group = KConfigGroup(this, "<default>");
    } else {
        d->group = KConfigGroup(this, _group);
    }
}

KConfigGroup& KConfigBase::internalGroup( ) const
{
    return d->group;
}

QString KConfigBase::group() const
{
    return d->group.group();
}

bool KConfigBase::hasKey(const QString &key) const
{
   return hasKey(key.toUtf8().constData());
}

bool KConfigBase::hasKey(const char *pKey) const
{
    return d->group.hasKey(pKey);
}

bool KConfigBase::hasGroup(const QString &_group) const
{
    return internalHasGroup( _group.toUtf8());
}

bool KConfigBase::hasGroup(const char *_pGroup) const
{
    return internalHasGroup( QByteArray(_pGroup) );
}

bool KConfigBase::hasGroup(const QByteArray &_pGroup) const
{
  return internalHasGroup( _pGroup);
}

bool KConfigBase::isImmutable() const
{
  return (getConfigState() != ReadWrite);
}

bool KConfigBase::groupIsImmutable(const QString& _group) const
{
  if (getConfigState() != ReadWrite)
     return true;

  KEntryKey groupKey(_group.toUtf8(), 0);
  KEntry entry = lookupData(groupKey);
  return entry.bImmutable;
}

bool KConfigBase::entryIsImmutable(const QString &key) const
{
  return d->group.entryIsImmutable(key);
}


QString KConfigBase::readEntryUntranslated( const QString& pKey,
                                const QString& aDefault ) const
{
    return d->group.readEntryUntranslated(pKey.toUtf8().constData(), aDefault);
}


QString KConfigBase::readEntryUntranslated( const char *pKey,
                                const QString& aDefault ) const
{
    return d->group.readEntryUntranslated(pKey, aDefault);
}

QString KConfigBase::readEntry( const char *pKey, const char *aDefault ) const
{
    return d->group.readEntry(pKey, aDefault);
}

#ifdef KDE3_SUPPORT
#include <q3strlist.h>
int KConfigBase::readListEntry( const QString& pKey, Q3StrList &list, char sep = ',' ) const
{
    return readListEntry( pKey.toUtf8().constData(), list, sep);
}

int KConfigBase::readListEntry( const char *pKey,
                                Q3StrList &list, char sep ) const
{
  if( !hasKey( pKey ) )
    return 0;

  QByteArray str_list = readEntryUtf8( pKey );
  if (str_list.isEmpty())
    return 0;

  list.clear();
  QByteArray value = "";
  int len = str_list.length();

  for (int i = 0; i < len; i++) {
    if (str_list[i] != sep && str_list[i] != '\\') {
      value += str_list[i];
      continue;
    }
    if (str_list[i] == '\\') {
      i++;
      if ( i < len )
        value += str_list[i];
      continue;
    }
    // if we fell through to here, we are at a separator.  Append
    // contents of value to the list
    // !!! Sergey A. Sukiyazov <corwin@micom.don.ru> !!!
    // A QStrList may contain values in 8bit locale cpecified
    // encoding
    list.append( value );
    value.truncate(0);
  }

  if ( str_list[len-1] != sep || ( len > 1 && str_list[len-2] == '\\' ) )
    list.append( value );
  return list.count();
}
#endif

QStringList KConfigBase::readEntry(const char* pKey, const QStringList& aDefault, char sep) const
{
  return d->group.readEntry(pKey, aDefault, sep);
}

QString KConfigBase::readPathEntry( const QString& pKey, const QString& pDefault ) const
{
  return d->group.readPathEntry(pKey.toUtf8().constData(), pDefault);
}

QString KConfigBase::readPathEntry( const char *pKey, const QString& pDefault ) const
{
  return d->group.readPathEntry(pKey, pDefault);
}

QStringList KConfigBase::readPathListEntry( const QString& pKey, char sep ) const
{
  return d->group.readPathListEntry(pKey.toUtf8().constData(), sep);
}

QStringList KConfigBase::readPathListEntry( const char *pKey, char sep ) const
{
  return d->group.readPathListEntry(pKey, sep);
}


QRect KConfigBase::readRectEntry( const QString& pKey, const QRect* pDefault ) const
{
  return d->group.readEntry(pKey, (pDefault? *pDefault: QRect()));
}

QRect KConfigBase::readRectEntry( const char *pKey, const QRect* pDefault ) const
{
  return d->group.readEntry(pKey, (pDefault? *pDefault: QRect()));
}

QPoint KConfigBase::readPointEntry( const QString& pKey,
                                    const QPoint* pDefault ) const
{
  return d->group.readEntry(pKey, (pDefault? *pDefault: QPoint()));
}

QPoint KConfigBase::readPointEntry( const char *pKey,
                                    const QPoint* pDefault ) const
{
  return d->group.readEntry(pKey, (pDefault? *pDefault: QPoint()));
}

QSize KConfigBase::readSizeEntry( const QString& pKey,
                                  const QSize* pDefault ) const
{
  return d->group.readEntry(pKey, (pDefault? *pDefault: QSize()));
}

QSize KConfigBase::readSizeEntry( const char *pKey,
                                  const QSize* pDefault ) const
{
  return d->group.readEntry(pKey, (pDefault? *pDefault: QSize()));
}

QDateTime KConfigBase::readDateTimeEntry( const QString& pKey,
                                          const QDateTime* pDefault ) const
{
  return d->group.readEntry(pKey, (pDefault? *pDefault: QDateTime::currentDateTime()));
}

QDateTime KConfigBase::readDateTimeEntry( const char *pKey,
                                          const QDateTime* pDefault ) const
{
  return d->group.readEntry(pKey, (pDefault? *pDefault: QDateTime::currentDateTime()));
}

void KConfigBase::writePathEntry( const QString& pKey, const QString & path,
                                  WriteConfigFlags pFlags )
{
   d->group.writePathEntry(pKey.toUtf8().constData(), path, pFlags);
}

#ifdef KDE3_SUPPORT
void KConfigBase::writeEntry( const QString& pKey, const Q3StrList &value,
                              char sep,  bool bPersistent,
                              bool bGlobal, bool bNLS )
{
    writeEntry(pKey.toUtf8().constData(), value, sep, bPersistent, bGlobal, bNLS);
}
#endif

// defined in kconfiggroup.cpp
extern QString translatePath( QString path ); // krazy:exclude=passbyvalue

void KConfigBase::writePathEntry( const char *pKey, const QString & path,
                                  WriteConfigFlags pFlags)
{
   d->group.writeEntry(pKey, translatePath(path), pFlags);
}

void KConfigBase::writePathEntry( const QString& pKey, const QStringList &list,
                                  char sep , WriteConfigFlags pFlags )
{
  d->group.writePathEntry(pKey.toUtf8().constData(), list, sep, pFlags);
}

void KConfigBase::writePathEntry ( const char *pKey, const QStringList &list,
                                   char sep , WriteConfigFlags pFlags )
{
  if( list.isEmpty() )
    {
      d->group.writeEntry( pKey, QString::fromLatin1(""), pFlags );
      return;
    }
  QStringList new_list;
  QStringList::ConstIterator it = list.begin();
  for( ; it != list.end(); ++it )
    {
      QString value = *it;
      new_list.append( translatePath(value) );
    }
  d->group.writeEntry( pKey, new_list, sep, pFlags );
}

void KConfigBase::deleteEntry( const QString& pKey, WriteConfigFlags pFlags)
{
   d->group.deleteEntry(pKey.toUtf8().constData(), pFlags);
}

void KConfigBase::deleteEntry( const char *pKey, WriteConfigFlags pFlags)
{
  d->group.deleteEntry(pKey, pFlags);
}

void KConfigBase::deleteGroup( const QString& _group, WriteConfigFlags pFlags )
{
  KEntryMap aEntryMap = internalEntryMap(_group);

  bool dirty = false;
  bool checkGroup = true;
  // we want to remove all entries in the group
  KEntryMapIterator aIt;
  for (aIt = aEntryMap.begin(); aIt != aEntryMap.end(); ++aIt)
  {
    if (!aIt.key().mKey.isEmpty() && !aIt.key().bDefault && !(*aIt).bDeleted)
    {
      (*aIt).bDeleted = true;
      (*aIt).bDirty = true;
      (*aIt).bGlobal = pFlags & Global;
      (*aIt).mValue = 0;
      putData(aIt.key(), *aIt, checkGroup);
      checkGroup = false;
      dirty = true;
    }
  }
  if (dirty)
     setDirty(true);
}

void KConfigBase::setDollarExpansion( bool _bExpand )
{
    d->bExpand = _bExpand;
}

bool KConfigBase::isDollarExpansion() const
{
    return d->bExpand;
}

bool KConfigBase::localeInitialized() const
{
    return d->bLocaleInitialized;
}

#ifdef KDE3_SUPPORT
void KConfigBase::writeEntry( const QString& pKey, const Q3StrList &value,
                 char sep, bool bPersistent,
                 bool bGlobal, bool bNLS )
{
    writeEntry(pKey.toUtf8().constData(), value, sep, bPersistent, bGlobal, bNLS);
}

void KConfigBase::writeEntry ( const char *pKey, const Q3StrList &list,
                               char sep , bool bPersistent,
                               bool bGlobal, bool bNLS )
{
  WriteConfigFlags flags(bPersistent? Persistent: 0);
  if( list.isEmpty() )
    {
      writeEntry( pKey, QString::fromLatin1(""), flags );
      return;
    }
  QString str_list;
  Q3StrListIterator it( list );
  for( ; it.current(); ++it )
    {
      int i;
      QString value;
      // !!! Sergey A. Sukiyazov <corwin@micom.don.ru> !!!
      // A QStrList may contain values in 8bit locale specified
      // encoding or in UTF8 encoding.
      value = KStringHandler::from8Bit(it.current());
	  int strLengh(value.length());
      for( i = 0; i < strLengh; i++ )
        {
          if( value[i] == sep || value[i] == '\\' )
            str_list += '\\';
          str_list += value[i];
        }
      str_list += sep;
    }
  if( str_list.at(str_list.length() - 1) == sep )
    str_list.truncate( str_list.length() -1 );

  if (bGlobal) flags |= Global;
  if (bNLS) flags |= NLS;
  writeEntry( pKey, str_list, flags );
}
#endif

void KConfigBase::writeEntry( const QString& pKey, const QStringList &value,
		 char sep, WriteConfigFlags pFlags )
{
    writeEntry( pKey.toUtf8().constData(), value, sep, pFlags );
}

void KConfigBase::writeEntry ( const char *pKey, const QStringList &list,
                               char sep , WriteConfigFlags pFlags )
{
    d->group.writeEntry(pKey, list, sep, pFlags);
}

void KConfigBase::parseConfigFiles()
{
    if (!d->bLocaleInitialized && KGlobal::hasLocale()) {
        setLocale();
    }
    if (d->backEnd)
    {
        d->backEnd->parseConfigFiles();
    }
}

void KConfigBase::sync()
{
    if (d->backEnd) {
        d->backEnd->sync();
    }

    if (d->bDirty) {
        rollback();
    }
}

bool KConfigBase::isDirty() const
{
    return d->bDirty;
}

void KConfigBase::setDirty(bool _bDirty)
{
    d->bDirty = _bDirty;
}

void KConfigBase::setBackEnd( KConfigBackEnd* backEnd )
{
    d->backEnd = backEnd;
}

KConfigBackEnd* KConfigBase::backEnd() const
{
    return d->backEnd;
}


KConfigBase::ConfigState KConfigBase::getConfigState() const {
    if ( d->backEnd ) {
       return d->backEnd->getConfigState();
    }
    return ReadOnly;
}

void KConfigBase::rollback( bool /*bDeep = true*/ )
{
    d->bDirty = false;
}


void KConfigBase::setReadDefaults(bool b)
{
    d->readDefaults = b;
}

bool KConfigBase::readDefaults() const
{
    return d->readDefaults;
}

void KConfigBase::revertToDefault(const QString &key)
{
    d->group.revertToDefault(key);
}

bool KConfigBase::hasDefault(const QString &key) const
{
    return d->group.hasDefault(key);
}

void KConfigBase::virtual_hook( int, void* )
{ /*BASE::virtual_hook( id, data );*/ }

bool KConfigBase::checkConfigFilesWritable(bool warnUser)
{
    if (d->backEnd) {
        return d->backEnd->checkConfigFilesWritable(warnUser);
    } else {
        return false;
    }
}

QVariant KConfigBase::readPropertyEntry( const QString& pKey, const QVariant& aDefault) const
{ return d->group.readEntry(pKey, aDefault); }
QVariant KConfigBase::readPropertyEntry( const char *pKey, const QVariant& aDefault) const
{ return d->group.readEntry(pKey, aDefault); }
QStringList KConfigBase::readListEntry( const char *pKey, char sep ) const
{ return d->group.readEntry(pKey, QStringList(), sep); }
QStringList KConfigBase::readListEntry( const char* pKey,
                                        const QStringList& aDefault,
                                        char sep) const
{ return d->group.readEntry(pKey, aDefault, sep); }
QStringList KConfigBase::readEntry(const QString& pKey, const QStringList& aDefault,
                                   char sep) const
{ return d->group.readEntry(pKey.toUtf8().constData(), aDefault, sep); }
QList<int> KConfigBase::readIntListEntry( const QString& pKey ) const
{ return d->group.readEntry( pKey, QList<int>() ); }
QStringList KConfigBase::readListEntry( const QString& pKey, char sep ) const
{ return d->group.readEntry(pKey, QStringList(), sep); }
QList<int> KConfigBase::readIntListEntry( const char *pKey ) const
{ return d->group.readEntry( pKey, QList<int>() ); }
int KConfigBase::readNumEntry( const QString& pKey, int nDefault ) const
{ return d->group.readEntry( pKey, nDefault ); }
int KConfigBase::readNumEntry( const char *pKey, int nDefault ) const
{ return d->group.readEntry( pKey, nDefault ); }
unsigned int KConfigBase::readUnsignedNumEntry( const QString& pKey, unsigned int nDefault ) const
{ return d->group.readEntry( pKey, nDefault ); }
unsigned int KConfigBase::readUnsignedNumEntry( const char *pKey, unsigned int nDefault  ) const
{ return d->group.readEntry( pKey, nDefault ); }
long KConfigBase::readLongNumEntry( const QString& pKey, long nDefault  ) const
{ return d->group.readEntry(pKey, static_cast<int>(nDefault)); }
long KConfigBase::readLongNumEntry( const char *pKey, long nDefault  ) const
{ return d->group.readEntry(pKey, static_cast<int>(nDefault)); }
unsigned long KConfigBase::readUnsignedLongNumEntry( const QString& pKey, unsigned long nDefault  ) const
{ return d->group.readEntry(pKey, static_cast<unsigned int>(nDefault)); }
unsigned long KConfigBase::readUnsignedLongNumEntry( const char *pKey, unsigned long nDefault  ) const
{ return d->group.readEntry(pKey, static_cast<unsigned int>(nDefault)); }
qint64 KConfigBase::readNum64Entry( const QString& pKey, qint64 nDefault  ) const
{ return d->group.readEntry( pKey, nDefault ); }
qint64 KConfigBase::readNum64Entry( const char *pKey, qint64 nDefault  ) const
{ return d->group.readEntry( pKey, nDefault ); }
quint64 KConfigBase::readUnsignedNum64Entry( const QString& pKey, quint64 nDefault  ) const
{ return d->group.readEntry( pKey, nDefault ); }
quint64 KConfigBase::readUnsignedNum64Entry( const char *pKey, quint64 nDefault  ) const
{ return d->group.readEntry( pKey, nDefault ); }
double KConfigBase::readDoubleNumEntry( const QString& pKey, double nDefault ) const
{ return d->group.readEntry( pKey, nDefault ); }
double KConfigBase::readDoubleNumEntry( const char *pKey, double nDefault  ) const
{ return d->group.readEntry( pKey, nDefault ); }
bool KConfigBase::readBoolEntry( const QString& pKey, bool bDefault ) const
{ return d->group.readEntry( pKey, bDefault ); }
bool KConfigBase::readBoolEntry( const char *pKey, bool bDefault ) const
{ return d->group.readEntry( pKey, bDefault ); }
