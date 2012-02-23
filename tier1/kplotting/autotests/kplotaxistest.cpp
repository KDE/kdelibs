/*
    Copyright (C) 2012 Benjamin Port <benjamin.port@ben2367.fr>

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

#include <kplotaxis.h>

#include <QtTest>

class KPlotAxisTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase()
    {
        m_kPlotAxis = new KPlotAxis(QString::fromLatin1("label"));
    }

    void cleanupTestCase()
    {
        delete m_kPlotAxis;
    }

    void testVisible()
    {
        m_kPlotAxis->setVisible(true);
        QCOMPARE(m_kPlotAxis->isVisible(), true);

        m_kPlotAxis->setVisible(false);
        QCOMPARE(m_kPlotAxis->isVisible(), false);
    }
    
    void testTickLabelsShown()
    {
        m_kPlotAxis->setTickLabelsShown(true);
        QCOMPARE(m_kPlotAxis->areTickLabelsShown(), true);

        m_kPlotAxis->setTickLabelsShown(false);
        QCOMPARE(m_kPlotAxis->areTickLabelsShown(), false);
    }

    void testLabel()
    {
        QCOMPARE(m_kPlotAxis->label(), QString::fromLatin1("label"));

        m_kPlotAxis->setLabel(QString::fromLatin1("newLabel"));
        QCOMPARE(m_kPlotAxis->label(), QString::fromLatin1("newLabel"));
    }

    void testTickLabelFormat()
    {
        m_kPlotAxis->setTickLabelFormat('e', 3, 2);
        QCOMPARE(m_kPlotAxis->tickLabelFormat(), 'e');
        QCOMPARE(m_kPlotAxis->tickLabelWidth(), 3);
        QCOMPARE(m_kPlotAxis->tickLabelPrecision(), 2);
    }

private:
    KPlotAxis* m_kPlotAxis;
};

QTEST_MAIN(KPlotAxisTest)

#include "kplotaxistest.moc"
