#ifndef KCALENDARTEST_H
#define KCALENDARTEST_H

#include <qobject.h>

class KCalendarTest : public QObject
{
    Q_OBJECT

private slots:
    void listTypes();
    void testGregorian();
    void testHijri();
};

#endif
