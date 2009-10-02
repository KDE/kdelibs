/*
   Copyright (c) 2000 Bernd Johannes Wuebben <wuebben@math.cornell.edu>
   Copyright (c) 2000 Stephan Kulow <coolo@kde.org>

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "main.h"
#include <sys/types.h>
#include <pwd.h>
#include <stdlib.h>
#include <unistd.h>

#include <QtCore/QTextStream>

#include <kapplication.h>
#include <kemailsettings.h>
#include <klocale.h>
#include <kcmdlineargs.h>
#include <kaboutdata.h>
#include <kdebug.h>
#include <kconfig.h>

#include "smtp.h"

void BugMailer::slotError(int errornum) {
    QString lstr;

    switch(errornum) {
        case SMTP::ConnectError:
            lstr = i18n("Error connecting to server.");
            break;
        case SMTP::NotConnected:
            lstr = i18n("Not connected.");
            break;
        case SMTP::ConnectTimeout:
            lstr = i18n("Connection timed out.");
            break;
        case SMTP::InteractTimeout:
            lstr = i18n("Time out waiting for server interaction.");
            break;
        default:
            lstr = sm->getLastLine().trimmed();
            lstr = i18n("Server said: \"%1\"", lstr);
    }
    kDebug() << lstr;

    fputs(lstr.toUtf8().data(), stdout);
    fflush(stdout);

    qApp->exit(1);
}

void BugMailer::slotSend() {
    kDebug();
    qApp->exit(0);
}

int main(int argc, char **argv) {

    KAboutData d("ksendbugmail", "kdelibs4", ki18n("KSendBugMail"), "1.0",
                 ki18n("Sends a bug report by email"),
                 KAboutData::License_GPL, ki18n("(c) 2000 Stephan Kulow"));
    d.addAuthor(ki18n("Stephan Kulow"), ki18n("Author"), "coolo@kde.org");

    KCmdLineOptions options;
    options.add("subject <argument>", ki18n("Subject line"));
    options.add("recipient <argument>", ki18n("Recipient"), "submit@bugs.kde.org");

    KCmdLineArgs::init(argc, argv, &d);
    KCmdLineArgs::addCmdLineOptions(options);
    KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

    KApplication a(false);

    QString recipient = args->getOption("recipient");
    if (recipient.isEmpty())
        recipient = "submit@bugs.kde.org";
    else {
        if (recipient.at(0) == '\'') {
            recipient = recipient.mid(1).left(recipient.length() - 2);
        }
    }
    kDebug() << "recp" << recipient;

    QString subject = args->getOption("subject");
    if (subject.isEmpty())
        subject = "(no subject)";
    else {
        if (subject.at(0) == '\'')
            subject = subject.mid(1).left(subject.length() - 2);
    }
    QTextStream input(stdin, QIODevice::ReadOnly);
    input.setCodec("UTF-8");
    QString text, line;
    while (!input.atEnd()) {
        line = input.readLine();
        text += line + "\r\n";
    }
    kDebug() << text;

    KEMailSettings emailConfig;
    emailConfig.setProfile(emailConfig.defaultProfileName());
    QString fromaddr = emailConfig.getSetting(KEMailSettings::EmailAddress);
    if (!fromaddr.isEmpty()) {
        QString name = emailConfig.getSetting(KEMailSettings::RealName);
        if (!name.isEmpty())
            fromaddr = name + QLatin1String(" <") + fromaddr + QString::fromLatin1(">");
    } else {
        struct passwd *p;
        p = getpwuid(getuid());
        fromaddr = QLatin1String(p->pw_name);
        fromaddr += '@';
        char buffer[256];
	buffer[0] = '\0';
        if(!gethostname(buffer, sizeof(buffer)))
	    buffer[sizeof(buffer)-1] = '\0';
        fromaddr += buffer;
    }
    kDebug() << "fromaddr \"" << fromaddr << "\"";

    QString  server = emailConfig.getSetting(KEMailSettings::OutServer);
    if (server.isEmpty())
        server=QLatin1String("bugs.kde.org");

    SMTP *sm = new SMTP;
    BugMailer bm(sm);

    QObject::connect(sm, SIGNAL(messageSent()), &bm, SLOT(slotSend()));
    QObject::connect(sm, SIGNAL(error(int)), &bm, SLOT(slotError(int)));
    sm->setServerHost(server);
    sm->setPort(25);
    sm->setSenderAddress(fromaddr);
    sm->setRecipientAddress(recipient);
    sm->setMessageSubject(subject);
    sm->setMessageHeader(QString::fromLatin1("From: %1\r\nTo: %2\r\n").arg(fromaddr).arg(QString(recipient)));
    sm->setMessageBody(text);
    sm->sendMessage();

    int r = a.exec();
    kDebug() << "execing " << r;
    delete sm;
    return r;
}

#include "main.moc"
