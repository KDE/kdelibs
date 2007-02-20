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

#include <stdio.h>
#include <stdlib.h>

#include "kconfigbase.h"
#include "kconfigbackend.h"
#include "kconfigdata.h"
#include "kdebug.h"
#include "kglobal.h"
#include "klocale.h"
#include "kstandarddirs.h"
#include "kstringhandler.h"
#include "kcomponentdata.h"

#include <qcolor.h>
#include <qdatetime.h>
#include <qdir.h>
#include <qfile.h>
#include <qfont.h>
#include <qpoint.h>
#include <qrect.h>
#include <qstringlist.h>
#include <qtextstream.h>

class KConfigBase::Private
{
public:
     Private() : readDefaults(false) { };

     bool readDefaults;
};

KConfigBase::KConfigBase()
  : backEnd(0L), bDirty(false), bLocaleInitialized(false),
    bExpand(false),
    mComponentData(new KComponentData(KGlobal::mainComponent())),
    mGroup(this, QString()), d(0)
{
}

KConfigBase::KConfigBase(const KComponentData &componentData)
    : backEnd(0L),
    bDirty(false),
    bLocaleInitialized(false),
    bExpand(false),
    mComponentData(new KComponentData(componentData)),
    mGroup(this, QString()), d(0)
{
}

KConfigBase::~KConfigBase()
{
    delete mComponentData;
    mComponentData = 0;
    delete d;
}

const KComponentData &KConfigBase::componentData() const
{
    return *mComponentData;
}

void KConfigBase::setLocale()
{
  bLocaleInitialized = true;

  if (KGlobal::locale())
    aLocaleString = KGlobal::locale()->language().toUtf8();
  else
    aLocaleString = KLocale::defaultLanguage().toUtf8();
  if (backEnd)
     backEnd->setLocaleString(aLocaleString);
}

QString KConfigBase::locale() const
{
  return QString::fromUtf8(aLocaleString);
}

void KConfigBase::setGroup( const QString& _group )
{
  setGroup( _group.toUtf8() );
}

void KConfigBase::setGroup( const char *pGroup )
{
   setGroup(QByteArray(pGroup));
}

void KConfigBase::setGroup( const QByteArray &_group )
{
  if ( _group.isEmpty() )
    mGroup = KConfigGroup(this, "<default>");
  else
    mGroup = KConfigGroup(this, _group);
}

QString KConfigBase::group() const {
  return mGroup.group();
}

bool KConfigBase::hasKey(const QString &key) const
{
   return hasKey(key.toUtf8().constData());
}

bool KConfigBase::hasKey(const char *pKey) const
{
  return mGroup.hasKey(pKey);
}

bool KConfigBase::hasGroup(const QString &_group) const
{
  return internalHasGroup( _group.toUtf8());
}

bool KConfigBase::hasGroup(const char *_pGroup) const
{
  return internalHasGroup( QByteArray(_pGroup));
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
  return mGroup.entryIsImmutable(key);
}


QString KConfigBase::readEntryUntranslated( const QString& pKey,
                                const QString& aDefault ) const
{
   return mGroup.readEntryUntranslated(pKey.toUtf8().constData(), aDefault);
}


QString KConfigBase::readEntryUntranslated( const char *pKey,
                                const QString& aDefault ) const
{
  return mGroup.readEntryUntranslated(pKey, aDefault);
}


QString KConfigBase::readEntry( const char *pKey, const char *aDefault ) const
{
  return mGroup.readEntry(pKey, aDefault);
}

#ifdef KDE3_SUPPORT
#include <q3strlist.h>
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
  return mGroup.readEntry(pKey, aDefault, sep);
}

QString KConfigBase::readPathEntry( const QString& pKey, const QString& pDefault ) const
{
  return mGroup.readPathEntry(pKey.toUtf8().constData(), pDefault);
}

QString KConfigBase::readPathEntry( const char *pKey, const QString& pDefault ) const
{
  return mGroup.readPathEntry(pKey, pDefault);
}

QStringList KConfigBase::readPathListEntry( const QString& pKey, char sep ) const
{
  return mGroup.readPathListEntry(pKey.toUtf8().constData(), sep);
}

QStringList KConfigBase::readPathListEntry( const char *pKey, char sep ) const
{
  return mGroup.readPathListEntry(pKey, sep);
}


QFont KConfigBase::readFontEntry( const QString& pKey, const QFont* pDefault ) const
{
  return mGroup.readEntry(pKey, (pDefault? *pDefault: QFont()));
}

QFont KConfigBase::readFontEntry( const char *pKey, const QFont* pDefault ) const
{
  return mGroup.readEntry(pKey, (pDefault? *pDefault: QFont()));
}

QRect KConfigBase::readRectEntry( const QString& pKey, const QRect* pDefault ) const
{
  return mGroup.readEntry(pKey, (pDefault? *pDefault: QRect()));
}

QRect KConfigBase::readRectEntry( const char *pKey, const QRect* pDefault ) const
{
  return mGroup.readEntry(pKey, (pDefault? *pDefault: QRect()));
}

QPoint KConfigBase::readPointEntry( const QString& pKey,
                                    const QPoint* pDefault ) const
{
  return mGroup.readEntry(pKey, (pDefault? *pDefault: QPoint()));
}

QPoint KConfigBase::readPointEntry( const char *pKey,
                                    const QPoint* pDefault ) const
{
  return mGroup.readEntry(pKey, (pDefault? *pDefault: QPoint()));
}

QSize KConfigBase::readSizeEntry( const QString& pKey,
                                  const QSize* pDefault ) const
{
  return mGroup.readEntry(pKey, (pDefault? *pDefault: QSize()));
}

QSize KConfigBase::readSizeEntry( const char *pKey,
                                  const QSize* pDefault ) const
{
  return mGroup.readEntry(pKey, (pDefault? *pDefault: QSize()));
}

QDateTime KConfigBase::readDateTimeEntry( const QString& pKey,
                                          const QDateTime* pDefault ) const
{
  return mGroup.readEntry(pKey, (pDefault? *pDefault: QDateTime::currentDateTime()));
}

QDateTime KConfigBase::readDateTimeEntry( const char *pKey,
                                          const QDateTime* pDefault ) const
{
  return mGroup.readEntry(pKey, (pDefault? *pDefault: QDateTime::currentDateTime()));
}

void KConfigBase::writePathEntry( const QString& pKey, const QString & path,
                                  WriteConfigFlags pFlags )
{
   mGroup.writePathEntry(pKey.toUtf8().constData(), path, pFlags);
}


static bool cleanHomeDirPath( QString &path, const QString &homeDir )
{
#ifdef Q_WS_WIN //safer
   if (!QDir::convertSeparators(path).startsWith(QDir::convertSeparators(homeDir)))
        return false;
#else
   if (!path.startsWith(homeDir))
        return false;
#endif

   int len = homeDir.length();
   // replace by "$HOME" if possible
   if (len && (path.length() == len || path[len] == '/')) {
        path.replace(0, len, QString::fromLatin1("$HOME"));
        return true;
   } else
        return false;
}

static QString translatePath( QString path )
{
   if (path.isEmpty())
       return path;

   // only "our" $HOME should be interpreted
   path.replace('$', "$$");

   bool startsWithFile = path.startsWith(QLatin1String("file:"), Qt::CaseInsensitive);

   // return original path, if it refers to another type of URL (e.g. http:/), or
   // if the path is already relative to another directory
   if ((!startsWithFile && path[0] != '/') ||
        (startsWithFile && path[5] != '/'))
	return path;

   if (startsWithFile)
        path.remove(0,5); // strip leading "file:/" off the string

   // keep only one single '/' at the beginning - needed for cleanHomeDirPath()
   while (path[0] == '/' && path[1] == '/')
	path.remove(0,1);

   // we can not use KGlobal::dirs()->relativeLocation("home", path) here,
   // since it would not recognize paths without a trailing '/'.
   // All of the 3 following functions to return the user's home directory
   // can return different paths. We have to test all them.
   const QString homeDir0 = QFile::decodeName(getenv("HOME"));
   const QString homeDir1 = QDir::homePath();
   const QString homeDir2 = QDir(homeDir1).canonicalPath();
   if (cleanHomeDirPath(path, homeDir0) ||
       cleanHomeDirPath(path, homeDir1) ||
       cleanHomeDirPath(path, homeDir2) ) {
     // kDebug() << "Path was replaced\n";
   }

   if (startsWithFile)
      path.prepend( "file://" );

   return path;
}

void KConfigBase::writePathEntry( const char *pKey, const QString & path,
                                  WriteConfigFlags pFlags)
{
   mGroup.writeEntry(pKey, translatePath(path), pFlags);
}

void KConfigBase::writePathEntry( const QString& pKey, const QStringList &list,
                                  char sep , WriteConfigFlags pFlags )
{
  mGroup.writePathEntry(pKey.toUtf8().constData(), list, sep, pFlags);
}

void KConfigBase::writePathEntry ( const char *pKey, const QStringList &list,
                                   char sep , WriteConfigFlags pFlags )
{
  if( list.isEmpty() )
    {
      mGroup.writeEntry( pKey, QString::fromLatin1(""), pFlags );
      return;
    }
  QStringList new_list;
  QStringList::ConstIterator it = list.begin();
  for( ; it != list.end(); ++it )
    {
      QString value = *it;
      new_list.append( translatePath(value) );
    }
  mGroup.writeEntry( pKey, new_list, sep, pFlags );
}

void KConfigBase::deleteEntry( const QString& pKey, WriteConfigFlags pFlags)
{
   mGroup.deleteEntry(pKey.toUtf8().constData(), pFlags);
}

void KConfigBase::deleteEntry( const char *pKey, WriteConfigFlags pFlags)
{
  mGroup.deleteEntry(pKey, pFlags);
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

#ifdef KDE3_SUPPORT
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

void KConfigBase::writeEntry ( const char *pKey, const QStringList &list,
                               char sep , WriteConfigFlags pFlags )
{
  mGroup.writeEntry(pKey, list, sep, pFlags);
}

void KConfigBase::parseConfigFiles()
{
  if (!bLocaleInitialized && KGlobal::hasLocale()) {
    setLocale();
  }
  if (backEnd)
  {
     backEnd->parseConfigFiles();
  }
}

void KConfigBase::sync()
{
    if (backEnd) {
        backEnd->sync();
    }

    if (bDirty) {
        rollback();
    }
}

KConfigBase::ConfigState KConfigBase::getConfigState() const {
    if (backEnd) {
       return backEnd->getConfigState();
    }
    return ReadOnly;
}

void KConfigBase::rollback( bool /*bDeep = true*/ )
{
  bDirty = false;
}


void KConfigBase::setReadDefaults(bool b)
{
  if (!d)
  {
     if (!b) return;
     d = new Private;
  }

  d->readDefaults = b;
}

bool KConfigBase::readDefaults() const
{
  return (d && d->readDefaults);
}

void KConfigBase::revertToDefault(const QString &key)
{
  mGroup.revertToDefault(key);
}

bool KConfigBase::hasDefault(const QString &key) const
{
  return mGroup.hasDefault(key);
}

void KConfigBase::virtual_hook( int, void* )
{ /*BASE::virtual_hook( id, data );*/ }

bool KConfigBase::checkConfigFilesWritable(bool warnUser)
{
  if (backEnd)
    return backEnd->checkConfigFilesWritable(warnUser);
  else
    return false;
}

QColor KConfigBase::readEntry(const char* pKey, Qt::GlobalColor aDefault) const
{ return mGroup.readEntry(pKey, QColor(aDefault)); }
QVariant KConfigBase::readPropertyEntry( const QString& pKey, const QVariant& aDefault) const
{ return mGroup.readEntry(pKey, aDefault); }
QVariant KConfigBase::readPropertyEntry( const char *pKey, const QVariant& aDefault) const
{ return mGroup.readEntry(pKey, aDefault); }
QStringList KConfigBase::readListEntry( const char *pKey, char sep ) const
{ return mGroup.readEntry(pKey, QStringList(), sep); }
QStringList KConfigBase::readListEntry( const char* pKey,
                                        const QStringList& aDefault,
                                        char sep) const
{ return mGroup.readEntry(pKey, aDefault, sep); }
QStringList KConfigBase::readEntry(const QString& pKey, const QStringList& aDefault,
                                   char sep) const
{ return mGroup.readEntry(pKey.toUtf8().constData(), aDefault, sep); }
QList<int> KConfigBase::readIntListEntry( const QString& pKey ) const
{ return mGroup.readEntry( pKey, QList<int>() ); }
QStringList KConfigBase::readListEntry( const QString& pKey, char sep ) const
{ return mGroup.readEntry(pKey, QStringList(), sep); }
QList<int> KConfigBase::readIntListEntry( const char *pKey ) const
{ return mGroup.readEntry( pKey, QList<int>() ); }
int KConfigBase::readNumEntry( const QString& pKey, int nDefault ) const
{ return mGroup.readEntry( pKey, nDefault ); }
int KConfigBase::readNumEntry( const char *pKey, int nDefault ) const
{ return mGroup.readEntry( pKey, nDefault ); }
unsigned int KConfigBase::readUnsignedNumEntry( const QString& pKey, unsigned int nDefault ) const
{ return mGroup.readEntry( pKey, nDefault ); }
unsigned int KConfigBase::readUnsignedNumEntry( const char *pKey, unsigned int nDefault  ) const
{ return mGroup.readEntry( pKey, nDefault ); }
long KConfigBase::readLongNumEntry( const QString& pKey, long nDefault  ) const
{ return mGroup.readEntry(pKey, static_cast<int>(nDefault)); }
long KConfigBase::readLongNumEntry( const char *pKey, long nDefault  ) const
{ return mGroup.readEntry(pKey, static_cast<int>(nDefault)); }
unsigned long KConfigBase::readUnsignedLongNumEntry( const QString& pKey, unsigned long nDefault  ) const
{ return mGroup.readEntry(pKey, static_cast<unsigned int>(nDefault)); }
unsigned long KConfigBase::readUnsignedLongNumEntry( const char *pKey, unsigned long nDefault  ) const
{ return mGroup.readEntry(pKey, static_cast<unsigned int>(nDefault)); }
qint64 KConfigBase::readNum64Entry( const QString& pKey, qint64 nDefault  ) const
{ return mGroup.readEntry( pKey, nDefault ); }
qint64 KConfigBase::readNum64Entry( const char *pKey, qint64 nDefault  ) const
{ return mGroup.readEntry( pKey, nDefault ); }
quint64 KConfigBase::readUnsignedNum64Entry( const QString& pKey, quint64 nDefault  ) const
{ return mGroup.readEntry( pKey, nDefault ); }
quint64 KConfigBase::readUnsignedNum64Entry( const char *pKey, quint64 nDefault  ) const
{ return mGroup.readEntry( pKey, nDefault ); }
double KConfigBase::readDoubleNumEntry( const QString& pKey, double nDefault ) const
{ return mGroup.readEntry( pKey, nDefault ); }
double KConfigBase::readDoubleNumEntry( const char *pKey, double nDefault  ) const
{ return mGroup.readEntry( pKey, nDefault ); }
bool KConfigBase::readBoolEntry( const QString& pKey, bool bDefault ) const
{ return mGroup.readEntry( pKey, bDefault ); }
bool KConfigBase::readBoolEntry( const char *pKey, bool bDefault ) const
{ return mGroup.readEntry( pKey, bDefault ); }
QColor KConfigBase::readColorEntry( const QString& pKey, const QColor* pDefault ) const
{ return mGroup.readEntry(pKey, (pDefault? *pDefault: QColor())); }
QColor KConfigBase::readColorEntry( const char *pKey, const QColor* pDefault  ) const
{ return mGroup.readEntry(pKey, (pDefault? *pDefault: QColor())); }
