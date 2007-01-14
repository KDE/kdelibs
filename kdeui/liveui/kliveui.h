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

#ifndef KLIVEUI_H
#define KLIVEUI_H

#include <QString>
#include <QObject>
#include <QKeySequence>
#include <QPointer>

#include <kdelibs_export.h>
#include <kinstance.h>
#include <kglobal.h>

#include "kstdaction.h"

class QAction;
class QString;
class QWidget;
class QMenu;
class QToolBar;
class QIcon;
class KAction;
class KActionCollection;
class KMainWindow;
class KMenu;
class KToolBar;

class KLiveUiBuilder;
class KLiveUiBuilderPrivate;
class KLiveUiComponentPrivate;
class KLiveUiStorage;

namespace KLiveUi
{
    enum StandardMenu {
        FileMenu,
        GameMenu,
        EditMenu,
        MoveMenu,
        ViewMenu,
        GoWebMenu,
        GoDocumentMenu,
        BookmarksMenu,
        ToolsMenu,
        SettingsMenu,
        HelpMenu
    };
}

class KDEUI_EXPORT KLiveUiComponent : public QObject
{
    Q_OBJECT
    friend class KLiveUiBuilder;

public:
    KLiveUiComponent(QObject* parent = 0);
    virtual ~KLiveUiComponent();

    /**
     * Retrieves the top action collection for the component.
     */
    KActionCollection *actionCollection() const;

    void setBuilderWidget(QWidget *w);
    QWidget *builderWidget() const;

    virtual void createComponentGui();
    virtual void removeComponentGui();

    virtual KInstance *instance() const;

    void addSubComponent(KLiveUiComponent *component);
    void removeSubComponent(KLiveUiComponent *component);
    void setSubComponents(const QList<KLiveUiComponent *> &components);
    QList<KLiveUiComponent *> subComponents() const;

private:
    KLiveUiStorage* storage() const;
    void setStorage(KLiveUiStorage* storage);
    Q_PRIVATE_SLOT(d, void _k_subComponentDestroyed(QObject *))

    KLiveUiComponentPrivate * const d;
};

class KDEUI_EXPORT KLiveUiBuilder
{
public:
    explicit KLiveUiBuilder(KMainWindow *mw);
    explicit KLiveUiBuilder(KLiveUiComponent *component);
    KLiveUiBuilder();
    ~KLiveUiBuilder();

    void begin(KMainWindow *mw);
    void begin(KLiveUiComponent *component);
    void end();

    void beginMenuBar();

    KMenu *beginMenu(const QString &title, const QString &name = QString());
    KMenu *beginMenu(KLiveUi::StandardMenu menu);
    void endMenu();

    KToolBar *beginToolBar(const QString &title = QString());
    void endToolBar();

    void addAction(QAction *action);

    // convenience
    KAction *addAction(const QString &text);
    KAction *addAction(const QIcon &icon, const QString &text);
    KAction *addAction(KStdAction::StdAction standardAction, const QObject *receiver = 0, const char *member = 0);
    void addMenu(QMenu *menu);

    QAction *addSeparator();
    QAction *addWidget(QWidget *widget);

    QAction *addMergePlaceholder(const QString &name);
    void beginMerge(const QString &placeholder);
    void endMerge();

    void populateFromXmlGui(const QString &fileName);

private:
    KLiveUiBuilderPrivate *d;
    Q_DISABLE_COPY(KLiveUiBuilder)
};

#endif // KLIVEUI_H

