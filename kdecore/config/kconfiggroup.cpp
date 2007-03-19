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

#include "kconfiggroup.h"
#include "kconfig.h"
#include "kstringhandler.h"
#include "kcomponentdata.h"
#include "kstandarddirs.h"

#include <qdatetime.h>
#include <qdir.h>
#include <qfile.h>
#include <qpoint.h>
#include <qrect.h>
#include <qstringlist.h>
#include <qtextstream.h>

#include "kconfiggroup_p.h"

class KConfigGroup::Private
{
public:
  KConfigBase *master;
  KSharedConfig::Ptr masterShared;
  QByteArray group;
};

KConfigGroupGui _kde_internal_KConfigGroupGui;
static inline bool readEntryGui(const KConfigGroup *cg, const char *key, const QVariant &input,
                                QVariant &output)
{
  if (_kde_internal_KConfigGroupGui.readEntryGui)
    return _kde_internal_KConfigGroupGui.readEntryGui(cg, key, input, output);
  return false;
}

static inline bool writeEntryGui(KConfigGroup *cg, const char *key, const QVariant &input,
                                 KConfigBase::WriteConfigFlags flags)
{
  if (_kde_internal_KConfigGroupGui.writeEntryGui)
    return _kde_internal_KConfigGroupGui.writeEntryGui(cg, key, input, flags);
  return false;
}

KConfigGroup::KConfigGroup(KConfigBase *master, const QString &_group)
{
    init( master );
    changeGroup( _group.toUtf8( ) );
}

KConfigGroup::KConfigGroup(KConfigBase *master, const QByteArray &_group)
{
    init( master );
    changeGroup( _group );
}

KConfigGroup::KConfigGroup(KConfigBase *master, const char * _group)
{
    init( master );
    changeGroup( _group );
}

KConfigGroup::KConfigGroup(KSharedConfig::Ptr master, const QString &_group)
{
    init( master.data() );
    changeGroup( _group.toUtf8() );
    d->masterShared = master;
}

KConfigGroup::KConfigGroup(KSharedConfig::Ptr master, const QByteArray &_group)
{
    init( master.data() );
    changeGroup( _group );
    d->masterShared = master;
}

KConfigGroup::KConfigGroup(KSharedConfig::Ptr master, const char * _group)
{
    init( master.data() );
    changeGroup( _group );
    d->masterShared = master;
}

void KConfigGroup::init(KConfigBase *master)
{
  d = new Private();
  d->master = master;
  d->masterShared = 0;
}

KConfigGroup &KConfigGroup::operator=(const KConfigGroup &rhs)
{
  *d = *rhs.d;
  return *this;
}

KConfigGroup::KConfigGroup(const KConfigGroup &rhs)
{
  d = new Private();
  *this = rhs;
}

KConfigGroup::~KConfigGroup()
{
  delete d;
}

void KConfigGroup::deleteGroup(KConfigBase::WriteConfigFlags pFlags)
{
  d->master->deleteGroup(d->group, pFlags);
}

void KConfigGroup::changeGroup(char const* group)
{
    if ( !group || !qstrcmp( group, "") ) {
        d->group = "<default>";
    } else {
        d->group = group;
    }
}

QString KConfigGroup::group() const
{
    return d->group;
}

bool KConfigGroup::exists() const
{
    return d->master->hasGroup( d->group );
}

void KConfigGroup::setDirty(bool _bDirty)
{
    d->master->setDirty( _bDirty );
}

void KConfigGroup::putData(const KEntryKey &_key, const KEntry &_data, bool _checkGroup)
{
  d->master->putData(_key, _data, _checkGroup);
}

KEntry KConfigGroup::lookupData(const KEntryKey &_key) const
{
  return d->master->lookupData(_key);
}

void KConfigGroup::sync()
{
  d->master->sync();
}

QMap<QString, QString> KConfigGroup::entryMap() const
{
  return d->master->entryMap(d->group);
}

KConfigBase* KConfigGroup::config() const
{
  return d->master;
}

void KConfigGroup::virtual_hook( int , void*  )
{ }

bool KConfigGroup::entryIsImmutable(const QString &key) const
{
  if ( d->master->getConfigState() != KConfigBase::ReadWrite ) {
     return true;
  }

  KEntryKey entryKey(d->group, 0);
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


QString KConfigGroup::readEntryUntranslated( const QString& pKey,
                                const QString& aDefault ) const
{
   return readEntryUntranslated(pKey.toUtf8().constData(), aDefault);
}


QString KConfigGroup::readEntryUntranslated( const char *pKey,
                                const QString& aDefault ) const
{
   QByteArray result = readEntryUtf8(pKey);
   if (result.isNull())
      return aDefault;
   return QString::fromUtf8(result);
}


QString KConfigGroup::readEntry( const char *pKey, const char *aDefault ) const
{
   return readEntry(pKey, QString::fromLatin1(aDefault));
}

QString KConfigGroup::readEntry( const char *pKey,
                                const QString& aDefault ) const
{
  // we need to access hasLocale() instead of the method locale()
  // because calling locale() will create a locale object if it
  // doesn't exist, which requires KConfig, which will create a infinite
  // loop, and nobody likes those.
  if ( !d->master->localeInitialized() && KGlobal::hasLocale() ) {
    // get around const'ness.
    KConfigBase *that = const_cast<KConfigBase *>(d->master);
    that->setLocale();
  }

  QString aValue;

  bool expand = false;
  // construct a localized version of the key
  // try the localized key first
  KEntry aEntryData;
  KEntryKey entryKey(d->group, 0);
  entryKey.c_key = pKey;
  entryKey.bDefault = d->master->readDefaults();
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
  if( expand || d->master->isDollarExpansion() )
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
          QByteArray oldpath = qgetenv( "PATH" );
          QByteArray newpath = QFile::encodeName( d->master->componentData().dirs()->resourceDirs( "exe" ).join( QChar( KPATH_SEPARATOR ) ) );
          if( !newpath.isEmpty() && !oldpath.isEmpty() )
                  newpath += KPATH_SEPARATOR;
          newpath += oldpath;
          setenv( "PATH", newpath, 1/*overwrite*/ );
          FILE *fs = popen(QFile::encodeName(cmd).data(), "r");
          if (fs)
          {
             {
             QTextStream ts(fs, QIODevice::ReadOnly);
             result = ts.readAll().trimmed();
             }
             pclose(fs);
          }
          setenv( "PATH", oldpath, 1/*overwrite*/ );
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

QByteArray KConfigGroup::readEntryUtf8( const char *pKey) const
{
  // We don't try the localized key
  KEntryKey entryKey(d->group, 0);
  entryKey.bDefault = d->master->readDefaults();
  entryKey.c_key = pKey;
  KEntry aEntryData = lookupData(entryKey);
  if (aEntryData.bExpand)
  {
     // We need to do fancy, take the slow route.
     return readEntry(pKey, QString()).toUtf8();
  }
  return aEntryData.mValue;
}

QVariant KConfigGroup::readEntry( const char *pKey, const QVariant &aDefault ) const
{
  if ( !hasKey( pKey ) ) return aDefault;
  const QString errString = QString::fromLatin1("\"%1\" - conversion from \"%3\" to %2 failed")
    .arg(pKey).arg(QVariant::typeToName(aDefault.type()));
  const QString formatError = QString::fromLatin1(" (wrong format: expected '%1' items, read '%2')");

  QVariant tmp;
  if (readEntryGui( this, pKey, aDefault, tmp ))
    return tmp;

  tmp = aDefault;

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
      case QVariant::PointF: {
          const QList<qreal> list = readEntry( pKey, QList<qreal>() );

          if ( list.count() != 2 ) {
              kcbError() << errString.arg(readEntry(pKey))
                         << formatError.arg(2).arg(list.count())
                         << endl;
              return aDefault;
          }
          return QPointF(list.at( 0 ), list.at( 1 ));
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
      case QVariant::RectF: {
          const QList<qreal> list = readEntry( pKey, QList<qreal>() );

          if ( list.count() != 4 ) {
              kcbError() << errString.arg(readEntry(pKey))
                         << formatError.arg(4).arg(list.count())
                         << endl;
              return aDefault;
          }
          const QRectF rect(list.at( 0 ), list.at( 1 ), list.at( 2 ), list.at( 3 ));
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
      case QVariant::SizeF: {
          const QList<qreal> list = readEntry( pKey, QList<qreal>() );

          if ( list.count() != 2 ) {
              kcbError() << errString.arg(readEntry(pKey))
                         << formatError.arg(2).arg(list.count())
                         << endl;
              return aDefault;
          }
          const QSizeF size(list.at( 0 ), list.at( 1 ));
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

      case QVariant::Color:
        kFatal() << "KConfigGroup::readEntry was passed GUI type '"
                 << aDefault.typeName()
                 << "' but kdeui isn't linked! If it is linked to your program, this is a platform bug. "
                    "Please inform the KDE developers" << endl;
        break;

      default:
          break;
  }

  Q_ASSERT( 0 );
  return QVariant();
}



QVariantList KConfigGroup::readEntry( const char* pKey, const QVariantList& aDefault) const
{
  if (!hasKey(pKey))
    return aDefault;

  const QStringList slist = readEntry( pKey, QVariant(aDefault).toStringList() );

  return QVariant(slist).toList();
}

QStringList KConfigGroup::readEntry(const char* pKey, const QStringList& aDefault, char sep) const
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

QString KConfigGroup::readPathEntry( const QString& pKey, const QString& pDefault ) const
{
  return readPathEntry(pKey.toUtf8().constData(), pDefault);
}

QString KConfigGroup::readPathEntry( const char *pKey, const QString& pDefault ) const
{
  const bool bExpandSave = d->master->isDollarExpansion();
  d->master->setDollarExpansion(true);
  QString aValue = readEntry( pKey, pDefault );
  d->master->setDollarExpansion(bExpandSave);
  return aValue;
}

QStringList KConfigGroup::readPathListEntry( const QString& pKey, char sep ) const
{
  return readPathListEntry(pKey.toUtf8().constData(), sep);
}

QStringList KConfigGroup::readPathListEntry( const char *pKey, char sep ) const
{
  const bool bExpandSave = d->master->isDollarExpansion();
  d->master->setDollarExpansion(true);
  const QStringList aValue = readEntry( pKey, QStringList(), sep );
  d->master->setDollarExpansion(bExpandSave);
  return aValue;
}

void KConfigGroup::writeEntry( const char *pKey, const QString& value,
                               KConfigBase::WriteConfigFlags pFlags )
{
    // the KConfig object is dirty now
    // set this before any IO takes place so that if any derivative
    // classes do caching, they won't try and flush the cache out
    // from under us before we read. A race condition is still
    // possible but minimized.
    if( pFlags & KConfigBase::Persistent ) {
        setDirty(true);
    }

    if ( !d->master->localeInitialized() && KGlobal::locale() ) {
        d->master->setLocale();
    }

    KEntryKey entryKey(d->group, pKey);
    entryKey.bLocal = pFlags & KConfigBase::NLS;

    KEntry aEntryData;
    aEntryData.mValue = value.toUtf8();  // set new value
    aEntryData.bGlobal = pFlags & KConfigBase::Global;
    aEntryData.bNLS = pFlags & KConfigBase::NLS;

    if (pFlags & KConfigBase::Persistent) {
        aEntryData.bDirty = true;
    }

    // rewrite the new value
    putData(entryKey, aEntryData, true);
}

void KConfigGroup::writePathEntry( const QString& pKey, const QString & path,
        KConfigBase::WriteConfigFlags pFlags )
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

static QString translatePath( QString path ) // krazy:exclude=passbyvalue
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

void KConfigGroup::writePathEntry( const char *pKey, const QString & path,
                                   KConfigBase::WriteConfigFlags pFlags)
{
   writeEntry(pKey, translatePath(path), pFlags);
}

void KConfigGroup::writePathEntry( const QString& pKey, const QStringList &list,
                                  char sep , KConfigBase::WriteConfigFlags pFlags )
{
  writePathEntry(pKey.toUtf8().constData(), list, sep, pFlags);
}

void KConfigGroup::writePathEntry ( const char *pKey, const QStringList &list,
                                   char sep , KConfigBase::WriteConfigFlags pFlags )
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

void KConfigGroup::deleteEntry( const QString& pKey, KConfigBase::WriteConfigFlags pFlags)
{
   deleteEntry(pKey.toUtf8().constData(), pFlags);
}

void KConfigGroup::deleteEntry( const char *pKey, KConfigBase::WriteConfigFlags pFlags)
{
    // the KConfig object is dirty now
    // set this before any IO takes place so that if any derivative
    // classes do caching, they won't try and flush the cache out
    // from under us before we read. A race condition is still
    // possible but minimized.
    setDirty(true);

    if ( !d->master->localeInitialized() && KGlobal::locale() ) {
        d->master->setLocale();
    }

    KEntryKey entryKey(d->group, pKey);
    KEntry aEntryData;

    aEntryData.bGlobal = pFlags & KConfigBase::Global;
    aEntryData.bNLS = pFlags & KConfigBase::NLS;
    aEntryData.bDirty = true;
    aEntryData.bDeleted = true;

    // rewrite the new value
    putData( entryKey, aEntryData, true );
}

void KConfigGroup::writeEntry ( const char *pKey, const QVariant &prop,
                               KConfigBase::WriteConfigFlags pFlags )
{
  if ( writeEntryGui( this, pKey, prop, pFlags ) )
    return;                     // GUI type that was handled

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
    case QVariant::Int:
    case QVariant::UInt:
    case QVariant::Double:
    case QVariant::Bool:
//    case QVariant::KeySequence:
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

    case QVariant::Color:
    case QVariant::Font:
        kFatal() << "KConfigGroup::writeEntry was passed GUI type '"
                 << prop.typeName()
                 << "' but kdeui isn't linked! If it is linked to your program, this is a platform bug. "
                    "Please inform the KDE developers" << endl;
        break;

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
void KConfigGroup::writeEntry ( const char *pKey, const Q3StrList &list,
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

void KConfigGroup::writeEntry ( const char *pKey, const QStringList &list,
                               char sep , KConfigBase::WriteConfigFlags pFlags )
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

void KConfigGroup::revertToDefault(const QString &key)
{
  setDirty(true);

  KEntryKey aEntryKey(d->group, key.toUtf8());
  aEntryKey.bDefault = true;

  if (!d->master->locale().isNull()) {
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

bool KConfigGroup::hasDefault(const QString &key) const
{
  KEntryKey aEntryKey(d->group, key.toUtf8());
  aEntryKey.bDefault = true;

  if (!d->master->locale().isNull()) {
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

bool KConfigGroup::hasKey(const QString &key) const
{
   return hasKey(key.toUtf8().constData());
}

bool KConfigGroup::hasKey(const char *pKey) const
{
  KEntryKey aEntryKey(d->group, 0);
  aEntryKey.c_key = pKey;
  aEntryKey.bDefault = d->master->readDefaults();

  if (!d->master->locale().isNull()) {
    // try the localized key first
    aEntryKey.bLocal = true;
    KEntry entry = lookupData(aEntryKey);
    if (!entry.mValue.isNull()) {
       return true;
    }
    aEntryKey.bLocal = false;
  }

  // try the non-localized version
  KEntry entry = lookupData(aEntryKey);
  return !entry.mValue.isNull();
}

bool KConfigGroup::isImmutable() const
{
  return d->master->groupIsImmutable( d->group );
}
