// -*- c++ -*-

#ifndef KIO_DAEMON_H
#define KIO_DAEMON_H

#include <qlist.h>
#include <kuniqueapp.h>

struct SlaveEntry;

class KIODaemon : public KUniqueApplication {
    Q_OBJECT

public:
    KIODaemon(int& argc, char** argv);

    virtual bool process(const QCString &fun, const QByteArray &data,
			 QCString &replyType, QByteArray &replyData);

protected:
    QString createSlave(const QString& protocol);
    void connectSlave(const QString& ticket, const QString& path);

    QString createTicket();

private:
    QList<SlaveEntry> pendingSlaves;

};

#endif
