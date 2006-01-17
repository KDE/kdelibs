#ifndef BUG_MAILER_H
#define BUG_MAILER_H

#include <qobject.h>

class SMTP;

class BugMailer : public QObject {
    Q_OBJECT
public:
    BugMailer(SMTP* s) : QObject(0), sm(s) { setObjectName( "mailer" ); }

public Q_SLOTS:
    void slotError(int);
    void slotSend();
private:
    SMTP *sm;
};

#endif
