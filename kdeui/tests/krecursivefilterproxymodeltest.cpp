/*
    Copyright (c) 2014 Christian Mollekopf <mollekopf@kolabsys.com>

    This library is free software; you can redistribute it and/or modify it
    under the terms of the GNU Library General Public License as published by
    the Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    This library is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
    License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to the
    Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
    02110-1301, USA.
*/


#include <qtest_kde.h>

#include <krecursivefilterproxymodel.h>
#include <QStandardItemModel>

Q_DECLARE_METATYPE(QModelIndex)

class ModelSignalSpy : public QObject {
    Q_OBJECT
public:
    explicit ModelSignalSpy(QAbstractItemModel &model) {
        connect(&model, SIGNAL(rowsInserted(QModelIndex, int, int)), this, SLOT(onRowsInserted(QModelIndex,int,int)));
        connect(&model, SIGNAL(rowsRemoved(QModelIndex, int, int)), this, SLOT(onRowsRemoved(QModelIndex,int,int)));
        connect(&model, SIGNAL(rowsAboutToBeInserted(QModelIndex, int, int)), this, SLOT(onRowsAboutToBeInserted(QModelIndex,int,int)));
        connect(&model, SIGNAL(rowsAboutToBeRemoved(QModelIndex, int, int)), this, SLOT(onRowsAboutToBeRemoved(QModelIndex,int,int)));
        connect(&model, SIGNAL(rowsMoved(QModelIndex, int, int, QModelIndex, int)), this, SLOT(onRowsMoved(QModelIndex,int,int, QModelIndex, int)));
        connect(&model, SIGNAL(dataChanged(QModelIndex,QModelIndex)), this, SLOT(onDataChanged(QModelIndex,QModelIndex)));
        connect(&model, SIGNAL(layoutChanged()), this, SLOT(onLayoutChanged()));
        connect(&model, SIGNAL(modelReset()), this, SLOT(onModelReset()));
    }

    QStringList mSignals;

private Q_SLOTS:
    void onRowsInserted(QModelIndex p, int start, int end) {
        mSignals << QLatin1String("rowsInserted(") + textForRowSpy(p, start, end) + ')';
    }
    void onRowsRemoved(QModelIndex p, int start, int end) {
        mSignals << QLatin1String("rowsRemoved(") + textForRowSpy(p, start, end) + ')';
    }
    void onRowsAboutToBeInserted(QModelIndex p, int start, int end) {
        mSignals << QLatin1String("rowsAboutToBeInserted(") + textForRowSpy(p, start, end) + ')';
    }
    void onRowsAboutToBeRemoved(QModelIndex p, int start, int end) {
        mSignals << QLatin1String("rowsAboutToBeRemoved(") + textForRowSpy(p, start, end) + ')';
    }
    void onRowsMoved(QModelIndex,int,int,QModelIndex,int) {
        mSignals << QLatin1String("rowsMoved");
    }
    void onDataChanged(const QModelIndex &from, const QModelIndex& ) {
        mSignals << QString("dataChanged(%1)").arg(from.data().toString());
    }
    void onLayoutChanged() {
        mSignals << QLatin1String("layoutChanged");
    }
    void onModelReset() {
        mSignals << QLatin1String("modelReset");
    }
private:
    QString textForRowSpy(const QModelIndex &parent, int start, int end)
    {
        QString txt = parent.data().toString();
        if (!txt.isEmpty())
            txt += QLatin1Char('.');
        txt += QString::number(start+1);
        if (start != end)
            txt += QLatin1Char('-') + QString::number(end+1);
        return txt;
    }
};

class TestModel : public KRecursiveFilterProxyModel
{
    Q_OBJECT
public:
    TestModel(QAbstractItemModel *sourceModel)
        : KRecursiveFilterProxyModel()
    {
        setSourceModel(sourceModel);
    }

    virtual bool acceptRow(int sourceRow, const QModelIndex &sourceParent) const
    {
        // qDebug() << sourceModel()->index(sourceRow, 0, sourceParent).data().toString() << sourceModel()->index(sourceRow, 0, sourceParent).data(Qt::UserRole+1).toBool();
        return sourceModel()->index(sourceRow, 0, sourceParent).data(Qt::UserRole+1).toBool();
    }
};

// Represents this tree
// - A
// - - B
// - - - C
// - - - D
// - - E
// as a single string, englobing children in brackets, like this:
// [A[B[C D] E]]
// In addition, items that match the filtering (data(UserRole+1) == true) have a * after their value.
static QString treeAsString(const QAbstractItemModel &model, const QModelIndex &parent = QModelIndex())
{
    QString ret;
    const int rowCount = model.rowCount(parent);
    if (rowCount > 0) {
        ret += QLatin1Char('[');
        for (int row = 0 ; row < rowCount; ++row) {
            if (row > 0) {
                ret += ' ';
            }
            const QModelIndex child = model.index(row, 0, parent);
            ret += child.data().toString();
            if (child.data(Qt::UserRole+1).toBool())
                ret += QLatin1Char('*');
            ret += treeAsString(model, child);
        }
        ret += QLatin1Char(']');
    }
    return ret;
}

// Fill a tree model based on a string representation (see treeAsString)
static void fillModel(QStandardItemModel &model, const QString &str)
{
    QCOMPARE(str.count('['), str.count(']'));
    QStandardItem *item = 0;
    QString data;
    for ( int i = 0 ; i < str.length() ; ++i ) {
        const QChar ch = str.at(i);
        if ((ch == '[' || ch == ']' || ch == ' ') && !data.isEmpty()) {
            if (data.endsWith('*')) {
                item->setData(true);
                data.chop(1);
            }
            item->setText(data);
            data.clear();
        }
        if (ch == '[') {
            // Create new child
            QStandardItem *child = new QStandardItem;
            if (item)
                item->appendRow(child);
            else
                model.appendRow(child);
            item = child;
        } else if (ch == ']') {
            // Go up to parent
            item = item->parent();
        } else if (ch == ' ') {
            // Create new sibling
            QStandardItem *child = new QStandardItem;
            QStandardItem *parent = item->parent();
            if (parent)
                parent->appendRow(child);
            else
                model.appendRow(child);
            item = child;
        } else {
            data += ch;
        }
    }
}

class KRecursiveFilterProxyModelTest : public QObject
{
    Q_OBJECT
private:
private slots:
    void testInitialFiltering_data()
    {
        QTest::addColumn<QString>("sourceStr");
        QTest::addColumn<QString>("proxyStr");

        QTest::newRow("empty") << "[]" << "";
        QTest::newRow("no") << "[1]" << "";
        QTest::newRow("yes") << "[1*]" << "[1*]";
        QTest::newRow("second") << "[1 2*]" << "[2*]";
        QTest::newRow("child_yes") << "[1 2[2.1*]]" << "[2[2.1*]]";
        QTest::newRow("grandchild_yes") << "[1 2[2.1[2.1.1*]]]" << "[2[2.1[2.1.1*]]]";
        // 1, 3.1 and 4.2.1 match, so their parents are in the model
        QTest::newRow("more") << "[1* 2[2.1] 3[3.1*] 4[4.1 4.2[4.2.1*]]]" << "[1* 3[3.1*] 4[4.2[4.2.1*]]]";
    }

    void testInitialFiltering()
    {
        QFETCH(QString, sourceStr);
        QFETCH(QString, proxyStr);

        QStandardItemModel model;
        fillModel(model, sourceStr);
        QCOMPARE(treeAsString(model), sourceStr);

        TestModel proxy(&model);
        QCOMPARE(treeAsString(proxy), proxyStr);
    }

    // Test changing a role that is unrelated to the filtering.
    void testUnrelatedDataChange()
    {
        QStandardItemModel model;
        const QString sourceStr = "[1[1.1[1.1.1*]]]";
        fillModel(model, sourceStr);
        QCOMPARE(treeAsString(model), sourceStr);

        TestModel proxy(&model);
        QCOMPARE(treeAsString(proxy), sourceStr);

        ModelSignalSpy spy(proxy);
        QStandardItem *item_1_1_1 = model.item(0)->child(0)->child(0);

        // When changing the text on the item
        item_1_1_1->setText("ME");

        QCOMPARE(treeAsString(proxy), QString("[1[1.1[ME*]]]"));

        QCOMPARE(spy.mSignals, QStringList()
                 << QLatin1String("dataChanged(ME)")
                 << QLatin1String("dataChanged(1.1)") // ### yep, unneeded, but the proxy has no way to know that...
                 << QLatin1String("dataChanged(1)") // ### unneeded too
                 );
    }

    // Test changing a role that is unrelated to the filtering, in a hidden item.
    void testHiddenDataChange()
    {
        QStandardItemModel model;
        const QString sourceStr = "[1[1.1[1.1.1]]]";
        fillModel(model, sourceStr);
        QCOMPARE(treeAsString(model), sourceStr);

        TestModel proxy(&model);
        QCOMPARE(treeAsString(proxy), QString());

        ModelSignalSpy spy(proxy);
        QStandardItem *item_1_1_1 = model.item(0)->child(0)->child(0);

        // When changing the text on a hidden item
        item_1_1_1->setText("ME");

        QCOMPARE(treeAsString(proxy), QString());
        QCOMPARE(spy.mSignals, QStringList());
    }

    // Test that we properly react to a data-changed signal in a descendant and include all required rows
    void testDataChangeIn_data()
    {
        QTest::addColumn<QString>("sourceStr");
        QTest::addColumn<QString>("initialProxyStr");
        QTest::addColumn<QString>("add"); // set the flag on this item
        QTest::addColumn<QString>("expectedProxyStr");
        QTest::addColumn<QStringList>("expectedSignals");

        QTest::newRow("toplevel") << "[1]" << "" << "1" << "[1*]"
                                  << (QStringList() << "rowsAboutToBeInserted(1)" << "rowsInserted(1)");
        QTest::newRow("show_parents") << "[1[1.1[1.1.1]]]" << "" << "1.1.1" << "[1[1.1[1.1.1*]]]"
                                      << (QStringList() << "rowsAboutToBeInserted(1)" << "rowsInserted(1)");

        const QStringList insert_1_1_1 = QStringList() << "rowsAboutToBeInserted(1.1.1)" << "rowsInserted(1.1.1)"
                                                       << "dataChanged(1.1)" << "dataChanged(1)"; // both unneeded
        QTest::newRow("parent_visible") << "[1[1.1*[1.1.1]]]" << "[1[1.1*]]" << "1.1.1" << "[1[1.1*[1.1.1*]]]"
                                        << insert_1_1_1;

        QTest::newRow("sibling_visible") << "[1[1.1[1.1.1 1.1.2*]]]" << "[1[1.1[1.1.2*]]]" << "1.1.1" << "[1[1.1[1.1.1* 1.1.2*]]]"
                                         << insert_1_1_1;

        QTest::newRow("visible_cousin") << "[1[1.1[1.1.1 1.1.2[1.1.2.1*]]]]" << "[1[1.1[1.1.2[1.1.2.1*]]]]" << "1.1.1" << "[1[1.1[1.1.1* 1.1.2[1.1.2.1*]]]]"
                                        << insert_1_1_1;

        QTest::newRow("show_parent") << "[1[1.1[1.1.1 1.1.2] 1.2*]]" << "[1[1.2*]]" << "1.1.1" << "[1[1.1[1.1.1*] 1.2*]]"
                                     << (QStringList() << "rowsAboutToBeInserted(1.1)" << "rowsInserted(1.1)"
                                                       << "dataChanged(1)"); // unneeded

        QTest::newRow("with_children") << "[1[1.1[1.1.1[1.1.1.1*]]] 2*]" << "[1[1.1[1.1.1[1.1.1.1*]]] 2*]" << "1.1.1" << "[1[1.1[1.1.1*[1.1.1.1*]]] 2*]"
                                       << (QStringList() << "dataChanged(1.1.1)"
                                                         << "dataChanged(1.1)" << "dataChanged(1)" // both unneeded
                       );

    }

    void testDataChangeIn()
    {
        QFETCH(QString, sourceStr);
        QFETCH(QString, initialProxyStr);
        QFETCH(QString, add);
        QFETCH(QString, expectedProxyStr);
        QFETCH(QStringList, expectedSignals);

        QStandardItemModel model;
        fillModel(model, sourceStr);
        QCOMPARE(treeAsString(model), sourceStr);

        TestModel proxy(&model);
        QCOMPARE(treeAsString(proxy), initialProxyStr);

        ModelSignalSpy spy(proxy);
        // When changing the data on the designated item to show this row
        QStandardItem *itemToChange = itemByText(model, add);
        QVERIFY(!itemToChange->data().toBool());
        itemToChange->setData(true);

        // The proxy should update as expected
        QCOMPARE(treeAsString(proxy), expectedProxyStr);

        //qDebug() << spy.mSignals;
        QCOMPARE(spy.mSignals, expectedSignals);
    }

    void testDataChangeOut_data()
    {
        QTest::addColumn<QString>("sourceStr");
        QTest::addColumn<QString>("initialProxyStr");
        QTest::addColumn<QString>("remove"); // unset the flag on this item
        QTest::addColumn<QString>("expectedProxyStr");
        QTest::addColumn<QStringList>("expectedSignals");

        const QStringList remove1_1_1 = (QStringList() << "rowsAboutToBeRemoved(1.1.1)" << "rowsRemoved(1.1.1)"
                                         << QLatin1String("dataChanged(1.1)") // unneeded
                                         << QLatin1String("dataChanged(1)") // unneeded
                                         );

        QTest::newRow("toplevel") << "[1*]" << "[1*]" << "1" << ""
                                  << (QStringList() << "rowsAboutToBeRemoved(1)" << "rowsRemoved(1)");

        QTest::newRow("hide_parent") << "[1[1.1[1.1.1*]]]" << "[1[1.1[1.1.1*]]]" << "1.1.1" << "" <<
                                        (QStringList()
                                         << QLatin1String("rowsAboutToBeRemoved(1.1.1)") // ### unneeded but the proxy has no way to know that...
                                         << QLatin1String("rowsRemoved(1.1.1)") // unneeded
                                         << QLatin1String("rowsAboutToBeRemoved(1.1)") // unneeded
                                         << QLatin1String("rowsRemoved(1.1)") // unneeded
                                         << QLatin1String("rowsAboutToBeRemoved(1)")
                                         << QLatin1String("rowsRemoved(1)")
                                         );

        QTest::newRow("parent_visible") << "[1[1.1*[1.1.1*]]]" << "[1[1.1*[1.1.1*]]]" << "1.1.1" << "[1[1.1*]]"
                                        << remove1_1_1;

        QTest::newRow("visible") << "[1[1.1[1.1.1* 1.1.2*]]]" << "[1[1.1[1.1.1* 1.1.2*]]]" << "1.1.1" << "[1[1.1[1.1.2*]]]"
                                 << remove1_1_1;
        QTest::newRow("visible_cousin") << "[1[1.1[1.1.1* 1.1.2[1.1.2.1*]]]]" << "[1[1.1[1.1.1* 1.1.2[1.1.2.1*]]]]" << "1.1.1" << "[1[1.1[1.1.2[1.1.2.1*]]]]"
                                 << remove1_1_1;

        // The following tests trigger the removal of an ascendant.
        QTest::newRow("remove_parent") << "[1[1.1[1.1.1* 1.1.2] 1.2*]]" << "[1[1.1[1.1.1*] 1.2*]]" << "1.1.1" << "[1[1.2*]]"
                                      << (QStringList() << "rowsAboutToBeRemoved(1.1.1)" << "rowsRemoved(1.1.1)"
                                                        << "rowsAboutToBeRemoved(1.1)" << "rowsRemoved(1.1)"
                                                        << "dataChanged(1)" // unneeded
                                          );

        QTest::newRow("with_children") << "[1[1.1[1.1.1*[1.1.1.1*]]] 2*]" << "[1[1.1[1.1.1*[1.1.1.1*]]] 2*]" << "1.1.1" << "[1[1.1[1.1.1[1.1.1.1*]]] 2*]"
                                       << (QStringList() << "dataChanged(1.1.1)"
                                                         << "dataChanged(1.1)" << "dataChanged(1)" // both unneeded
                       );

        QTest::newRow("last_visible") << "[1[1.1[1.1.1* 1.1.2]]]" << "[1[1.1[1.1.1*]]]" << "1.1.1" << ""
                                      << (QStringList() << "rowsAboutToBeRemoved(1.1.1)" << "rowsRemoved(1.1.1)" // unneeded
                                                        << "rowsAboutToBeRemoved(1.1)" << "rowsRemoved(1.1)" // unneeded
                                                        << "rowsAboutToBeRemoved(1)" << "rowsRemoved(1)"
                                          );

    }

    void testDataChangeOut()
    {
        QFETCH(QString, sourceStr);
        QFETCH(QString, initialProxyStr);
        QFETCH(QString, remove);
        QFETCH(QString, expectedProxyStr);
        QFETCH(QStringList, expectedSignals);

        QStandardItemModel model;
        fillModel(model, sourceStr);
        QCOMPARE(treeAsString(model), sourceStr);

        TestModel proxy(&model);
        QCOMPARE(treeAsString(proxy), initialProxyStr);

        ModelSignalSpy spy(proxy);

        // When changing the data on the designated item to exclude this row again
        QStandardItem *itemToChange = itemByText(model, remove);
        QVERIFY(itemToChange->data().toBool());
        itemToChange->setData(false);

        // The proxy should update as expected
        QCOMPARE(treeAsString(proxy), expectedProxyStr);

        //qDebug() << spy.mSignals;
        QCOMPARE(spy.mSignals, expectedSignals);
    }

    void testInsert()
    {
        QStandardItemModel model;
        const QString sourceStr = "[1[1.1[1.1.1]]]";
        fillModel(model, sourceStr);
        QCOMPARE(treeAsString(model), sourceStr);

        TestModel proxy(&model);
        QCOMPARE(treeAsString(proxy), QString());

        ModelSignalSpy spy(proxy);
        QStandardItem *item_1_1_1 = model.item(0)->child(0)->child(0);
        QStandardItem *item_1_1_1_1 = new QStandardItem("1.1.1.1");
        item_1_1_1_1->setData(true);
        item_1_1_1->appendRow(item_1_1_1_1);
        QCOMPARE(treeAsString(proxy), QString("[1[1.1[1.1.1[1.1.1.1*]]]]"));

        QCOMPARE(spy.mSignals, QStringList() << QLatin1String("rowsAboutToBeInserted(1)")
                                             << QLatin1String("rowsInserted(1)"));
    }

    // Start from [1[1.1[1.1.1 1.1.2[1.1.2.1*]]]]
    // where 1.1.1 is hidden but 1.1 is shown, we want to insert a shown child in 1.1.1.
    // The proxy ensures dataChanged is called on 1.1,
    // so that 1.1.1 and 1.1.1.1 are included in the model.
    void testInsertCousin()
    {
        QStandardItemModel model;
        const QString sourceStr = "[1[1.1[1.1.1 1.1.2[1.1.2.1*]]]]";
        fillModel(model, sourceStr);
        QCOMPARE(treeAsString(model), sourceStr);

        TestModel proxy(&model);
        QCOMPARE(treeAsString(proxy), QString("[1[1.1[1.1.2[1.1.2.1*]]]]"));

        ModelSignalSpy spy(proxy);
        {
            QStandardItem *item_1_1_1_1 = new QStandardItem("1.1.1.1");
            item_1_1_1_1->setData(true);
            QStandardItem *item_1_1_1 = model.item(0)->child(0)->child(0);
            item_1_1_1->appendRow(item_1_1_1_1);
        }

        QCOMPARE(treeAsString(proxy), QString("[1[1.1[1.1.1[1.1.1.1*] 1.1.2[1.1.2.1*]]]]"));
        QCOMPARE(spy.mSignals, QStringList()
                 << QLatin1String("rowsAboutToBeInserted(1.1.1)")
                 << QLatin1String("rowsInserted(1.1.1)"));
    }

    void testInsertWithChildren()
    {
        QStandardItemModel model;
        const QString sourceStr = "[1[1.1]]";
        fillModel(model, sourceStr);
        QCOMPARE(treeAsString(model), sourceStr);

        TestModel proxy(&model);
        QCOMPARE(treeAsString(proxy), QString());

        ModelSignalSpy spy(proxy);
        {
            QStandardItem *item_1_1_1 = new QStandardItem("1.1.1");
            QStandardItem *item_1_1_1_1 = new QStandardItem("1.1.1.1");
            item_1_1_1_1->setData(true);
            item_1_1_1->appendRow(item_1_1_1_1);

            QStandardItem *item_1_1 = model.item(0)->child(0);
            item_1_1->appendRow(item_1_1_1);
        }

        QCOMPARE(treeAsString(proxy), QString("[1[1.1[1.1.1[1.1.1.1*]]]]"));
        QCOMPARE(spy.mSignals, QStringList()
                 << QLatin1String("rowsAboutToBeInserted(1)")
                 << QLatin1String("rowsInserted(1)"));
    }

    void testInsertIntoVisibleWithChildren()
    {
        QStandardItemModel model;
        const QString sourceStr = "[1[1.1[1.1.1*]]]";
        fillModel(model, sourceStr);
        QCOMPARE(treeAsString(model), sourceStr);

        TestModel proxy(&model);
        QCOMPARE(treeAsString(proxy), sourceStr);

        ModelSignalSpy spy(proxy);
        {
            QStandardItem *item_1_1_2 = new QStandardItem("1.1.2");
            QStandardItem *item_1_1_2_1 = new QStandardItem("1.1.2.1");
            item_1_1_2_1->setData(true);
            item_1_1_2->appendRow(item_1_1_2_1);

            QStandardItem *item_1_1 = model.item(0)->child(0);
            item_1_1->appendRow(item_1_1_2);
        }

        QCOMPARE(treeAsString(proxy), QString("[1[1.1[1.1.1* 1.1.2[1.1.2.1*]]]]"));
        QCOMPARE(spy.mSignals, QStringList()
                 << QLatin1String("rowsAboutToBeInserted(1.1.2)")
                 << QLatin1String("rowsInserted(1.1.2)"));
    }

    void testInsertHidden() // inserting filtered-out rows shouldn't emit anything
    {
        QStandardItemModel model;
        const QString sourceStr = "[1[1.1]]";
        fillModel(model, sourceStr);
        QCOMPARE(treeAsString(model), sourceStr);

        TestModel proxy(&model);
        QCOMPARE(treeAsString(proxy), QString());

        ModelSignalSpy spy(proxy);
        {
            QStandardItem *item_1_1_1 = new QStandardItem("1.1.1");
            QStandardItem *item_1_1_1_1 = new QStandardItem("1.1.1.1");
            item_1_1_1->appendRow(item_1_1_1_1);

            QStandardItem *item_1_1 = model.item(0)->child(0);
            item_1_1->appendRow(item_1_1_1);
        }

        QCOMPARE(treeAsString(proxy), QString());
        QCOMPARE(spy.mSignals, QStringList());
    }

    void testConsecutiveInserts_data()
    {
        testInitialFiltering_data();
    }

    void testConsecutiveInserts()
    {
        QFETCH(QString, sourceStr);
        QFETCH(QString, proxyStr);

        QStandardItemModel model;
        TestModel proxy(&model); // this time the proxy listens to the model while we fill it

        fillModel(model, sourceStr);
        QCOMPARE(treeAsString(model), sourceStr);
        QCOMPARE(treeAsString(proxy), proxyStr);
    }

    void testRemove_data()
    {
        QTest::addColumn<QString>("sourceStr");
        QTest::addColumn<QString>("initialProxyStr");
        QTest::addColumn<QString>("remove"); // remove this item
        QTest::addColumn<QString>("expectedProxyStr");
        QTest::addColumn<QStringList>("expectedSignals");

        const QStringList remove1_1_1 = (QStringList() << "rowsAboutToBeRemoved(1.1.1)" << "rowsRemoved(1.1.1)");

        QTest::newRow("toplevel") << "[1* 2* 3*]" << "[1* 2* 3*]" << "1" << "[2* 3*]"
                                  << (QStringList() << "rowsAboutToBeRemoved(1)" << "rowsRemoved(1)");

        QTest::newRow("remove_hidden") << "[1 2* 3*]" << "[2* 3*]" << "1" << "[2* 3*]" << QStringList();

        QTest::newRow("parent_hidden") << "[1[1.1[1.1.1]]]" << "" << "1.1.1" << "" << QStringList();

        QTest::newRow("child_hidden") << "[1[1.1*[1.1.1]]]" << "[1[1.1*]]" << "1.1.1" << "[1[1.1*]]" << QStringList();

        QTest::newRow("parent_visible") << "[1[1.1*[1.1.1*]]]" << "[1[1.1*[1.1.1*]]]" << "1.1.1" << "[1[1.1*]]"
                                        << remove1_1_1;

        QTest::newRow("visible") << "[1[1.1[1.1.1* 1.1.2*]]]" << "[1[1.1[1.1.1* 1.1.2*]]]" << "1.1.1" << "[1[1.1[1.1.2*]]]"
                                 << remove1_1_1;
        QTest::newRow("visible_cousin") << "[1[1.1[1.1.1* 1.1.2[1.1.2.1*]]]]" << "[1[1.1[1.1.1* 1.1.2[1.1.2.1*]]]]" << "1.1.1" << "[1[1.1[1.1.2[1.1.2.1*]]]]"
                                 << remove1_1_1;

        // The following tests trigger the removal of an ascendant.
        // We could optimize the rows{AboutToBe,}Removed(1.1.1) away, but that would
        // require a filterAcceptsRow variant that ignores the about-to-be-removed row,
        // in order to move the going-up loop from Removed to AboutToBeRemoved.
        // It doesn't really hurt to have both pairs of signals though.

        QTest::newRow("remove_parent") << "[1[1.1[1.1.1* 1.1.2] 1.2*]]" << "[1[1.1[1.1.1*] 1.2*]]" << "1.1.1" << "[1[1.2*]]"
                                      << (QStringList() << "rowsAboutToBeRemoved(1.1.1)" << "rowsRemoved(1.1.1)"
                                                        << "rowsAboutToBeRemoved(1.1)" << "rowsRemoved(1.1)"
                                          );

        QTest::newRow("with_children") << "[1[1.1[1.1.1[1.1.1.1*]]] 2*]" << "[1[1.1[1.1.1[1.1.1.1*]]] 2*]" << "1.1.1" << "[2*]"
                                       << (QStringList() << "rowsAboutToBeRemoved(1.1.1)" << "rowsRemoved(1.1.1)"
                                       << "rowsAboutToBeRemoved(1)" << "rowsRemoved(1)"
                       );

        QTest::newRow("last_visible") << "[1[1.1[1.1.1* 1.1.2]]]" << "[1[1.1[1.1.1*]]]" << "1.1.1" << ""
                                      << (QStringList() << "rowsAboutToBeRemoved(1.1.1)" << "rowsRemoved(1.1.1)"
                                                        << "rowsAboutToBeRemoved(1)" << "rowsRemoved(1)"
                                          );


    }

    void testRemove()
    {
        QFETCH(QString, sourceStr);
        QFETCH(QString, initialProxyStr);
        QFETCH(QString, remove);
        QFETCH(QString, expectedProxyStr);
        QFETCH(QStringList, expectedSignals);

        QStandardItemModel model;
        fillModel(model, sourceStr);
        QCOMPARE(treeAsString(model), sourceStr);

        TestModel proxy(&model);
        QCOMPARE(treeAsString(proxy), initialProxyStr);

        ModelSignalSpy spy(proxy);
        QStandardItem *itemToRemove = itemByText(model, remove);
        QVERIFY(itemToRemove);
        if (itemToRemove->parent())
            itemToRemove->parent()->removeRow(itemToRemove->row());
        else
            model.removeRow(itemToRemove->row());
        QCOMPARE(treeAsString(proxy), expectedProxyStr);

        //qDebug() << spy.mSignals;
        QCOMPARE(spy.mSignals, expectedSignals);
    }

private:
    QStandardItem *itemByText(const QStandardItemModel& model, const QString &text) const {
        QModelIndexList list = model.match(model.index(0, 0), Qt::DisplayRole, text, 1, Qt::MatchRecursive);
        return list.isEmpty() ? 0 : model.itemFromIndex(list.first());
    }
};

QTEST_KDEMAIN(KRecursiveFilterProxyModelTest, NoGUI)

#include "krecursivefilterproxymodeltest.moc"
