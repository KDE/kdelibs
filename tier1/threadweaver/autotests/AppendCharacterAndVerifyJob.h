#ifndef APPENDCHARACTERANDVERIFYJOB_H
#define APPENDCHARACTERANDVERIFYJOB_H

#include <AppendCharacterJob.h>

class AppendCharacterAndVerifyJob : public AppendCharacterJob
{
public:
    AppendCharacterAndVerifyJob();
    void setValues(QChar character, QString *stringref, const QString& expected);
    void run();

private:
    QString m_expected;
};

#endif // APPENDCHARACTERANDVERIFYJOB_H
