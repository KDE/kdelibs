#ifndef KCALENDARTEST_H
#define KCALENDARTEST_H

#include <QtCore/QObject>

class KCalendarTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void listTypes();
    void testGregorian();
    void testHijri();
};

#endif
