// -*- c++ -*-
#ifndef _SPEED_H
#define _SPEED_H

#include <kio/global.h>
#include <kio/udsentry.h>

class KJob;
namespace KIO {
    class Job;
}

class SpeedTest : public QObject {
    Q_OBJECT

public:
    SpeedTest(const QUrl & url);

private Q_SLOTS:
    void entries( KIO::Job *, const KIO::UDSEntryList& );
    void finished( KJob *job );

};

#endif
