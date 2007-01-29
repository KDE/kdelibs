/* kate: tab-indents off; replace-tabs on; tab-width 4; remove-trailing-space on; encoding utf-8;*/
/*
This file is part of the KDE libraries
This file has been placed in the Public Domain.
*/

#ifndef ktemporaryfiletest_h
#define ktemporaryfiletest_h

#include <QtCore/QObject>
#include <QtCore/QString>

class KTemporaryFileTest : public QObject
{
Q_OBJECT
private slots:
    void initTestCase();
    void cleanupTestCase();
    void testKTemporaryFile();

private:
    QString kdeTempDir;
    QString componentName;
};

#endif
