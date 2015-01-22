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

class ModelSignalSpy : public QObject {
    Q_OBJECT
public:
    explicit ModelSignalSpy(QAbstractItemModel &model) {
        connect(&model, SIGNAL(rowsInserted(QModelIndex, int, int)), this, SLOT(onRowsInserted(QModelIndex,int,int)));
        connect(&model, SIGNAL(rowsRemoved(QModelIndex, int, int)), this, SLOT(onRowsRemoved(QModelIndex,int,int)));
        connect(&model, SIGNAL(rowsMoved(QModelIndex, int, int, QModelIndex, int)), this, SLOT(onRowsMoved(QModelIndex,int,int, QModelIndex, int)));
        connect(&model, SIGNAL(dataChanged(QModelIndex,QModelIndex)), this, SLOT(onDataChanged(QModelIndex,QModelIndex)));
        connect(&model, SIGNAL(layoutChanged()), this, SLOT(onLayoutChanged()));
        connect(&model, SIGNAL(modelReset()), this, SLOT(onModelReset()));
    }

    QStringList mSignals;
    QModelIndex parent;
    int start;
    int end;

public Q_SLOTS:
    void onRowsInserted(QModelIndex p, int s, int e) {
        mSignals << QLatin1String("rowsInserted");
        parent = p;
        start = s;
        end = e;
    }
    void onRowsRemoved(QModelIndex p, int s, int e) {
        mSignals << QLatin1String("rowsRemoved");
        parent = p;
        start = s;
        end = e;
    }
    void onRowsMoved(QModelIndex,int,int,QModelIndex,int) {
        mSignals << QLatin1String("rowsMoved");
    }
    void onDataChanged(QModelIndex,QModelIndex) {
        mSignals << QLatin1String("dataChanged");
    }
    void onLayoutChanged() {
        mSignals << QLatin1String("layoutChanged");
    }
    void onModelReset() {
        mSignals << QLatin1String("modelReset");
    }
};

class TestModel : public KRecursiveFilterProxyModel
{
    Q_OBJECT
public:
    virtual bool acceptRow(int sourceRow, const QModelIndex &sourceParent) const
    {
        // qDebug() << sourceModel()->index(sourceRow, 0, sourceParent).data().toString() << sourceModel()->index(sourceRow, 0, sourceParent).data(Qt::UserRole+1).toBool();
        return sourceModel()->index(sourceRow, 0, sourceParent).data(Qt::UserRole+1).toBool();
    }
};

static QModelIndex getIndex(const char *string, const QAbstractItemModel &model)
{
    QModelIndexList list = model.match(model.index(0, 0), Qt::DisplayRole, QString::fromLatin1(string), 1, Qt::MatchRecursive);
    if (list.isEmpty()) {
        return QModelIndex();
    }
    return list.first();
}

class KRecursiveFilterProxyModelTest : public QObject
{
    Q_OBJECT
private:

private slots:
    // Test that we properly react to a data-changed signal in a descendant and include all required rows
    void testDataChange()
    {
        QStandardItemModel model;
        TestModel proxy;
        proxy.setSourceModel(&model);

        QStandardItem *index1 = new QStandardItem("1");
        index1->setData(false);
        model.appendRow(index1);

        QVERIFY(!getIndex("1", proxy).isValid());

        QStandardItem *index1_1_1 = new QStandardItem("1.1.1");
        index1_1_1->setData(false);
        QStandardItem *index1_1 = new QStandardItem("1.1");
        index1_1->setData(false);
        index1_1->appendRow(index1_1_1);
        index1->appendRow(index1_1);

        ModelSignalSpy spy(proxy);
        index1_1_1->setData(true);

        QVERIFY(getIndex("1", proxy).isValid());
        QVERIFY(getIndex("1.1", proxy).isValid());
        QVERIFY(getIndex("1.1.1", proxy).isValid());

        QCOMPARE(spy.mSignals, QStringList() << QLatin1String("rowsInserted"));
    }

    void testInsert()
    {
        QStandardItemModel model;
        TestModel proxy;
        proxy.setSourceModel(&model);

        QStandardItem *index1 = new QStandardItem("index1");
        index1->setData(false);
        model.appendRow(index1);

        QStandardItem *index1_1 = new QStandardItem("index1_1");
        index1_1->setData(false);
        index1->appendRow(index1_1);

        QStandardItem *index1_1_1 = new QStandardItem("index1_1_1");
        index1_1_1->setData(false);
        index1_1->appendRow(index1_1_1);

        QVERIFY(!getIndex("index1", proxy).isValid());
        QVERIFY(!getIndex("index1_1", proxy).isValid());
        QVERIFY(!getIndex("index1_1_1", proxy).isValid());

        ModelSignalSpy spy(proxy);
        {
            QStandardItem *index1_1_1_1 = new QStandardItem("index1_1_1_1");
            index1_1_1_1->setData(true);
            index1_1_1->appendRow(index1_1_1_1);
        }

        QVERIFY(getIndex("index1", proxy).isValid());
        QVERIFY(getIndex("index1_1", proxy).isValid());
        QVERIFY(getIndex("index1_1_1", proxy).isValid());
        QVERIFY(getIndex("index1_1_1_1", proxy).isValid());
        QCOMPARE(spy.mSignals, QStringList() << QLatin1String("rowsInserted"));
        QCOMPARE(spy.parent, QModelIndex());
    }


    // We want to get index1_1_1_1 into the model which is a descendant of index1_1.
    // index1_1 is already in the model from the neighbor2 branch. We must ensure dataChange is called on index1_1, 
    // so index1_1_1_1 is included in the model.
    void testNeighborPath()
    {
        QStandardItemModel model;
        TestModel proxy;
        proxy.setSourceModel(&model);

        QStandardItem *index1 = new QStandardItem("index1");
        index1->setData(false);
        model.appendRow(index1);

        QStandardItem *index1_1 = new QStandardItem("index1_1");
        index1_1->setData(false);
        index1->appendRow(index1_1);

        QStandardItem *index1_1_1 = new QStandardItem("index1_1_1");
        index1_1_1->setData(false);
        index1_1->appendRow(index1_1_1);

        {
            QStandardItem *nb1 = new QStandardItem("neighbor");
            nb1->setData(false);
            index1_1->appendRow(nb1);

            QStandardItem *nb2 = new QStandardItem("neighbor2");
            nb2->setData(true);
            nb1->appendRow(nb2);
        }

        //These tests affect the test. It seems without them the mapping is not created in qsortfilterproxymodel, resulting in the item
        //simply getting added later on. With these the model didn't react to the added index1_1_1_1 as it should.
        QVERIFY(!getIndex("index1_1_1", proxy).isValid());
        QVERIFY(getIndex("index1_1", proxy).isValid());
        QVERIFY(getIndex("neighbor", proxy).isValid());
        QVERIFY(getIndex("neighbor2", proxy).isValid());

        ModelSignalSpy spy(proxy);

        {
            QStandardItem *index1_1_1_1 = new QStandardItem("index1_1_1_1");
            index1_1_1_1->setData(true);
            index1_1_1->appendRow(index1_1_1_1);
        }

        QVERIFY(getIndex("index1_1_1", proxy).isValid());
        QVERIFY(getIndex("index1_1_1_1", proxy).isValid());
        //The dataChanged signals are not intentional and caused by refreshAscendantMapping. Unfortunately we can't avoid them.
        QCOMPARE(spy.mSignals, QStringList() << QLatin1String("rowsInserted") << QLatin1String("dataChanged") << QLatin1String("dataChanged"));
    }

};

QTEST_KDEMAIN(KRecursiveFilterProxyModelTest, NoGUI)

#include "krecursivefilterproxymodeltest.moc"
