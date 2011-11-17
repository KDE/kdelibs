/*
 *   Copyright 2007 by Aaron Seigo <aseigo@kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "scripting/appletscript.h"

#include "kconfig.h"
#include "kconfigdialog.h"

#include "applet.h"
#include "package.h"
#include "private/applet_p.h"

namespace Plasma
{

class AppletScriptPrivate
{
public:
    Applet *applet;
};

AppletScript::AppletScript(QObject *parent)
    : ScriptEngine(parent),
      d(new AppletScriptPrivate)
{
    d->applet = 0;
}

AppletScript::~AppletScript()
{
    delete d;
}

void AppletScript::setApplet(Plasma::Applet *applet)
{
    d->applet = applet;
}

Applet *AppletScript::applet() const
{
    Q_ASSERT(d->applet);
    return d->applet;
}

void AppletScript::paintInterface(QPainter *painter,
                                  const QStyleOptionGraphicsItem *option,
                                  const QRect &contentsRect)
{
    Q_UNUSED(painter);
    Q_UNUSED(option);
    Q_UNUSED(contentsRect);
}

QSizeF AppletScript::size() const
{
    if (applet()) {
        return applet()->size();
    }

    return QSizeF();
}

void AppletScript::constraintsEvent(Plasma::Constraints constraints)
{
    Q_UNUSED(constraints);
}

QList<QAction*> AppletScript::contextualActions()
{
    return QList<QAction*>();
}

QPainterPath AppletScript::shape() const
{
    if (applet()) {
        QPainterPath path;
        path.addRect(applet()->boundingRect());
        return path;
    }

    return QPainterPath();
}

void AppletScript::setHasConfigurationInterface(bool hasInterface)
{
    if (applet()) {
        applet()->setHasConfigurationInterface(hasInterface);
    }
}

void AppletScript::setConfigurationRequired(bool req, const QString &reason)
{
    if (applet()) {
        applet()->setConfigurationRequired(req, reason);
    }
}

void AppletScript::setFailedToLaunch(bool failed, const QString &reason)
{
    if (applet()) {
        applet()->setFailedToLaunch(failed, reason);
    }
}

void AppletScript::configNeedsSaving() const
{
    if (applet()) {
        emit applet()->configNeedsSaving();
    }
}

void AppletScript::showConfigurationInterface()
{
    if (applet()) {
        KConfigDialog *dialog = applet()->d->generateGenericConfigDialog();
        applet()->d->addStandardConfigurationPages(dialog);
        dialog->show();
    }
}

KConfigDialog *AppletScript::standardConfigurationDialog()
{
    if (applet()) {
        return applet()->d->generateGenericConfigDialog();
    }

    return 0;
}

void AppletScript::addStandardConfigurationPages(KConfigDialog *dialog)
{
    if (applet()) {
        applet()->d->addStandardConfigurationPages(dialog);
    }
}

void AppletScript::showMessage(const QIcon &icon, const QString &message, const MessageButtons buttons)
{
    if (applet()) {
        applet()->showMessage(icon, message, buttons);
    }
}

void AppletScript::registerAsDragHandle(QGraphicsItem *item)
{
    if (applet()) {
        applet()->registerAsDragHandle(item);
    }
}

void AppletScript::unregisterAsDragHandle(QGraphicsItem *item)
{
    if (applet()) {
        applet()->unregisterAsDragHandle(item);
    }
}

bool AppletScript::isRegisteredAsDragHandle(QGraphicsItem *item)
{
    if (applet()) {
        return applet()->isRegisteredAsDragHandle(item);
    }
    return false;
}

void AppletScript::configChanged()
{
}

DataEngine *AppletScript::dataEngine(const QString &engine) const
{
    Q_ASSERT(d->applet);
    return d->applet->dataEngine(engine);
}

QString AppletScript::mainScript() const
{
    Q_ASSERT(d->applet);
    return d->applet->package().filePath("mainscript");
}

Package AppletScript::package() const
{
    Q_ASSERT(d->applet);
    return d->applet->package();
}

KPluginInfo AppletScript::description() const
{
    Q_ASSERT(d->applet);
    return d->applet->d->appletDescription;
}

bool AppletScript::drawWallpaper() const
{
    Q_ASSERT(d->applet);
    Plasma::Containment *cont = qobject_cast<Plasma::Containment *>(d->applet);
    if (cont) {
        return cont->drawWallpaper();
    } else {
        return false;
    }
}

void AppletScript::setDrawWallpaper(bool drawWallpaper)
{
    Q_ASSERT(d->applet);
    Plasma::Containment *cont = qobject_cast<Plasma::Containment *>(d->applet);
    if (cont) {
        cont->setDrawWallpaper(drawWallpaper);
    }
}

Containment::Type AppletScript::containmentType() const
{
    Q_ASSERT(d->applet);
    Plasma::Containment *cont = qobject_cast<Plasma::Containment *>(d->applet);
    if (cont) {
        return cont->containmentType();
    } else {
        return Containment::NoContainmentType;
    }
}

void AppletScript::setContainmentType(Containment::Type type)
{
    Q_ASSERT(d->applet);
    Plasma::Containment *cont = qobject_cast<Plasma::Containment *>(d->applet);
    if (cont) {
        cont->setContainmentType(type);
    }
}

} // Plasma namespace

#include "appletscript.moc"
