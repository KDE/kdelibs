// -*- c-basic-offset: 2 -*-
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
#include "kstringhandler.h"

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
    bReadOnly(false), bExpand(false), d(0)
{
    setGroup(QString());
}

KConfigBase::~KConfigBase()
{
    delete d;
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
  if ( _group.isEmpty() )
    mGroup = "<default>";
  else
    mGroup = _group.toUtf8();
}

void KConfigBase::setGroup( const char *pGroup )
{
  setGroup(QByteArray(pGroup));
}

void KConfigBase::setGroup( const QByteArray &_group )
{
  if ( _group.isEmpty() )
    mGroup = "<default>";
  else
    mGroup = _group;
}

QString KConfigBase::group() const {
  return QString::fromUtf8(mGroup);
}

void KConfigBase::setDesktopGroup()
{
  mGroup = "Desktop Entry";
}

bool KConfigBase::hasKey(const QString &key) const
{
   return hasKey(key.toUtf8().constData());
}

bool KConfigBase::hasKey(const char *pKey) const
{
  KEntryKey aEntryKey(mGroup, 0);
  aEntryKey.c_key = pKey;
  aEntryKey.bDefault = readDefaults();

  if (!locale().isNull()) {
    // try the localized key first
    aEntryKey.bLocal = true;
    KEntry entry = lookupData(aEntryKey);
    if (!entry.mValue.isNull())
       return true;
    aEntryKey.bLocal = false;
  }

  // try the non-localized version
  KEntry entry = lookupData(aEntryKey);
  return !entry.mValue.isNull();
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
  if (getConfigState() != ReadWrite)
     return true;

  KEntryKey entryKey(mGroup, 0);
  KEntry aEntryData = lookupData(entryKey); // Group
  if (aEntryData.bImmutable)
    return true;

  QByteArray utf8_key = key.toUtf8();
  entryKey.c_key = utf8_key.data();
  aEntryData = lookupData(entryKey); // Normal entry
  if (aEntryData.bImmutable)
    return true;

  entryKey.bLocal = true;
  aEntryData = lookupData(entryKey); // Localized entry
  return aEntryData.bImmutable;
}


QString KConfigBase::readEntryUntranslated( const QString& pKey,
                                const QString& aDefault ) const
{
   return readEntryUntranslated(pKey.toUtf8().constData(), aDefault);
}


QString KConfigBase::readEntryUntranslated( const char *pKey,
                                const QString& aDefault ) const
{
   QByteArray result = readEntryUtf8(pKey);
   if (result.isNull())
      return aDefault;
   return QString::fromUtf8(result);
}


QString KConfigBase::readEntry( const char *pKey, const char *aDefault ) const
{
   return readEntry(pKey, QString::fromLatin1(aDefault));
}

QString KConfigBase::readEntry( const char *pKey,
                                const QString& aDefault ) const
{
  // we need to access _locale instead of the method locale()
  // because calling locale() will create a locale object if it
  // doesn't exist, which requires KConfig, which will create a infinite
  // loop, and nobody likes those.
  if (!bLocaleInitialized && KGlobal::_locale) {
    // get around const'ness.
    KConfigBase *that = const_cast<KConfigBase *>(this);
    that->setLocale();
  }

  QString aValue;

  bool expand = false;
  // construct a localized version of the key
  // try the localized key first
  KEntry aEntryData;
  KEntryKey entryKey(mGroup, 0);
  entryKey.c_key = pKey;
  entryKey.bDefault = readDefaults();
  entryKey.bLocal = true;
  aEntryData = lookupData(entryKey);
  if (!aEntryData.mValue.isNull()) {
    // for GNOME .desktop
    aValue = KStringHandler::from8Bit( aEntryData.mValue.data() );
    expand = aEntryData.bExpand;
  } else {
    entryKey.bLocal = false;
    aEntryData = lookupData(entryKey);
    if (!aEntryData.mValue.isNull()) {
      aValue = QString::fromUtf8(aEntryData.mValue.data());
      if (aValue.isNull())
      {
        static const QString &emptyString = KGlobal::staticQString("");
        aValue = emptyString;
      }
      expand = aEntryData.bExpand;
    } else {
      aValue = aDefault;
    }
  }

  // only do dollar expansion if so desired
  if( expand || bExpand )
    {
      // check for environment variables and make necessary translations
      int nDollarPos = aValue.indexOf( '$' );

      while( nDollarPos != -1 && nDollarPos+1 < static_cast<int>(aValue.length())) {
        // there is at least one $
        if( (aValue)[nDollarPos+1] == '(' ) {
          int nEndPos = nDollarPos+1;
          // the next character is no $
          while ( (nEndPos <= aValue.length()) && (aValue[nEndPos]!=')') )
              nEndPos++;
          nEndPos++;
          QString cmd = aValue.mid( nDollarPos+2, nEndPos-nDollarPos-3 );

          QString result;
          FILE *fs = popen(QFile::encodeName(cmd).data(), "r");
          if (fs)
          {
             {
             QTextStream ts(fs, QIODevice::ReadOnly);
             result = ts.readAll().trimmed();
             }
             pclose(fs);
          }
          aValue.replace( nDollarPos, nEndPos-nDollarPos, result );
        } else if( (aValue)[nDollarPos+1] != '$' ) {
          int nEndPos = nDollarPos+1;
          // the next character is no $
          QString aVarName;
          if (aValue[nEndPos]=='{')
          {
            while ( (nEndPos <= aValue.length()) && (aValue[nEndPos]!='}') )
                nEndPos++;
            nEndPos++;
            aVarName = aValue.mid( nDollarPos+2, nEndPos-nDollarPos-3 );
          }
          else
          {
            while ( nEndPos <= aValue.length() && (aValue[nEndPos].isNumber()
                    || aValue[nEndPos].isLetter() || aValue[nEndPos]=='_' )  )
                nEndPos++;
            aVarName = aValue.mid( nDollarPos+1, nEndPos-nDollarPos-1 );
          }
          const char* pEnv = 0;
          if (!aVarName.isEmpty())
               pEnv = getenv( aVarName.toAscii() );
          if( pEnv ) {
	    // !!! Sergey A. Sukiyazov <corwin@micom.don.ru> !!!
	    // An environment variable may contain values in 8bit
	    // locale specified encoding or UTF8 encoding
	    aValue.replace( nDollarPos, nEndPos-nDollarPos, KStringHandler::from8Bit( pEnv ) );
          } else
            aValue.remove( nDollarPos, nEndPos-nDollarPos );
        } else {
          // remove one of the dollar signs
          aValue.remove( nDollarPos, 1 );
          nDollarPos++;
        }
        nDollarPos = aValue.indexOf( '$', nDollarPos );
      }
    }

  return aValue;
}

QByteArray KConfigBase::readEntryUtf8( const char *pKey) const
{
  // We don't try the localized key
  KEntryKey entryKey(mGroup, 0);
  entryKey.bDefault = readDefaults();
  entryKey.c_key = pKey;
  KEntry aEntryData = lookupData(entryKey);
  if (aEntryData.bExpand)
  {
     // We need to do fancy, take the slow route.
     return readEntry(pKey, QString()).toUtf8();
  }
  return aEntryData.mValue;
}

QVariant KConfigBase::readEntry( const char *pKey, const QVariant &aDefault ) const
{
  if ( !hasKey( pKey ) ) return aDefault;

  const QString errString = QString::fromLatin1("\"%1\" - conversion from \"%3\" to %2 failed")
    .arg(pKey).arg(QVariant::typeToName(aDefault.type()));
  const QString formatError = QString::fromLatin1(" (wrong format: expected '%1' items, read '%2')");

  QVariant tmp = aDefault;

  // if a type handler is added here you must add a QVConversions definition
  // to conversion_check.h, or ConversionCheck::to_QVariant will not allow
  // readEntry<T> to convert to QVariant.
  switch( aDefault.type() )
  {
      case QVariant::Invalid:
          return QVariant();
      case QVariant::String:
          return readEntry( pKey, aDefault.toString() );
      case QVariant::StringList:
          return readEntry( pKey, aDefault.toStringList() );
      case QVariant::List:
          return readEntry( pKey, aDefault.toList() );
      case QVariant::ByteArray:
            return readEntryUtf8(pKey);
      case QVariant::Font:
//      case QVariant::KeySequence:
      case QVariant::Bool:
      case QVariant::Double:
      case QVariant::Int:
      case QVariant::UInt:
            tmp = QString::fromUtf8(readEntryUtf8(pKey));
            if ( !tmp.convert(aDefault.type()) )
                tmp = aDefault;
            return tmp;
      case QVariant::Color: {
          // invalid QColor's are stored as the string "invalid"
          // this needs to be checked first, or we will catch the
          // asserts in readEntry for an integer list
          if (readEntry(pKey) == QLatin1String("invalid"))
            return QColor();
      
          const QList<int> list = readEntry( pKey, QList<int>() );
          const int count = list.count();

          if (count != 3 && count != 4) {
              kcbError() << errString.arg(readEntry(pKey))
                         << formatError.arg("3' or '4").arg(count)
                         << endl;
              return aDefault;
          }

          // bounds check components
          for(int i=0; i < count; i++) {
              const int j = list.at(i);
              if (j < 0 || j > 255) {
                  const char *const components[] = {
                      "red", "green", "blue", "alpha"
                  };
                  const QString boundsError = QLatin1String(" (bounds error: %1 component %2)");
                  kcbError() << errString.arg(readEntry(pKey))
                             << boundsError.arg(components[i]).arg(j < 0? "< 0": "> 255")
                             << endl;
                  return aDefault;
              }
          }
          QColor color(list.at(0), list.at(1), list.at(2));
          if (count == 4)
              color.setAlpha(list.at(3));

          if ( !color.isValid() ) {
              kcbError() << errString.arg(readEntry(pKey)) << endl;
              return aDefault;
          }
          return color;
      }
      case QVariant::Point: {
          const QList<int> list = readEntry( pKey, QList<int>() );

          if ( list.count() != 2 ) {
              kcbError() << errString.arg(readEntry(pKey))
                         << formatError.arg(2).arg(list.count())
                         << endl;
              return aDefault;
          }
          return QPoint(list.at( 0 ), list.at( 1 ));
      }
      case QVariant::Rect: {
          const QList<int> list = readEntry( pKey, QList<int>() );

          if ( list.count() != 4 ) {
              kcbError() << errString.arg(readEntry(pKey))
                         << formatError.arg(4).arg(list.count())
                         << endl;
              return aDefault;
          }
          const QRect rect(list.at( 0 ), list.at( 1 ), list.at( 2 ), list.at( 3 ));
          if ( !rect.isValid() ) {
              kcbError() << errString.arg(readEntry(pKey)) << endl;
              return aDefault;
          }
          return rect;
      }
      case QVariant::Size: {
          const QList<int> list = readEntry( pKey, QList<int>() );

          if ( list.count() != 2 ) {
              kcbError() << errString.arg(readEntry(pKey))
                         << formatError.arg(2).arg(list.count())
                         << endl;
              return aDefault;
          }
          const QSize size(list.at( 0 ), list.at( 1 ));
          if ( !size.isValid() ) {
              kcbError() << errString.arg(readEntry(pKey)) << endl;
              return aDefault;
          }
          return size;
      }
      case QVariant::LongLong: {
          const QByteArray aValue = readEntryUtf8(pKey);

          if ( !aValue.isEmpty() ) {
              bool ok;
              qint64 rc = aValue.toLongLong( &ok );
              if ( ok )
                  tmp = rc;
          }
          return tmp;
      }
      case QVariant::ULongLong: {
          const QByteArray aValue = readEntryUtf8(pKey);

          if( !aValue.isEmpty() ) {
              bool ok;
              quint64 rc = aValue.toULongLong( &ok );
              if ( ok )
                  tmp = rc;
          }
          return tmp;
      }
      case QVariant::DateTime: {
          const QList<int> list = readEntry( pKey, QList<int>() );
          if ( list.count() != 6 ) {
              kcbError() << errString.arg(readEntry(pKey))
                         << formatError.arg(6).arg(list.count())
                         << endl;
              return aDefault;
          }
          const QDate date( list.at( 0 ), list.at( 1 ), list.at( 2 ) );
          const QTime time( list.at( 3 ), list.at( 4 ), list.at( 5 ) );
          const QDateTime dt( date, time );
          if ( !dt.isValid() ) {
              kcbError() << errString.arg(readEntry(pKey)) << endl;
              return aDefault;
          }
          return dt;
      }
      case QVariant::Date: {
          QList<int> list = readEntry( pKey, QList<int>() );
          if ( list.count() == 6 )
              list = list.mid(0, 3); // don't break config files that stored QDate as QDateTime
          if ( list.count() != 3 ) {
              kcbError() << errString.arg(readEntry(pKey))
                         << formatError.arg(3).arg(list.count())
                         << endl;
              return aDefault;
          }
          const QDate date( list.at( 0 ), list.at( 1 ), list.at( 2 ) );
          if ( !date.isValid() ) {
              kcbError() << errString.arg(readEntry(pKey)) << endl;
              return aDefault;
          }
          return date;
      }

      default:
          break;
  }

  Q_ASSERT( 0 );
  return QVariant();
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

QVariantList KConfigBase::readEntry( const char* pKey, const QVariantList& aDefault) const
{
  if (!hasKey(pKey))
    return aDefault;

  const QStringList slist = readEntry( pKey, QVariant(aDefault).toStringList() );

  return QVariant(slist).toList();
}

QStringList KConfigBase::readEntry(const QString& pKey, const QStringList& aDefault, char sep) const
{
  return readEntry(pKey.toUtf8().constData(), aDefault, sep);
}

QStringList KConfigBase::readEntry(const char* pKey, const QStringList& aDefault, char sep) const
{
  if( !hasKey( pKey ) )
    return aDefault;

  const QString str_list = readEntry( pKey );
  QStringList list;
  if( str_list.isEmpty() )
    return list;
  QString value;
  const int len = str_list.length();
  // obviously too big, but faster than letting each += resize the string.
  value.reserve( len );
  for( int i = 0; i < len; i++ )
    {
      if( str_list[i] != sep && str_list[i] != '\\' )
        {
          value += str_list[i];
          continue;
        }
      if( str_list[i] == '\\' )
        {
          i++;
          if ( i < len )
            value += str_list[i];
          continue;
        }
      QString finalvalue( value );
      finalvalue.squeeze();
      list.append( finalvalue );
      value.truncate( 0 );
    }
  if ( str_list[len-1] != sep || ( len > 1 && str_list[len-2] == '\\' ) )
  {
    value.squeeze();
    list.append( value );
  }
  return list;
}

QString KConfigBase::readPathEntry( const QString& pKey, const QString& pDefault ) const
{
  return readPathEntry(pKey.toUtf8().constData(), pDefault);
}

QString KConfigBase::readPathEntry( const char *pKey, const QString& pDefault ) const
{
  const bool bExpandSave = bExpand;
  bExpand = true;
  QString aValue = readEntry( pKey, pDefault );
  bExpand = bExpandSave;
  return aValue;
}

QStringList KConfigBase::readPathListEntry( const QString& pKey, char sep ) const
{
  return readPathListEntry(pKey.toUtf8().constData(), sep);
}

QStringList KConfigBase::readPathListEntry( const char *pKey, char sep ) const
{
  const bool bExpandSave = bExpand;
  bExpand = true;
  const QStringList aValue = readEntry( pKey, QStringList(), sep );
  bExpand = bExpandSave;
  return aValue;
}


QFont KConfigBase::readFontEntry( const QString& pKey, const QFont* pDefault ) const
{
  return readEntry(pKey, (pDefault? *pDefault: QFont()));
}

QFont KConfigBase::readFontEntry( const char *pKey, const QFont* pDefault ) const
{
  return readEntry(pKey, (pDefault? *pDefault: QFont()));
}

QRect KConfigBase::readRectEntry( const QString& pKey, const QRect* pDefault ) const
{
  return readEntry(pKey, (pDefault? *pDefault: QRect()));
}

QRect KConfigBase::readRectEntry( const char *pKey, const QRect* pDefault ) const
{
  return readEntry(pKey, (pDefault? *pDefault: QRect()));
}

QPoint KConfigBase::readPointEntry( const QString& pKey,
                                    const QPoint* pDefault ) const
{
  return readEntry(pKey, (pDefault? *pDefault: QPoint()));
}

QPoint KConfigBase::readPointEntry( const char *pKey,
                                    const QPoint* pDefault ) const
{
  return readEntry(pKey, (pDefault? *pDefault: QPoint()));
}

QSize KConfigBase::readSizeEntry( const QString& pKey,
                                  const QSize* pDefault ) const
{
  return readEntry(pKey, (pDefault? *pDefault: QSize()));
}

QSize KConfigBase::readSizeEntry( const char *pKey,
                                  const QSize* pDefault ) const
{
  return readEntry(pKey, (pDefault? *pDefault: QSize()));
}

QDateTime KConfigBase::readDateTimeEntry( const QString& pKey,
                                          const QDateTime* pDefault ) const
{
  return readEntry(pKey, (pDefault? *pDefault: QDateTime::currentDateTime()));
}

QDateTime KConfigBase::readDateTimeEntry( const char *pKey,
                                          const QDateTime* pDefault ) const
{
  return readEntry(pKey, (pDefault? *pDefault: QDateTime::currentDateTime()));
}

void KConfigBase::writeEntry( const char *pKey, const QString& value,
                                 WriteConfigFlags pFlags )
{
  // the KConfig object is dirty now
  // set this before any IO takes place so that if any derivative
  // classes do caching, they won't try and flush the cache out
  // from under us before we read. A race condition is still
  // possible but minimized.
  if( pFlags & Persistent )
    setDirty(true);

  if (!bLocaleInitialized && KGlobal::locale())
    setLocale();

  KEntryKey entryKey(mGroup, pKey);
  entryKey.bLocal = pFlags & NLS;

  KEntry aEntryData;
  aEntryData.mValue = value.toUtf8();  // set new value
  aEntryData.bGlobal = pFlags & Global;
  aEntryData.bNLS = pFlags & NLS;

  if (pFlags & Persistent)
    aEntryData.bDirty = true;

  // rewrite the new value
  putData(entryKey, aEntryData, true);
}

void KConfigBase::writePathEntry( const QString& pKey, const QString & path,
                                  WriteConfigFlags pFlags )
{
   writePathEntry(pKey.toUtf8().constData(), path, pFlags);
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
   if (!startsWithFile && path[0] != '/' ||
        startsWithFile && path[5] != '/')
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
     // kdDebug() << "Path was replaced\n";
   }

   if (startsWithFile)
      path.prepend( "file://" );

   return path;
}

void KConfigBase::writePathEntry( const char *pKey, const QString & path,
                                  WriteConfigFlags pFlags)
{
   writeEntry(pKey, translatePath(path), pFlags);
}

void KConfigBase::writePathEntry( const QString& pKey, const QStringList &list,
                                  char sep , WriteConfigFlags pFlags )
{
  writePathEntry(pKey.toUtf8().constData(), list, sep, pFlags);
}

void KConfigBase::writePathEntry ( const char *pKey, const QStringList &list,
                                   char sep , WriteConfigFlags pFlags )
{
  if( list.isEmpty() )
    {
      writeEntry( pKey, QString::fromLatin1(""), pFlags );
      return;
    }
  QStringList new_list;
  QStringList::ConstIterator it = list.begin();
  for( ; it != list.end(); ++it )
    {
      QString value = *it;
      new_list.append( translatePath(value) );
    }
  writeEntry( pKey, new_list, sep, pFlags );
}

void KConfigBase::deleteEntry( const QString& pKey, WriteConfigFlags pFlags)
{
   deleteEntry(pKey.toUtf8().constData(), pFlags);
}

void KConfigBase::deleteEntry( const char *pKey, WriteConfigFlags pFlags)
{
  // the KConfig object is dirty now
  // set this before any IO takes place so that if any derivative
  // classes do caching, they won't try and flush the cache out
  // from under us before we read. A race condition is still
  // possible but minimized.
  setDirty(true);

  if (!bLocaleInitialized && KGlobal::locale())
    setLocale();

  KEntryKey entryKey(mGroup, pKey);
  KEntry aEntryData;

  aEntryData.bGlobal = pFlags & Global;
  aEntryData.bNLS = pFlags & NLS;
  aEntryData.bDirty = true;
  aEntryData.bDeleted = true;

  // rewrite the new value
  putData(entryKey, aEntryData, true);
}

bool KConfigBase::deleteGroup( const QString& _group, WriteConfigFlags pFlags )
{
  KEntryMap aEntryMap = internalEntryMap(_group);

  if (pFlags & Recursive) {
    // Check if it empty
    return aEntryMap.isEmpty();
  }

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
  return true;
}

void KConfigBase::writeEntry ( const char *pKey, const QVariant &prop,
                               WriteConfigFlags pFlags )
{
  // if a type handler is added here you must add a QVConversions definition
  // to conversion_check.h, or ConversionCheck::to_QVariant will not allow
  // writeEntry<T> to convert to QVariant.
  switch( prop.type() )
    {
    case QVariant::Invalid:
      writeEntry( pKey, "", pFlags );
      return;
    case QVariant::String:
      writeEntry( pKey, prop.toString(), pFlags );
      return;
    case QVariant::List:
      kcbError(!prop.canConvert(QVariant::StringList))
        << "not all types in \"" << pKey << "\" can convert to QString,"
           " information will be lost" << endl;
    case QVariant::StringList:
      writeEntry( pKey, prop.toStringList(), ',', pFlags );
      return;
    case QVariant::ByteArray: {
      const QByteArray ba = prop.toByteArray();
      writeEntry( pKey, QString::fromUtf8(ba.constData(), ba.length()), pFlags );
      return;
    }
    case QVariant::Point: {
        QList<int> list;
        const QPoint rPoint = prop.toPoint();
        list.insert( 0, rPoint.x() );
        list.insert( 1, rPoint.y() );

        writeEntry( pKey, list, pFlags );
        return;
    }
    case QVariant::Rect:{
        QList<int> list;
        const QRect rRect = prop.toRect();
        list.insert( 0, rRect.left() );
        list.insert( 1, rRect.top() );
        list.insert( 2, rRect.width() );
        list.insert( 3, rRect.height() );

        writeEntry( pKey, list, pFlags );
        return;
    }
    case QVariant::Size:{
        QList<int> list;
        const QSize rSize = prop.toSize();
        list.insert( 0, rSize.width() );
        list.insert( 1, rSize.height() );

        writeEntry( pKey, list, pFlags );
        return;
    }
    case QVariant::Color: {
        QList<int> list;
        const QColor rColor = prop.value<QColor>();

        if (!rColor.isValid()) {
            writeEntry(pKey, "invalid", pFlags);
            return;
        }
        list.insert(0, rColor.red());
        list.insert(1, rColor.green());
        list.insert(2, rColor.blue());
        if (rColor.alpha() != 255)
            list.insert(3, rColor.alpha());

        writeEntry( pKey, list, pFlags );
        return;
    }
    case QVariant::Int:
    case QVariant::UInt:
    case QVariant::Double:
    case QVariant::Bool:
//    case QVariant::KeySequence:
    case QVariant::Font:
        writeEntry( pKey, prop.toString(), pFlags );
        return;
    case QVariant::LongLong:
      writeEntry( pKey, QString::number(prop.toLongLong()), pFlags );
      return;
    case QVariant::ULongLong:
      writeEntry( pKey, QString::number(prop.toULongLong()), pFlags );
      return;
    case QVariant::Date: {
        QList<int> list;
        const QDate date = prop.toDate();

        list.insert( 0, date.year() );
        list.insert( 1, date.month() );
        list.insert( 2, date.day() );

        writeEntry( pKey, list, pFlags );
        return;
    }
    case QVariant::DateTime: {
        QList<int> list;
        const QDateTime rDateTime = prop.toDateTime();

        const QTime time = rDateTime.time();
        const QDate date = rDateTime.date();

        list.insert( 0, date.year() );
        list.insert( 1, date.month() );
        list.insert( 2, date.day() );

        list.insert( 3, time.hour() );
        list.insert( 4, time.minute() );
        list.insert( 5, time.second() );

        writeEntry( pKey, list, pFlags );
        return;
    }

    case QVariant::Pixmap:
    case QVariant::Image:
    case QVariant::Brush:
    case QVariant::Palette:
    case QVariant::Map:
    case QVariant::Icon:
    case QVariant::Region:
    case QVariant::Bitmap:
    case QVariant::Cursor:
    case QVariant::SizePolicy:
    case QVariant::Time:
    case QVariant::BitArray:
    case QVariant::Pen:
    default:
        break;
    }

  Q_ASSERT( 0 );
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

void KConfigBase::writeEntry ( const QString& pKey, const QStringList &list,
                               char sep , WriteConfigFlags pFlags )
{
  writeEntry(pKey.toUtf8().constData(), list, sep, pFlags);
}

void KConfigBase::writeEntry ( const char *pKey, const QStringList &list,
                               char sep , WriteConfigFlags pFlags )
{
  if( list.isEmpty() )
    {
      writeEntry( pKey, QString::fromLatin1(""), pFlags );
      return;
    }
  QString str_list;
  str_list.reserve( 4096 );
  QStringList::ConstIterator it = list.begin();
  const QStringList::ConstIterator end = list.end();
  for( ; it != end; ++it )
    {
      const QString value = *it;
      const int strLength(value.length());
      for( int i = 0; i < strLength; i++ )
        {
          if( value[i] == sep || value[i] == '\\' )
            str_list += '\\';
          str_list += value[i];
        }
      str_list += sep;
    }
  if( str_list.at(str_list.length() - 1) == sep )
    str_list.truncate( str_list.length() -1 );
  writeEntry( pKey, str_list, pFlags );
}

void KConfigBase::parseConfigFiles()
{
  if (!bLocaleInitialized && KGlobal::_locale) {
    setLocale();
  }
  if (backEnd)
  {
     backEnd->parseConfigFiles();
     bReadOnly = (backEnd->getConfigState() == ReadOnly);
  }
}

void KConfigBase::sync()
{
  if (isReadOnly())
    return;

  if (backEnd)
     backEnd->sync();
  if (bDirty)
    rollback();
}

KConfigBase::ConfigState KConfigBase::getConfigState() const {
    if (backEnd)
       return backEnd->getConfigState();
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
  setDirty(true);

  KEntryKey aEntryKey(mGroup, key.toUtf8());
  aEntryKey.bDefault = true;

  if (!locale().isNull()) {
    // try the localized key first
    aEntryKey.bLocal = true;
    KEntry entry = lookupData(aEntryKey);
    if (entry.mValue.isNull())
        entry.bDeleted = true;

    entry.bDirty = true;
    putData(aEntryKey, entry, true); // Revert
    aEntryKey.bLocal = false;
  }

  // try the non-localized version
  KEntry entry = lookupData(aEntryKey);
  if (entry.mValue.isNull())
     entry.bDeleted = true;
  entry.bDirty = true;
  putData(aEntryKey, entry, true); // Revert
}

bool KConfigBase::hasDefault(const QString &key) const
{
  KEntryKey aEntryKey(mGroup, key.toUtf8());
  aEntryKey.bDefault = true;

  if (!locale().isNull()) {
    // try the localized key first
    aEntryKey.bLocal = true;
    KEntry entry = lookupData(aEntryKey);
    if (!entry.mValue.isNull())
        return true;

    aEntryKey.bLocal = false;
  }

  // try the non-localized version
  KEntry entry = lookupData(aEntryKey);
  if (!entry.mValue.isNull())
     return true;

  return false;
}

KConfigGroup::KConfigGroup(KConfigBase *master, const QString &_group)
{
  mMaster = master;
  backEnd = mMaster->backEnd; // Needed for getConfigState()
  bLocaleInitialized = true;
  bReadOnly = mMaster->bReadOnly;
  bExpand = false;
  bDirty = false; // Not used
  mGroup = _group.toUtf8();
  aLocaleString = mMaster->aLocaleString;
  setReadDefaults(mMaster->readDefaults());
}

KConfigGroup::KConfigGroup(KConfigBase *master, const QByteArray &_group)
{
  mMaster = master;
  backEnd = mMaster->backEnd; // Needed for getConfigState()
  bLocaleInitialized = true;
  bReadOnly = mMaster->bReadOnly;
  bExpand = false;
  bDirty = false; // Not used
  mGroup = _group;
  aLocaleString = mMaster->aLocaleString;
  setReadDefaults(mMaster->readDefaults());
}

KConfigGroup::KConfigGroup(KConfigBase *master, const char * _group)
{
  mMaster = master;
  backEnd = mMaster->backEnd; // Needed for getConfigState()
  bLocaleInitialized = true;
  bReadOnly = mMaster->bReadOnly;
  bExpand = false;
  bDirty = false; // Not used
  mGroup = _group;
  aLocaleString = mMaster->aLocaleString;
  setReadDefaults(mMaster->readDefaults());
}

void KConfigGroup::deleteGroup(bool bGlobal)
{
  mMaster->deleteGroup(KConfigBase::group(),
                   KConfigBase::WriteConfigFlags(bGlobal?Global:0)|Recursive);
}

bool KConfigGroup::groupIsImmutable() const
{
    return mMaster->groupIsImmutable(KConfigBase::group());
}

void KConfigGroup::setDirty(bool _bDirty)
{
  mMaster->setDirty(_bDirty);
}

void KConfigGroup::putData(const KEntryKey &_key, const KEntry &_data, bool _checkGroup)
{
  mMaster->putData(_key, _data, _checkGroup);
}

KEntry KConfigGroup::lookupData(const KEntryKey &_key) const
{
  return mMaster->lookupData(_key);
}

void KConfigGroup::sync()
{
  mMaster->sync();
}

QStringList KConfigGroup::groupList() const
{
  return QStringList();
}

KEntryMap KConfigGroup::internalEntryMap( const QString&) const
{
  return KEntryMap();
}

KEntryMap KConfigGroup::internalEntryMap() const
{
  return KEntryMap();
}

void KConfigBase::virtual_hook( int, void* )
{ /*BASE::virtual_hook( id, data );*/ }

void KConfigGroup::virtual_hook( int id, void* data )
{ KConfigBase::virtual_hook( id, data ); }

bool KConfigBase::checkConfigFilesWritable(bool warnUser)
{
  if (backEnd)
    return backEnd->checkConfigFilesWritable(warnUser);
  else
    return false;
}
