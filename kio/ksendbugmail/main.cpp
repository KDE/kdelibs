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
#include <pwd.h>

static KCmdLineOptions options[] = {
    { "subject", I18N_NOOP("Subject line"), 0 },
    { "recipient", I18N_NOOP("Recipient"), "submit@bugs.kde.org" },
    { 0, 0, 0 }
};

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
    QString text;
    input >> text;
    kdDebug() << text << endl;

    KConfig emailConf( QString::fromLatin1("emaildefaults") );
    emailConf.setGroup( QString::fromLatin1("UserInfo") );
    QString fromaddr = emailConf.readEntry( QString::fromLatin1("EmailAddress") );
    if (fromaddr.isEmpty()) {
        struct passwd *p;
        p = getpwuid(getuid());
        fromaddr = QString::fromLatin1(p->pw_name);
    } else {
        QString name = emailConf.readEntry( QString::fromLatin1("FullName"));
        if (!name.isEmpty())
            fromaddr = name + QString::fromLatin1(" <") + fromaddr + QString::fromLatin1(">");
    }

    emailConf.setGroup( QString::fromLatin1("ServerInfo") );
    QString  server = emailConf.readEntry(QString::fromLatin1("Outgoing"), "bugs.kde.org");

    SMTP * sm = new SMTP();
    QObject::connect(sm, SIGNAL(messageSent()), &a, SLOT(quit()));
    QObject::connect(sm, SIGNAL(error(int)), &a, SLOT(quit()));
    sm->setServerHost(server);
    sm->setSenderAddress(fromaddr);
    sm->setRecipientAddress(recipient);
    sm->setMessageSubject(subject);
    sm->setMessageBody(text);
    sm->sendMessage();

    return a.exec();
}
