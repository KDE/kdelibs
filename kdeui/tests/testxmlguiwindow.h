/* This file is part of the KDE libraries
    Copyright (c) 2009 David Faure <faure@kde.org>

    This library is free software; you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation; either version 2 of the License or ( at
    your option ) version 3 or, at the discretion of KDE e.V. ( which shall
    act as a proxy as in section 14 of the GPLv3 ), any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#ifndef TESTXMLGUIWINDOW_H
#define TESTXMLGUIWINDOW_H

#include <kactioncollection.h>
#include <kaction.h>
#include <kdebug.h>
#include <QMenu>
#include <QResizeEvent>
#include <ktoolbar.h>
#include <kxmlguifactory.h>
#include <kxmlguiwindow.h>
#include <QApplication>
#include <ktemporaryfile.h>

class TestXmlGuiWindow : public KXmlGuiWindow
{
public:
    TestXmlGuiWindow(const QByteArray& xml = QByteArray()) : KXmlGuiWindow() {
        QVERIFY(m_userFile.open());
        m_userFile.write(xml);
        m_fileName = m_userFile.fileName(); // remember filename
        Q_ASSERT(!m_fileName.isEmpty());
        m_userFile.close(); // write to disk
    }
    void createGUI() {
        KXmlGuiWindow::createGUI(m_fileName);
    }
    void createGUIBad() {
        KXmlGuiWindow::createGUI("dontexist.rc");
    }

    // Same as in KMainWindow_UnitTest
    void reallyResize(int width, int height) {
        const QSize oldSize = size();
        resize(width, height);
        // Send the pending resize event (resize() only sets Qt::WA_PendingResizeEvent)
        QResizeEvent e(size(), oldSize);
        QApplication::sendEvent(this, &e);
    }

    // KMainWindow::toolBar(name) creates it if not found, and we don't want that.
    // Also this way we test container() rather than just doing a findChild.
    KToolBar* toolBarByName(const QString& name) {
        KXMLGUIFactory* factory = guiFactory();
        //qDebug() << "containers:" << factory->containers("ToolBar");
        QWidget* toolBarW = factory->container(name, this);
        if (!toolBarW) {
            kWarning() << "No toolbar found with name" << name;
        }
        Q_ASSERT(toolBarW);
        KToolBar* toolBar = qobject_cast<KToolBar *>(toolBarW);
        Q_ASSERT(toolBar);
        return toolBar;
    }
    QMenu* menuByName(const QString& name) {
        KXMLGUIFactory* factory = guiFactory();
        QWidget* menuW = factory->container(name, this);
        Q_ASSERT(menuW);
        QMenu* menu = qobject_cast<QMenu *>(menuW);
        Q_ASSERT(menu);
        return menu;
    }

    void createActions(const QStringList& actionNames)
    {
        KActionCollection* coll = actionCollection();
        Q_FOREACH(const QString& actionName, actionNames) {
            coll->addAction(actionName)->setText("Action");
        }
    }

private:
    KTemporaryFile m_userFile;
    QString m_fileName;
};

#endif /* TESTXMLGUIWINDOW_H */

