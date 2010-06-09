/* This file is part of the KDE libraries
    Copyright (C) 2000 Torben Weis <weis@kde.org>
    Copyright (C) 2006 David Faure <faure@kde.org>
    Copyright (C) 2009 Michael Pyne <michael.pyne@kdemail.net>

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

#include "krun.h"
#include "krun_p.h"

#include <config.h>

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <typeinfo>
#include <sys/stat.h>

#include <QtGui/QWidget>
#include <QtGui/QLabel>
#include <QtGui/QVBoxLayout>
#include <QtGui/QHBoxLayout>
#include <QtGui/QPlainTextEdit>
#include <QtGui/QApplication>
#include <QtGui/QDesktopWidget>

#include "kmimetypetrader.h"
#include "kmimetype.h"
#include "kio/jobclasses.h" // for KIO::JobFlags
#include "kio/job.h"
#include "kio/jobuidelegate.h"
#include "kio/global.h"
#include "kio/scheduler.h"
#include "kio/netaccess.h"
#include "kfile/kopenwithdialog.h"
#include "kfile/krecentdocument.h"
#include "kdesktopfileactions.h"

#include <kauthorized.h>
#include <kmessageboxwrapper.h>
#include <kurl.h>
#include <kglobal.h>
#include <kglobalsettings.h>
#include <ktoolinvocation.h>
#include <kdebug.h>
#include <klocale.h>
#include <kprotocolmanager.h>
#include <kstandarddirs.h>
#include <kprocess.h>
#include <QtCore/QFile>
#include <QtCore/QFileInfo>
#include <QtCore/QTextIStream>
#include <QtCore/QDate>
#include <QtCore/QRegExp>
#include <kdesktopfile.h>
#include <kmacroexpander.h>
#include <kshell.h>
#include <QTextDocument>
#include <kde_file.h>
#include <kconfiggroup.h>
#include <kdialog.h>
#include <kstandardguiitem.h>
#include <kguiitem.h>
#include <ksavefile.h>

#ifdef Q_WS_X11
#include <kwindowsystem.h>
#endif

KRun::KRunPrivate::KRunPrivate(KRun *parent)
        : q(parent),
        m_showingDialog(false)
{
}

void KRun::KRunPrivate::startTimer()
{
    m_timer.start(0);
}

// ---------------------------------------------------------------------------

bool KRun::isExecutableFile(const KUrl& url, const QString &mimetype)
{
    if (!url.isLocalFile()) {
        return false;
    }
    QFileInfo file(url.toLocalFile());
    if (file.isExecutable()) {    // Got a prospective file to run
        KMimeType::Ptr mimeType = KMimeType::mimeType(mimetype, KMimeType::ResolveAliases);
        if (mimeType && (mimeType->is(QLatin1String("application/x-executable")) ||
#ifdef Q_WS_WIN
                         mimeType->is(QLatin1String("application/x-ms-dos-executable")) ||
#endif
                         mimeType->is(QLatin1String("application/x-executable-script")))
           )
        {
            return true;
        }
    }
    return false;
}

// This is called by foundMimeType, since it knows the mimetype of the URL
bool KRun::runUrl(const KUrl& u, const QString& _mimetype, QWidget* window, bool tempFile, bool runExecutables, const QString& suggestedFileName, const QByteArray& asn)
{
    bool noRun = false;
    bool noAuth = false;
    if (_mimetype == QLatin1String("inode/directory-locked")) {
        KMessageBoxWrapper::error(window,
                                  i18n("<qt>Unable to enter <b>%1</b>.\nYou do not have access rights to this location.</qt>", Qt::escape(u.prettyUrl())));
        return false;
    }
    else if (_mimetype == QLatin1String("application/x-desktop")) {
        if (u.isLocalFile() && runExecutables) {
            return KDesktopFileActions::run(u, true);
        }
    }
    else if (isExecutableFile(u, _mimetype)) {
        if (u.isLocalFile() && runExecutables) {
            if (KAuthorized::authorize("shell_access")) {
                return (KRun::runCommand(KShell::quoteArg(u.toLocalFile()), QString(), QString(), window, asn)); // just execute the url as a command
                // ## TODO implement deleting the file if tempFile==true
            }
            else {
                noAuth = true;
            }
        }
        else if (_mimetype == QLatin1String("application/x-executable")) {
            noRun = true;
        }
    }
    else if (isExecutable(_mimetype)) {
        if (!runExecutables) {
            noRun = true;
        }

        if (!KAuthorized::authorize("shell_access")) {
            noAuth = true;
        }
    }

    if (noRun) {
        KMessageBox::sorry(window,
                           i18n("<qt>The file <b>%1</b> is an executable program. "
                                "For safety it will not be started.</qt>", Qt::escape(u.prettyUrl())));
        return false;
    }
    if (noAuth) {
        KMessageBoxWrapper::error(window,
                                  i18n("<qt>You do not have permission to run <b>%1</b>.</qt>", Qt::escape(u.prettyUrl())));
        return false;
    }

    KUrl::List lst;
    lst.append(u);

    KService::Ptr offer = KMimeTypeTrader::self()->preferredService(_mimetype);

    if (!offer) {
        // Open-with dialog
        // TODO : pass the mimetype as a parameter, to show it (comment field) in the dialog !
        // Hmm, in fact KOpenWithDialog::setServiceType already guesses the mimetype from the first URL of the list...
        return displayOpenWithDialog(lst, window, tempFile, suggestedFileName, asn);
    }

    return KRun::run(*offer, lst, window, tempFile, suggestedFileName, asn);
}

bool KRun::displayOpenWithDialog(const KUrl::List& lst, QWidget* window, bool tempFiles,
                                 const QString& suggestedFileName, const QByteArray& asn)
{
    if (!KAuthorized::authorizeKAction("openwith")) {
        KMessageBox::sorry(window,
                           i18n("You are not authorized to select an application to open this file."));
        return false;
    }

#ifdef Q_WS_WIN
    KConfigGroup cfgGroup(KGlobal::config(), "KOpenWithDialog Settings");
    if (cfgGroup.readEntry("Native", true)) {
        return KRun::KRunPrivate::displayNativeOpenWithDialog(lst, window, tempFiles,
                suggestedFileName, asn);
    }
#endif
    KOpenWithDialog l(lst, i18n("Open with:"), QString(), window);
    if (l.exec()) {
        KService::Ptr service = l.service();
        if (service) {
            return KRun::run(*service, lst, window, tempFiles, suggestedFileName, asn);
        }

        kDebug(7010) << "No service set, running " << l.text();
        return KRun::run(l.text(), lst, window, false, suggestedFileName, asn);   // TODO handle tempFiles
    }
    return false;
}

void KRun::shellQuote(QString &_str)
{
    // Credits to Walter, says Bernd G. :)
    if (_str.isEmpty()) { // Don't create an explicit empty parameter
        return;
    }
    QChar q('\'');
    _str.replace(q, "'\\''").prepend(q).append(q);
}


class KRunMX1 : public KMacroExpanderBase
{
public:
    KRunMX1(const KService &_service) :
            KMacroExpanderBase('%'), hasUrls(false), hasSpec(false), service(_service) {}

    bool hasUrls: 1, hasSpec: 1;

protected:
    virtual int expandEscapedMacro(const QString &str, int pos, QStringList &ret);

private:
    const KService &service;
};

int
KRunMX1::expandEscapedMacro(const QString &str, int pos, QStringList &ret)
{
    uint option = str[pos + 1].unicode();
    switch (option) {
    case 'c':
        ret << service.name().replace('%', "%%");
        break;
    case 'k':
        ret << service.entryPath().replace('%', "%%");
        break;
    case 'i':
        ret << "-icon" << service.icon().replace('%', "%%");
        break;
    case 'm':
//       ret << "-miniicon" << service.icon().replace( '%', "%%" );
        kWarning() << "-miniicon isn't supported anymore (service"
        << service.name() << ')';
        break;
    case 'u':
    case 'U':
        hasUrls = true;
        /* fallthrough */
    case 'f':
    case 'F':
    case 'n':
    case 'N':
    case 'd':
    case 'D':
    case 'v':
        hasSpec = true;
        /* fallthrough */
    default:
        return -2; // subst with same and skip
    }
    return 2;
}

class KRunMX2 : public KMacroExpanderBase
{
public:
    KRunMX2(const KUrl::List &_urls) :
            KMacroExpanderBase('%'), ignFile(false), urls(_urls) {}

    bool ignFile: 1;

protected:
    virtual int expandEscapedMacro(const QString &str, int pos, QStringList &ret);

private:
    void subst(int option, const KUrl &url, QStringList &ret);

    const KUrl::List &urls;
};

void
KRunMX2::subst(int option, const KUrl &url, QStringList &ret)
{
    switch (option) {
    case 'u':
        ret << ((url.isLocalFile() && url.fragment().isNull() && url.encodedQuery().isNull()) ?
                url.toLocalFile()  : url.url());
        break;
    case 'd':
        ret << url.directory();
        break;
    case 'f':
        ret << url.path();
        break;
    case 'n':
        ret << url.fileName();
        break;
    case 'v':
        if (url.isLocalFile() && QFile::exists(url.toLocalFile())) {
            ret << KDesktopFile(url.path()).desktopGroup().readEntry("Dev");
        }
        break;
    }
    return;
}

int
KRunMX2::expandEscapedMacro(const QString &str, int pos, QStringList &ret)
{
    uint option = str[pos + 1].unicode();
    switch (option) {
    case 'f':
    case 'u':
    case 'n':
    case 'd':
    case 'v':
        if (urls.isEmpty()) {
            if (!ignFile) {
                kDebug() << "No URLs supplied to single-URL service" << str;
            }
        }
        else if (urls.count() > 1) {
            kWarning() << urls.count() << "URLs supplied to single-URL service" << str;
        }
        else {
            subst(option, urls.first(), ret);
        }
        break;
    case 'F':
    case 'U':
    case 'N':
    case 'D':
        option += 'a' - 'A';
        for (KUrl::List::ConstIterator it = urls.begin(); it != urls.end(); ++it)
            subst(option, *it, ret);
        break;
    case '%':
        ret = QStringList(QLatin1String("%"));
        break;
    default:
        return -2; // subst with same and skip
    }
    return 2;
}

QStringList KRun::processDesktopExec(const KService &_service, const KUrl::List& _urls, bool tempFiles, const QString& suggestedFileName)
{
    QString exec = _service.exec();
    if (exec.isEmpty()) {
        kWarning() << "KRun: no Exec field in `" << _service.entryPath() << "' !";
        return QStringList();
    }

    QStringList result;
    bool appHasTempFileOption;

    KRunMX1 mx1(_service);
    KRunMX2 mx2(_urls);

    if (!mx1.expandMacrosShellQuote(exec)) {    // Error in shell syntax
        kWarning() << "KRun: syntax error in command" << _service.exec() << ", service" << _service.name();
        return QStringList();
    }

    // FIXME: the current way of invoking kioexec disables term and su use

    // Check if we need "tempexec" (kioexec in fact)
    appHasTempFileOption = tempFiles && _service.property("X-KDE-HasTempFileOption").toBool();
    if (tempFiles && !appHasTempFileOption && _urls.size()) {
        const QString kioexec = KStandardDirs::findExe("kioexec");
        Q_ASSERT(!kioexec.isEmpty());
        result << kioexec << "--tempfiles" << exec;
        if (!suggestedFileName.isEmpty()) {
            result << "--suggestedfilename";
            result << suggestedFileName;
        }
        result += _urls.toStringList();
        return result;
    }

    // Check if we need kioexec
    if (!mx1.hasUrls) {
        for (KUrl::List::ConstIterator it = _urls.begin(); it != _urls.end(); ++it)
            if (!(*it).isLocalFile() && !KProtocolInfo::isHelperProtocol(*it)) {
                // We need to run the app through kioexec
                const QString kioexec = KStandardDirs::findExe("kioexec");
                Q_ASSERT(!kioexec.isEmpty());
                result << kioexec;
                if (tempFiles) {
                    result << "--tempfiles";
                }
                if (!suggestedFileName.isEmpty()) {
                    result << "--suggestedfilename";
                    result << suggestedFileName;
                }
                result << exec;
                result += _urls.toStringList();
                return result;
            }
    }

    if (appHasTempFileOption) {
        exec += " --tempfile";
    }

    // Did the user forget to append something like '%f'?
    // If so, then assume that '%f' is the right choice => the application
    // accepts only local files.
    if (!mx1.hasSpec) {
        exec += " %f";
        mx2.ignFile = true;
    }

    mx2.expandMacrosShellQuote(exec);   // syntax was already checked, so don't check return value

    /*
     1 = need_shell, 2 = terminal, 4 = su

     0                                                           << split(cmd)
     1                                                           << "sh" << "-c" << cmd
     2 << split(term) << "-e"                                    << split(cmd)
     3 << split(term) << "-e"                                    << "sh" << "-c" << cmd

     4                        << "kdesu" << "-u" << user << "-c" << cmd
     5                        << "kdesu" << "-u" << user << "-c" << ("sh -c " + quote(cmd))
     6 << split(term) << "-e" << "su"            << user << "-c" << cmd
     7 << split(term) << "-e" << "su"            << user << "-c" << ("sh -c " + quote(cmd))

     "sh -c" is needed in the "su" case, too, as su uses the user's login shell, not sh.
     this could be optimized with the -s switch of some su versions (e.g., debian linux).
    */

    if (_service.terminal()) {
        KConfigGroup cg(KGlobal::config(), "General");
        QString terminal = cg.readPathEntry("TerminalApplication", "konsole");
        if (terminal == "konsole") {
            if (!_service.path().isEmpty()) {
                terminal += " --workdir " + KShell::quoteArg(_service.path());
            }
            terminal += " -caption=%c %i %m";
        }
        terminal += ' ';
        terminal += _service.terminalOptions();
        if (!mx1.expandMacrosShellQuote(terminal)) {
            kWarning() << "KRun: syntax error in command" << terminal << ", service" << _service.name();
            return QStringList();
        }
        mx2.expandMacrosShellQuote(terminal);
        result = KShell::splitArgs(terminal);   // assuming that the term spec never needs a shell!
        result << "-e";
    }

    KShell::Errors err;
    QStringList execlist = KShell::splitArgs(exec, KShell::AbortOnMeta | KShell::TildeExpand, &err);
    if (err == KShell::NoError && !execlist.isEmpty()) { // mx1 checked for syntax errors already
        // Resolve the executable to ensure that helpers in lib/kde4/libexec/ are found.
        // Too bad for commands that need a shell - they must reside in $PATH.
        const QString exePath = KStandardDirs::findExe(execlist[0]);
        if (!exePath.isEmpty()) {
            execlist[0] = exePath;
        }
    }
    if (_service.substituteUid()) {
        if (_service.terminal()) {
            result << "su";
        }
        else {
            result << KStandardDirs::findExe("kdesu") << "-u";
        }

        result << _service.username() << "-c";
        if (err == KShell::FoundMeta) {
            exec = "/bin/sh -c " + KShell::quoteArg(exec);
        }
        else {
            exec = KShell::joinArgs(execlist);
        }
        result << exec;
    }
    else {
        if (err == KShell::FoundMeta) {
            result << "/bin/sh" << "-c" << exec;
        }
        else {
            result += execlist;
        }
    }

    return result;
}

//static
QString KRun::binaryName(const QString & execLine, bool removePath)
{
    // Remove parameters and/or trailing spaces.
    const QStringList args = KShell::splitArgs(execLine);
    for (QStringList::ConstIterator it = args.begin(); it != args.end(); ++it)
        if (!(*it).contains('=')) {
            // Remove path if wanted
            return removePath ? (*it).mid((*it).lastIndexOf('/') + 1) : *it;
        }
    return QString();
}

static bool runCommandInternal(KProcess* proc, const KService* service, const QString& executable,
                               const QString &userVisibleName, const QString & iconName, QWidget* window,
                               const QByteArray& asn)
{
    if (window != NULL) {
        window = window->topLevelWidget();
    }
    if (service && !service->entryPath().isEmpty()
            && !KDesktopFile::isAuthorizedDesktopFile(service->entryPath()))
    {
        kWarning() << "No authorization to execute " << service->entryPath();
        KMessageBox::sorry(window, i18n("You are not authorized to execute this file."));
        delete proc;
        return false;
    }

    QString bin = KRun::binaryName(executable, true);
#ifdef Q_WS_X11 // Startup notification doesn't work with QT/E, service isn't needed without Startup notification
    bool silent;
    QByteArray wmclass;
    KStartupInfoId id;
    bool startup_notify = (asn != "0" && KRun::checkStartupNotify(QString() /*unused*/, service, &silent, &wmclass));
    if (startup_notify) {
        id.initId(asn);
        id.setupStartupEnv();
        KStartupInfoData data;
        data.setHostname();
        data.setBin(bin);
        if (!userVisibleName.isEmpty()) {
            data.setName(userVisibleName);
        }
        else if (service && !service->name().isEmpty()) {
            data.setName(service->name());
        }
        data.setDescription(i18n("Launching %1" ,  data.name()));
        if (!iconName.isEmpty()) {
            data.setIcon(iconName);
        }
        else if (service && !service->icon().isEmpty()) {
            data.setIcon(service->icon());
        }
        if (!wmclass.isEmpty()) {
            data.setWMClass(wmclass);
        }
        if (silent) {
            data.setSilent(KStartupInfoData::Yes);
        }
        data.setDesktop(KWindowSystem::currentDesktop());
        if (window) {
            data.setLaunchedBy(window->winId());
        }
        if(service)
            data.setApplicationId(service->entryPath());
        KStartupInfo::sendStartup(id, data);
    }
    int pid = KProcessRunner::run(proc, executable, id);
    if (startup_notify && pid) {
        KStartupInfoData data;
        data.addPid(pid);
        KStartupInfo::sendChange(id, data);
        KStartupInfo::resetStartupEnv();
    }
    return pid != 0;
#else
    Q_UNUSED(userVisibleName);
    Q_UNUSED(iconName);
    return KProcessRunner::run(proc, bin) != 0;
#endif
}

// This code is also used in klauncher.
bool KRun::checkStartupNotify(const QString& /*binName*/, const KService* service, bool* silent_arg, QByteArray* wmclass_arg)
{
    bool silent = false;
    QByteArray wmclass;
    if (service && service->property("StartupNotify").isValid()) {
        silent = !service->property("StartupNotify").toBool();
        wmclass = service->property("StartupWMClass").toString().toLatin1();
    }
    else if (service && service->property("X-KDE-StartupNotify").isValid()) {
        silent = !service->property("X-KDE-StartupNotify").toBool();
        wmclass = service->property("X-KDE-WMClass").toString().toLatin1();
    }
    else { // non-compliant app
        if (service) {
            if (service->isApplication()) { // doesn't have .desktop entries needed, start as non-compliant
                wmclass = "0"; // krazy:exclude=doublequote_chars
            }
            else {
                return false; // no startup notification at all
            }
        }
        else {
#if 0
            // Create startup notification even for apps for which there shouldn't be any,
            // just without any visual feedback. This will ensure they'll be positioned on the proper
            // virtual desktop, and will get user timestamp from the ASN ID.
            wmclass = '0';
            silent = true;
#else   // That unfortunately doesn't work, when the launched non-compliant application
            // launches another one that is compliant and there is any delay inbetween (bnc:#343359)
            return false;
#endif
        }
    }
    if (silent_arg != NULL) {
        *silent_arg = silent;
    }
    if (wmclass_arg != NULL) {
        *wmclass_arg = wmclass;
    }
    return true;
}

static bool runTempService(const KService& _service, const KUrl::List& _urls, QWidget* window,
                           bool tempFiles, const QString& suggestedFileName, const QByteArray& asn)
{
    if (!_urls.isEmpty()) {
        kDebug(7010) << "runTempService: first url " << _urls.first().url();
    }

    QStringList args;
    if ((_urls.count() > 1) && !_service.allowMultipleFiles()) {
        // We need to launch the application N times. That sucks.
        // We ignore the result for application 2 to N.
        // For the first file we launch the application in the
        // usual way. The reported result is based on this
        // application.
        KUrl::List::ConstIterator it = _urls.begin();
        while (++it != _urls.end()) {
            KUrl::List singleUrl;
            singleUrl.append(*it);
            runTempService(_service, singleUrl, window, tempFiles, suggestedFileName, QByteArray());
        }
        KUrl::List singleUrl;
        singleUrl.append(_urls.first());
        args = KRun::processDesktopExec(_service, singleUrl, tempFiles, suggestedFileName);
    }
    else {
        args = KRun::processDesktopExec(_service, _urls, tempFiles, suggestedFileName);
    }
    if (args.isEmpty()) {
        KMessageBox::sorry(window, i18n("Error processing Exec field in %1", _service.entryPath()));
        return false;
    }
    kDebug(7010) << "runTempService: KProcess args=" << args;

    KProcess * proc = new KProcess;
    *proc << args;

    if (!_service.path().isEmpty()) {
        proc->setWorkingDirectory(_service.path());
    }

    return runCommandInternal(proc, &_service, KRun::binaryName(_service.exec(), false),
                              _service.name(), _service.icon(), window, asn);
}

// WARNING: don't call this from processDesktopExec, since klauncher uses that too...
static KUrl::List resolveURLs(const KUrl::List& _urls, const KService& _service)
{
    // Check which protocols the application supports.
    // This can be a list of actual protocol names, or just KIO for KDE apps.
    QStringList supportedProtocols = _service.property("X-KDE-Protocols").toStringList();
    KRunMX1 mx1(_service);
    QString exec = _service.exec();
    if (mx1.expandMacrosShellQuote(exec) && !mx1.hasUrls) {
        Q_ASSERT(supportedProtocols.isEmpty());   // huh? If you support protocols you need %u or %U...
    }
    else {
        if (supportedProtocols.isEmpty()) {
            // compat mode: assume KIO if not set and it's a KDE app
            const QStringList categories = _service.property("Categories").toStringList();
            if (categories.contains("KDE")) {
                supportedProtocols.append("KIO");
            }
            else { // if no KDE app, be a bit over-generic
                supportedProtocols.append("http");
                supportedProtocols.append("ftp");
            }
        }
    }
    kDebug(7010) << "supportedProtocols:" << supportedProtocols;

    KUrl::List urls(_urls);
    if (!supportedProtocols.contains("KIO")) {
        for (KUrl::List::Iterator it = urls.begin(); it != urls.end(); ++it) {
            const KUrl url = *it;
            bool supported = url.isLocalFile() || supportedProtocols.contains(url.protocol().toLower());
            kDebug(7010) << "Looking at url=" << url << " supported=" << supported;
            if (!supported && KProtocolInfo::protocolClass(url.protocol()) == ":local") {
                // Maybe we can resolve to a local URL?
                KUrl localURL = KIO::NetAccess::mostLocalUrl(url, 0);
                if (localURL != url) {
                    *it = localURL;
                    kDebug(7010) << "Changed to " << localURL;
                }
            }
        }
    }
    return urls;
}

// Simple KDialog that resizes the given text edit after being shown to more
// or less fit the enclosed text.
class SecureMessageDialog : public KDialog
{
    public:
    SecureMessageDialog(QWidget *parent) : KDialog(parent), m_textEdit(0)
    {
    }

    void setTextEdit(QPlainTextEdit *textEdit)
    {
        m_textEdit = textEdit;
    }

    protected:
    virtual void showEvent(QShowEvent* e)
    {
        // Now that we're shown, use our width to calculate a good
        // bounding box for the text, and resize m_textEdit appropriately.
        KDialog::showEvent(e);

        if(!m_textEdit)
            return;

        QSize fudge(20, 24); // About what it sounds like :-/

        // Form rect with a lot of height for bounding.  Use no more than
        // 5 lines.
        QRect curRect(m_textEdit->rect());
        QFontMetrics metrics(fontMetrics());
        curRect.setHeight(5 * metrics.lineSpacing());
        curRect.setWidth(qMax(curRect.width(), 300)); // At least 300 pixels ok?

        QString text(m_textEdit->toPlainText());
        curRect = metrics.boundingRect(curRect, Qt::TextWordWrap | Qt::TextSingleLine, text);

        // Scroll bars interfere.  If we don't think there's enough room, enable
        // the vertical scrollbar however.
        m_textEdit->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        if(curRect.height() < m_textEdit->height()) { // then we've got room
            m_textEdit->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
            m_textEdit->setMaximumHeight(curRect.height() + fudge.height());
        }

        m_textEdit->setMinimumSize(curRect.size() + fudge);
        m_textEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
        updateGeometry();
    }

    private:
    QPlainTextEdit *m_textEdit;
};

// Helper function to make the given .desktop file executable by ensuring
// that a #!/usr/bin/env xdg-open line is added if necessary and the file has
// the +x bit set for the user.  Returns false if either fails.
static bool makeFileExecutable(const QString &fileName)
{
    // Open the file and read the first two characters, check if it's
    // #!.  If not, create a new file, prepend appropriate lines, and copy
    // over.
    QFile desktopFile(fileName);
    if (!desktopFile.open(QFile::ReadOnly)) {
        kError(7010) << "Error opening service" << fileName << desktopFile.errorString();
        return false;
    }

    QByteArray header = desktopFile.peek(2);   // First two chars of file
    if (header.size() == 0) {
        kError(7010) << "Error inspecting service" << fileName << desktopFile.errorString();
        return false; // Some kind of error
    }

    if (header != "#!") {
        // Add header
        KSaveFile saveFile;
        saveFile.setFileName(fileName);
        if (!saveFile.open()) {
            kError(7010) << "Unable to open replacement file for" << fileName << saveFile.errorString();
            return false;
        }

        QByteArray shebang("#!/usr/bin/env xdg-open\n");
        if (saveFile.write(shebang) != shebang.size()) {
            kError(7010) << "Error occurred adding header for" << fileName << saveFile.errorString();
            saveFile.abort();
            return false;
        }

        // Now copy the one into the other and then close and reopen desktopFile
        QByteArray desktopData(desktopFile.readAll());
        if (desktopData.isEmpty()) {
            kError(7010) << "Unable to read service" << fileName << desktopFile.errorString();
            saveFile.abort();
            return false;
        }

        if (saveFile.write(desktopData) != desktopData.size()) {
            kError(7010) << "Error copying service" << fileName << saveFile.errorString();
            saveFile.abort();
            return false;
        }

        desktopFile.close();
        if (!saveFile.finalize()) { // Figures....
            kError(7010) << "Error committing changes to service" << fileName << saveFile.errorString();
            return false;
        }

        if (!desktopFile.open(QFile::ReadOnly)) {
            kError(7010) << "Error re-opening service" << fileName << desktopFile.errorString();
            return false;
        }
    } // Add header

    // corresponds to owner on unix, which will have to do since if the user
    // isn't the owner we can't change perms anyways.
    if (!desktopFile.setPermissions(QFile::ExeUser | desktopFile.permissions())) {
        kError(7010) << "Unable to change permissions for" << fileName << desktopFile.errorString();
        return false;
    }

    // whew
    return true;
}

// Helper function to make a .desktop file executable if prompted by the user.
// returns true if KRun::run() should continue with execution, false if user declined
// to make the file executable or we failed to make it executable.
static bool makeServiceExecutable(const KService& service, QWidget* window)
{
    if (!KAuthorized::authorize("run_desktop_files")) {
        kWarning() << "No authorization to execute " << service.entryPath();
        KMessageBox::sorry(window, i18n("You are not authorized to execute this service."));
        return false; // Don't circumvent the Kiosk
    }

    KGuiItem continueItem = KStandardGuiItem::cont();

    SecureMessageDialog *baseDialog = new SecureMessageDialog(window);

    baseDialog->setButtons(KDialog::Ok | KDialog::Cancel);
    baseDialog->setButtonGuiItem(KDialog::Ok, continueItem);
    baseDialog->setDefaultButton(KDialog::Cancel);
    baseDialog->setButtonFocus(KDialog::Cancel);
    baseDialog->setCaption(i18nc("Warning about executing unknown .desktop file", "Warning"));

    // Dialog will have explanatory text with a disabled lineedit with the
    // Exec= to make it visually distinct.
    QWidget *baseWidget = new QWidget(baseDialog);
    QHBoxLayout *mainLayout = new QHBoxLayout(baseWidget);

    QLabel *iconLabel = new QLabel(baseWidget);
    QPixmap warningIcon(KIconLoader::global()->loadIcon("dialog-warning", KIconLoader::NoGroup, KIconLoader::SizeHuge));
    mainLayout->addWidget(iconLabel);
    iconLabel->setPixmap(warningIcon);

    QVBoxLayout *contentLayout = new QVBoxLayout;
    QString warningMessage = i18nc("program name follows in a line edit below",
                                   "This will start the program:");

    QLabel *message = new QLabel(warningMessage, baseWidget);
    contentLayout->addWidget(message);

    // We can use KStandardDirs::findExe to resolve relative pathnames
    // but that gets rid of the command line arguments.
    QString program = KStandardDirs::realFilePath(service.exec());

    QPlainTextEdit *textEdit = new QPlainTextEdit(baseWidget);
    textEdit->setPlainText(program);
    textEdit->setReadOnly(true);
    contentLayout->addWidget(textEdit);

    QLabel *footerLabel = new QLabel(i18n("If you do not trust this program, click Cancel"));
    contentLayout->addWidget(footerLabel);
    contentLayout->addStretch(0); // Don't allow the text edit to expand

    mainLayout->addLayout(contentLayout);

    baseDialog->setMainWidget(baseWidget);
    baseDialog->setTextEdit(textEdit);

    // Constrain maximum size.  Minimum size set in
    // the dialog's show event.
    QSize screenSize = QApplication::desktop()->screen()->size();
    baseDialog->resize(screenSize.width() / 4, 50);
    baseDialog->setMaximumHeight(screenSize.height() / 3);
    baseDialog->setMaximumWidth(screenSize.width() / 10 * 8);

    int result = baseDialog->exec();
    if (result != KDialog::Accepted) {
        return false;
    }

    // Assume that service is an absolute path since we're being called (relative paths
    // would have been allowed unless Kiosk said no, therefore we already know where the
    // .desktop file is.  Now add a header to it if it doesn't already have one
    // and add the +x bit.

    if (!::makeFileExecutable(service.entryPath())) {
        QString serviceName = service.name();
        if(serviceName.isEmpty())
            serviceName = service.genericName();

        KMessageBox::sorry(
            window,
            i18n("Unable to make the service %1 executable, aborting execution", serviceName)
        );

        return false;
    }

    return true;
}

bool KRun::run(const KService& _service, const KUrl::List& _urls, QWidget* window,
               bool tempFiles, const QString& suggestedFileName, const QByteArray& asn)
{
    if (!_service.entryPath().isEmpty() &&
            !KDesktopFile::isAuthorizedDesktopFile(_service.entryPath()) &&
            !::makeServiceExecutable(_service, window))
    {
        return false;
    }

    if (!tempFiles) {
        // Remember we opened those urls, for the "recent documents" menu in kicker
        KUrl::List::ConstIterator it = _urls.begin();
        for (; it != _urls.end(); ++it) {
            //kDebug(7010) << "KRecentDocument::adding " << (*it).url();
            KRecentDocument::add(*it, _service.desktopEntryName());
        }
    }

    if (tempFiles || _service.entryPath().isEmpty() || !suggestedFileName.isEmpty()) {
        return runTempService(_service, _urls, window, tempFiles, suggestedFileName, asn);
    }

    kDebug(7010) << "KRun::run " << _service.entryPath();

    if (!_urls.isEmpty()) {
        kDebug(7010) << "First url " << _urls.first().url();
    }

    // Resolve urls if needed, depending on what the app supports
    const KUrl::List urls = resolveURLs(_urls, _service);

    QString error;
    int pid = 0;

    QByteArray myasn = asn;
    // startServiceByDesktopPath() doesn't take QWidget*, add it to the startup info now
    if (window != NULL) {
        if (myasn.isEmpty()) {
            myasn = KStartupInfo::createNewStartupId();
        }
        if (myasn != "0") {
            KStartupInfoId id;
            id.initId(myasn);
            KStartupInfoData data;
            data.setLaunchedBy(window->winId());
            KStartupInfo::sendChange(id, data);
        }
    }

    int i = KToolInvocation::startServiceByDesktopPath(
                _service.entryPath(), urls.toStringList(), &error, 0L, &pid, myasn
            );

    if (i != 0) {
        kDebug(7010) << error;
        KMessageBox::sorry(window, error);
        return false;
    }

    kDebug(7010) << "startServiceByDesktopPath worked fine";
    return true;
}


bool KRun::run(const QString& _exec, const KUrl::List& _urls, QWidget* window, const QString& _name,
               const QString& _icon, const QByteArray& asn)
{
    KService::Ptr service(new KService(_name, _exec, _icon));

    return run(*service, _urls, window, false, QString(), asn);
}

bool KRun::runCommand(const QString &cmd, QWidget* window)
{
    return runCommand(cmd, window, QString());
}

bool KRun::runCommand(const QString& cmd, QWidget* window, const QString& workingDirectory)
{
    if (cmd.isEmpty()) {
        kWarning() << "Command was empty, nothing to run";
        return false;
    }
    const QString bin = KShell::splitArgs(cmd).first();
    return KRun::runCommand(cmd, bin, bin /*iconName*/, window, QByteArray(), workingDirectory);
}

bool KRun::runCommand(const QString& cmd, const QString &execName, const QString & iconName, QWidget* window, const QByteArray& asn)
{
    return runCommand(cmd, execName, iconName, window, asn, QString());
}

bool KRun::runCommand(const QString& cmd, const QString &execName, const QString & iconName,
                      QWidget* window, const QByteArray& asn, const QString& workingDirectory)
{
    kDebug(7010) << "runCommand " << cmd << "," << execName;
    KProcess * proc = new KProcess;
    proc->setShellCommand(cmd);
    if (workingDirectory.isEmpty()) {
        // see bug 108510, and we need "alt+f2 editor" (which starts a desktop file via klauncher)
        // and "alt+f2 editor -someoption" (which calls runCommand) to be consistent.
        proc->setWorkingDirectory(KGlobalSettings::documentPath());
    } else {
        proc->setWorkingDirectory(workingDirectory);
    }
    QString bin = binaryName(execName, true);
    KService::Ptr service = KService::serviceByDesktopName(bin);
    return runCommandInternal(proc, service.data(),
                              execName /*executable to check for in slotProcessExited*/,
                              execName /*user-visible name*/,
                              iconName, window, asn);
}

KRun::KRun(const KUrl& url, QWidget* window, mode_t mode, bool isLocalFile,
           bool showProgressInfo, const QByteArray& asn)
        : d(new KRunPrivate(this))
{
    d->m_timer.setObjectName("KRun::timer");
    d->m_timer.setSingleShot(true);
    d->init(url, window, mode, isLocalFile, showProgressInfo, asn);
}

void KRun::KRunPrivate::init(const KUrl& url, QWidget* window, mode_t mode, bool isLocalFile,
                             bool showProgressInfo, const QByteArray& asn)
{
    m_bFault = false;
    m_bAutoDelete = true;
    m_bProgressInfo = showProgressInfo;
    m_bFinished = false;
    m_job = 0L;
    m_strURL = url;
    m_bScanFile = false;
    m_bIsDirectory = false;
    m_bIsLocalFile = isLocalFile;
    m_mode = mode;
    m_runExecutables = true;
    m_window = window;
    m_asn = asn;
    q->setEnableExternalBrowser(true);

    // Start the timer. This means we will return to the event
    // loop and do initialization afterwards.
    // Reason: We must complete the constructor before we do anything else.
    m_bInit = true;
    q->connect(&m_timer, SIGNAL(timeout()), q, SLOT(slotTimeout()));
    startTimer();
    //kDebug(7010) << "new KRun" << q << url << "timer=" << &m_timer;

    KGlobal::ref();
}

void KRun::init()
{
    kDebug(7010) << "INIT called";
    if (!d->m_strURL.isValid()) {
        // TODO KDE5: call virtual method on error (see BrowserRun::init)
        d->m_showingDialog = true;
        KMessageBoxWrapper::error(d->m_window, i18n("Malformed URL\n%1", d->m_strURL.url()));
        d->m_showingDialog = false;
        d->m_bFault = true;
        d->m_bFinished = true;
        d->startTimer();
        return;
    }
    if (!KAuthorized::authorizeUrlAction("open", KUrl(), d->m_strURL)) {
        QString msg = KIO::buildErrorString(KIO::ERR_ACCESS_DENIED, d->m_strURL.prettyUrl());
        d->m_showingDialog = true;
        KMessageBoxWrapper::error(d->m_window, msg);
        d->m_showingDialog = false;
        d->m_bFault = true;
        d->m_bFinished = true;
        d->startTimer();
        return;
    }

    if (!d->m_bIsLocalFile && d->m_strURL.isLocalFile()) {
        d->m_bIsLocalFile = true;
    }

    QString exec;
    if (d->m_strURL.protocol().startsWith(QLatin1String("http"))) {
        exec = d->m_externalBrowser;
    }

    if (d->m_bIsLocalFile) {
        if (d->m_mode == 0) {
            KDE_struct_stat buff;
            if (KDE::stat(d->m_strURL.path(), &buff) == -1) {
                d->m_showingDialog = true;
                KMessageBoxWrapper::error(d->m_window,
                                          i18n("<qt>Unable to run the command specified. "
                                          "The file or folder <b>%1</b> does not exist.</qt>" ,
                                          Qt::escape(d->m_strURL.prettyUrl())));
                d->m_showingDialog = false;
                d->m_bFault = true;
                d->m_bFinished = true;
                d->startTimer();
                return;
            }
            d->m_mode = buff.st_mode;
        }

        KMimeType::Ptr mime = KMimeType::findByUrl(d->m_strURL, d->m_mode, d->m_bIsLocalFile);
        assert(mime);
        kDebug(7010) << "MIME TYPE is " << mime->name();
        mimeTypeDetermined(mime->name());
        return;
    }
    else if (!exec.isEmpty() || KProtocolInfo::isHelperProtocol(d->m_strURL)) {
        kDebug(7010) << "Helper protocol";

        bool ok = false;
        KUrl::List urls;
        urls.append(d->m_strURL);
        if (exec.isEmpty()) {
            exec = KProtocolInfo::exec(d->m_strURL.protocol());
            if (exec.isEmpty()) {
                mimeTypeDetermined(KProtocolManager::defaultMimetype(d->m_strURL));
                return;
            }
            run(exec, urls, d->m_window, false, QString(), d->m_asn);
            ok = true;
        }
        else if (exec.startsWith('!')) {
            exec = exec.mid(1); // Literal command
            exec += " %u";
            run(exec, urls, d->m_window, false, QString(), d->m_asn);
            ok = true;
        }
        else {
            KService::Ptr service = KService::serviceByStorageId(exec);
            if (service) {
                run(*service, urls, d->m_window, false, QString(), d->m_asn);
                ok = true;
            }
        }

        if (ok) {
            d->m_bFinished = true;
            // will emit the error and autodelete this
            d->startTimer();
            return;
        }
    }

    // Did we already get the information that it is a directory ?
    if (S_ISDIR(d->m_mode)) {
        mimeTypeDetermined("inode/directory");
        return;
    }

    // Let's see whether it is a directory

    if (!KProtocolManager::supportsListing(d->m_strURL)) {
        //kDebug(7010) << "Protocol has no support for listing";
        // No support for listing => it can't be a directory (example: http)
        scanFile();
        return;
    }

    kDebug(7010) << "Testing directory (stating)";

    // It may be a directory or a file, let's stat
    KIO::JobFlags flags = d->m_bProgressInfo ? KIO::DefaultFlags : KIO::HideProgressInfo;
    KIO::StatJob *job = KIO::stat(d->m_strURL, KIO::StatJob::SourceSide, 0 /* no details */, flags);
    job->ui()->setWindow(d->m_window);
    connect(job, SIGNAL(result(KJob *)),
            this, SLOT(slotStatResult(KJob *)));
    d->m_job = job;
    kDebug(7010) << " Job " << job << " is about stating " << d->m_strURL.url();
}

KRun::~KRun()
{
    //kDebug(7010) << this;
    d->m_timer.stop();
    killJob();
    KGlobal::deref();
    //kDebug(7010) << this << "done";
    delete d;
}

void KRun::scanFile()
{
    kDebug(7010) << d->m_strURL;
    // First, let's check for well-known extensions
    // Not when there is a query in the URL, in any case.
    if (d->m_strURL.query().isEmpty()) {
        KMimeType::Ptr mime = KMimeType::findByUrl(d->m_strURL);
        assert(mime);
        if (!mime->isDefault() || d->m_bIsLocalFile) {
            kDebug(7010) << "Scanfile: MIME TYPE is " << mime->name();
            mimeTypeDetermined(mime->name());
            return;
        }
    }

    // No mimetype found, and the URL is not local  (or fast mode not allowed).
    // We need to apply the 'KIO' method, i.e. either asking the server or
    // getting some data out of the file, to know what mimetype it is.

    if (!KProtocolManager::supportsReading(d->m_strURL)) {
        kError(7010) << "#### NO SUPPORT FOR READING!";
        d->m_bFault = true;
        d->m_bFinished = true;
        d->startTimer();
        return;
    }
    kDebug(7010) << this << " Scanning file " << d->m_strURL.url();

    KIO::JobFlags flags = d->m_bProgressInfo ? KIO::DefaultFlags : KIO::HideProgressInfo;
    KIO::TransferJob *job = KIO::get(d->m_strURL, KIO::NoReload /*reload*/, flags);
    job->ui()->setWindow(d->m_window);
    connect(job, SIGNAL(result(KJob *)),
            this, SLOT(slotScanFinished(KJob *)));
    connect(job, SIGNAL(mimetype(KIO::Job *, const QString &)),
            this, SLOT(slotScanMimeType(KIO::Job *, const QString &)));
    d->m_job = job;
    kDebug(7010) << " Job " << job << " is about getting from " << d->m_strURL.url();
}

// When arriving in that method there are 5 possible states:
// must_init, must_scan_file, found_dir, done+error or done+success.
void KRun::slotTimeout()
{
    kDebug(7010) << this << " slotTimeout called";
    if (d->m_bInit) {
        d->m_bInit = false;
        init();
        return;
    }

    if (d->m_bFault) {
        emit error();
    }
    if (d->m_bFinished) {
        emit finished();
    }
    else {
        if (d->m_bScanFile) {
            d->m_bScanFile = false;
            scanFile();
            return;
        }
        else if (d->m_bIsDirectory) {
            d->m_bIsDirectory = false;
            mimeTypeDetermined("inode/directory");
            return;
        }
    }

    if (d->m_bAutoDelete) {
        deleteLater();
        return;
    }
}

void KRun::slotStatResult(KJob * job)
{
    d->m_job = 0L;
    if (job->error()) {
        d->m_showingDialog = true;
        kError(7010) << this << "ERROR" << job->error() << job->errorString();
        job->uiDelegate()->showErrorMessage();
        //kDebug(7010) << this << " KRun returning from showErrorDialog, starting timer to delete us";
        d->m_showingDialog = false;

        d->m_bFault = true;
        d->m_bFinished = true;

        // will emit the error and autodelete this
        d->startTimer();

    }
    else {

        kDebug(7010) << "Finished";
        if (!qobject_cast<KIO::StatJob*>(job)) {
            kFatal() << "job is a " << typeid(*job).name() << " should be a StatJob";
        }

        const KIO::UDSEntry entry = ((KIO::StatJob*)job)->statResult();
        const mode_t mode = entry.numberValue(KIO::UDSEntry::UDS_FILE_TYPE);
        if (S_ISDIR(mode)) {
            d->m_bIsDirectory = true; // it's a dir
        }
        else {
            d->m_bScanFile = true; // it's a file
        }

        d->m_localPath = entry.stringValue(KIO::UDSEntry::UDS_LOCAL_PATH);

        // mimetype already known? (e.g. print:/manager)
        const QString knownMimeType = entry.stringValue(KIO::UDSEntry::UDS_MIME_TYPE) ;

        if (!knownMimeType.isEmpty()) {
            mimeTypeDetermined(knownMimeType);
            d->m_bFinished = true;
        }

        // We should have found something
        assert(d->m_bScanFile || d->m_bIsDirectory);

        // Start the timer. Once we get the timer event this
        // protocol server is back in the pool and we can reuse it.
        // This gives better performance than starting a new slave
        d->startTimer();
    }
}

void KRun::slotScanMimeType(KIO::Job *, const QString &mimetype)
{
    if (mimetype.isEmpty()) {
        kWarning(7010) << "get() didn't emit a mimetype! Probably a kioslave bug, please check the implementation of" << url().protocol();
    }
    mimeTypeDetermined(mimetype);
    d->m_job = 0;
}

void KRun::slotScanFinished(KJob *job)
{
    d->m_job = 0;
    if (job->error()) {
        d->m_showingDialog = true;
        kError(7010) << this << "ERROR (stat):" << job->error() << ' ' << job->errorString();
        job->uiDelegate()->showErrorMessage();
        //kDebug(7010) << this << " KRun returning from showErrorDialog, starting timer to delete us";
        d->m_showingDialog = false;

        d->m_bFault = true;
        d->m_bFinished = true;

        // will emit the error and autodelete this
        d->startTimer();
    }
}

void KRun::mimeTypeDetermined(const QString& mimeType)
{
    // foundMimeType reimplementations might show a dialog box;
    // make sure some timer doesn't kill us meanwhile (#137678, #156447)
    Q_ASSERT(!d->m_showingDialog);
    d->m_showingDialog = true;

    foundMimeType(mimeType);

    d->m_showingDialog = false;

    // We cannot assume that we're finished here. Some reimplementations
    // start a KIO job and call setFinished only later.
}

void KRun::foundMimeType(const QString& type)
{
    kDebug(7010) << "Resulting mime type is " << type;

    KIO::TransferJob *job = qobject_cast<KIO::TransferJob *>(d->m_job);
    if (job) {
        // Update our URL in case of a redirection
        setUrl( job->url() );

        job->putOnHold();
        KIO::Scheduler::publishSlaveOnHold();
        d->m_job = 0;
    }

    Q_ASSERT(!d->m_bFinished);

    KMimeType::Ptr mime = KMimeType::mimeType(type, KMimeType::ResolveAliases);
    if (!mime) {
        kWarning(7010) << "Unknown mimetype " << type;
    }

    // Support for preferred service setting, see setPreferredService
    if (!d->m_preferredService.isEmpty()) {
        kDebug(7010) << "Attempting to open with preferred service: " << d->m_preferredService;
        KService::Ptr serv = KService::serviceByDesktopName(d->m_preferredService);
        if (serv && serv->hasMimeType(mime.data())) {
            KUrl::List lst;
            lst.append(d->m_strURL);
            if (KRun::run(*serv, lst, d->m_window, false, QString(), d->m_asn)) {
                setFinished(true);
                return;
            }
            /// Note: if that service failed, we'll go to runUrl below to
            /// maybe find another service, even though an error dialog box was
            /// already displayed. That's good if runUrl tries another service,
            /// but it's not good if it tries the same one :}
        }
    }

    // Resolve .desktop files from media:/, remote:/, applications:/ etc.
    if (mime && mime->is("application/x-desktop") && !d->m_localPath.isEmpty()) {
        d->m_strURL = KUrl();
        d->m_strURL.setPath(d->m_localPath);
    }

    if (!KRun::runUrl(d->m_strURL, type, d->m_window, false /*tempfile*/, d->m_runExecutables, d->m_suggestedFileName, d->m_asn)) {
        d->m_bFault = true;
    }
    setFinished(true);
}

void KRun::killJob()
{
    if (d->m_job) {
        kDebug(7010) << this << "m_job=" << d->m_job;
        d->m_job->kill();
        d->m_job = 0L;
    }
}

void KRun::abort()
{
    if (d->m_bFinished) {
        return;
    }
    kDebug(7010) << this << "m_showingDialog=" << d->m_showingDialog;
    killJob();
    // If we're showing an error message box, the rest will be done
    // after closing the msgbox -> don't autodelete nor emit signals now.
    if (d->m_showingDialog) {
        return;
    }
    d->m_bFault = true;
    d->m_bFinished = true;
    d->m_bInit = false;
    d->m_bScanFile = false;

    // will emit the error and autodelete this
    d->startTimer();
}

bool KRun::hasError() const
{
    return d->m_bFault;
}

bool KRun::hasFinished() const
{
    return d->m_bFinished;
}

bool KRun::autoDelete() const
{
    return d->m_bAutoDelete;
}

void KRun::setAutoDelete(bool b)
{
    d->m_bAutoDelete = b;
}

void KRun::setEnableExternalBrowser(bool b)
{
    if (b) {
        d->m_externalBrowser = KConfigGroup(KGlobal::config(), "General").readEntry("BrowserApplication");
    }
    else {
        d->m_externalBrowser.clear();
    }
}

void KRun::setPreferredService(const QString& desktopEntryName)
{
    d->m_preferredService = desktopEntryName;
}

void KRun::setRunExecutables(bool b)
{
    d->m_runExecutables = b;
}

void KRun::setSuggestedFileName(const QString& fileName)
{
    d->m_suggestedFileName = fileName;
}

QString KRun::suggestedFileName() const
{
    return d->m_suggestedFileName;
}

bool KRun::isExecutable(const QString& serviceType)
{
    return (serviceType == "application/x-desktop" ||
            serviceType == "application/x-executable" ||
            serviceType == "application/x-ms-dos-executable" ||
            serviceType == "application/x-shellscript");
}

void KRun::setUrl(const KUrl &url)
{
    d->m_strURL = url;
}

KUrl KRun::url() const
{
    return d->m_strURL;
}

void KRun::setError(bool error)
{
    d->m_bFault = error;
}

void KRun::setProgressInfo(bool progressInfo)
{
    d->m_bProgressInfo = progressInfo;
}

bool KRun::progressInfo() const
{
    return d->m_bProgressInfo;
}

void KRun::setFinished(bool finished)
{
    d->m_bFinished = finished;
    if (finished)
        d->startTimer();
}

void KRun::setJob(KIO::Job *job)
{
    d->m_job = job;
}

KIO::Job* KRun::job()
{
    return d->m_job;
}

QTimer& KRun::timer()
{
    return d->m_timer;
}

void KRun::setDoScanFile(bool scanFile)
{
    d->m_bScanFile = scanFile;
}

bool KRun::doScanFile() const
{
    return d->m_bScanFile;
}

void KRun::setIsDirecory(bool isDirectory)
{
    d->m_bIsDirectory = isDirectory;
}

bool KRun::isDirectory() const
{
    return d->m_bIsDirectory;
}

void KRun::setInitializeNextAction(bool initialize)
{
    d->m_bInit = initialize;
}

bool KRun::initializeNextAction() const
{
    return d->m_bInit;
}

void KRun::setIsLocalFile(bool isLocalFile)
{
    d->m_bIsLocalFile = isLocalFile;
}

bool KRun::isLocalFile() const
{
    return d->m_bIsLocalFile;
}

void KRun::setMode(mode_t mode)
{
    d->m_mode = mode;
}

mode_t KRun::mode() const
{
    return d->m_mode;
}

/****************/

#ifndef Q_WS_X11
int KProcessRunner::run(KProcess * p, const QString & executable)
{
    return (new KProcessRunner(p, executable))->pid();
}
#else
int KProcessRunner::run(KProcess * p, const QString & executable, const KStartupInfoId& id)
{
    return (new KProcessRunner(p, executable, id))->pid();
}
#endif

#ifndef Q_WS_X11
KProcessRunner::KProcessRunner(KProcess * p, const QString & executable)
#else
KProcessRunner::KProcessRunner(KProcess * p, const QString & executable, const KStartupInfoId& _id) :
        id(_id)
#endif
{
    m_pid = 0;
    process = p;
    m_executable = executable;
    connect(process, SIGNAL(finished(int, QProcess::ExitStatus)),
            this, SLOT(slotProcessExited(int, QProcess::ExitStatus)));

    process->start();
    if (!process->waitForStarted()) {
        //kDebug() << "wait for started failed, exitCode=" << process->exitCode()
        //         << "exitStatus=" << process->exitStatus();
        // Note that exitCode is 255 here (the first time), and 0 later on (bug?).
        slotProcessExited(255, process->exitStatus());
    }
    else {
#ifdef Q_WS_X11
        m_pid = process->pid();
#endif
    }
}

KProcessRunner::~KProcessRunner()
{
    delete process;
}

int KProcessRunner::pid() const
{
    return m_pid;
}

void KProcessRunner::terminateStartupNotification()
{
#ifdef Q_WS_X11
    if (!id.none()) {
        KStartupInfoData data;
        data.addPid(m_pid); // announce this pid for the startup notification has finished
        data.setHostname();
        KStartupInfo::sendFinish(id, data);
    }
#endif

}

void
KProcessRunner::slotProcessExited(int exitCode, QProcess::ExitStatus exitStatus)
{
    kDebug(7010) << m_executable << "exitCode=" << exitCode << "exitStatus=" << exitStatus;
    Q_UNUSED(exitStatus);

    terminateStartupNotification(); // do this before the messagebox
    if (exitCode != 0 && !m_executable.isEmpty()) {
        // Let's see if the error is because the exe doesn't exist.
        // When this happens, waitForStarted returns false, but not if kioexec
        // was involved, then we come here, that's why the code is here.
        //
        // We'll try to find the executable relatively to current directory,
        // (or with a full path, if m_executable is absolute), and then in the PATH.
        if (!QFile(m_executable).exists() && KStandardDirs::findExe(m_executable).isEmpty()) {
            KGlobal::ref();
            KMessageBox::sorry(0L, i18n("Could not find the program '%1'", m_executable));
            KGlobal::deref();
        }
        else {
            kDebug() << process->readAllStandardError();
        }
    }
    deleteLater();
}

#include "krun.moc"
#include "krun_p.moc"
