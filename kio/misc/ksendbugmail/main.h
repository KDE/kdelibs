#ifndef BUG_MAILER_H
#define BUG_MAILER_H "$Id$"

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
