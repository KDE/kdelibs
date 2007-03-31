/* This file is part of the KDE libraries
    Copyright (C) 2006 Simon Hausmann <hausmann@kde.org>
    Copyright (C) 2006 Hamish Rodda <rodda@kde.org>

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

#ifndef KLIVEUI_P_H
#define KLIVEUI_P_H

#include <kdeui_export.h>
#include <kcomponentdata.h>

#include <QtXml/QXmlContentHandler>
#include <QStack>
#include <QObject>
#include <QSet>
#include <QAction>
#include <QPointer>

namespace KLiveUi
{
    enum WidgetType {
        MenuBar,
        Menu,
        ToolBar
    };

    enum CommandType {
        NoCommand,
        BeginWidgetCommand,
        EndWidgetCommand,
        AddActionCommand,
        BeginMergeCommand,
        EndMergeCommand
    };
}

struct KLiveUiCommand {
    inline KLiveUiCommand()
        : type(KLiveUi::NoCommand), widgetType(KLiveUi::Menu),
          action(0)
    {}

    KLiveUi::CommandType type;
    KLiveUi::WidgetType widgetType;
    QAction *action;
    QString title;
    QString name;
};

typedef QList<KLiveUiCommand> KLiveUiStorage;

class KLiveUiComponentPrivate
{
public:
    KLiveUiComponentPrivate()
    {
    }

    ~KLiveUiComponentPrivate()
    {
    }

    inline void _k_subComponentDestroyed(QObject *o) {
        subComponents.removeAll(static_cast<KLiveUiComponent *>(o));
    }

    void addActiveAction(QAction *a);

    inline void _k_activeActionDestroyed(QObject *o) {
        activeActions.remove(static_cast<QAction *>(o));
    }

    QPointer<QWidget> builderWidget;
    KLiveUiStorage    storage;
    QList<KLiveUiComponent *> subComponents;
    QSet<QAction *> activeActions;
    KComponentData componentData;
    KLiveUiComponent *q;
};

namespace KLiveUiPrivate {
// ######################################
    class MenuOrWidgetDeleter : public QObject
    {
        Q_OBJECT
    public:
        MenuOrWidgetDeleter(QWidget *widgetOrMenu, QObject *component);
        virtual ~MenuOrWidgetDeleter();

        QPointer<QWidget> widgetOrMenu;
    };

class XmlGuiHandler : public QXmlContentHandler
{
public:
    XmlGuiHandler(KLiveUiBuilder *builder, QObject *component, KActionCollection *collection);

    virtual bool startElement(const QString & /*namespaceURI*/, const QString & /*localName*/, const QString &qName, const QXmlAttributes &attributes);
    virtual bool endElement(const QString & /*namespaceURI*/, const QString & /*localName*/, const QString &qName);
    virtual bool characters(const QString &text);

    virtual void setDocumentLocator(QXmlLocator*) { }
    virtual bool startDocument() { return true; }
    virtual bool endDocument() { return true; }
    virtual bool startPrefixMapping(const QString&, const QString&) { return true; }
    virtual bool endPrefixMapping(const QString&) { return true; }
    virtual bool ignorableWhitespace(const QString&) { return true; }
    virtual bool processingInstruction(const QString&, const QString&) { return true; }
    virtual bool skippedEntity(const QString&) { return true; }
    virtual QString errorString() const { return QString(); }
private:
    KLiveUiBuilder *builder;
    QWidget *currentWidget;
    QObject *component;
    KActionCollection *actionCollection;
    bool inTextTag;
};

}

class KLiveUiEngine
{
public:
    inline virtual ~KLiveUiEngine() {}

    virtual QWidget *beginWidget(QObject *component, KLiveUi::WidgetType type, const QString &title = QString(), const QString &name = QString()) = 0;
    virtual void endWidget(KLiveUi::WidgetType type) = 0;

    virtual void addAction(QAction *action) = 0;

    QAction *addMergePlaceholder(const QString &name, QObject *parent);
    virtual void beginMerge(const QString &name) = 0;
    virtual void endMerge() = 0;

    void replay(QObject *component, const KLiveUiStorage &storage);
};

class KLiveUiMainWindowEngine : public KLiveUiEngine
{
public:
    KLiveUiMainWindowEngine(KMainWindow *mw);

    virtual QWidget *beginWidget(QObject *component, KLiveUi::WidgetType type, const QString &title = QString(), const QString &name = QString());
    virtual void endWidget(KLiveUi::WidgetType);

    virtual void addAction(QAction *action);

    virtual void beginMerge(const QString &name);
    virtual void endMerge();

private:
    KMainWindow *mainWindow;
    QStack<QWidget *> widgets;
    QPointer<QAction> beforeAction;
};

class KLiveUiRecordingEngine : public KLiveUiEngine
{
public:
    KLiveUiRecordingEngine();

    virtual QWidget *beginWidget(QObject *component, KLiveUi::WidgetType type, const QString &title = QString(), const QString &name = QString());
    virtual void endWidget(KLiveUi::WidgetType);

    virtual void addAction(QAction *action);

    virtual void beginMerge(const QString &name);
    virtual void endMerge();

    KLiveUiStorage storage;
};

#endif
