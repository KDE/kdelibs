#include <weaver.h>

using namespace ThreadWeaver;

class DummyJob : public Job
{
public:
    DummyJob(QObject* parent = 0 , const char* name = 0, int seconds = 3)
        : Job (parent, name),
          m_wait (seconds)
        {
        }

    ~DummyJob() {}
protected:
    void run ()
        {
            debug (1, "DummyJob::run: doing it - sleeping %i milliseconds.\n",
                   m_wait );
            thread()->msleep( m_wait );
            debug (1, "DummyJob::run: done.\n" );
        }

    int m_wait;
};

int main()

{
    const int NoOfThreads = 16 ;
    const int NoOfJobs = NoOfThreads * 64  ; // 64 jobs for every thread
    const int Multiplier = 10; // [0..9]* Multiplier sleep time for each job

    Weaver weaver(0, 0, NoOfThreads);
    DummyJob *dummies[ NoOfJobs];

    // ----- create a number of dummy jobs:
    for ( int count  = 0; count < NoOfJobs; ++count )
    {
        dummies[count] = new DummyJob (0, 0, Multiplier * (count % 10) );
    }

    // ----- enqueue the jobs:
    for ( int count  = 0; count < NoOfJobs; ++count )
    {
        weaver.enqueue( dummies[count] );
    }

    weaver.finish();

    for ( int count  = 0; count < NoOfJobs; ++count )
    {
        delete dummies[count];
    }

    return 0;
}

