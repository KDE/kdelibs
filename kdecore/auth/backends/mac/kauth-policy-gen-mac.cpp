#include <QCoreApplication>
#include <QSettings>
#include <QString>
#include <QStringList>
#include <QRegExp>
#include <iostream>
#include <Security/Security.h>

using namespace std;

void do_actions(QSettings &ini);

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv); // FIXME: Is this needed?

    if(argc < 2)
    {
        cerr << "Too few arguments";
        return 1;
    }

    QSettings ini(argv[1], QSettings::IniFormat);
    if(ini.status())
    {
        cerr << "Error loading file: " << argv[1];
        return 1;
    }

    do_actions(ini);
}

void do_actions(QSettings &ini)
{
    AuthorizationRef auth;
    AuthorizationCreate(NULL, NULL, kAuthorizationFlagDefaults, &auth);

    OSStatus err;

    foreach(const QString &action, ini.childGroups())
    {
        QRegExp exp("[a-z]+(\\.[a-z]+)*");
        if(!exp.exactMatch(action))
        {
             cerr << "Wrong action syntax: " << action.toAscii().data() << endl;
             exit(1);
        }

        QString message, policy;
        if( !ini.contains(action + "/message") ||
            !ini.contains(action + "/policy"))
        {
            cerr << "Missing parameter in action: " << action.toAscii().data() << endl;
            exit(1);
        }

        message = ini.value(action + "/message").toString();
        policy = ini.value(action + "/policy").toString();

        err = AuthorizationRightGet(action.toAscii(), NULL);

        QString rule;

        if(policy == "yes")
            rule = kAuthorizationRuleClassAllow;
        else if(policy == "no")
            rule = kAuthorizationRuleClassDeny;
        else if(policy == "auth_self")
            rule = kAuthorizationRuleAuthenticateAsSessionUser;
        else if(policy == "auth_admin")
            rule = kAuthorizationRuleAuthenticateAsAdmin;

        CFStringRef cfRule = CFStringCreateWithCString(NULL, rule.toAscii(), kCFStringEncodingASCII);
        CFStringRef cfPrompt = CFStringCreateWithCString(NULL, message.toAscii(), kCFStringEncodingASCII);

        if(err == errAuthorizationDenied)
        {
            err = AuthorizationRightSet(auth, action.toAscii(), cfRule, cfPrompt, NULL, NULL);
            if(err != noErr)
            {
                cerr << "You don't have the right to edit the security database (try to run cmake with sudo): " << err << endl;
                exit(1);
            }
        }
    }
}
