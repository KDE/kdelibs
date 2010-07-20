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

#define KDE_EXTENDED_DEBUG_OUTPUT

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
#include <QThreadStorage>

#ifdef Q_WS_WIN
#include <fcntl.h>
#include <windows.h>
#ifndef _WIN32_WCE
#include <wincon.h>
#endif
#else
#include <unistd.h>
#include <stdio.h>
#endif

#ifdef NDEBUG
#undef kDebug
#undef kBacktrace
#endif

#include <config.h>

#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#endif
#ifdef HAVE_TIME_H
#include <time.h>
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

#ifdef Q_OS_SOLARIS
// For the purposes of KDebug Solaris has a GNU-libc-compatible
// backtrace() function. This isn't detected by the CMake checks
// normally (check_function_exists fails), but we know it's there.
// For better results, we would use walk_context(), but that's
// a little more code -- see also the crash handler in kcrash.cpp .
#define HAVE_BACKTRACE (1)
#endif

#ifdef HAVE_BACKTRACE
#include <execinfo.h>
#ifdef __GNUC__
#define HAVE_BACKTRACE_DEMANGLE
#include <cxxabi.h>
#endif
#endif

#include "kdebugdbusiface_p.h"
#include <QMutex>



KDECORE_EXPORT bool kde_kdebug_enable_dbus_interface = false;

class KNoDebugStream: public QIODevice
{
    // Q_OBJECT
public:
    KNoDebugStream() { open(WriteOnly); }
    bool isSequential() const { return true; }
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
                // not using fromRawData because we need a terminating NUL
                const QByteArray buf(data, len);
                syslog(m_priority, "%s", buf.constData());
            }
            return len;
        }
    void setPriority(int priority) { m_priority = priority; }
private:
    int m_priority;
};

class KFileDebugStream: public KNoDebugStream
{
    // Q_OBJECT
public:
    qint64 writeData(const char *data, qint64 len)
        {
            if (len) {
                QFile aOutputFile(m_fileName);
                aOutputFile.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Unbuffered);
                QByteArray buf = QByteArray::fromRawData(data, len);
                aOutputFile.write(buf.trimmed());
                aOutputFile.putChar('\n');
                aOutputFile.close();
            }
            return len;
        }
    void setFileName(const QString& fn) { m_fileName = fn; }
private:
    QString m_fileName;
};

class KMessageBoxDebugStream: public KNoDebugStream
{
    // Q_OBJECT
public:
    qint64 writeData(const char *data, qint64 len)
        {
            if (len) {
                // Since we are in kdecore here, we cannot use KMsgBox
                QString msg = QString::fromAscii(data, len);
                KMessage::message(KMessage::Information, msg, m_caption);
            }
            return len;
        }
    void setCaption(const QString& h) { m_caption = h; }
private:
    QString m_caption;
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
        DefaultOutput = QtOutput, // if you change DefaultOutput, also change the defaults in kdebugdialog!
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

        QByteArray name;
        QString logFileName[4];
        OutputMode mode[4];
    };
    typedef QHash<unsigned int, Area> Cache;

    KDebugPrivate()
        : config(0), kDebugDBusIface(0), m_disableAll(false)
    {
        Q_ASSERT(int(QtDebugMsg) == 0);
        Q_ASSERT(int(QtFatalMsg) == 3);

        // Create the D-Bus interface if it has not been created yet
        // But only register to D-Bus if we are in a process with a D-Bus event loop,
        // otherwise introspection will just hang.
        // Examples of processes without a D-Bus event loop: kioslaves and the main kdeinit process.
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

        for (int i = 0; i < 8; i++) {
            m_nullOutputYesNoCache[i] = -1;
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
        areaData.clear();

        //AB: this is necessary here, otherwise all output with area 0 won't be
        //prefixed with anything, unless something with area != 0 is called before
        areaData.name = KGlobal::mainComponent().componentName().toUtf8();

        for (int i = 0; i < 8; i++) {
            m_nullOutputYesNoCache[i] = -1;
        }

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
            const QByteArray line = file.readLine().trimmed();
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

            do {
                ch=line[++i];
            } while (ch >= '0' && ch <= '9' && i < line.length());

            unsigned int number = line.left(i).toUInt();

            while (i < line.length() && line[i] <= ' ')
                i++;

            Area areaData;
            areaData.name = line.mid(i);
            cache.insert(number, areaData);
        }
        file.close();
    }

    inline int level(QtMsgType type)
    { return int(type) - int(QtDebugMsg); }

    QString groupNameForArea(unsigned int area) const
    {
        QString groupName = QString::number(area);
        if (area == 0 || !config->hasGroup(groupName)) {
            groupName = QString::fromLocal8Bit(cache.value(area).name);
        }
        return groupName;
    }

    OutputMode areaOutputMode(QtMsgType type, unsigned int area, bool enableByDefault)
    {
        if (!config)
            return QtOutput;

        QString key;
        switch (type) {
        case QtDebugMsg:
            key = QLatin1String( "InfoOutput" );
            if (m_disableAll)
                return NoOutput;
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

        const KConfigGroup cg(config, groupNameForArea(area));
        const int mode = cg.readEntry(key, int(enableByDefault ? DefaultOutput : NoOutput));
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

        KConfigGroup cg(config, groupNameForArea(area));
        return cg.readPathEntry(aKey, QLatin1String("kdebug.dbg"));
    }

    KConfig* configObject()
    {
        if (!config) {
            if (!KGlobal::hasMainComponent()) {
                return NULL;
            }

            config = new KConfig(QLatin1String("kdebugrc"), KConfig::NoGlobals);
            m_disableAll = config->group(QString()).readEntry("DisableAll", false);
        }
        return config;
    }

    Cache::Iterator areaData(QtMsgType type, unsigned int num, bool enableByDefault = true)
    {
        if (!configObject()) {
            // we don't have a config and we can't create one...
            Area &area = cache[0]; // create a dummy entry
            if (KGlobal::hasMainComponent())
                area.name = KGlobal::mainComponent().componentName().toUtf8();
            return cache.find(0);
        }

        if (cache.count() <= 1) { // empty or containing only entry "0"
            loadAreaNames(); // fills 'cache'
        }

        Cache::Iterator it = cache.find(num);
        if (it == cache.end()) {
            // unknown area
            return cache.find(0);
        }

        if (num == 0 && type == QtDebugMsg) { // area 0 is special, it becomes the named area "appname"
            static bool s_firstDebugFromApplication = true;
            if (s_firstDebugFromApplication && !m_disableAll) {
                s_firstDebugFromApplication = false;
                writeGroupForNamedArea(it->name, enableByDefault);
            }
        }

        const int lev = level(type);
        //qDebug() << "in cache for" << num << ":" << it->mode[lev];
        if (it->mode[lev] == Unknown)
            it->mode[lev] = areaOutputMode(type, num, enableByDefault);
        if (it->mode[lev] == FileOutput && it->logFileName[lev].isEmpty())
            it->logFileName[lev] = logFileName(type, num);

        return it;
    }

    QDebug setupFileWriter(const QString &fileName)
    {
        if (!filewriter.hasLocalData())
            filewriter.setLocalData(new KFileDebugStream);
        filewriter.localData()->setFileName(fileName);
        QDebug result(filewriter.localData());
        return result;
    }

    QDebug setupMessageBoxWriter(QtMsgType type, const QByteArray &areaName)
    {
        if (!messageboxwriter.hasLocalData())
            messageboxwriter.setLocalData(new KMessageBoxDebugStream);
        QDebug result(messageboxwriter.localData());
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

        header += areaName;
        header += ')';
        messageboxwriter.localData()->setCaption(QString::fromAscii(header));
        return result;
    }

    QDebug setupSyslogWriter(QtMsgType type)
    {
        if (!syslogwriter.hasLocalData())
            syslogwriter.setLocalData(new KSyslogDebugStream);
        QDebug result(syslogwriter.localData());
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
        syslogwriter.localData()->setPriority(level);
        return result;
    }

    QDebug setupQtWriter(QtMsgType type)
    {
        if (type != QtDebugMsg) {
            if (type == QtWarningMsg) {
                // KDE warnings are not the same thing as Qt warnings
                // in Qt, warnings indicate bad code, which must be corrected before the release
                // in KDE, it's just something that everyone sees (including users)
                type = QtDebugMsg;
            }
            return QDebug(type);
        }
        return QDebug(&lineendstrippingwriter);
    }

    QDebug printHeader(QDebug s, const QByteArray &areaName, const char *, int, const char *funcinfo, QtMsgType type, bool colored)
    {
#ifdef KDE_EXTENDED_DEBUG_OUTPUT
        static bool printProcessInfo = (qgetenv("KDE_DEBUG_NOPROCESSINFO").isEmpty());
        static bool printAreaName = (qgetenv("KDE_DEBUG_NOAREANAME").isEmpty());
        static bool printMethodName = (qgetenv("KDE_DEBUG_NOMETHODNAME").isEmpty());

        static int printTimeStamp = qgetenv("KDE_DEBUG_TIMESTAMP").toInt();
        QByteArray programName;
        s = s.nospace();
        if (printTimeStamp > 0) {
            if (printTimeStamp >= 2) {
                // the extended print: 17:03:24.123
                const QString sformat = QString::fromLatin1("hh:mm:ss.zzz");
                s << qPrintable(QDateTime::currentDateTime().time().toString(sformat));
            } else {
                // the default print: 17:03:24
                s << qPrintable(QDateTime::currentDateTime().time().toString());
            }
            s << ' ';
        }

        if (printProcessInfo) {
            programName = cache.value(0).name;
            if (programName.isEmpty()) {
                if (QCoreApplication::instance())
                    programName = QCoreApplication::instance()->applicationName().toLocal8Bit();
                else
                    programName = "<unknown program name>";
            }
            s << programName.constData() << "(" << unsigned(getpid()) << ")";
        }
        if (printAreaName && (!printProcessInfo || areaName != programName)) {
            if (printProcessInfo)
                s << "/";
            s << areaName.constData();
        }

        if (funcinfo && printMethodName) {
            if (colored) {
                if (type <= QtDebugMsg)
                    s << "\033[0;34m"; //blue
                else
                    s << "\033[0;31m"; //red
            }
# ifdef Q_CC_GNU
            // strip the function info down to the base function name
            // note that this throws away the template definitions,
            // the parameter types (overloads) and any const/volatile qualifiers
            QByteArray info = funcinfo;
            int pos = info.indexOf('(');
            Q_ASSERT_X(pos != -1, "kDebug",
                       "Bug in kDebug(): I don't know how to parse this function name");
            while (info.at(pos - 1) == ' ')
                // that '(' we matched was actually the opening of a function-pointer
                pos = info.indexOf('(', pos + 1);

            info.truncate(pos);
            // gcc 4.1.2 don't put a space between the return type and
            // the function name if the function is in an anonymous namespace
            int index = 1;
            forever {
                index = info.indexOf("<unnamed>::", index);
                if ( index == -1 )
                    break;

                if ( info.at(index-1) != ':' )
                    info.insert(index, ' ');

                index += strlen("<unnamed>::");
            }
            pos = info.lastIndexOf(' ');
            if (pos != -1) {
                int startoftemplate = info.lastIndexOf('<');
                if (startoftemplate != -1 && pos > startoftemplate &&
                    pos < info.lastIndexOf(">::"))
                    // we matched a space inside this function's template definition
                    pos = info.lastIndexOf(' ', startoftemplate);
            }

            if (pos + 1 == info.length())
                // something went wrong, so gracefully bail out
                s << " " << funcinfo;
            else
                s << " " << info.constData() + pos + 1;
# else
            s << " " << funcinfo;
# endif
           if(colored)
               s  << "\033[0m";
        }

        s << ":";
#else
        Q_UNUSED(funcinfo);
        if (!areaName.isEmpty())
            s << areaName.constData() << ':';
#endif
        return s.space();
    }

    QDebug stream(QtMsgType type, unsigned int area, const char *debugFile, int line,
                  const char *funcinfo)
    {
        static bool env_colored = (!qgetenv("KDE_COLOR_DEBUG").isEmpty());
        Cache::Iterator it = areaData(type, area);
        OutputMode mode = it->mode[level(type)];
        QString file = it->logFileName[level(type)];
        QByteArray areaName = it->name;

        if (areaName.isEmpty())
            areaName = cache.value(0).name;

        bool colored=false;

        QDebug s(&devnull);
        switch (mode) {
        case FileOutput:
            s = setupFileWriter(file);
            break;
        case MessageBoxOutput:
            s = setupMessageBoxWriter(type, areaName);
            break;
        case SyslogOutput:
            s = setupSyslogWriter(type);
            break;
        case NoOutput:
            s = QDebug(&devnull);
            return s; //no need to take the time to "print header" if we don't want to output anyway
            break;
        default:                // QtOutput
            s = setupQtWriter(type);
#ifndef Q_OS_WIN
            //only color if the debug goes to a tty.
            colored = env_colored && isatty(fileno(stderr));
#endif
            break;
        }

        return printHeader(s, areaName, debugFile, line, funcinfo, type, colored);
    }

    void writeGroupForNamedArea(const QByteArray& areaName, bool enabled)
    {
        // Ensure that this area name appears in kdebugrc, so that users (via kdebugdialog)
        // can turn it off.
        KConfig* cfgObj = configObject();
        if (cfgObj) {
            KConfigGroup cg(cfgObj, QString::fromUtf8(areaName));
            const QString key = QString::fromLatin1("InfoOutput");
            if (!cg.hasKey(key)) {
                cg.writeEntry(key, int(enabled ? KDebugPrivate::QtOutput : KDebugPrivate::NoOutput));
            }
        }
    }

    QMutex mutex;
    KConfig *config;
    KDebugDBusIface *kDebugDBusIface;
    Cache cache;
    bool m_disableAll;
    int m_nullOutputYesNoCache[8];

    KNoDebugStream devnull;
    QThreadStorage<KSyslogDebugStream*> syslogwriter;
    QThreadStorage<KFileDebugStream*> filewriter;
    QThreadStorage<KMessageBoxDebugStream*> messageboxwriter;
    KLineEndStrippingDebugStream lineendstrippingwriter;
};

K_GLOBAL_STATIC(KDebugPrivate, kDebug_data)

#ifdef HAVE_BACKTRACE
static QString maybeDemangledName(char *name)
{
#ifdef HAVE_BACKTRACE_DEMANGLE
    const int len = strlen(name);
    QByteArray in = QByteArray::fromRawData(name, len);
    const int mangledNameStart = in.indexOf("(_");
    if (mangledNameStart >= 0) {
        const int mangledNameEnd = in.indexOf('+', mangledNameStart + 2);
        if (mangledNameEnd >= 0) {
            int status;
            // if we forget about this line and the one that undoes its effect we don't change the
            // internal data of the QByteArray::fromRawData() ;)
            name[mangledNameEnd] = 0;
            char *demangled = abi::__cxa_demangle(name + mangledNameStart + 1, 0, 0, &status);
            name[mangledNameEnd] = '+';
            if (demangled) {
                QString ret = QString::fromLatin1(name, mangledNameStart + 1) +
                              QString::fromLatin1(demangled) +
                              QString::fromLatin1(name + mangledNameEnd, len - mangledNameEnd);
                free(demangled);
                return ret;
            }
        }
    }
#endif
    return QString::fromLatin1(name);
}
#endif

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
        s += QString::number(i) + QLatin1String(": ") +
             maybeDemangledName(strings[i]) + QLatin1Char('\n');
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
        qCritical().nospace() << "kDebugStream called after destruction (from "
                              << (funcinfo ? funcinfo : "")
                              << (file ? " file " : " unknown file")
                              << (file ? file :"")
                              << " line " << line << ")";
        return QDebug(level);
    }

    QMutexLocker locker(&kDebug_data->mutex);
    return kDebug_data->stream(level, area, file, line, funcinfo);
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
    KDebugPrivate* d = kDebug_data;
    QMutexLocker locker(&d->mutex);
    delete d->config;
    d->config = 0;

    KDebugPrivate::Cache::Iterator it = d->cache.begin(),
                                  end = d->cache.end();
    for ( ; it != end; ++it)
        it->clear();

    for (int i = 0; i < 8; i++) {
        d->m_nullOutputYesNoCache[i] = -1;
    }
}

// static
bool KDebug::hasNullOutput(QtMsgType type,
                           bool condition,
                           int area,
                           bool enableByDefault)
{
    if (!condition) {
        return true;
    }
    if (kDebug_data.isDestroyed()) {
         // kDebugStream() will generate a warning anyway, so we don't.
        return false;
    }
    KDebugPrivate *const d = kDebug_data;
    QMutexLocker locker(&d->mutex);

    if (type == QtDebugMsg) {
        int *entries = d->m_nullOutputYesNoCache;
        for (int i = 0; i < 8; i += 2) {
            if (entries[i] == area) {
                return entries[i + 1];
            }
        }
    }

    KDebugPrivate::Cache::Iterator it = d->areaData(type, area, enableByDefault);
    const bool ret = it->mode[d->level(type)] == KDebugPrivate::NoOutput;

    // cache result for next time...
    if (type == QtDebugMsg) {
        int *entries = d->m_nullOutputYesNoCache;
        int idx = (qrand() % 4) * 2;
        entries[idx] = area;
        entries[idx + 1] = ret;
    }

    return ret;
}

int KDebug::registerArea(const QByteArray& areaName, bool enabled)
{
    // TODO for optimization: static int s_lastAreaNumber = 1;
    KDebugPrivate* d = kDebug_data;
    QMutexLocker locker(&d->mutex);
    int areaNumber = 1;
    while (d->cache.contains(areaNumber)) {
        ++areaNumber;
    }
    KDebugPrivate::Area areaData;
    areaData.name = areaName;
    d->cache.insert(areaNumber, areaData);
    d->writeGroupForNamedArea(areaName, enabled);
    return areaNumber;
}
