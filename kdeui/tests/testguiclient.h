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

#ifndef TESTGUICLIENT_H
#define TESTGUICLIENT_H

#include <kstandarddirs.h>
#include <kcomponentdata.h>
#include <kxmlguiclient.h>

// because setDOMDocument and setXML are protected
class TestGuiClient : public KXMLGUIClient
{
public:
    TestGuiClient(const QByteArray& xml = QByteArray())
        : KXMLGUIClient()
    {
        if (!xml.isNull())
            setXML(QString::fromLatin1(xml));
    }
    void setXMLFilePublic(const QString& file, bool merge = false, bool setXMLDoc = true)
    {
      	setXMLFile(file, merge, setXMLDoc);
    }
    void createGUI(const QByteArray& xml, bool withUiStandards = false)
    {
        if (withUiStandards) {
            QString uis = KStandardDirs::locate("config", "ui/ui_standards.rc", componentData());
            QVERIFY(!uis.isEmpty());
            setXMLFile(uis);
        }

        setXML(QString::fromLatin1(xml), true);
    }
    void createActions(const QStringList& actionNames)
    {
        KActionCollection* coll = actionCollection();
        Q_FOREACH(const QString& actionName, actionNames) {
            coll->addAction(actionName)->setText("Action");
        }
    }

    // Find a toolbar (created by this guiclient)
    KToolBar* toolBarByName(const QString& name) {
        //qDebug() << "containers:" << factory()->containers("ToolBar");
        QWidget* toolBarW = factory()->container(name, this);
        if (!toolBarW) {
            kWarning() << "No toolbar found with name" << name;
        }
        Q_ASSERT(toolBarW);
        KToolBar* toolBar = qobject_cast<KToolBar *>(toolBarW);
        Q_ASSERT(toolBar);
        return toolBar;
    }
};

#endif /* TESTGUICLIENT_H */

