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
#include "QtTest/qttesttable.h"
#include "QtTest/qttestdata.h"

#include <QtCore/qmetaobject.h>

#include <string.h>

#include "QtTest/qttestassert.h"

class QtTestTablePrivate
{
public:
    struct ElementList
    {
        ElementList(): elementName(0), elementType(0), next(0) {}
        const char *elementName;
        const char *elementType;
        ElementList *next;
    };

    struct DataList
    {
        DataList(): data(0), next(0) {}
        QtTestData *data;
        DataList *next;
    };

    QtTestTablePrivate(): list(0), dataList(0) {}
    ~QtTestTablePrivate();

    ElementList *list;
    DataList *dataList;

    void append(const char *elemType, const char *elemName);
    void append(QtTestData *data);
    ElementList *elementAt(int index);
    QtTestData *dataAt(int index);
};

QtTestTablePrivate::ElementList *QtTestTablePrivate::elementAt(int index)
{
    ElementList *iter = list;
    for (int i = 0; i < index; ++i) {
        if (!iter)
            return 0;
        iter = iter->next;
    }
    return iter;
}

QtTestData *QtTestTablePrivate::dataAt(int index)
{
    DataList *iter = dataList;
    for (int i = 0; i < index; ++i) {
        if (!iter)
            return 0;
        iter = iter->next;
    }
    return iter ? iter->data : 0;
}

QtTestTablePrivate::~QtTestTablePrivate()
{
    DataList *dit = dataList;
    while (dit) {
        DataList *next = dit->next;
        delete dit->data;
        delete dit;
        dit = next;
    }
    ElementList *iter = list;
    while (iter) {
        ElementList *next = iter->next;
        delete iter;
        iter = next;
    }
}

void QtTestTablePrivate::append(const char *elemType, const char *elemName)
{
    ElementList *item = new ElementList;
    item->elementName = elemName;
    item->elementType = elemType;
    if (!list) {
        list = item;
        return;
    }
    ElementList *last = list;
    while (last->next != 0)
        last = last->next;
    last->next = item;
}

void QtTestTablePrivate::append(QtTestData *data)
{
    DataList *item = new DataList;
    item->data = data;
    if (!dataList) {
        dataList = item;
        return;
    }
    DataList *last = dataList;
    while (last->next != 0)
        last = last->next;
    last->next = item;
}

void QtTestTable::defineElement(const char *elementType, const char *elementName)
{
    QTEST_ASSERT(elementType);
    QTEST_ASSERT(elementName);

    d->append(elementType, elementName);
}

int QtTestTable::elementCount() const
{
    QtTestTablePrivate::ElementList *item = d->list;
    int count = 0;
    while (item) {
        ++count;
        item = item->next;
    }
    return count;
}


int QtTestTable::dataCount() const
{
    QtTestTablePrivate::DataList *item = d->dataList;
    int count = 0;
    while (item) {
        ++count;
        item = item->next;
    }
    return count;
}

bool QtTestTable::isEmpty() const
{
    return !d->list;
}

QtTestData *QtTestTable::newData(const char *tag)
{
    QtTestData *dt = new QtTestData(tag, this);
    d->append(dt);
    return dt;
}

QtTestTable::QtTestTable()
{
    d = new QtTestTablePrivate;
}

QtTestTable::~QtTestTable()
{
    delete d;
}

int QtTestTable::elementTypeId(int index) const
{
    QtTestTablePrivate::ElementList *item = d->elementAt(index);
    if (!item)
        return -1;
    return QMetaType::type(item->elementType);
}

const char *QtTestTable::dataTag(int index) const
{
    QtTestTablePrivate::ElementList *item = d->elementAt(index);
    if (!item)
        return 0;
    return item->elementName;
}

const char *QtTestTable::elementType(int index) const
{
    QtTestTablePrivate::ElementList *item = d->elementAt(index);
    if (!item)
        return 0;
    return item->elementType;
}

QtTestData *QtTestTable::testData(int index) const
{
    return d->dataAt(index);
}

int QtTestTable::indexOf(const char *elementName) const
{
    QTEST_ASSERT(elementName);

    QtTestTablePrivate::ElementList *item = d->list;
    int i = 0;
    while (item) {
        if (strcmp(elementName, item->elementName) == 0)
            return i;
        item = item->next;
        ++i;
    }
    return -1;
}

QtTestTable *QtTestTable::globalTestTable()
{
    static QtTestTable *gTable = new QtTestTable;
    return gTable;
}

