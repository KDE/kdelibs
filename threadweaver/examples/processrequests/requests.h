#ifndef REQUESTS_H
#define REQUESTS_H

#include <qobject.h>
#include <qpushbutton.h>

#include <weaver.h>
#include <weaverlogger.h>

#include "processrequest.h"

class SPRExample : public ThreadWeaver::Job
{
    Q_OBJECT
public:
    SPRExample (QObject *parent = 0, const char* name = 0);
    ~SPRExample () {}
protected:
    void run ();
};


class APRExample : public ThreadWeaver::Job
{
    Q_OBJECT
public:
    APRExample (QObject *parent = 0, const char* name = 0);
    ~APRExample () {}
protected:
    void run ();
};

class Main : public ProcessRequestBase
{
    Q_OBJECT

public:
    Main ();
    ~Main () {}
    void slotStartSPR ();
    void slotStartAPR ();
    void slotFinishAPR ();
    void slotQuit();

protected:
    SPRExample *spr;
    APRExample *apr;
    ThreadWeaver::Weaver weaver;
    ThreadWeaver::WeaverThreadLogger logger;

protected slots:
    void processSPR ();
    void processAPR ();
    void sprDone ();
    void aprDone ();
};	    
	    
#endif // REQUESTS.H
