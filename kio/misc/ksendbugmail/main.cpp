#include <sys/types.h>
#include "main.h"
#include <pwd.h>
#include <stdlib.h>
#include <unistd.h>

#include <qtextstream.h>

#include <kapplication.h>
#include <kemailsettings.h>
#include <klocale.h>
#include <kcmdlineargs.h>
#include <kaboutdata.h>
#include <kdebug.h>
#include <kconfig.h>

#include "smtp.h"

static KCmdLineOptions options[] = {
    { "subject <argument>", I18N_NOOP("Subject line"), 0 },
    { "recipient <argument>", I18N_NOOP("Recipient"), "submit@bugs.kde.org" },
    KCmdLineLastOption
};

void BugMailer::slotError(int errornum) {
    kdDebug() << "slotError\n";
    QString str, lstr;

    switch(errornum) {
        case SMTP::CONNECTERROR:
            lstr = i18n("Error connecting to server.");
            break;
        case SMTP::NOTCONNECTED:
            lstr = i18n("Not connected.");
            break;
        case SMTP::CONNECTTIMEOUT:
            lstr = i18n("Connection timed out.");
            break;
        case SMTP::INTERACTTIMEOUT:
            lstr = i18n("Time out waiting for server interaction.");
            break;
        default:
            lstr = sm->getLastLine().stripWhiteSpace();
            lstr = i18n("Server said: \"%1\"").arg(lstr);
    }
    fputs(lstr.utf8().data(), stdout);
    fflush(stdout);

    ::exit(1);
}

void BugMailer::slotSend() {
    kdDebug() << "slotSend\n";
    ::exit(0);
}

int main(int argc, char **argv) {

    KLocale::setMainCatalog("kdelibs");
    KAboutData d("ksendbugmail", I18N_NOOP("KSendBugMail"), "1.0",
                 I18N_NOOP("Sends a short bug report to submit@bugs.kde.org"),
                 KAboutData::License_GPL, "(c) 2000 Stephan Kulow");
    d.addAuthor("Stephan Kulow", I18N_NOOP("Author"), "coolo@kde.org");

    KCmdLineArgs::init(argc, argv, &d);
    KCmdLineArgs::addCmdLineOptions(options);
    KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

    KApplication a(false, false);

    QByteArray recipient = args->getOption("recipient");
    if (recipient.isEmpty())
        recipient = "submit@bugs.kde.org";
    else {
        if (recipient.at(0) == '\'') {
            recipient = recipient.mid(1).left(recipient.length() - 2);
        }
    }
    kdDebug() << "recp \"" << recipient << "\"\n";

    Q3CString subject = args->getOption("subject");
    if (subject.isEmpty())
        subject = "(no subject)";
    else {
        if (subject.at(0) == '\'')
            subject = subject.mid(1).left(subject.length() - 2);
    }
    QTextStream input(stdin, QIODevice::ReadOnly);
    QString text, line;
    while (!input.atEnd()) {
        line = input.readLine();
        text += line + "\r\n";
    }
    kdDebug() << text << endl;

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
        fromaddr += "@";
        char buffer[256];
	buffer[0] = '\0';
        if(!gethostname(buffer, sizeof(buffer)))
	    buffer[sizeof(buffer)-1] = '\0';
        fromaddr += buffer;
    }
    kdDebug() << "fromaddr \"" << fromaddr << "\"" << endl;

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
    kdDebug() << "execing " << r << endl;
    delete sm;
    return r;
}

#include "main.moc"
