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

#include "kliveui.h"
#include "kliveui_p.h"

#include <QMainWindow>
#include <QMenuBar>
#include <QMenu>
#include <QToolBar>
#include <QAction>
#include <QStack>
#include <QPointer>
#include <QDebug>
#include <QFile>
#include <QtXml>
#include <QWidgetAction>

namespace KLiveUiPrivate {
    class MenuOrWidgetDeleter : public QObject
    {
        Q_OBJECT
    public:
        MenuOrWidgetDeleter(QWidget *widgetOrMenu, QObject *component)
            : QObject(component), widgetOrMenu(widgetOrMenu)
        {
        }
        virtual ~MenuOrWidgetDeleter()
        {
            delete widgetOrMenu;
        }

        QPointer<QWidget> widgetOrMenu;
    };
}

class KLiveUiBuilderPrivate
{
public:
    QObject *component;

    QStack<QWidget *> widgets;

    QAction *findActionGroup(QString groupName);

    QPointer<QAction> beforeAction;
};

QAction *KLiveUiBuilderPrivate::findActionGroup(QString groupName)
{
    groupName.prepend("ActionGroup:");
    foreach (QAction *action, widgets.top()->actions())
        if (action->isSeparator()
            && !action->isVisible()
            && action->objectName() == groupName)
            return action;
    return 0;
}

KLiveUiBuilder::KLiveUiBuilder(QMainWindow *mw)
{
    d = 0;
    begin(mw);
}

KLiveUiBuilder::KLiveUiBuilder(QObject *plugin)
{
    d = 0;
    begin(plugin);
}

KLiveUiBuilder::KLiveUiBuilder()
{
    d = 0;
}

KLiveUiBuilder::~KLiveUiBuilder()
{
    end();
}

void KLiveUiBuilder::begin(QMainWindow *mw)
{
    if (d) {
        qWarning("KLiveUiBuilder::begin(): editor is already active.");
        return;
    }
    d = new KLiveUiBuilderPrivate;
    d->component = mw;
    d->widgets.push(mw);
}

void KLiveUiBuilder::begin(QObject *plugin)
{
    if (d) {
        qWarning("KLiveUiBuilder::begin(): editor is already active.");
        return;
    }
    d = new KLiveUiBuilderPrivate;
    d->component = plugin;
    KLiveUiComponent *iface = qobject_cast<KLiveUiComponent *>(plugin);
    Q_ASSERT(iface);
    d->widgets.push(iface->currentBuilderMainWindow);
}

void KLiveUiBuilder::end()
{
    delete d;
    d = 0;
}

void KLiveUiBuilder::beginMenuBar()
{
    QMainWindow *mw = qobject_cast<QMainWindow *>(d->widgets.top());
    if (!mw) {
        qWarning("GuiEditor: beginMenuBar() called without available main window");
        return;
    }
    d->widgets.push(mw->menuBar());
}

QMenu *KLiveUiBuilder::beginMenu(const QString &name, const QString &title)
{
    QMenu *menu = 0;
    foreach (QAction *a, d->widgets.top()->actions())
        if (a->menu() && a->menu()->objectName() == name) {
            menu = a->menu();
            break;
        }
    if (!menu) {
        menu = new QMenu(d->widgets.top());
        menu->setObjectName(name);
        menu->setTitle(title);
        (void)new KLiveUiPrivate::MenuOrWidgetDeleter(menu, d->component);
        d->widgets.top()->insertAction(d->beforeAction, menu->menuAction());
    }
    d->widgets.push(menu);
    return menu;
}

void KLiveUiBuilder::endMenu()
{
    if (d->widgets.count() <= 1) {
        qWarning("GuiEditor: endMenu called without previous endMenu");
        return;
    }
    QWidget *w = d->widgets.pop();
    if (!qobject_cast<QMenu *>(w))
        qWarning("GuiEditor: endMenu called but current widget is not a menu");
}

QToolBar *KLiveUiBuilder::beginToolBar(const QString &title)
{
    if (qobject_cast<QMenuBar *>(d->widgets.top())
        || qobject_cast<QToolBar *>(d->widgets.top())) {
        d->widgets.pop();
    }
    QMainWindow *mw = qobject_cast<QMainWindow *>(d->widgets.top());
    if (!mw) {
        qWarning("GuiEditor: beginToolBar called but current widget is not the main window");
        return 0;
    }

    QToolBar *tb = mw->addToolBar(title);
    d->widgets.push(tb);
    return tb;
}

void KLiveUiBuilder::endToolBar()
{
    if (d->widgets.count() <= 1) {
        qWarning("GuiEditor: endMenu called without previous endMenu");
        return;
    }
    QWidget *w = d->widgets.pop();
    if (!qobject_cast<QToolBar *>(w))
        qWarning("GuiEditor: endToolBar called but current widget is not a toolbar");
}

void KLiveUiBuilder::addAction(QAction *action)
{
    if (action->parent() != d->component) {
        qWarning("GuiEditor: addAction called with action not belonging to editing component");
    }
    d->widgets.top()->insertAction(d->beforeAction, action);
}

void KLiveUiBuilder::addActions(const QList<QAction *> actions)
{
    foreach (QAction *action, actions)
        addAction(action);
}

QAction *KLiveUiBuilder::addAction(const QString &text)
{
    QAction *a = new QAction(d->component);
    a->setText(text);
    addAction(a);
    return a;
}

QAction *KLiveUiBuilder::addAction(const QIcon &icon, const QString &text)
{
    QAction *a = new QAction(d->component);
    a->setIcon(icon);
    a->setText(text);
    addAction(a);
    return a;
}

QAction *KLiveUiBuilder::addAction(const QString &text, const QObject *receiver, const char *member,
                              const QKeySequence &shortcut)
{
    QAction *a = new QAction(d->component);
    a->setText(text);
    a->setShortcut(shortcut);
    QObject::connect(a, SIGNAL(triggered()), receiver, member);
    addAction(a);
    return a;
}

QAction *KLiveUiBuilder::addAction(const QIcon &icon, const QString &text,
                              const QObject *receiver, const char *member,
                              const QKeySequence &shortcut)
{
    QAction *a = new QAction(d->component);
    a->setIcon(icon);
    a->setText(text);
    a->setShortcut(shortcut);
    QObject::connect(a, SIGNAL(triggered()), receiver, member);
    addAction(a);
    return a;
}

QAction *KLiveUiBuilder::addSeparator()
{
    QAction *a = new QAction(d->component);
    a->setSeparator(true);
    addAction(a);
    return a;
}

QAction *KLiveUiBuilder::addWidget(QWidget *widget)
{
    QWidgetAction *a = new QWidgetAction(d->component);
    a->setDefaultWidget(widget);
    (void)new KLiveUiPrivate::MenuOrWidgetDeleter(widget, d->component);
    addAction(a);
    return a;
}

void KLiveUiBuilder::addActionGroup(const QString &name)
{
    QAction *a = new QAction(d->component);
    a->setSeparator(true);
    a->setVisible(false);
    a->setObjectName(QString::fromLatin1("ActionGroup:") + name);
    addAction(a);
}

void KLiveUiBuilder::beginActionGroup(const QString &name)
{
    d->beforeAction = d->findActionGroup(name);
}

void KLiveUiBuilder::endActionGroup()
{
    d->beforeAction = 0;
}

void KLiveUiComponent::activateComponentGui(QMainWindow *mw)
{
    currentBuilderMainWindow = mw;
    buildGui();
    currentBuilderMainWindow = 0;
}

static bool isAncestor(QObject *toplevel, QObject *child)
{
    while (child) {
        if (child == toplevel)
            return true;
        child = child->parent();
    }
    return false;
}

void KLiveUiComponent::deactivateComponentGui(QMainWindow *mw)
{
    foreach (QAction *a, qObject()->findChildren<QAction *>()) {
        foreach (QWidget *widget, a->associatedWidgets()) {
            if (isAncestor(mw, widget))
                widget->removeAction(a);
        }
    }
    foreach (KLiveUiPrivate::MenuOrWidgetDeleter *deleter,
             qObject()->findChildren<KLiveUiPrivate::MenuOrWidgetDeleter *>())
        delete deleter;
}

XmlGuiHandler::XmlGuiHandler(KLiveUiBuilder *builder, QObject *component)
    : builder(builder), component(component)
{
    currentWidget = 0;
    inTextTag = false;
}

bool XmlGuiHandler::startElement(const QString & /*namespaceURI*/, const QString & /*localName*/, const QString &qName, const QXmlAttributes &attributes)
{
    QString tag = qName.toLower();
    if (tag == QLatin1String("menubar")) {
        builder->beginMenuBar();
    } else if (tag == QLatin1String("menu")) {
        currentWidget = builder->beginMenu(attributes.value("name"), /*title=*/QString());
    } else if (tag == QLatin1String("toolbar")) {
        currentWidget = builder->beginToolBar();
    } else if (tag == QLatin1String("separator")) {
        builder->addSeparator();
    } else if (tag == QLatin1String("text")) {
        inTextTag = true;
    } else if (tag == QLatin1String("action")) {
        QString group = attributes.value("group");
        QAction *a = component->findChild<QAction *>(attributes.value("name"));
        if ( a ) {
          if (!group.isEmpty())
              builder->beginActionGroup(group);
              builder->addAction(a);
          if (!group.isEmpty())
              builder->endActionGroup();
        }
    }
    return true;
}

bool XmlGuiHandler::endElement(const QString & /*namespaceURI*/, const QString & /*localName*/, const QString &qName)
{
    QString tag = qName.toLower();
    if (tag == QLatin1String("menubar")) {
        currentWidget = 0;
    } else if (tag == QLatin1String("menu")) {
        builder->endMenu();
        currentWidget = 0;
    } else if (tag == QLatin1String("toolbar")) {
        builder->endToolBar();
        currentWidget = 0;
    } else if (tag == QLatin1String("text")) {
        inTextTag = false;
    }
    return true;
}

bool XmlGuiHandler::characters(const QString &text)
{
    if (currentWidget && inTextTag)
        currentWidget->setProperty("title", text);
    return true;
}

void KLiveUiBuilder::populateFromXmlGui(const QString &fileName)
{
    QFile f(fileName);
    f.open(QIODevice::ReadOnly);
    QXmlInputSource source(&f);
    QXmlSimpleReader reader;
    XmlGuiHandler handler(this, d->component);
    reader.setContentHandler(&handler);
    reader.parse(&source, /*incremental=*/false);
}

#include "kliveui.moc"
