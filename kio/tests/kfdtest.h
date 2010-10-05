/****************************************************************************
**
** Copyright (C) 2003 Carsten Pfeiffer <pfeiffer@kde.org>
**
****************************************************************************/

#ifndef KFDTEST_H
#define KFDTEST_H

#include <QtCore/QObject>
#include <kurl.h>

class KFDTest : public QObject
{
    Q_OBJECT

public:
    KFDTest( const KUrl& startDir, QObject *parent = 0, const char *name = 0);

public Q_SLOTS:
    void doit();

private:
    KUrl m_startDir;
};


#endif // KFDTEST_H
