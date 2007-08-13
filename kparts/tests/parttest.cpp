/*
    Copyright (c) 2007 David Faure <faure@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/
#include "parttest.h"
#include "qtest_kde.h"

#include <kparts/part.h>
#include <QWidget>

QTEST_KDEMAIN( PartTest, GUI )

class TestPart : public KParts::ReadOnlyPart
{
public:
    TestPart(QObject* parent, QWidget* parentWidget)
        : KParts::ReadOnlyPart(parent),
          m_openFileCalled(false) {
        setWidget(new QWidget(parentWidget));
    }

    bool openFileCalled() const { return m_openFileCalled; }
protected:
    /*reimp*/ bool openFile() {
        m_openFileCalled = true;
        return true;
    }
private:
    bool m_openFileCalled;
};

void PartTest::testAutoDeletePart()
{
    KParts::Part* part = new TestPart(0, 0);
    QPointer<KParts::Part> partPointer(part);
    delete part->widget();
    QVERIFY(partPointer.isNull());
}

void PartTest::testAutoDeleteWidget()
{
    KParts::Part* part = new TestPart(0, 0);
    QPointer<KParts::Part> partPointer(part);
    QPointer<QWidget> widgetPointer(part->widget());
    delete part;
    QVERIFY(widgetPointer.isNull());
}

void PartTest::testNoAutoDeletePart()
{
    KParts::Part* part = new TestPart(0, 0);
    part->setAutoDeletePart(false);
    QPointer<KParts::Part> partPointer(part);
    delete part->widget();
    QVERIFY(part->widget() == 0);
    QCOMPARE(static_cast<KParts::Part*>(partPointer), part);
    delete part;
}

void PartTest::testNoAutoDeleteWidget()
{
    KParts::Part* part = new TestPart(0, 0);
    part->setAutoDeleteWidget(false);
    QWidget* widget = part->widget();
    QVERIFY(widget);
    QPointer<QWidget> widgetPointer(part->widget());
    delete part;
    QCOMPARE(static_cast<QWidget*>(widgetPointer), widget);
    delete widget;
}

// There is no operator== in OpenUrlArguments because it's only useful in unit tests
static bool compareArgs(const KParts::OpenUrlArguments& arg1,
                        const KParts::OpenUrlArguments& arg2)
{
    return arg1.mimeType() == arg2.mimeType() &&
        arg1.xOffset() == arg2.xOffset() &&
        arg1.yOffset() == arg2.yOffset() &&
        arg1.reload() == arg2.reload();
}

void PartTest::testOpenUrlArguments()
{
    TestPart* part = new TestPart(0, 0);
    QVERIFY(part->closeUrl()); // nothing to do, no error
    QVERIFY(part->arguments().mimeType().isEmpty());
    KParts::OpenUrlArguments args;
    args.setMimeType("application/xml");
    args.setXOffset(50);
    args.setYOffset(10);
    args.setReload(true);
    part->setArguments(args);
    QVERIFY(compareArgs(args, part->arguments()));
    part->openUrl(KUrl(KDESRCDIR "/parttest.cpp"));
    QVERIFY(part->openFileCalled());
    QVERIFY(compareArgs(args, part->arguments()));

    // Explicit call to closeUrl: arguments are cleared
    part->closeUrl();
    QVERIFY(part->arguments().mimeType().isEmpty());

    // Calling openUrl with local file: mimetype is determined
    part->openUrl(KUrl(KDESRCDIR "/parttest.cpp"));
    QCOMPARE(part->arguments().mimeType(), QString("text/x-c++src"));
    // (for a remote url it would be determined during downloading)

    delete part;
}

#include "parttest.moc"
