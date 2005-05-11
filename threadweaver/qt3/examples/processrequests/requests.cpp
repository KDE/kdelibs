#include <qstring.h>
#include <qlabel.h>
#include <qapplication.h>

#include "requests.h"

namespace ThreadWeaver {

    SPRExample::SPRExample (QObject *parent, const char* name)
        : Job (parent, name)
    {
    }

    void SPRExample::run ()
    {
        debug (0, "SPRExample::run[thread %i]: performing "
               "SPR.\n", thread()->id () );
        triggerSPR ();
        debug (0, "SPRExample::run[thread %i]: returned from "
               "SPR.\n", thread()->id () );
    }

    APRExample::APRExample (QObject *parent, const char* name)
        : Job (parent, name)
    {
    }

    void APRExample::run ()
    {
        debug (0, "APRExample::run[thread %i]: performing "
               "APR.\n", thread()->id () );
        triggerAPR ();
        debug (0, "APRExample::run[thread %i]: returned from "
               "APR.\n", thread()->id () );
    }

    Main::Main ()
        : ProcessRequestBase (0, 0),
          spr (0),
          apr (0)
    {
        logger.attach (&weaver);
    }

    void Main::slotStartSPR ()
    {
        spr = new SPRExample (this);
        connect (spr, SIGNAL ( SPR () ), SLOT ( processSPR () ) );
        connect (spr, SIGNAL ( done ( Job* ) ), SLOT ( sprDone ( Job* ) ) );
        pbSPR->setEnabled (false);
        weaver.enqueue (spr);
    }

    void Main::slotStartAPR ()
    {
        apr = new APRExample (this);
        connect (apr, SIGNAL ( APR () ), SLOT ( processAPR () ) );
        connect (apr, SIGNAL ( done ( Job* ) ), SLOT ( aprDone ( Job* ) ) );
        pbAPR->setEnabled (false);
        weaver.enqueue (apr);
    }

    void Main::processAPR ()
    {
        debug (0, "Main::processSPR[Main Thread]: performing "
               "APR code in the main thread.\n");
        pbFinishAPR->setEnabled (true);
        // ...
    }

    void Main::slotFinishAPR ()
    {
        debug (0, "Main::slotFinishAPR[Main Thread]: finishing APR, "
               "waking job.\n");
        int count = tlNumAPR->text().toInt();
        ++count;
        tlNumAPR->setText ( QString().setNum (count) );

        pbFinishAPR->setEnabled (false);
        apr->wakeAPR ();
    }

    void Main::slotQuit ()
    {
        if (pbFinishAPR->isEnabled())
        {
            slotFinishAPR();
        }
        close();
    }

    void Main::aprDone ( Job* )
    {
        debug (0, "Main::aprDone[Main Thread]: deleting job object.\n");
        delete apr;
        pbAPR->setEnabled (true);
    }

    void Main::processSPR ()
    {
        debug (0, "Main::processSPR[Main Thread]: performing "
               "SPR code in the main thread.\n");

        int count = tlNumSPR->text().toInt();
        ++count;
        tlNumSPR->setText ( QString().setNum (count) );
        // do not reenable the button here, do this when done () is received!
    }

    void Main::sprDone ( Job* )
    {
        delete spr;
        pbSPR->setEnabled (true);
    }

}

int main (int argc, char **argv)
{
    QApplication app (argc, argv);

    ThreadWeaver::Main main;
    main.show ();
    app.setMainWidget (&main);

    return app.exec ();
}
