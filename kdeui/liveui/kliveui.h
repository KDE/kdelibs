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
#include <QMainWindow>

#include <kdelibs_export.h>

class KLiveUiBuilderPrivate;
class QAction;
class QString;
class QWidget;
class QMenu;
class QToolBar;
class QIcon;
class KLiveUiBuilder;

class KDEUI_EXPORT KLiveUiComponent
{
    friend class KLiveUiBuilder;
public:
    inline virtual ~KLiveUiComponent() {}

    QObject *qObject() { return static_cast<QObject *>(qt_metacast("QObject")); }

    void activateComponentGui(QMainWindow *mw);
    void deactivateComponentGui(QMainWindow *mw);

protected:
    virtual void buildGui() = 0;
private:
    QPointer<QMainWindow> currentBuilderMainWindow;
    virtual void *qt_metacast(const char *) = 0;
};

Q_DECLARE_INTERFACE(KLiveUiComponent, "org.kde.framework.KLiveUiComponent/1.0")

class KDEUI_EXPORT KLiveUiBuilder
{
public:
    explicit KLiveUiBuilder(QMainWindow *mw);
    explicit KLiveUiBuilder(QObject *plugin);
    KLiveUiBuilder();
    ~KLiveUiBuilder();

    void begin(QMainWindow *mw);
    void begin(QObject *plugin);
    void end();

    void beginMenuBar();

    QMenu *beginMenu(const QString &name, const QString &title);
    void endMenu();

    QToolBar *beginToolBar(const QString &title = QString());
    void endToolBar();

    void addAction(QAction *action);
    void addActions(const QList<QAction *> actions);

    // convenience
    QAction *addAction(const QString &text);
    QAction *addAction(const QIcon &icon, const QString &text);
    QAction *addAction(const QString &text, const QObject *receiver, const char *member,
                       const QKeySequence &shortcut = QKeySequence());
    QAction *addAction(const QIcon &icon, const QString &text,
                       const QObject *receiver, const char *member,
                       const QKeySequence &shortcut = QKeySequence());

    QAction *addSeparator();
    QAction *addWidget(QWidget *widget);

    void addActionGroup(const QString &name);

    void beginActionGroup(const QString &name);
    void endActionGroup();

    void populateFromXmlGui(const QString &fileName);

private:
    KLiveUiBuilderPrivate *d;
    Q_DISABLE_COPY(KLiveUiBuilder)
};

#endif // KLIVEUI_H

