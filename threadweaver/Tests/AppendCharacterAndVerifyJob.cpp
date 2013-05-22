#include "AppendCharacterAndVerifyJob.h"

AppendCharacterAndVerifyJob::AppendCharacterAndVerifyJob()
    : AppendCharacterJob()
{
}

void AppendCharacterAndVerifyJob::setValues(QChar c, QString *stringref, const QString &expected)
{
    AppendCharacterJob::setValues(c, stringref);
    m_expected = expected;
}

void AppendCharacterAndVerifyJob::run()
{
    using namespace ThreadWeaver;
    QMutexLocker locker ( &s_GlobalMutex );
    stringRef()->append( c() );
    if (m_expected.mid(0, stringRef()->length()) != *stringRef()) {
        debug(3, "It broke!");
    }
    debug( 3, "AppendCharacterJob::run: %c appended, result is %s.\n",
           c().toLatin1(), qPrintable( *stringRef() ) );
}
