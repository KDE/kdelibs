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

void PartTest::testAutomaticMimeType()
{
    TestPart* part = new TestPart(0, 0);
    QVERIFY(part->closeUrl()); // nothing to do, no error
    QVERIFY(part->arguments().mimeType().isEmpty());
    // open a file, and test the detected mimetype
    part->openUrl(KUrl(KDESRCDIR "/notepad.desktop"));
    QCOMPARE(part->arguments().mimeType(), QString::fromLatin1("application/x-desktop"));

    // manually closing, no mimetype should be stored now
    part->closeUrl();
    QVERIFY(part->arguments().mimeType().isEmpty());

    // open a new file, and test again its (autdetected) mimetype
    part->openUrl(KUrl(KDESRCDIR "/parttest.cpp"));
    QCOMPARE(part->arguments().mimeType(), QString("text/x-c++src"));

    // open a new file, but without explicitely close the first
    part->openUrl(KUrl(KDESRCDIR "/notepad.desktop"));
    // test again its (autdetected) mimetype
    QCOMPARE(part->arguments().mimeType(), QString::fromLatin1("application/x-desktop"));

    // open a new file, but manually forcing a mimetype
    KParts::OpenUrlArguments args;
    args.setMimeType("application/xml");
    part->setArguments(args);
    QVERIFY(compareArgs(args, part->arguments()));
    part->openUrl(KUrl(KDESRCDIR "/parttest.cpp"));
    QCOMPARE(part->arguments().mimeType(), QString::fromLatin1("application/xml"));

    // clear the args and open a new file, reactivating the automatic mimetype detection again
    part->setArguments(KParts::OpenUrlArguments());
    part->openUrl(KUrl(KDESRCDIR "/notepad.desktop"));
    // test again its (autdetected) mimetype
    QCOMPARE(part->arguments().mimeType(), QString::fromLatin1("application/x-desktop"));

    delete part;
}

#include <kparts/mainwindow.h>
#include <ktoolbar.h>
#include <kconfiggroup.h>
#include <ktoggletoolbaraction.h>
class MyMainWindow : public KParts::MainWindow
{
public:
    MyMainWindow() : KParts::MainWindow() {
        tb = new KToolBar(this);
        tb->setObjectName("testtbvisibility");
    }

    // createGUI and saveAutoSaveSettings are protected, so the whole test is here:
    void testToolbarVisibility()
    {
        QVERIFY(tb->isVisible());

        TestPart* part = new TestPart(0, 0);
        // TODO define xml with a toolbar for the part
        // and put some saved settings into qttestrc in order to test
        // r347935+r348051, i.e. the fact that KParts::MainWindow::createGUI
        // will apply the toolbar settings (and that they won't have been
        // erased by the previous call to saveMainWindowSettings...)
        this->createGUI(part);

        QVERIFY(tb->isVisible());
        this->saveAutoSaveSettings();

        // Hide the toolbar using the action (so that setSettingsDirty is called, too)
        KToggleToolBarAction action(tb, QString(), 0);
        action.trigger();
        QVERIFY(!tb->isVisible());

        // Switch the active part, and check that
        // the toolbar doesn't magically reappear,
        // as it did when createGUI was calling applyMainWindowSettings
        this->createGUI(0);
        QVERIFY(!tb->isVisible());
        this->createGUI(part);
        QVERIFY(!tb->isVisible());

        // All ok, show it again so that test can be run again :)
        action.trigger();
        QVERIFY(tb->isVisible());
        close();
    }
private:
    KToolBar* tb;
};

// A KParts::MainWindow unit test
void PartTest::testToolbarVisibility()
{
    // The bug was: hide a toolbar in konqueror,
    // then switch tabs -> the toolbar shows again
    // (unless you waited for the autosave timer to kick in)
    MyMainWindow window;
    KConfigGroup cg(KGlobal::config(), "kxmlgui_unittest");
    window.setAutoSaveSettings(cg.name());
    window.show();
    window.testToolbarVisibility();
}

#include "parttest.moc"
