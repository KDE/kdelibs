#ifndef REQUESTS_H
#define REQUESTS_H

#include <qobject.h>
#include <qpushbutton.h>

#include <weaver.h>
#include <weaverlogger.h>

#include "processrequest.h"

namespace ThreadWeaver {

    class SPRExample : public Job
	{
	    Q_OBJECT
		public:
	    SPRExample (QObject *parent = 0, const char* name = 0);
	    ~SPRExample () {}
	protected:
	    void run ();
	};


    class APRExample : public Job
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
	    Weaver weaver;
	    WeaverThreadLogger logger;

	    protected slots:
		void processSPR ();
	    void processAPR ();
	    void sprDone ( Job* );
	    void aprDone ( Job* );
	};	    
	    
}
#endif // REQUESTS.H
