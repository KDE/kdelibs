#include <kapp.h>
#include <klocale.h>
#include <kcmdlineargs.h>
#include <kaboutdata.h>
#include <kdebug.h>
#include <qtextstream.h>
#include <smtp.h>
#include <kconfig.h>
#include <unistd.h>
#include <sys/types.h>
#include <main.h>
#include <pwd.h>
#include <stdlib.h>

static KCmdLineOptions options[] = {
    { "subject <argument>", I18N_NOOP("Subject line"), 0 },
    { "recipient <argument>", I18N_NOOP("Recipient"), "submit@bugs.kde.org" },
    { 0, 0, 0 }
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

    ::exit(42);
}

void BugMailer::slotSend() {
    kdDebug() << "slotSend\n";
    ::exit(0);
}

int main(int argc, char **argv) {

    KLocale::setMainCatalogue("kdelibs");
    KAboutData d("ksendbugmail", I18N_NOOP("KSendBugMail"), "1.0",
                 I18N_NOOP("Sends a little mail to submit@bugs.kde.org"),
                 KAboutData::License_GPL, "(c) 2000 Stephan Kulow");
    d.addAuthor("Stephan Kulow", I18N_NOOP("Author"), "coolo@kde.org");

    KCmdLineArgs::init(argc, argv, &d);
    KCmdLineArgs::addCmdLineOptions(options);
    KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

    KApplication a;


    QCString recipient = args->getOption("recipient");
    if (recipient.isEmpty())
        recipient = "submit@bugs.kde.org";

    QCString subject = args->getOption("subject");
    if (subject.isEmpty())
        subject = "(no subject)";

    QTextIStream input(stdin);
    QString text, line;
    while (!input.eof()) {
        line = input.readLine();
        text += line + "\r\n";
    }
    kdDebug() << text << endl;

    KConfig emailConf( QString::fromLatin1("emaildefaults") );
    emailConf.setGroup( QString::fromLatin1("UserInfo") );
    QString fromaddr = emailConf.readEntry( QString::fromLatin1("EmailAddress") );
    if (fromaddr.isEmpty()) {
        struct passwd *p;
        p = getpwuid(getuid());
        fromaddr = QString::fromLatin1(p->pw_name);
        fromaddr += "@";
        char buffer[200];
        gethostname(buffer, 200);
        fromaddr += buffer;
    } else {
        QString name = emailConf.readEntry( QString::fromLatin1("FullName"));
        if (!name.isEmpty())
            fromaddr = name + QString::fromLatin1(" <") + fromaddr + QString::fromLatin1(">");
    }

    emailConf.setGroup( QString::fromLatin1("ServerInfo") );
    QString  server = emailConf.readEntry(QString::fromLatin1("Outgoing"), "bugs.kde.org");

    SMTP sm;
    BugMailer bm(&sm);

    QObject::connect(&sm, SIGNAL(messageSent()), &bm, SLOT(slotSend()));
    QObject::connect(&sm, SIGNAL(error(int)), &bm, SLOT(slotError(int)));
    sm.setServerHost(server);
    sm.setPort(25);
    sm.setSenderAddress(fromaddr);
    sm.setRecipientAddress(recipient);
    sm.setMessageSubject(subject);
    sm.setMessageHeader(QString::fromLatin1("From: %1\r\nTo: %2\r\n").arg(fromaddr).arg(recipient));
    sm.setMessageBody(text);
    sm.sendMessage();

    return a.exec();
}

#include "main.moc"
