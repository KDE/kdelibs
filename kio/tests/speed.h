// -*- c++ -*-
#ifndef _SPEED_H
#define _SPEED_H

#include <kio/global.h>
#include <kurl.h>

namespace KIO {
    class Job;
}

class SpeedTest : public QObject {
    Q_OBJECT

public:
    SpeedTest(const KURL & url);

private Q_SLOTS:
    void entries( KIO::Job *, const KIO::UDSEntryList& );
    void finished( KIO::Job *job );

};

#endif
