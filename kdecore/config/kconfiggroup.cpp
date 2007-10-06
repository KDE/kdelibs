/*
   This file is part of the KDE libraries
   Copyright (c) 2006, 2007 Thomas Braxton <kde.braxton@gmail.com>
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
#include "kconfiggroup_p.h"

#include <config.h>

#include "kconfig.h"
#include "kconfig_p.h"
#include "ksharedconfig.h"
#include "kstringhandler.h"
#include "kcomponentdata.h"
#include "kstandarddirs.h"
#include "kconfigdata.h"
#include <kdebug.h>

#include <QtCore/QDate>
#include <QtCore/QSharedData>
#include <QtCore/QFile>
#include <QtCore/QPoint>
#include <QtCore/QRect>
#include <QtCore/QString>
#include <QtCore/QTextStream>
#include <QtCore/QDir>

#include <stdlib.h>

class KConfigGroupPrivate : public QSharedData
{
 public:
    KConfigGroupPrivate(KConfig* owner, bool isConst, const QByteArray &name)
        : mOwner(owner), bImmutable(false), bConst(isConst), mName(name)
    {
        checkImmutable();
    }

    KConfigGroupPrivate(KSharedConfigPtr owner, bool isConst=false)
        : mOwner(owner.data()), sOwner(owner), bImmutable(false), bConst(false)
    {
        checkImmutable();
    }

    KConfigGroupPrivate(KConfigGroupPrivate* other, bool isImmutable, bool isConst)
        : mOwner(other->mOwner), sOwner(other->sOwner), mParent(other->mParent), mName(other->mName),
          bImmutable(isImmutable), bConst(isConst)
    {
        ;
    }


    KConfigGroupPrivate(const KConfigGroupPrivate* other, const QByteArray &name)
        : mOwner(other->mOwner), sOwner(other->sOwner), mParent(other->mParent), mName(name),
          bImmutable(other->bImmutable), bConst(other->bConst)
    {
        checkImmutable();
    }

    KConfig *mOwner;
    KSharedConfig::Ptr sOwner;
    QExplicitlySharedDataPointer<KConfigGroupPrivate> mParent;
    QByteArray mName;

    /* bitfield */
    bool bImmutable:1; // is this group immutable?
    const bool bConst:1; // is this group read-only?

    QByteArray fullName() const
    {
        if (!mParent) {
            if (mName.isEmpty())
                return "<default>";
            return mName;
        }
        return mParent->fullName() + '/' + mName;
    }

    void checkImmutable()
    {
        if (mName.isEmpty())
            bImmutable = mOwner->isImmutable();
        else
            bImmutable = mOwner->groupIsImmutable(fullName());
    }

    static QString expandString(const QString& value);

    static QExplicitlySharedDataPointer<KConfigGroupPrivate> create(KConfigBase *master, const QByteArray &name, bool isConst, KConfigGroup *q)
    {
        if (dynamic_cast<KConfigGroup *>(master)) {
            KConfigGroup tmp(isConst ? static_cast<const KConfigGroup *>(master)->group(name) :
                        static_cast<KConfigGroup *>(master)->group(name));
            return tmp.d;
        } else {
            return QExplicitlySharedDataPointer<KConfigGroupPrivate>(new KConfigGroupPrivate(dynamic_cast<KConfig*>(master), isConst, name));
        }
    }

    void writeListEntry(const QByteArray &key, const QList<QByteArray> &list, char sep , KConfigBase::WriteConfigFlags flags, bool expand = false);
};

KConfigGroup::KConfigGroup() : d(0)
{
}

bool KConfigGroup::isValid() const
{
    return 0 != d.constData();
}

KConfigGroupGui _kde_internal_KConfigGroupGui;
static inline bool readEntryGui(const QByteArray& data, const char *key, const QVariant &input,
                                QVariant &output)
{
  if (_kde_internal_KConfigGroupGui.readEntryGui)
    return _kde_internal_KConfigGroupGui.readEntryGui(data, key, input, output);
  return false;
}

static inline bool writeEntryGui(KConfigGroup *cg, const char *key, const QVariant &input,
                                 KConfigGroup::WriteConfigFlags flags)
{
  if (_kde_internal_KConfigGroupGui.writeEntryGui)
    return _kde_internal_KConfigGroupGui.writeEntryGui(cg, key, input, flags);
  return false;
}

KConfigGroup::KConfigGroup(KConfigBase *master, const QString &_group)
    : d(KConfigGroupPrivate::create(master, _group.toUtf8(), false, this))
{
}

KConfigGroup::KConfigGroup(KConfigBase *master, const QByteArray &_group)
 : d(KConfigGroupPrivate::create(master, _group, false, this))
{
}

KConfigGroup::KConfigGroup(KConfigBase *master, const char *_group)
 : d(KConfigGroupPrivate::create(master, _group, false, this))
{
    changeGroup( _group );
}

KConfigGroup::KConfigGroup(KSharedConfigPtr &master, const QString &_group)
 : d(new KConfigGroupPrivate(master))
{
    changeGroup( _group.toUtf8() );
}

KConfigGroup::KConfigGroup(KSharedConfigPtr &master, const QByteArray &_group)
 : d(new KConfigGroupPrivate(master))
{
    changeGroup( _group );
}

KConfigGroup::KConfigGroup(KSharedConfigPtr &master, const char * _group)
 : d(new KConfigGroupPrivate(master))
{
    changeGroup( _group );
}

KConfigGroup::KConfigGroup(const KConfigBase *master, const QString &_group)
    : d(KConfigGroupPrivate::create(const_cast<KConfigBase*>(master), _group.toUtf8(), true, this))
{
}

KConfigGroup::KConfigGroup(const KConfigBase *master, const QByteArray &_group)
    : d(KConfigGroupPrivate::create(const_cast<KConfigBase*>(master), _group, true, this))
{
}

KConfigGroup::KConfigGroup(const KConfigBase *master, const char * _group)
    : d(KConfigGroupPrivate::create(const_cast<KConfigBase*>(master), _group, true, this))
{
}

KConfigGroup::KConfigGroup(const KSharedConfigPtr &master, const QString &_group)
    : d(new KConfigGroupPrivate(master, true))
{
    changeGroup( _group.toUtf8() );
}

KConfigGroup::KConfigGroup(const KSharedConfigPtr &master, const QByteArray &_group)
    : d(new KConfigGroupPrivate(master, true))
{
    changeGroup( _group );
}

KConfigGroup::KConfigGroup(const KSharedConfigPtr &master, const char * _group)
    : d(new KConfigGroupPrivate(master, true))
{
    changeGroup( _group );
}

KConfigGroup &KConfigGroup::operator=(const KConfigGroup &rhs)
{
    d = rhs.d;
    return *this;
}

KConfigGroup::KConfigGroup(const KConfigGroup &rhs)
    : KConfigBase(), d(rhs.d)
{
}

KConfigGroup::~KConfigGroup()
{
    d = 0;
}

KConfigGroup KConfigGroup::groupImpl(const QByteArray& aGroup)
{
    Q_ASSERT(isValid() && !d->mName.isEmpty());
    KConfigGroup newGroup;

    KConfigGroupPrivate sub = *d;
    sub.mParent = d;
    sub.mName = aGroup;
    bool subImmutable = config()->groupIsImmutable(sub.fullName());
    newGroup.d = new KConfigGroupPrivate(&sub, subImmutable, d->bConst);

    return newGroup;
}

const KConfigGroup KConfigGroup::groupImpl(const QByteArray& aGroup) const
{
    Q_ASSERT(isValid() && !d->mName.isEmpty());
    KConfigGroup newGroup;

    KConfigGroupPrivate sub = *d;
    sub.mParent = d;
    sub.mName = aGroup;
    bool subImmutable = config()->groupIsImmutable(sub.fullName().constData());
    newGroup.d = new KConfigGroupPrivate(&sub, subImmutable, true);

    return newGroup;
}

void KConfigGroup::deleteGroup(WriteConfigFlags flags)
{
    Q_ASSERT(!d->bConst);

    config()->deleteGroup(d->fullName().constData(), flags);
}

void KConfigGroup::changeGroup( const QString &group )
{
    changeGroup(group.toUtf8());
}

void KConfigGroup::changeGroup( const char *group )
{
    changeGroup(QByteArray(group));
}

void KConfigGroup::changeGroup( const QByteArray &group)
{
    // detach (QExplicitlySharedDataPointer takes care of deleting the old d if necessary)
    // ### temporary solution until QExplicitlySharedDataPointer has detach()
    d = new KConfigGroupPrivate(d, group);
}

QString KConfigGroup::name() const
{
    if (d->mName.isEmpty())
        return QString("");
    const QByteArray me(d->fullName());
    return QString::fromUtf8(me.constData(), me.size());
}

bool KConfigGroup::exists() const
{
    return config()->hasGroup( d->fullName() );
}

void KConfigGroup::sync()
{
    if (!d->bConst)
        config()->sync();
}

QMap<QString, QString> KConfigGroup::entryMap() const
{
  return config()->entryMap(d->fullName());
}

KConfig* KConfigGroup::config()
{
  return d->mOwner;
}

const KConfig* KConfigGroup::config() const
{
    return d->mOwner;
}

bool KConfigGroup::entryIsImmutable(const QByteArray& key) const
{
    return (isImmutable() ||
        !config()->d_func()->canWriteEntry(d->fullName(), key, config()->readDefaults()));
}

bool KConfigGroup::entryIsImmutable(const char* key) const
{
    return entryIsImmutable(QByteArray(key));
}

bool KConfigGroup::entryIsImmutable(const QString& key) const
{
    return entryIsImmutable(key.toUtf8());
}

QString KConfigGroup::readEntryUntranslated( const QByteArray& key, const QString& aDefault ) const
{
    QString result = config()->d_func()->lookupData(d->fullName(), key, KEntryMap::EntryOptions(), 0);
    if (result.isNull())
        return aDefault;
    return result;
}

QString KConfigGroup::readEntryUntranslated( const QString& key, const QString& aDefault ) const
{
   return readEntryUntranslated(key.toUtf8(), aDefault);
}


QString KConfigGroup::readEntryUntranslated( const char *key, const QString& aDefault ) const
{
    return readEntryUntranslated(QByteArray(key), aDefault);
}

QString KConfigGroup::readEntry( const QByteArray &key, const char* aDefault) const
{
    return readEntry(key, QString::fromUtf8(aDefault));
}

QString KConfigGroupPrivate::expandString(const QString& value)
{
    QString aValue = value;

    // check for environment variables and make necessary translations
    int nDollarPos = aValue.indexOf( '$' );

    while( nDollarPos != -1 && nDollarPos+1 < aValue.length()) {
        // there is at least one $
        if( aValue[nDollarPos+1] == '(' ) {
            int nEndPos = nDollarPos+1;
            // the next character is not $
            while ( (nEndPos <= aValue.length()) && (aValue[nEndPos]!=')') )
                nEndPos++;
            nEndPos++;
            QString cmd = aValue.mid( nDollarPos+2, nEndPos-nDollarPos-3 );

            QString result;
            QByteArray oldpath = qgetenv( "PATH" );
            QByteArray newpath = QFile::encodeName( KGlobal::dirs()->resourceDirs( "exe" ).join( QChar( KPATH_SEPARATOR ) ) );
            if( !newpath.isEmpty() && !oldpath.isEmpty() )
                newpath += KPATH_SEPARATOR;
            newpath += oldpath;
            setenv( "PATH", newpath, 1/*overwrite*/ );
            FILE *fs = popen(QFile::encodeName(cmd).data(), "r");
            if (fs) {
                QTextStream ts(fs, QIODevice::ReadOnly);
                result = ts.readAll().trimmed();
                pclose(fs);
            }
            setenv( "PATH", oldpath, 1/*overwrite*/ );
            aValue.replace( nDollarPos, nEndPos-nDollarPos, result );
        } else if( aValue[nDollarPos+1] != '$' ) {
            int nEndPos = nDollarPos+1;
            // the next character is not $
            QString aVarName;
            if ( aValue[nEndPos]=='{' ) {
                while ( (nEndPos <= aValue.length()) && (aValue[nEndPos]!='}') )
                    nEndPos++;
                nEndPos++;
                aVarName = aValue.mid( nDollarPos+2, nEndPos-nDollarPos-3 );
            } else {
                while ( nEndPos <= aValue.length() &&
                        (aValue[nEndPos].isNumber() ||
                        aValue[nEndPos].isLetter() ||
                        aValue[nEndPos]=='_' ) )
                    nEndPos++;
                aVarName = aValue.mid( nDollarPos+1, nEndPos-nDollarPos-1 );
            }
            QString env;
            if (!aVarName.isEmpty()) {
#ifdef Q_OS_WIN
                if (aVarName == "HOME")
                    env = QDir::homePath();
                else
#endif
                {
                    const char* pEnv = getenv( aVarName.toAscii() );
                    if( pEnv )
                    // !!! Sergey A. Sukiyazov <corwin@micom.don.ru> !!!
                    // An environment variable may contain values in 8bit
                    // locale specified encoding or UTF8 encoding
                        env = KStringHandler::from8Bit( pEnv );
                }
                if (!env.isEmpty())
                    aValue.replace(nDollarPos, nEndPos-nDollarPos, env);
            } else
                aValue.remove( nDollarPos, nEndPos-nDollarPos );
        } else {
            // remove one of the dollar signs
            aValue.remove( nDollarPos, 1 );
            nDollarPos++;
        }
        nDollarPos = aValue.indexOf( '$', nDollarPos );
    }

    return aValue;
}

template <>
QString KConfigGroup::readEntry<QString>( const QByteArray& key, const QString& aDefault ) const
{
    const char *pKey = key.constData();
    if ( !hasKey(pKey) )
        return aDefault;

    bool expand = false;

    // read value from the entry map
    QString aValue = config()->d_func()->lookupData(d->fullName().constData(), pKey, KEntryMap::SearchLocalized,
                                           &expand);
    if (aValue.isNull())
        aValue = aDefault;

    return aValue;
}

static QList<int> asIntList(const QByteArray& string)
{
    QList<int> list;
    Q_FOREACH(const QByteArray& s, string.split(','))
        list << s.toInt();
    return list;
}

static QList<qreal> asRealList(const QByteArray& string)
{
    QList<qreal> list;
    Q_FOREACH(const QByteArray& s, string.split(','))
        list << s.toDouble();
    return list;
}

QVariant KConfigGroup::convertToQVariant(const char *pKey, const QByteArray& value, const QVariant& aDefault)
{
    const QString errString = QString::fromLatin1("\"%1\" - conversion from \"%3\" to %2 failed")
            .arg(pKey).arg(QVariant::typeToName(aDefault.type())).arg(value.constData());
    const QString formatError = QString::fromLatin1(" (wrong format: expected '%1' items, read '%2')");
    QVariant tmp = aDefault;

    // if a type handler is added here you must add a QVConversions definition
    // to conversion_check.h, or ConversionCheck::to_QVariant will not allow
    // readEntry<T> to convert to QVariant.
    switch( aDefault.type() ) {
        case QVariant::Invalid:
            return QVariant();
        case QVariant::String:
            // this should return the raw string not the dollar expanded string.
            // imho if processed string is wanted should call
            // readEntry(key, QString) not readEntry(key, QVariant)
            return QString::fromUtf8(value);
        case QVariant::List:
        case QVariant::StringList:
            return QString::fromUtf8(value).split(QLatin1Char(','));
        case QVariant::ByteArray:
            return value;
        case QVariant::Bool: {
            const QByteArray lower(value.toLower());
            if (lower == "false" || lower == "no" || lower == "off" || lower == "0")
                return false;
            return true;
        }
        case QVariant::Double:
        case QVariant::Int:
        case QVariant::UInt:
        case QVariant::LongLong:
        case QVariant::ULongLong:
            tmp = value;
            if ( !tmp.convert(aDefault.type()) )
                tmp = aDefault;
            return tmp;
        case QVariant::Point: {
            const QList<int> list = asIntList(value);

            if ( list.count() != 2 ) {
                kError() << errString
                         << formatError.arg(2).arg(list.count())
                         << endl;
                return aDefault;
            }
            return QPoint(list.at( 0 ), list.at( 1 ));
        }
        case QVariant::PointF: {
            const QList<qreal> list = asRealList(value);

            if ( list.count() != 2 ) {
                kError() << errString
                         << formatError.arg(2).arg(list.count())
                         << endl;
                return aDefault;
            }
            return QPointF(list.at( 0 ), list.at( 1 ));
        }
        case QVariant::Rect: {
            const QList<int> list = asIntList(value);

            if ( list.count() != 4 ) {
                kError() << errString
                         << formatError.arg(4).arg(list.count())
                         << endl;
                return aDefault;
            }
            const QRect rect(list.at( 0 ), list.at( 1 ), list.at( 2 ), list.at( 3 ));
            if ( !rect.isValid() ) {
                kError() << errString << endl;
                return aDefault;
            }
            return rect;
        }
        case QVariant::RectF: {
            const QList<qreal> list = asRealList(value);

            if ( list.count() != 4 ) {
                kError() << errString
                         << formatError.arg(4).arg(list.count())
                         << endl;
                return aDefault;
            }
            const QRectF rect(list.at( 0 ), list.at( 1 ), list.at( 2 ), list.at( 3 ));
            if ( !rect.isValid() ) {
                kError() << errString << endl;
                return aDefault;
            }
            return rect;
        }
        case QVariant::Size: {
            const QList<int> list = asIntList(value);

            if ( list.count() != 2 ) {
                kError() << errString
                         << formatError.arg(2).arg(list.count())
                         << endl;
                return aDefault;
            }
            const QSize size(list.at( 0 ), list.at( 1 ));
            if ( !size.isValid() ) {
                kError() << errString << endl;
                return aDefault;
            }
            return size;
        }
        case QVariant::SizeF: {
            const QList<qreal> list = asRealList(value);

            if ( list.count() != 2 ) {
                kError() << errString
                         << formatError.arg(2).arg(list.count())
                         << endl;
                return aDefault;
            }
            const QSizeF size(list.at( 0 ), list.at( 1 ));
            if ( !size.isValid() ) {
                kError() << errString << endl;
                return aDefault;
            }
            return size;
        }
        case QVariant::DateTime: {
            const QList<int> list = asIntList(value);
            if ( list.count() != 6 ) {
                kError() << errString
                         << formatError.arg(6).arg(list.count())
                         << endl;
                return aDefault;
            }
            const QDate date( list.at( 0 ), list.at( 1 ), list.at( 2 ) );
            const QTime time( list.at( 3 ), list.at( 4 ), list.at( 5 ) );
            const QDateTime dt( date, time );
            if ( !dt.isValid() ) {
                kError() << errString << endl;
                return aDefault;
            }
            return dt;
        }
        case QVariant::Date: {
            QList<int> list = asIntList(value);
            if ( list.count() == 6 )
                list = list.mid(0, 3); // don't break config files that stored QDate as QDateTime
            if ( list.count() != 3 ) {
                kError() << errString
                         << formatError.arg(3).arg(list.count())
                         << endl;
                return aDefault;
            }
            const QDate date( list.at( 0 ), list.at( 1 ), list.at( 2 ) );
            if ( !date.isValid() ) {
                kError() << errString << endl;
                return aDefault;
            }
            return date;
        }
        case QVariant::Color:
        case QVariant::Font:
            kFatal() << "KConfigGroup::readEntry was passed GUI type '"
                    << aDefault.typeName()
                    << "' but kdeui isn't linked! If it is linked to your program, "
                    "this is a platform bug. Please inform the KDE developers" << endl;
            break;
        case QVariant::Url:
            return QUrl(QString::fromUtf8(value));

        default:
            if( aDefault.canConvert<KUrl>() ) {
                const KUrl url(QString::fromUtf8(value));
                return qVariantFromValue<KUrl>( url );
            }
            break;
    }

    kFatal() << "unhandled type " << aDefault.typeName() << endl;
    return QVariant();
}

template<>
QVariant KConfigGroup::readEntry<QVariant>( const QByteArray &key, const QVariant &aDefault ) const
{
    const QByteArray data = config()->d_func()->lookupData(d->fullName(), key, KEntryMap::EntryOptions());
    if (data.isNull())
        return aDefault;

    QVariant value;
    if (!readEntryGui( data, key, aDefault, value ))
        value = convertToQVariant(key, data, aDefault);

    return value;
}

template<>
QVariantList KConfigGroup::readEntry<QVariantList>( const QByteArray &key, const QVariantList& aDefault) const
{
    const QByteArray data = config()->d_func()->lookupData(d->fullName(), key, KEntryMap::EntryOptions());

    if (data.isNull())
        return aDefault;

    if (!data.contains("\\,")) { // easy no escaped commas
        QVariantList list;
        foreach (const QByteArray& v, data.split(','))
            list << QString::fromUtf8(v.constData(), v.length());
        return list;
    }

    // now look out for escaped commas
    QList<QByteArray> list;
    for (int i=0; i < data.size(); /* nothing */) {
        int end = data.indexOf(',', i);
    again:
        if (end < 0) { // no more commas found, end of entry
            list << data.mid(i);
            i = data.size();
        } else if (end == 0) { // empty first element
            list << QByteArray();
            i++;
        } else if (data.at(end-1) == '\\') { // escaped comma
            end = data.indexOf(',', end+1);
            goto again;
        } else {
            list << data.mid(i, end-i);
            i = end+1;
        }
    }

    QVariantList value;
    foreach(const QByteArray& v, list)
        value << QString::fromUtf8(QByteArray(v).replace("\\,", ","));

    return value;
}

QStringList KConfigGroup::readEntry(const QByteArray &key, const QStringList& aDefault, char sep) const
{
    const QString data = readEntry(key, QString());
    if (data.isNull())
        return aDefault;

    const QString separator = QChar(sep);
    const QString escaped = QString(separator).prepend(QLatin1Char('\\'));

    if (!data.contains(escaped))
        return data.split(separator); // easy no escaped separators

    // now look out for escaped separators
    QStringList value;
    for(int i=0; i < data.size(); /* nothing */) {
        int end = data.indexOf(separator, i);
    again:
        if (end < 0) { // no more separators found, end of entry
            value << data.mid(i).replace(escaped, separator);
            i = data.size();
        } else if (end == 0) { // empty first element
            value << QString();
            i++;
        } else if (data.at(end-1) == QLatin1Char('\\')) { // escaped separator
            end = data.indexOf(separator, end+1);
            goto again;
        } else {
            value << data.mid(i, end-i).replace(escaped, separator);
            i = end+1;
        }
    }
    return value;
}

QStringList KConfigGroup::readEntry(const QString& pKey, const QStringList& aDefault, char sep) const
{
    return readEntry(pKey.toUtf8().constData(), aDefault, sep);
}

QStringList KConfigGroup::readEntry(const char *key, const QStringList& aDefault, char sep) const
{
    return readEntry(QByteArray(key), aDefault, sep);
}

QString KConfigGroup::readPathEntry( const QString& pKey, const QString& pDefault ) const
{
    return readPathEntry(pKey.toUtf8(), pDefault);
}

QString KConfigGroup::readPathEntry( const char *pKey, const QString& aDefault ) const
{
    return readPathEntry(QByteArray(pKey), aDefault);
}

QString KConfigGroup::readPathEntry( const QByteArray &key, const QString& aDefault ) const
{
    bool expand = false;

    QString aValue = config()->d_func()->lookupData(d->fullName(), key, KEntryMap::SearchLocalized,
                            &expand);
    if (aValue.isNull())
        aValue = aDefault;

    // only do dollar expansion if it's an expandable string
    if (expand)
        return KConfigGroupPrivate::expandString(aValue);
    return aValue;
}

QStringList KConfigGroup::readPathListEntry( const QString& pKey, char sep ) const
{
    return readPathListEntry(pKey.toUtf8(), sep);
}

QStringList KConfigGroup::readPathListEntry( const char *pKey, char sep ) const
{
    return readPathListEntry(QByteArray(pKey), sep);
}

QStringList KConfigGroup::readPathListEntry( const QByteArray &key, char sep ) const
{
    bool expand = false;
    const QString data = config()->d_func()->lookupData(d->fullName(), key, KEntryMap::SearchLocalized,
                                             &expand);
    if (data.isNull())
        return QStringList();

    const QString separator = QChar(sep);
    const QString escaped = QString(separator).prepend(QLatin1Char('\\'));
    QStringList value;

    if (!data.contains(escaped)) { // easy no escaped separators
        foreach(const QString& s, data.split(separator)) {
            if (expand)
                value << KConfigGroupPrivate::expandString(s);
            else
                value << s;
        }
    } else { // now look out for escaped separators
        QStringList value;
        for(int i=0; i < data.size(); /* nothing */) {
            int end = data.indexOf(separator, i);
    again:
            if (end < 0) { // no more separators found, end of entry
                if (expand)
                    value << KConfigGroupPrivate::expandString(data.mid(i).replace(escaped, separator));
                else
                    value << data.mid(i).replace(escaped, separator);
                i = data.size();
            } else if (end == 0) { // empty first element
                value << QString();
                i++;
            } else if (data.at(end-1) == QLatin1Char('\\')) { // escaped separator
                end = data.indexOf(separator, end+1);
                goto again;
            } else {
                if (expand)
                    value << KConfigGroupPrivate::expandString(data.mid(i, end-i).replace(escaped, separator));
                else
                    value << data.mid(i, end-i).replace(escaped, separator);
                i = end+1;
            }
        }
    }
    return value;
}

template<>
void KConfigGroup::writeEntry<QString>( const QByteArray &key, const QString& value, WriteConfigFlags flags )
{
    Q_ASSERT(!d->bConst);

    config()->d_func()->putData(d->fullName(), key, value.toUtf8(), flags);
}

template<>
void KConfigGroup::writeEntry<const char *>(const QByteArray &key, const char* const &value, WriteConfigFlags flags)
{
    writeEntry(key, QByteArray(value), flags);
}

template<>
void KConfigGroup::writeEntry<QVariantList>( const QByteArray &key, const QVariantList& list, WriteConfigFlags flags )
{
    Q_ASSERT(!d->bConst);

    QList<QByteArray> data;

    foreach(const QVariant& v, list) {
        if (v.type() == QVariant::ByteArray)
            data << v.toByteArray();
        else
            data << v.toString().toUtf8();
    }

    d->writeListEntry(key, data, ',', flags);
}

template<>
void KConfigGroup::writeEntry<QByteArray>( const QByteArray &key, const QByteArray& value,
                     WriteConfigFlags flags )
{
    Q_ASSERT(!d->bConst);

    config()->d_func()->putData(d->fullName(), key, value, flags);
}

void KConfigGroup::deleteEntry(const QByteArray& key, WriteConfigFlags flags)
{
   writeEntry(key, QByteArray(), flags);
}

void KConfigGroup::deleteEntry( const QString& key, WriteConfigFlags flags)
{
    deleteEntry(key.toUtf8(), flags);
}

void KConfigGroup::deleteEntry( const char *key, WriteConfigFlags flags)
{
    deleteEntry(QByteArray(key), flags);
}

template<>
void KConfigGroup::writeEntry<QVariant> ( const QByteArray &key, const QVariant &value,
                               WriteConfigFlags flags )
{
    Q_ASSERT(!d->bConst);

    const char * pKey = key.constData();
    if ( writeEntryGui( this, pKey, value, flags ) )
        return;                     // GUI type that was handled

    QByteArray data;
    // if a type handler is added here you must add a QVConversions definition
    // to conversion_check.h, or ConversionCheck::to_QVariant will not allow
    // writeEntry<T> to convert to QVariant.
    switch( value.type() ) {
        case QVariant::Invalid:
            data = "";
            break;
        case QVariant::ByteArray:
            data = value.toByteArray();
            break;
        case QVariant::String:
        case QVariant::Int:
        case QVariant::UInt:
        case QVariant::Double:
        case QVariant::Bool:
        case QVariant::LongLong:
        case QVariant::ULongLong:
            data = value.toString().toUtf8();
            break;
        case QVariant::List:
            kError(!value.canConvert(QVariant::StringList))
                << "not all types in \"" << pKey << "\" can convert to QString,"
                   " information will be lost" << endl;
        case QVariant::StringList:
            writeEntry( pKey, value.toList(), flags );
            return;
        case QVariant::Point: {
            QVariantList list;
            const QPoint rPoint = value.toPoint();
            list.insert( 0, rPoint.x() );
            list.insert( 1, rPoint.y() );

            writeEntry( pKey, list, flags );
            return;
        }
        case QVariant::Rect:{
            QVariantList list;
            const QRect rRect = value.toRect();
            list.insert( 0, rRect.left() );
            list.insert( 1, rRect.top() );
            list.insert( 2, rRect.width() );
            list.insert( 3, rRect.height() );

            writeEntry( pKey, list, flags );
            return;
        }
        case QVariant::Size:{
            QVariantList list;
            const QSize rSize = value.toSize();
            list.insert( 0, rSize.width() );
            list.insert( 1, rSize.height() );

            writeEntry( pKey, list, flags );
            return;
        }
        case QVariant::Date: {
            QVariantList list;
            const QDate date = value.toDate();

            list.insert( 0, date.year() );
            list.insert( 1, date.month() );
            list.insert( 2, date.day() );

            writeEntry( pKey, list, flags );
            return;
        }
        case QVariant::DateTime: {
            QVariantList list;
            const QDateTime rDateTime = value.toDateTime();

            const QTime time = rDateTime.time();
            const QDate date = rDateTime.date();

            list.insert( 0, date.year() );
            list.insert( 1, date.month() );
            list.insert( 2, date.day() );

            list.insert( 3, time.hour() );
            list.insert( 4, time.minute() );
            list.insert( 5, time.second() );

            writeEntry( pKey, list, flags );
            return;
        }

        case QVariant::Color:
        case QVariant::Font:
            kFatal() << "KConfigGroup::writeEntry was passed GUI type '"
                     << value.typeName()
                     << "' but kdeui isn't linked! If it is linked to your program, this is a platform bug. "
                        "Please inform the KDE developers" << endl;
            break;
        case QVariant::Url:
            data = KUrl(value.toUrl()).url().toUtf8();
            break;
        default:
            if( value.canConvert<KUrl>() ) {
                data = qvariant_cast<KUrl>(value).url().toUtf8();
                break;
            }
            kFatal() << "KConfigGroup::writeEntry - unhandled type" << endl;
        }

    writeEntry(key, data, flags);
}

void KConfigGroup::writeEntry(const QString& pKey, const QStringList &value, char sep, WriteConfigFlags flags)
{
    writeEntry( pKey.toUtf8(), value, sep, flags );
}

void KConfigGroup::writeEntry(const char *pKey, const QStringList &value, char sep, WriteConfigFlags flags)
{
    writeEntry(QByteArray(pKey), value, sep, flags );
}

void KConfigGroup::writeEntry(const QByteArray &key, const QStringList &list, char sep, WriteConfigFlags flags)
{
    Q_ASSERT(!d->bConst);

    QList<QByteArray> balist;

    foreach(const QString &entry, list)
        balist.append(entry.toUtf8());

    d->writeListEntry(key, balist, sep, flags);
}

void KConfigGroupPrivate::writeListEntry(const QByteArray &key, const QList<QByteArray> &list, char sep , KConfigBase::WriteConfigFlags flags, bool expand)
{
    Q_ASSERT(!bConst);

    const QByteArray escaped = QByteArray(1, '\\') + sep;

    QByteArray value;

    QList<QByteArray>::ConstIterator it = list.begin();
    const QList<QByteArray>::ConstIterator end = list.end();

    if (it != end) {
        value = *it;
        value.reserve(4084);

        value.replace(sep, escaped);

        while (++it != end) {
            value += sep;
            value += QByteArray(*it).replace(sep, escaped);
        }

        value.squeeze(); // release any unused memory
    }

    mOwner->d_func()->putData(fullName(), key, value, flags, expand);
}

void KConfigGroup::revertToDefault(const QByteArray& key)
{
    if (d->bConst)
        return;

    const QByteArray theDefault = config()->d_func()->lookupData(d->fullName(), key,
                      KEntryMap::SearchDefaults|KEntryMap::SearchLocalized);

    config()->d_func()->putData(d->fullName(), key, theDefault, KConfig::Normal);
}

void KConfigGroup::revertToDefault(const char *key)
{
    revertToDefault(QByteArray(key));
}

void KConfigGroup::revertToDefault(const QString &key)
{
    revertToDefault(key.toUtf8());
}

bool KConfigGroup::hasDefault(const QByteArray& key) const
{
    KEntryMap::SearchFlags flags = KEntryMap::SearchDefaults|KEntryMap::SearchLocalized;
    flags |= KEntryMap::SearchLocalized;

    return !config()->d_func()->lookupData(d->fullName(), key, flags).isNull();
}

bool KConfigGroup::hasDefault(const char *key) const
{
    return hasDefault(QByteArray(key));
}

bool KConfigGroup::hasDefault(const QString &key) const
{
    return hasDefault(key.toUtf8());
}

bool KConfigGroup::hasKey(const QByteArray& key) const
{
    KEntryMap::SearchFlags flags = KEntryMap::SearchLocalized;
    if ( config()->readDefaults() )
        flags |= KEntryMap::SearchDefaults;

    return !config()->d_func()->lookupData(d->fullName(), key, flags).isNull();
}

bool KConfigGroup::hasKey(const QString &key) const
{
   return hasKey(key.toUtf8());
}

bool KConfigGroup::hasKey(const char *key) const
{
    return hasKey(QByteArray(key));
}

bool KConfigGroup::isImmutable() const
{
    return d->bImmutable;
}



#include <QtCore/QDir>

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

QString translatePath( QString path ) // krazy:exclude=passbyvalue
{
   if (path.isEmpty())
       return path;

   // only "our" $HOME should be interpreted
   path.replace('$', "$$");

   bool startsWithFile = path.startsWith(QLatin1String("file:"), Qt::CaseInsensitive);

   // return original path, if it refers to another type of URL (e.g. http:/), or
   // if the path is already relative to another directory
   if ((!startsWithFile && QFileInfo(path).isRelative()) ||
       (startsWithFile && QFileInfo(path.mid(5)).isRelative()))
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

void KConfigGroup::writePathEntry(const QString &key, const QString & path, WriteConfigFlags flags)
{
   writePathEntry(key.toUtf8(), path, flags);
}

void KConfigGroup::writePathEntry(const char *key, const QString & path, WriteConfigFlags flags)
{
   writePathEntry(QByteArray(key), path, flags);
}

void KConfigGroup::writePathEntry(const QByteArray &key, const QString &path, WriteConfigFlags flags)
{
    Q_ASSERT(!d->bConst);

    config()->d_func()->putData(d->fullName(), key, translatePath(path).toUtf8(), flags, true);
}

void KConfigGroup::writePathEntry(const QString &key, const QStringList &value, char sep, WriteConfigFlags flags)
{
  writePathEntry(key.toUtf8(), value, sep, flags);
}

void KConfigGroup::writePathEntry(const char *key, const QStringList &value, char sep, WriteConfigFlags flags)
{
  writePathEntry(QByteArray(key), value, sep, flags);
}

void KConfigGroup::writePathEntry(const QByteArray &key, const QStringList &value, char sep, WriteConfigFlags flags)
{
    Q_ASSERT(!d->bConst);

    QList<QByteArray> list;
    foreach(const QString& path, value)
        list << translatePath(path).toUtf8();

    d->writeListEntry(key, list, sep, flags, true);
}

QStringList KConfigGroup::groupList() const
{
    return config()->d_func()->groupList(d->fullName());
}

QStringList KConfigGroup::keyList() const
{
    return entryMap().keys();
}

void KConfigGroup::clean()
{
    config()->clean();
}

KConfigGroup::ConfigState KConfigGroup::getConfigState() const
{
    return config()->getConfigState();
}

bool KConfigGroup::hasGroupImpl(const QByteArray & b) const
{
    return groupImpl(b).exists();
}

void KConfigGroup::deleteGroupImpl(const QByteArray &b, WriteConfigFlags flags)
{
    Q_ASSERT(!d->bConst);

    config()->deleteGroup(d->fullName() + '/' + b, flags);
}

bool KConfigGroup::groupIsImmutableImpl(const QByteArray& b) const
{
    return groupImpl(b).d->bImmutable;
}
