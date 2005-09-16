#include <QList>
#include <QPointer>

#include "ThreadWeaver.h"

namespace ThreadWeaver {

    class Thread;

    /** A JobSequence is a vector of Jobs that will be executed in a sequence.

        It is implemented by automatically creating the necessary dependencies
        between the Jobs in the sequence.

        JobSequence provides a handy cleanup and unwind mechanism: the stop()
        slot. If it is called, the processing of the sequence will stop, and
        all it's remaining Jobs will be dequeued.
    */
    class JobSequence : public Job
    {
        Q_OBJECT
    public:
        JobSequence ( QObject *parent );
        ~JobSequence ();
        /** Append a job to the sequence.

	To use JobSequence, create the Job objects first, add them to the
	sequence, and then queue it. After the sequence has been queued, no
	further Jobs are supposed to be added.
        */
        virtual void append ( Job* );

    public slots:
        /** Stop processing, dequeue all remaining Jobs.
            job is supposed to be an element of the sequence. */
        void stop ( Job *job );
    protected:
        /** Overload to queue the sequence. */
        void aboutToBeQueued ( WeaverInterface *weaver );
    private:
        /** Overload the execute method. */
        void execute ( Thread * );
        /** Overload run().
            We have to. */
        void run() {}
       /** The elements of the sequence. */
        QList < QPointer <Job> > m_elements;
        /** True if this sequence has been queued in the Job queue of a
            Weaver. */
        bool m_queued;
        /** The Weaver interface this sequence is queued in. */
        WeaverInterface *m_weaver;
    };

}
