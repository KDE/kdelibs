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

#include <QPushButton>

#include <kapplication.h>
#include <kxmlguiwindow.h>
#include <kaboutdata.h>
#include <kcmdlineargs.h>
#include <kactioncollection.h>
#include <kstandarddirs.h>
#include <kaction.h>

class MainWindow
    : public KXmlGuiWindow
{
public:
    MainWindow(QWidget *parent = 0);

private:
    void setupActions();
};

void MainWindow::setupActions()
{
    KAction *testAction = new KAction(this);
    testAction->setText("Test");
    testAction->setIcon(KIcon("document-new"));
    actionCollection()->addAction("test", testAction);

    KStandardAction::quit(kapp, SLOT(quit()), actionCollection());

    setupGUI();
}

MainWindow::MainWindow(QWidget *parent)
    : KXmlGuiWindow(parent)
{
    setXMLFile(KDESRCDIR "/kxmlguiwindowtestui.rc");
    setCentralWidget(new QPushButton("Click me", this));
    setupActions();
}

int main(int argc, char **argv)
{
    KAboutData aboutData("kxmlguiwindowtest", 0,
        ki18n("kxmlguiwindowtest"), "0.1",
        ki18n("kxmlguiwindowtest"),
        KAboutData::License_LGPL,
        ki18n("Copyright (c) 2008 Rafael Fernandez Lopez") );
    KCmdLineArgs::init(argc, argv, &aboutData);
    KApplication app;

    KGlobal::dirs()->addResourceDir("data", KDESRCDIR);

    MainWindow *mainWindow = new MainWindow;
    mainWindow->show();

    return app.exec();
}
