/* This file is part of the KDE libraries
    Copyright (c) 1997,1998 Matthias Kalle Dalheimer <kalle@kde.org>
    Copyright (c) 1999 Espen Sand <espen@kde.org>
    Copyright (c) 2000-2004 Frerich Raabe <raabe@kde.org>
    Copyright (c) 2003,2004 Oswald Buddenhagen <ossi@kde.org>
    Copyright (c) 2006 Thiago Macieira <thiago@kde.org>
    Copyright (C) 2008 Aaron Seigo <aseigo@kde.org>

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

#include <config.h>

#include "ktoolinvocation.h"

#include <kconfiggroup.h>

#include "klauncher_iface.h"
#include "kcmdlineargs.h"
#include "kconfig.h"
#include "kcodecs.h"
#include "kdebug.h"
#include "kglobal.h"
#include "kshell.h"
#include "kmacroexpander.h"
#include "klocale.h"
#include "kstandarddirs.h"
#include "kmessage.h"

#include <QtCore/QCoreApplication>
#include <QtCore/QProcess>
#include <QtCore/QHash>
#include <QtCore/QDebug>
#include <QtCore/QBool>
#include <QtCore/QFile>
#include <QtDBus/QtDBus>

static QStringList splitEmailAddressList( const QString & aStr )
{
    // This is a copy of KPIM::splitEmailAddrList().
    // Features:
    // - always ignores quoted characters
    // - ignores everything (including parentheses and commas)
    //   inside quoted strings
    // - supports nested comments
    // - ignores everything (including double quotes and commas)
    //   inside comments

    QStringList list;

    if (aStr.isEmpty())
        return list;

    QString addr;
    uint addrstart = 0;
    int commentlevel = 0;
    bool insidequote = false;

    for (int index=0; index<aStr.length(); index++) {
        // the following conversion to latin1 is o.k. because
        // we can safely ignore all non-latin1 characters
        switch (aStr[index].toLatin1()) {
        case '"' : // start or end of quoted string
            if (commentlevel == 0)
                insidequote = !insidequote;
            break;
        case '(' : // start of comment
            if (!insidequote)
                commentlevel++;
            break;
        case ')' : // end of comment
            if (!insidequote) {
                if (commentlevel > 0)
                    commentlevel--;
                else {
                    //kDebug() << "Error in address splitting: Unmatched ')'"
                    //          << endl;
                    return list;
                }
            }
            break;
        case '\\' : // quoted character
            index++; // ignore the quoted character
            break;
        case ',' :
            if (!insidequote && (commentlevel == 0)) {
                addr = aStr.mid(addrstart, index-addrstart);
                if (!addr.isEmpty())
                    list += addr.simplified();
                addrstart = index+1;
            }
            break;
        }
    }
    // append the last address to the list
    if (!insidequote && (commentlevel == 0)) {
        addr = aStr.mid(addrstart, aStr.length()-addrstart);
        if (!addr.isEmpty())
            list += addr.simplified();
    }
    //else
    //  kDebug() << "Error in address splitting: "
    //            << "Unexpected end of address list"
    //            << endl;

    return list;
}

void KToolInvocation::invokeMailer(const QString &_to, const QString &_cc, const QString &_bcc,
                                   const QString &subject, const QString &body,
                                   const QString & /*messageFile TODO*/, const QStringList &attachURLs,
                                   const QByteArray& startup_id )
{
    if (!isMainThreadActive())
        return;

    KConfig config("emaildefaults");
    KConfigGroup defaultsGrp(&config, "Defaults");

    QString group = defaultsGrp.readEntry("Profile","Default");

    KConfigGroup profileGrp(&config, QString("PROFILE_%1").arg(group) );
    QString command = profileGrp.readPathEntry("EmailClient", QString());

    QString to, cc, bcc;
    if (command.isEmpty() || command == QLatin1String("kmail")
        || command.endsWith("/kmail"))
    {
        command = QLatin1String("kmail --composer -s %s -c %c -b %b --body %B --attach %A -- %t");
        if ( !_to.isEmpty() )
        {
            // put the whole address lists into RFC2047 encoded blobs; technically
            // this isn't correct, but KMail understands it nonetheless
            to = QString( "=?utf8?b?%1?=" ).arg( _to.toUtf8().toBase64().constData() );
        }
        if ( !_cc.isEmpty() )
            cc = QString( "=?utf8?b?%1?=" ).arg( _cc.toUtf8().toBase64().constData() );
        if ( !_bcc.isEmpty() )
            bcc = QString( "=?utf8?b?%1?=" ).arg( _bcc.toUtf8().toBase64().constData() );
    } else {
        to = _to;
        cc = _cc;
        bcc = _bcc;
        if( !command.contains( '%' ))
            command += " %u";
    }

    if (profileGrp.readEntry("TerminalClient", false))
    {
        KConfigGroup confGroup( KGlobal::config(), "General" );
        QString preferredTerminal = confGroup.readPathEntry("TerminalApplication", "konsole");
        command = preferredTerminal + " -e " + command;
    }

    QStringList cmdTokens = KShell::splitArgs(command);
    QString cmd = cmdTokens.takeFirst();

    KUrl url;
    //QStringList qry;
    if (!to.isEmpty())
    {
        QStringList tos = splitEmailAddressList( to );
        url.setPath( tos.first() );
        tos.erase( tos.begin() );
        for (QStringList::ConstIterator it = tos.constBegin(); it != tos.constEnd(); ++it)
            url.addQueryItem("to",*it);
        //qry.append( "to=" + QLatin1String(KUrl::toPercentEncoding( *it ) ));
    }
    const QStringList ccs = splitEmailAddressList( cc );
    for (QStringList::ConstIterator it = ccs.constBegin(); it != ccs.constEnd(); ++it)
        url.addQueryItem("cc",*it);
    //qry.append( "cc=" + QLatin1String(KUrl::toPercentEncoding( *it ) ));
    const QStringList bccs = splitEmailAddressList( bcc );
    for (QStringList::ConstIterator it = bccs.constBegin(); it != bccs.constEnd(); ++it)
        url.addQueryItem("bcc",*it);
    //qry.append( "bcc=" + QLatin1String(KUrl::toPercentEncoding( *it ) ));
    for (QStringList::ConstIterator it = attachURLs.constBegin(); it != attachURLs.constEnd(); ++it)
        url.addQueryItem("attach",*it);
    //qry.append( "attach=" + QLatin1String(KUrl::toPercentEncoding( *it ) ));
    if (!subject.isEmpty())
        url.addQueryItem("subject",subject);
    //qry.append( "subject=" + QLatin1String(KUrl::toPercentEncoding( subject ) ));
    if (!body.isEmpty())
        url.addQueryItem("body",body);
    //qry.append( "body=" + QLatin1String(KUrl::toPercentEncoding( body ) ));
    //url.setQuery( qry.join( "&" ) );

    if ( ! (to.isEmpty() && (!url.hasQuery())) )
        url.setProtocol("mailto");

    QHash<QChar, QString> keyMap;
    keyMap.insert('t', to);
    keyMap.insert('s', subject);
    keyMap.insert('c', cc);
    keyMap.insert('b', bcc);
    keyMap.insert('B', body);
    keyMap.insert('u', url.url());

    QString attachlist = attachURLs.join(",");
    attachlist.prepend('\'');
    attachlist.append('\'');
    keyMap.insert('A', attachlist);

    for (QStringList::Iterator it = cmdTokens.begin(); it != cmdTokens.end(); )
    {
        if (*it == "%A")
        {
            if (it == cmdTokens.begin()) // better safe than sorry ...
                continue;
            QStringList::ConstIterator urlit = attachURLs.begin();
            QStringList::ConstIterator urlend = attachURLs.end();
            if ( urlit != urlend )
            {
                QStringList::Iterator previt = it;
                --previt;
                *it = *urlit;
                ++it;
                while ( ++urlit != urlend )
                {
                    cmdTokens.insert( it, *previt );
                    cmdTokens.insert( it, *urlit );
                }
            } else {
                --it;
                it = cmdTokens.erase( cmdTokens.erase( it ) );
            }
        } else {
            *it = KMacroExpander::expandMacros(*it, keyMap);
            ++it;
        }
    }

    QString error;
    // TODO this should check if cmd has a .desktop file, and use data from it, together
    // with sending more ASN data
    if (kdeinitExec(cmd, cmdTokens, &error, NULL, startup_id ))
    {
      KMessage::message(KMessage::Error,
                      i18n("Could not launch the mail client:\n\n%1", error),
                      i18n("Could not Launch Mail Client"));
    }
}

void KToolInvocation::invokeBrowser( const QString &url, const QByteArray& startup_id )
{
    if (!isMainThreadActive())
        return;

    QStringList args;
    args << url;

    // This method should launch a webbrowser, preferrably without doing a mimetype
    // check first, like KRun (i.e. kde-open) would do.

    // In a KDE session, call kfmclient (which honors BrowserApplication) if present,
    // otherwise xdg-open, otherwise kde-open (which does a mimetype check first though).

    // Outside KDE, call xdg-open if present, otherwise fallback to the above logic.

    QString exe; // the binary we are going to launch.

    const QString xdg_open = KStandardDirs::findExe("xdg-open");
    if (qgetenv("KDE_FULL_SESSION").isEmpty()) {
        exe = xdg_open;
    }

    if (exe.isEmpty()) {
        const QString kfmclient = KStandardDirs::findExe("kfmclient");
        if (!kfmclient.isEmpty()) {
            exe = kfmclient;
            args.prepend("openURL");
        } else
            exe = xdg_open;
    }

    if (exe.isEmpty()) {
        exe = "kde-open"; // it's from kdebase-runtime, it has to be there.
    }

    kDebug() << "Using" << exe << "to open" << url;
    QString error;
    if (kdeinitExec(exe, args, &error, NULL, startup_id ))
    {
        KMessage::message(KMessage::Error,
                          // TODO: i18n("Could not launch %1:\n\n%2", exe, error),
                          i18n("Could not launch the browser:\n\n%1", error),
                          i18n("Could not Launch Browser"));
    }
}

void KToolInvocation::invokeTerminal(const QString &command,
                                     const QString &workdir,
                                     const QByteArray &startup_id)
{
    if (!isMainThreadActive()) {
        return;
    }

    KConfigGroup confGroup( KGlobal::config(), "General" );
    QString exec = confGroup.readPathEntry("TerminalApplication", "konsole");

    if (!command.isEmpty()) {
        if (exec == "konsole") {
            exec += " --noclose";
        } else if (exec == "xterm") {
            exec += " -hold";
        }

        exec += " -e " + command;
    }

    QStringList cmdTokens = KShell::splitArgs(exec);
    QString cmd = cmdTokens.takeFirst();

    if (exec == "konsole" && !workdir.isEmpty()) {
        cmdTokens << "--workdir";
        cmdTokens << workdir;
        // For other terminals like xterm, we'll simply change the working
        // directory before launching them, see below.
    }

    QString error;
    if (self()->startServiceInternal("kdeinit_exec_with_workdir",
                                    cmd, cmdTokens, &error, 0, NULL, startup_id, false, workdir)) {
      KMessage::message(KMessage::Error,
                      i18n("Could not launch the terminal client:\n\n%1", error),
                      i18n("Could not Launch Terminal Client"));
    }
}
