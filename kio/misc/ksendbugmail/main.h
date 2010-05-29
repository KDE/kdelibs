#ifndef BUG_MAILER_H
#define BUG_MAILER_H "$Id: main.h 78339 2001-01-16 02:33:21Z garbanzo $"

#include <qobject.h>

class SMTP;

class BugMailer : public QObject {
    Q_OBJECT
public:
    BugMailer(SMTP* s) : QObject(0, "mailer"), sm(s) {}

public slots:
    void slotError(int);
    void slotSend();
private:
    SMTP *sm;
};

#endif
