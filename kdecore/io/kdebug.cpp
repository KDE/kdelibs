/* This file is part of the KDE libraries
    Copyright (C) 1997 Matthias Kalle Dalheimer (kalle@kde.org)
                  2002 Holger Freyther (freyther@kde.org)

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

//#define KDE_EXTENDED_DEBUG_OUTPUT

#ifndef QT_NO_CAST_FROM_ASCII
#define QT_NO_CAST_FROM_ASCII
#endif
#ifndef QT_NO_CAST_TO_ASCII
#define QT_NO_CAST_TO_ASCII
#endif
#ifndef KDE3_SUPPORT
#define KDE3_SUPPORT
#endif

#include "kdebug.h"

#ifdef Q_WS_WIN
#include <fcntl.h>
#include <windows.h>
#include <wincon.h>
#endif

#ifdef NDEBUG
#undef kDebug
#undef kBacktrace
#endif

#include "kglobal.h"
#include "kstandarddirs.h"
#include "kdatetime.h"
#include "kcmdlineargs.h"

#include <kmessage.h>
#include <klocale.h>
#include <kconfiggroup.h>
#include <kurl.h>

#include <QtCore/QFile>
#include <QtCore/QHash>
#include <QtCore/QObject>
#include <QtCore/QChar>
#include <QtCore/QCoreApplication>

#include <stdlib.h>	// abort
#include <unistd.h>	// getpid
#include <stdarg.h>	// vararg stuff
#include <ctype.h>      // isprint
#include <syslog.h>
#include <errno.h>
#include <string.h>
#include <kconfig.h>
#include "kcomponentdata.h"
#include <config.h>

#ifdef HAVE_BACKTRACE
#include <execinfo.h>
#endif

#include "kdebugdbusiface_p.h"
#include <QMutex>

KDECORE_EXPORT bool kde_kdebug_enable_dbus_interface = false;

class KNoDebugStream: public QIODevice
{
    // Q_OBJECT
public:
    KNoDebugStream() { open(WriteOnly); }
    bool isSequential() { return true; }
    qint64 readData(char *, qint64) { return 0; /* eof */ }
    qint64 readLineData(char *, qint64) { return 0; /* eof */ }
    qint64 writeData(const char *, qint64 len) { return len; }
};

class KSyslogDebugStream: public KNoDebugStream
{
    // Q_OBJECT
public:
    qint64 writeData(const char *data, qint64 len)
        {
            if (len) {
                int nPriority = *data++;
                // not using fromRawData because we need a terminating NUL
                QByteArray buf(data, len);

                syslog(nPriority, "%s", buf.constData());
            }
            return len;
        }
};

class KFileDebugStream: public KNoDebugStream
{
    // Q_OBJECT
public:
    qint64 writeData(const char *data, qint64 len)
        {
            if (len) {
                QByteArray buf = QByteArray::fromRawData(data, len);
                int pos = buf.indexOf('\0');
                Q_ASSERT(pos != -1);

                QFile aOutputFile(QFile::decodeName(data));
                aOutputFile.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Unbuffered);
                aOutputFile.write(data + pos + 1, len - pos - 1);
                aOutputFile.putChar('\n');
                aOutputFile.close();
            }
            return len;
        }
};

class KMessageBoxDebugStream: public KNoDebugStream
{
    // Q_OBJECT
public:
    qint64 writeData(const char *data, qint64 len)
        {
            if (len) {
                // Since we are in kdecore here, we cannot use KMsgBox
                QString caption = QString::fromAscii(data, len);
                int pos = caption.indexOf(QLatin1Char('\0'));
                Q_ASSERT(pos != -1);

                QString msg = caption.mid(pos + 1);
                caption.truncate(pos);
                KMessage::message(KMessage::Information, msg, caption);
            }
            return len;
        }
};

class KLineEndStrippingDebugStream: public KNoDebugStream
{
    // Q_OBJECT
public:
    qint64 writeData(const char *data, qint64 len)
        {
            QByteArray buf = QByteArray::fromRawData(data, len);
            qt_message_output(QtDebugMsg, buf.trimmed().constData());
            return len;
        }
};

struct KDebugPrivate
{
    enum OutputMode {
        FileOutput = 0,
        MessageBoxOutput = 1,
        QtOutput = 2,
        SyslogOutput = 3,
        NoOutput = 4,
        Unknown = 5
    };

    struct Area {
        inline Area() { clear(); }
        void clear(OutputMode set = Unknown)
        {
            for (int i = 0; i < 4; ++i) {
                logFileName[i].clear();
                mode[i] = set;
            }
        }

        QString name;
        QString logFileName[4];
        OutputMode mode[4];
    };
    typedef QHash<unsigned int, Area> Cache;

    KDebugPrivate()
        : config(0), kDebugDBusIface(0)
    {
        Q_ASSERT(int(QtDebugMsg) == 0);
        Q_ASSERT(int(QtFatalMsg) == 3);

        // Create the dbus interface if it has not been created yet
        // But only register to DBus if we are in a process with a dbus event loop,
        // otherwise introspection will just hang.
        // Examples of processes without a dbus event loop: kioslaves and the main kdeinit process.
        //
        // How to know that we have a real event loop? That's tricky.
        // We could delay registration in kDebugDBusIface with a QTimer, but
        // it would still get triggered by kioslaves that use enterLoop/exitLoop
        // to run kio jobs synchronously.
        //
        // Solution: we have a bool that is set by KApplication
        // (kioslaves should use QCoreApplication but not KApplication).
        if (kde_kdebug_enable_dbus_interface) {
            kDebugDBusIface = new KDebugDBusIface;
        }
    }

    ~KDebugPrivate()
    {
        delete config;
        delete kDebugDBusIface;
    }

    void loadAreaNames()
    {
        cache.clear();

        Area &areaData = cache[0];
        areaData.clear(QtOutput);

        //AB: this is necessary here, otherwise all output with area 0 won't be
        //prefixed with anything, unless something with area != 0 is called before
        areaData.name = KGlobal::mainComponent().componentName();

        QString filename(KStandardDirs::locate("config", QLatin1String("kdebug.areas")));
        if (filename.isEmpty()) {
            return;
        }
        QFile file(filename);
        if (!file.open(QIODevice::ReadOnly)) {
            qWarning("Couldn't open %s", filename.toLocal8Bit().constData());
            file.close();
            return;
        }

        uint lineNumber=0;

        while (!file.atEnd()) {
            QByteArray line = file.readLine().trimmed();
            ++lineNumber;
            if (line.isEmpty())
                continue;

            int i=0;
            unsigned char ch=line[i];

            if (ch =='#')
                continue; // We have an eof, a comment or an empty line

            if (ch < '0' && ch > '9') {
                qWarning("Syntax error: no number (line %u)",lineNumber);
                continue;
            }

            const int numStart=i;
            do {
                ch=line[++i];
            } while (ch >= '0' && ch <= '9' && i < line.length());

            unsigned int number = line.left(i).toUInt();

            while (i < line.length() && line[i] <= ' ')
                i++;

            Area areaData;
            areaData.name = QString::fromUtf8(line.mid(i));
            cache.insert(number, areaData);
        }
        file.close();
    }

    inline int level(QtMsgType type)
    { return int(type) - int(QtDebugMsg); }

    OutputMode areaOutputMode(QtMsgType type, unsigned int area)
    {
        if (!config)
            return QtOutput;

        QString key;
        switch (type) {
        case QtDebugMsg:
            key = QLatin1String( "InfoOutput" );
            break;
        case QtWarningMsg:
            key = QLatin1String( "WarnOutput" );
            break;
        case QtFatalMsg:
            key = QLatin1String( "FatalOutput" );
            break;
        case QtCriticalMsg:
        default:
            /* Programmer error, use "Error" as default */
            key = QLatin1String( "ErrorOutput" );
            break;
        }

        KConfigGroup cg(config, QString::number(area));
        int mode = cg.readEntry(key, 2);
        return OutputMode(mode);
    }

    QString logFileName(QtMsgType type, unsigned int area)
    {
        if (!config)
            return QString();

        const char* aKey;
        switch (type)
        {
        case QtDebugMsg:
            aKey = "InfoFilename";
            break;
        case QtWarningMsg:
            aKey = "WarnFilename";
            break;
        case QtFatalMsg:
            aKey = "FatalFilename";
            break;
        case QtCriticalMsg:
        default:
            aKey = "ErrorFilename";
            break;
        }

        KConfigGroup cg(config, QString::number(area));
        return cg.readPathEntry(aKey, QLatin1String("kdebug.dbg"));
    }

    Cache::Iterator areaData(QtMsgType type, unsigned int num)
    {
        if (!config) {
            if (!KGlobal::hasMainComponent()) {
                // we don't have a config and we can't create one...
                Area &area = cache[0]; // create a dummy entry
                return cache.find(0);
            }

            config = new KConfig(QLatin1String("kdebugrc"), KConfig::NoGlobals);
            loadAreaNames();
        }

        if (!cache.contains(num))
            // unknown area
            return cache.find(0);

        int l = level(type);
        Cache::Iterator it = cache.find(num);
        if (it->mode[l] == Unknown)
            it->mode[l] = areaOutputMode(type, num);
        if (it->mode[l] == FileOutput && it->logFileName[l].isEmpty())
            it->logFileName[l] = logFileName(type, num);

        return it;
    }

    QDebug setupFileWriter(const QString &fileName, const QString &areaName)
    {
        QDebug result(&filewriter);
        QString header = fileName;
        header += QLatin1Char('\0');
        if (!areaName.isEmpty()) {
            header += areaName;
            header += QLatin1Char(':');
        }

        result << qPrintable(header);
        return result.space();
    }

    QDebug setupMessageBoxWriter(QtMsgType type, const QString &areaName)
    {
        QDebug result(&messageboxwriter);
        QByteArray header;

        switch (type) {
        case QtDebugMsg:
            header = "Info (";
            break;
        case QtWarningMsg:
            header = "Warning (";
            break;
        case QtFatalMsg:
            header = "Fatal Error (";
            break;
        case QtCriticalMsg:
        default:
            header = "Error (";
            break;
        }

        header += areaName.toAscii();
        header += ')';
        header += '\0';
        header += areaName.toAscii();
        result << header.constData();
        return result.space();
    }

    QDebug setupSyslogWriter(QtMsgType type, const QString &areaName)
    {
        QDebug result(&syslogwriter);
        int level = 0;

        switch (type) {
        case QtDebugMsg:
            level = LOG_INFO;
            break;
        case QtWarningMsg:
            level = LOG_WARNING;
            break;
        case QtFatalMsg:
            level = LOG_CRIT;
            break;
        case QtCriticalMsg:
        default:
            level = LOG_ERR;
            break;
        }
        result.nospace() << char(level);
        if (!areaName.isEmpty())
            result << areaName.toAscii() << ":";
        return result.space();
    }

    QDebug setupQtWriter(QtMsgType type, const QString &areaName)
    {
        QDebug result(&lineendstrippingwriter);
        if (type != QtDebugMsg)
            result = QDebug(type);
        if (areaName.isEmpty())
            return result;
        result.nospace() << qPrintable(areaName) << ":";
        return result.space();
    }

    QDebug stream(QtMsgType type, unsigned int area)
    {
        Cache::Iterator it = areaData(type, area);
        OutputMode mode = it->mode[level(type)];
        QString file = it->logFileName[level(type)];
        QString areaName = it->name;

        if (areaName.isEmpty())
            areaName = cache.value(0).name;

        switch (mode) {
        case FileOutput:
            return setupFileWriter(file, areaName);
        case MessageBoxOutput:
            return setupMessageBoxWriter(type, areaName);
        case SyslogOutput:
            return setupSyslogWriter(type, areaName);
        case NoOutput:
            return QDebug(&devnull);
        default:                // QtOutput
            return setupQtWriter(type, areaName);
        }
    }

    QMutex mutex;
    KConfig *config;
    KDebugDBusIface *kDebugDBusIface;
    Cache cache;

    KNoDebugStream devnull;
    KSyslogDebugStream syslogwriter;
    KFileDebugStream filewriter;
    KMessageBoxDebugStream messageboxwriter;
    KLineEndStrippingDebugStream lineendstrippingwriter;
};

K_GLOBAL_STATIC(KDebugPrivate, kDebug_data)

static QDebug debugHeader(QDebug s, const char *file, int line, const char *funcinfo)
{
#ifdef KDE_EXTENDED_DEBUG_OUTPUT
    s.nospace() << "[";
    bool needSpace = true;

    static QBasicAtomicPointer<char> programHeader = Q_ATOMIC_INIT(0);
    if (!programHeader) {
        QString str;
        if (KGlobal::hasMainComponent())
            str = KGlobal::mainComponent().componentName();
        if (str.isEmpty()) {
            QStringList arguments = QCoreApplication::arguments();
            if (!arguments.isEmpty())
                str = arguments.first();
            if (str.isEmpty())
                str = KCmdLineArgs::appName();
            int pos = str.indexOf(QLatin1Char('/'));
            if (pos != -1)
                str = str.mid(pos);
        }

        if (!str.isEmpty()) {
            str += QLatin1String(" (");
            str += QString::number(getpid());
            str += QLatin1String(")");

            char *dup = strdup(str.toLocal8Bit());
            if (!programHeader.testAndSet(0, dup))
                free(dup);
        }
    }

    if (programHeader) {
        s << static_cast<char *>(programHeader);
        needSpace = true;
    }

    if (funcinfo) {
        if (needSpace) s << " ";
        s << "in \"" << funcinfo << "\"";
        needSpace = true;
    }

    if (file && line != -1) {
        if (needSpace) s << " ";
        s << "at " << file << ":" << line;
    }

    s << "]";
    s.space();
#else
    Q_UNUSED(file); Q_UNUSED(line); Q_UNUSED(funcinfo);
#endif

    return s;
}

QString kRealBacktrace(int levels)
{
    QString s;
#ifdef HAVE_BACKTRACE
    void* trace[256];
    int n = backtrace(trace, 256);
    if (!n)
	return s;
    char** strings = backtrace_symbols (trace, n);

    if ( levels != -1 )
        n = qMin( n, levels );
    s = QLatin1String("[\n");

    for (int i = 0; i < n; ++i)
        s += QString::number(i) +
             QLatin1String(": ") +
             QLatin1String(strings[i]) + QLatin1String("\n");
    s += QLatin1String("]\n");
    if (strings)
        free (strings);
#endif
    return s;
}

QDebug kDebugDevNull()
{
    return QDebug(&kDebug_data->devnull);
}

QDebug kDebugStream(QtMsgType level, int area, const char *file, int line, const char *funcinfo)
{
    if (kDebug_data.isDestroyed()) {
        // we don't know what to return now...
        qCritical() << "kDebugStream called after destruction; backtrace:"
                    << qPrintable(kRealBacktrace(-1));
        return QDebug(level);
    }

    QMutexLocker locker(&kDebug_data->mutex);
    return debugHeader(kDebug_data->stream(level, area), file, line, funcinfo);
}

QDebug perror(QDebug s, KDebugTag)
{
    return s << QString::fromLocal8Bit(strerror(errno));
}

QDebug operator<<(QDebug s, const KDateTime &time)
{
    if ( time.isDateOnly() )
        s.nospace() << "KDateTime(" << qPrintable(time.toString(KDateTime::QtTextDate)) << ")";
    else
        s.nospace() << "KDateTime(" << qPrintable(time.toString(KDateTime::ISODate)) << ")";
    return s.space();
}

QDebug operator<<(QDebug s, const KUrl &url)
{
    s.nospace() << "KUrl(" << url.prettyUrl() << ")";
    return s.space();
}

void kClearDebugConfig()
{
    if (!kDebug_data) return;
    QMutexLocker locker(&kDebug_data->mutex);
    delete kDebug_data->config;
    kDebug_data->config = 0;

    KDebugPrivate::Cache::Iterator it = kDebug_data->cache.begin(),
                                  end = kDebug_data->cache.end();
    for ( ; it != end; ++it)
        it->clear();
}
