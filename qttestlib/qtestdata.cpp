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
#include <QtCore/qmetaobject.h>

#include "QtTest/qttestassert.h"
#include "QtTest/qttestdata.h"
#include "QtTest/qttesttable.h"

#include <string.h>
#include <stdlib.h>

class QtTestDataPrivate
{
public:
    QtTestDataPrivate(): tag(0), parent(0), data(0), dataCount(0) {}

    char *tag;
    QtTestTable *parent;
    void **data;
    int dataCount;
};

QtTestData::QtTestData(const char *tag, QtTestTable *parent)
{
    QTEST_ASSERT(tag);
    QTEST_ASSERT(parent);
    d = new QtTestDataPrivate;
    d->tag = strdup(tag);
    d->parent = parent;
    d->data = new void *[parent->elementCount()];
    memset(d->data, 0, parent->elementCount() * sizeof(void*));
}

QtTestData::~QtTestData()
{
    for (int i = 0; i < d->dataCount; ++i) {
        if (d->data[i])
            QMetaType::destroy(d->parent->elementTypeId(i), d->data[i]);
    }
    delete [] d->data;
    free(d->tag);
    delete d;
}

void QtTestData::append(int type, const void *data)
{
    QTEST_ASSERT(d->dataCount < d->parent->elementCount());
    if (d->parent->elementTypeId(d->dataCount) != type) {
        qDebug("expected data of type '%s', got '%s' for element %d of data with tag '%s'",
                d->parent->elementType(d->dataCount), QMetaType::typeName(type),
                d->dataCount, d->tag);
        QTEST_ASSERT(false);
    }
    d->data[d->dataCount] = QMetaType::construct(type, data);
    ++d->dataCount;
}

const char *QtTestData::expectedDataType() const
{
    QTEST_ASSERT(d->dataCount < d->parent->elementCount());
    return d->parent->elementType(d->dataCount);
}

void *QtTestData::data(int index) const
{
    QTEST_ASSERT(index < d->parent->elementCount());
    return d->data[index];
}

QtTestTable *QtTestData::parent() const
{
    return d->parent;
}

const char *QtTestData::dataTag() const
{
    return d->tag;
}

