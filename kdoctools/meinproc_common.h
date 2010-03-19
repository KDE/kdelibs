
#ifndef MEINPROC_COMMON_H
#define MEINPROC_COMMON_H

#include <QDebug>
#include <QString>

enum CheckFileResult
{
    CheckFileSuccess,
    CheckFileDoesNotExist,
    CheckFileIsNotFile,
    CheckFileIsNotReadable
};

CheckFileResult checkFile( const QString &checkFilename );

enum CheckResult
{
    CheckSuccess,
    CheckNoOut,
    CheckNoXmllint
};

CheckResult check(const QString &checkFilename, const QString &exe, const QByteArray &catalogs);

void doOutput(QString output, bool usingStdOut, bool usingOutput, const QString &outputOption, bool replaceCharset);

#endif
