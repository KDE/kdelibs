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
#ifndef QTESTDATA_H
#define QTESTDATA_H

#include <QtCore/qmetatype.h>
#include <QtCore/qstring.h>

#include "QtTest/qttest_global.h"

class QtTestTable;
class QtTestDataPrivate;

class Q_TESTLIB_EXPORT QtTestData
{
public:
    ~QtTestData();

    void append(int type, const void *data);
    void *data(int index) const;
    const char *dataTag() const;
    QtTestTable *parent() const;

    const char *expectedDataType() const;

private:
    friend class QtTestTable;
    QtTestData(const char *tag = 0, QtTestTable *parent = 0);

    Q_DISABLE_COPY(QtTestData)

    QtTestDataPrivate *d;
};

template<typename T>
QtTestData &operator<<(QtTestData &data, const T &value)
{
    data.append(QMetaTypeId<T>::qt_metatype_id(), &value);
    return data;
}

inline QtTestData &operator<<(QtTestData &data, const char * value)
{
    QString str = QString::fromAscii(value);
    data.append(QMetaType::QString, &str);
    return data;
}

#endif

