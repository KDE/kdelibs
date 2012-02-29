#ifndef APPENDCHARACTER_JOB
#define APPENDCHARACTER_JOB

#include <QtCore/QObject>
#include <QtCore/QMutex>

#include <Job.h>
#include <DebuggingAids.h>

// define in test binary:

extern QMutex s_GlobalMutex;

class AppendCharacterJob : public ThreadWeaver::Job
{
    Q_OBJECT

public:
    AppendCharacterJob ( QChar c = QChar(), QString* stringref = 0 , QObject* parent = 0 )
        : ThreadWeaver::Job ( parent )
    {
        setValues( c, stringref );
    }

    void setValues ( QChar c, QString* stringref )
    {
        m_c = c;
        m_stringref = stringref;
        setObjectName ( tr ( "Job_" ) + m_c );
    }

    void run()
    {
      QMutexLocker locker ( &s_GlobalMutex );
        m_stringref->append( m_c );
        using namespace ThreadWeaver;
        debug( 3, "AppendCharacterJob::run: %c appended, result is %s.\n",
               m_c.toAscii(), qPrintable( *m_stringref ) );
    }

private:
    QChar m_c;
    QString* m_stringref;
};

class FailingAppendCharacterJob : public AppendCharacterJob
{
    Q_OBJECT

public:
    FailingAppendCharacterJob ( QChar c = QChar(), QString* stringref = 0, QObject* parent = 0 )
        : AppendCharacterJob ( c, stringref, parent )
    {
    }

    bool success () const
    {
        return false;
    }
};

class BusyJob : public ThreadWeaver::Job
{
    Q_OBJECT

public:
    BusyJob( QObject* parent = 0 )
        : ThreadWeaver::Job ( parent )
    {
    }

    void run()
    {
      for (int i = 0; i < 100; ++i) {
        int k = (i << 3) + (i >> 4);
        Q_UNUSED( k );
      }
    }
};

#endif

