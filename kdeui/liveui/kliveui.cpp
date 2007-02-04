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

#include <QMenuBar>
#include <QMenu>
#include <QToolBar>
#include <QAction>
#include <QPointer>
#include <QDebug>
#include <QFile>
#include <QtXml>
#include <QWidgetAction>

#include <kdebug.h>
#include <kcomponentdata.h>
#include <kstandarddirs.h>
#include <klocale.h>

#include "kaction.h"
#include "kactioncollection.h"
#include "kmainwindow.h"
#include "kmenu.h"
#include "kmenubar.h"
#include "ktoolbar.h"
#include <kicon.h>


/*
Issues....:
- XMLGUI used ui_standards.rc - need to reimplement the functionality from this
  - standard locations for actions in menus and toolbars
  - actions can be added in any order and they automatically got put in the right spots
  - this depended on actions having the correct objectName - this will have to stay I think

- Should we allow menus and toolbars to be populated at the same time? (do everything for
  one action in one block of code, don't have to assign it a method - persistant variable)

- Consider support for not-yet created action container widgets or custom containers (eg. begin/endGeneric)
*/

void KLiveUiComponentPrivate::addActiveAction(QAction *a)
{
    if (activeActions.contains(a))
        return;
    QObject::connect(a, SIGNAL(destroyed(QObject *)),
                     q, SLOT(_k_activeActionDestroyed(QObject *)));
    activeActions.insert(a);
}

KLiveUiComponent::KLiveUiComponent(QObject* parent)
  : QObject(parent)
  , d(new KLiveUiComponentPrivate)
{
    d->q = this;
}

KLiveUiComponent::~KLiveUiComponent()
{
    delete d;
}

void KLiveUiComponent::setBuilderWidget(QWidget *w)
{
    d->builderWidget = w;
}

QWidget *KLiveUiComponent::builderWidget() const
{
    if (d->builderWidget)
        return d->builderWidget;

    QObject *parent = this->parent();

    while (parent && !parent->isWidgetType())
        parent = parent->parent();

    if (parent && parent->isWidgetType())
      return static_cast<QWidget*>(parent);

    kWarning() << k_funcinfo << "Component is not a child object of a widget!" << endl;
    return 0;
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

void KLiveUiComponent::createGui()
{
    if (!qobject_cast<KMainWindow*>(builderWidget())) {
        kWarning() << k_funcinfo << "Attempted to create a gui on a non-mainwindow widget (not supported yet)." << endl;
        return;
    }

    if (!d->storage.isEmpty()) {
        KLiveUiMainWindowEngine e(static_cast<KMainWindow*>(builderWidget()));
        e.replay(this, d->storage);
    }

    createComponentGui();

    QWidget *bw = builderWidget();
    for (int i = 0; i < d->subComponents.count(); ++i) {
        KLiveUiComponent *c = d->subComponents.at(i);
        c->setBuilderWidget(bw);
        c->createGui();
    }
}

void KLiveUiComponent::removeGui()
{
    removeComponentGui();

    QWidget *bw = builderWidget();

    foreach (QAction *action, d->activeActions) {
        disconnect(action, SIGNAL(destroyed(QObject *)),
                   this, SLOT(_k_activeActionDestroyed(QObject *)));

        foreach (QWidget *widget, action->associatedWidgets()) {
            if (!bw || isAncestor(bw, widget))
                widget->removeAction(action);
        }
    }
    d->activeActions.clear();

    foreach (QObject *ch, children())
        delete qobject_cast<KLiveUiPrivate::MenuOrWidgetDeleter *>(ch);

    for (int i = 0; i < d->subComponents.count(); ++i) {
        KLiveUiComponent *c = d->subComponents.at(i);
        c->setBuilderWidget(bw);
        c->removeGui();
    }
}

void KLiveUiComponent::createComponentGui()
{
}

void KLiveUiComponent::removeComponentGui()
{
}

void KLiveUiComponent::setComponentData(const KComponentData &cData)
{
    d->componentData = cData;
}

KComponentData KLiveUiComponent::componentData() const
{
    return d->componentData.isValid() ? d->componentData : KGlobal::mainComponent();
}

void KLiveUiComponent::addSubComponent(KLiveUiComponent *component)
{
    if (d->subComponents.contains(component))
        return;

    d->subComponents.append(component);
    connect(component, SIGNAL(destroyed(QObject *)),
            this, SLOT(_k_subComponentDestroyed(QObject *)));
}

void KLiveUiComponent::removeSubComponent(KLiveUiComponent *component)
{
    disconnect(component, SIGNAL(destroyed(QObject *)),
               this, SLOT(_k_subComponentDestroyed(QObject *)));
    d->subComponents.removeAll(component);
}

void KLiveUiComponent::setSubComponents(const QList<KLiveUiComponent *> &components)
{
    if (d->subComponents == components)
        return;

    for (int i = 0; i < d->subComponents.count(); ++i)
        disconnect(d->subComponents.at(i), SIGNAL(destroyed(QObject *)),
                   this, SLOT(_k_subComponentDestroyed(QObject *)));

    d->subComponents = components;

    for (int i = 0; i < d->subComponents.count(); ++i)
        connect(d->subComponents.at(i), SIGNAL(destroyed(QObject *)),
                this, SLOT(_k_subComponentDestroyed(QObject *)));
}

QList<KLiveUiComponent *> KLiveUiComponent::subComponents() const
{
    return d->subComponents;
}

QSet<QAction *> KLiveUiComponent::activeActions()
{
    return d->activeActions;
}

QAction *KLiveUiEngine::addMergePlaceholder(const QString &name, QObject *parent)
{
    QAction *a = new QAction(parent);
    a->setSeparator(true);
    a->setVisible(false);
    a->setObjectName(QString::fromLatin1("ActionPlaceHolder:") + name);
    addAction(a);
    return a;
}

KLiveUiMainWindowEngine::KLiveUiMainWindowEngine(KMainWindow *mw)
    : mainWindow(mw)
{
    widgets.push(mw);
}

QWidget *KLiveUiMainWindowEngine::beginWidget(QObject *component, KLiveUi::WidgetType type, const QString &title, const QString &name)
{
    QWidget *top = widgets.top();
    QMenuBar *menuBar = qobject_cast<QMenuBar *>(top);
    KToolBar *toolBar = qobject_cast<KToolBar *>(top);

    switch (type) {
        case KLiveUi::MenuBar:
            if (!mainWindow) {
                qWarning("KLiveUiBuilder: beginMenuBar() called without available mainwindow");
                return 0;
            }
            widgets.push(mainWindow->menuBar());
            break;
        case KLiveUi::Menu: {
            if (top == mainWindow) {
               beginWidget(component, KLiveUi::MenuBar);
               top = widgets.top();
            }

            KMenu *menu = 0;
            foreach (QAction *a, top->actions())
                if (a->menu() && a->menu()->objectName() == name) {
                    menu = qobject_cast<KMenu *>(a->menu());
                    break;
                }
            if (!menu) {
                menu = new KMenu(top);
                menu->setObjectName(name);
                menu->setTitle(title);
                (void)new KLiveUiPrivate::MenuOrWidgetDeleter(menu, component);
                top->insertAction(beforeAction, menu->menuAction());
            }
            widgets.push(menu);
            break;
        }
        case KLiveUi::ToolBar:
            if (toolBar || menuBar)
                widgets.pop();
            if (!mainWindow)
                return 0;
            KToolBar *tb = new KToolBar(mainWindow);
            tb->setWindowTitle(title);
            mainWindow->addToolBar(tb);
            widgets.push(tb);
            break;
    }

    return widgets.top();
}

void KLiveUiMainWindowEngine::endWidget(KLiveUi::WidgetType type)
{
    if (widgets.count() <= 1)
        return;
    QWidget *w = widgets.pop();
    if (type == KLiveUi::MenuBar && !qobject_cast<QMenuBar *>(w)) {
    } else if (type == KLiveUi::Menu && !qobject_cast<QMenu *>(w)) {
        qWarning("KLiveUiBuilder: endMenu called but current widget is not a menu");
    } else if (type == KLiveUi::ToolBar && !qobject_cast<QToolBar *>(w)) {
        qWarning("KLiveUiBuilder: endToolBar called but current widget is not a toolbar");
    }
}

void KLiveUiMainWindowEngine::addAction(QAction *action)
{
    widgets.top()->insertAction(beforeAction, action);
}

void KLiveUiMainWindowEngine::beginMerge(const QString &name)
{
    endMerge();
    QString groupName = name;
    groupName.prepend("ActionPlaceHolder:");
    foreach (QAction *action, widgets.top()->actions())
        if (action->isSeparator()
            && !action->isVisible()
            && action->objectName() == groupName) {
            beforeAction = action;
            break;
        }
}

void KLiveUiMainWindowEngine::endMerge()
{
    beforeAction = 0;
}

KLiveUiRecordingEngine::KLiveUiRecordingEngine()
{
}

QWidget *KLiveUiRecordingEngine::beginWidget(QObject * /*component*/, KLiveUi::WidgetType type, const QString &title, const QString &name)
{
    KLiveUiCommand cmd;
    cmd.type = KLiveUi::BeginWidgetCommand;
    cmd.widgetType = type;
    cmd.title = title;
    cmd.name = name;
    storage << cmd;
    return 0;
}

void KLiveUiRecordingEngine::endWidget(KLiveUi::WidgetType type)
{
    KLiveUiCommand cmd;
    cmd.type = KLiveUi::EndWidgetCommand;
    cmd.widgetType = type;
    storage << cmd;
}

void KLiveUiRecordingEngine::addAction(QAction *action)
{
    KLiveUiCommand cmd;
    cmd.type = KLiveUi::AddActionCommand;
    cmd.action = action;
    storage << cmd;
}

void KLiveUiRecordingEngine::beginMerge(const QString &name)
{
    KLiveUiCommand cmd;
    cmd.type = KLiveUi::BeginMergeCommand;
    cmd.name = name;
    storage << cmd;
}

void KLiveUiRecordingEngine::endMerge()
{
    KLiveUiCommand cmd;
    cmd.type = KLiveUi::EndMergeCommand;
    storage << cmd;
}

void KLiveUiEngine::replay(QObject *component, const KLiveUiStorage &storage)
{
    for (int i = 0; i < storage.count(); ++i) {
        const KLiveUiCommand &cmd = storage.at(i);
        switch (cmd.type) {
            case KLiveUi::BeginWidgetCommand:
                beginWidget(component, cmd.widgetType, cmd.title, cmd.name);
                break;
            case KLiveUi::EndWidgetCommand:
                endWidget(cmd.widgetType);
                break;
            case KLiveUi::AddActionCommand:
                addAction(cmd.action);
                break;
            case KLiveUi::BeginMergeCommand:
                beginMerge(cmd.name);
                break;
            case KLiveUi::EndMergeCommand:
                endMerge();
                break;
            case KLiveUi::NoCommand:
                break;
        }
    }
}

class KLiveUiBuilderPrivate
{
public:
    KLiveUiBuilderPrivate()
    {
        object = 0;
        mainWindow = 0;
        component = 0;
        engine = 0;
    }

    QObject* object;
    KMainWindow* mainWindow;
    KLiveUiComponent* component;

    KLiveUiEngine *engine;
};

KLiveUiBuilder::KLiveUiBuilder(KMainWindow *mw)
{
    d = 0;
    begin(mw);
}

KLiveUiBuilder::KLiveUiBuilder(KLiveUiComponent *component)
{
    d = 0;
    begin(component);
}

KLiveUiBuilder::KLiveUiBuilder()
{
    d = 0;
}

KLiveUiBuilder::~KLiveUiBuilder()
{
    end();
}

void KLiveUiBuilder::begin(KMainWindow *mw)
{
    if (d) {
        qWarning("KLiveUiBuilder::begin(): editor is already active.");
        return;
    }
    d = new KLiveUiBuilderPrivate;
    d->engine = new KLiveUiMainWindowEngine(mw);
    d->object = d->mainWindow = mw;
}

void KLiveUiBuilder::begin(KLiveUiComponent *component)
{
    if (d) {
        qWarning("KLiveUiBuilder::begin(): editor is already active.");
        return;
    }
    d = new KLiveUiBuilderPrivate;
    d->object = d->component = component;
    d->engine = new KLiveUiRecordingEngine();
}

void KLiveUiBuilder::end()
{
    if (d) {
        if (d->component)
          d->component->d->storage = static_cast<KLiveUiRecordingEngine*>(d->engine)->storage;
        delete d->engine;
        delete d;
        d = 0;
    }
}

void KLiveUiBuilder::beginMenuBar()
{
    if (!d)
        return;
    d->engine->beginWidget(d->component, KLiveUi::MenuBar);
}

KMenu *KLiveUiBuilder::beginMenu(const QString &title, const QString &name)
{
    if (!d)
        return 0;
    return qobject_cast<KMenu *>(d->engine->beginWidget(d->component, KLiveUi::Menu, title, name));
}

KMenu* KLiveUiBuilder::beginMenu( KLiveUi::StandardMenu menu )
{
    if (!d)
        return 0;
    switch (menu) {
        case KLiveUi::FileMenu:
          return beginMenu(i18n("&File"), "file");
        case KLiveUi::GameMenu:
          return beginMenu(i18n("&Game"), "game");
        case KLiveUi::EditMenu:
          return beginMenu(i18n("&Edit"), "edit");
        case KLiveUi::MoveMenu:
          return beginMenu(i18n("&Move"), "move");
        case KLiveUi::ViewMenu:
          return beginMenu(i18n("&View"), "view");
        case KLiveUi::GoWebMenu:
          return beginMenu(i18n("&Go"), "go_web");
        case KLiveUi::GoDocumentMenu:
          return beginMenu(i18n("&Go"), "go_document");
        case KLiveUi::BookmarksMenu:
          return beginMenu(i18n("&Bookmarks"), "bookmarks");
        case KLiveUi::ToolsMenu:
          return beginMenu(i18n("&Tools"), "tools");
        case KLiveUi::SettingsMenu:
          return beginMenu(i18n("&Settings"), "settings");
        case KLiveUi::HelpMenu:
          return beginMenu(i18n("&Help"), "help");
        default:
          Q_ASSERT(false);
          return 0L;
    };
}

void KLiveUiBuilder::endMenu()
{
    if (!d)
        return;
    d->engine->endWidget(KLiveUi::Menu);
}

KToolBar *KLiveUiBuilder::beginToolBar(const QString &title)
{
    if (!d)
        return 0;
    return qobject_cast<KToolBar *>(d->engine->beginWidget(d->component, KLiveUi::ToolBar, title, /*name*/QString()));
}

void KLiveUiBuilder::endToolBar()
{
    if (!d)
        return;
    d->engine->endWidget(KLiveUi::ToolBar);
}

void KLiveUiBuilder::addAction(QAction *action)
{
    if (!d)
        return;
    d->engine->addAction(action);
    if (d->component)
        d->component->d->addActiveAction(action);
}

void KLiveUiBuilder::addActions(QList<QAction *> &actions)
{
    for (int i = 0; i < actions.count(); ++i)
        addAction(actions.at(i));
}

QAction *KLiveUiBuilder::addAction(const QString &text)
{
    if (!d)
        return 0;
    QAction *a = new QAction(d->component);
    a->setText(text);
    addAction(a);
    return a;
}

QAction *KLiveUiBuilder::addAction(const QIcon &icon, const QString &text)
{
    if (!d)
        return 0;
    QAction *a = new QAction(d->component);
    a->setIcon(KIcon(icon));
    a->setText(text);
    addAction(a);
    return a;
}

QAction *KLiveUiBuilder::addAction(KStandardAction::StandardAction standardAction,
                              const QObject *receiver, const char *member)
{
    if (!d)
        return 0;
    QAction* a = KStandardAction::create(standardAction, receiver, member, d->component);
    addAction(a);
    return a;
}

QAction *KLiveUiBuilder::addSeparator()
{
    if (!d)
        return 0;
    QAction *a = new KAction(d->component);
    a->setSeparator(true);
    addAction(a);
    return a;
}

QAction *KLiveUiBuilder::addWidget(QWidget *widget)
{
    if (!d)
        return 0;
    QWidgetAction *a = new QWidgetAction(d->component);
    a->setDefaultWidget(widget);
    (void)new KLiveUiPrivate::MenuOrWidgetDeleter(widget, d->object);
    addAction(a);
    return a;
}

void KLiveUiBuilder::addMenu(QMenu *menu)
{
    if (!d)
        return;
    addAction(menu->menuAction());
}

QAction *KLiveUiBuilder::addMergePlaceholder(const QString &name)
{
    if (!d)
        return 0;
    return d->engine->addMergePlaceholder(name, d->component);
}

void KLiveUiBuilder::beginMerge(const QString &name)
{
    if (!d)
        return;
    d->engine->beginMerge(name);
}

void KLiveUiBuilder::endMerge()
{
    if (!d)
        return;
    d->engine->endMerge();
}

using namespace KLiveUiPrivate;

XmlGuiHandler::XmlGuiHandler(KLiveUiBuilder *builder, QObject *component, KActionCollection *collection)
    : builder(builder), component(component), actionCollection(collection)
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
        currentWidget = builder->beginMenu(/*title=*/QString(), attributes.value("name"));
    } else if (tag == QLatin1String("toolbar")) {
        currentWidget = builder->beginToolBar();
    } else if (tag == QLatin1String("separator")) {
        builder->addSeparator();
    } else if (tag == QLatin1String("text")) {
        inTextTag = true;
    } else if (tag == QLatin1String("action")) {
        QString group = attributes.value("group");
        QAction *a = actionCollection->action(attributes.value("name"));
        if ( a ) {
          if (!group.isEmpty())
              builder->beginMerge(group);
              builder->addAction(a);
          if (!group.isEmpty())
              builder->endMerge();
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

void KLiveUiBuilder::populateFromXmlGui(const QString &fileName, KActionCollection *collection)
{
    QString file = fileName;

    if (QDir::isRelativePath(fileName) && !QFile::exists(file)) {
        KComponentData cData = KGlobal::mainComponent();
        if (d->component)
            cData = d->component->componentData();
        file = cData.dirs()->locate("data", fileName);
    }

    QFile f(file);
    if (!f.open(QIODevice::ReadOnly)) {
        kWarning() << k_funcinfo << "Could not find requested XMLGUI file " << file << endl;
        return;
    }

    QXmlInputSource source(&f);
    QXmlSimpleReader reader;
    XmlGuiHandler handler(this, d->object, collection);
    reader.setContentHandler(&handler);
    reader.parse(&source, /*incremental=*/false);
}

KLiveUiPrivate::MenuOrWidgetDeleter::MenuOrWidgetDeleter(QWidget *widgetOrMenu, QObject *component)
    : QObject(component), widgetOrMenu(widgetOrMenu)
{
}

KLiveUiPrivate::MenuOrWidgetDeleter::~MenuOrWidgetDeleter()
{
    delete widgetOrMenu;
}

#include "kliveui.moc"
#include "kliveui_p.moc"
