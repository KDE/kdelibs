/***************************************************************************
** This file may be distributed under the terms of the Q Public License
** as defined by Trolltech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
*/
#ifndef QTESTTABLE_H
#define QTESTTABLE_H

#include "QtTest/qttest_global.h"

class QtTestData;
class QtTestTablePrivate;

class Q_TESTLIB_EXPORT QtTestTable
{
public:
    QtTestTable();
    ~QtTestTable();

    void defineElement(const char *elementType, const char *elementName);
    QtTestData *newData(const char *tag);

    int elementCount() const;
    int dataCount() const;

    int elementTypeId(int index) const;
    const char *dataTag(int index) const;
    const char *elementType(int index) const;
    int indexOf(const char *elementName) const;
    bool isEmpty() const;
    QtTestData *testData(int index) const;

    static QtTestTable *globalTestTable();

private:
    Q_DISABLE_COPY(QtTestTable)

    QtTestTablePrivate *d;
};

#endif
