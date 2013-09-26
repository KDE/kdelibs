/* This file is part of the KDE libraries
   Copyright (C) 2008 Rafael Fernández López <ereslibre@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include <QAction>
#include <QTextEdit>
#include <QTimer>
#include <QApplication>
#include <QStandardPaths>
#include <QtTest/QtTest>

#include <kxmlguiwindow.h>
#include <kactioncollection.h>
#include <kmessagebox.h>
#include <kconfiggroup.h>

// BUG: if this symbol is defined the problem consists on:
//      - main window is created.
//      - settings are saved (and applied), but in this case no toolbars exist yet, so they don't
//        apply to any toolbar.
//      - after 1 second the GUI is created.
//
//      How to reproduce ?
//          - Move one toolbar to other place (bottom, left, right, or deattach it).
//          - Close the test (so settings are saved).
//          - Reopen the test. The toolbar you moved is not keeping the place you specified.
#define REPRODUCE_TOOLBAR_BUG

class MainWindow
    : public KXmlGuiWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = 0);

public Q_SLOTS:
    void slotTest();
    void slotCreate();

private:
    void setupActions();
};

void MainWindow::slotTest()
{
    KMessageBox::information(0, QStringLiteral("Test"), QLatin1String("Test"));
}

void MainWindow::slotCreate()
{
    setupGUI(ToolBar);
    createGUI(xmlFile());

    if (autoSaveConfigGroup().isValid()) {
        applyMainWindowSettings(autoSaveConfigGroup());
    }
}

void MainWindow::setupActions()
{
    QAction *testAction = new QAction(this);
    testAction->setText(QStringLiteral("Test"));
    testAction->setIcon(QIcon::fromTheme(QStringLiteral("kde")));
    actionCollection()->addAction(QStringLiteral("test"), testAction);
    connect(testAction, SIGNAL(triggered(bool)), this, SLOT(slotTest()));

    KStandardAction::quit(qApp, SLOT(quit()), actionCollection());

    setAutoSaveSettings();

    // BUG: if the GUI is created after an amount of time (so settings have been saved), then toolbars
    //      are shown misplaced. KMainWindow uses a 500 ms timer to save window settings.
#ifdef REPRODUCE_TOOLBAR_BUG
    QTimer::singleShot(1000, this, SLOT(slotCreate())); // more than 500 ms so the main window has saved settings.
                                                        // We can think of this case on natural applications when they
                                                        // load plugins and change parts. It can take 1 second perfectly.
#else
    QTimer::singleShot(0, this, SLOT(slotCreate()));
#endif
}

MainWindow::MainWindow(QWidget *parent)
    : KXmlGuiWindow(parent)
{
    setXMLFile(QFINDTESTDATA("kxmlguiwindowtestui.rc"), true);
    // Because we use a full path in setXMLFile, we need to call setLocalXMLFile too.
    // In your apps, just pass a relative filename to setXMLFile instead.
    setLocalXMLFile(QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + QLatin1Char('/') +
                                                     QStringLiteral("kxmlguiwindowtest/kxmlguiwindowtestui.rc"));

    setCentralWidget(new QTextEdit(this));
    setupActions();
}

int main(int argc, char **argv)
{
    QApplication app(argc, argv);

    MainWindow *mainWindow = new MainWindow;
    mainWindow->show();

    return app.exec();
}

#include "kxmlguiwindowtest.moc"
