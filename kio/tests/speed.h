// -*- c++ -*-
#ifndef _SPEED_H
#define _SPEED_H

#include <kio/global.h>

namespace KIO {
    class Job;
}

class SpeedTest : public QObject {
    Q_OBJECT

public:
    SpeedTest();

private slots:
    void entries( KIO::Job *, const KIO::UDSEntryList& );
    void finished( KIO::Job *job );

};

#endif
