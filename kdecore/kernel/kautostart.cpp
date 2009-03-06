/* This file is part of the KDE libraries
    Copyright (C) 2006 Aaron Seigo <aseigo@kde.org>

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


#include "kautostart.h"

#include "kaboutdata.h"
#include "kglobal.h"
#include "kcomponentdata.h"
#include "kdesktopfile.h"
#include "kstandarddirs.h"
#include "kconfiggroup.h"

#include <QtCore/QFile>
#include <QStringList>

class KAutostart::Private
{
    public:
        Private() : df(0)
        {
        }

        ~Private()
        {
            delete df;
        }

        QString name;
        KDesktopFile* df;
};

KAutostart::KAutostart(const QString& entryName,
           QObject* parent)
    : QObject(parent),
      d(new Private)
{
    KGlobal::dirs()->addResourceType("xdgconf-autostart", NULL, "autostart/"); // xdg ones
    KGlobal::dirs()->addResourceType("autostart", "xdgconf-autostart", "/"); // merge them
    KGlobal::dirs()->addResourceType("autostart", NULL, "share/autostart"); // KDE ones are higher priority
    if (entryName.isEmpty())
    {
        // XXX sure that the mainComponent is available at this point?
        d->name = KGlobal::mainComponent().aboutData()->appName();
    }
    else
    {
        d->name = entryName;
    }

    if (!d->name.endsWith(QLatin1String(".desktop")))
    {
        d->name.append(".desktop");
    }

    d->df = new KDesktopFile( "autostart", d->name);
}

KAutostart::~KAutostart()
{
    delete d;
}

void KAutostart::setAutostarts(bool autostart)
{
    d->df->desktopGroup().writeEntry("Hidden", !autostart);
}

bool KAutostart::autostarts(const QString& environment,
                            Conditions check) const
{
    // check if this is actually a .desktop file
    bool starts = d->df->desktopGroup().exists();

    // check the hidden field
    starts = starts && !d->df->desktopGroup().readEntry("Hidden", false);

    if (!environment.isEmpty())
    {
        starts = starts && (allowedEnvironments().indexOf(environment) != -1);
    }

    if (check == CheckCommand)
    {
        starts = starts && d->df->tryExec();
    }

    return starts;
}

QString KAutostart::command() const
{
    return d->df->desktopGroup().readEntry( "Exec", QString() );
}

void KAutostart::setCommand(const QString& command)
{
    d->df->desktopGroup().writeEntry( "Exec", command );
}

QString KAutostart::visibleName() const
{
    return d->df->readName();
}

void KAutostart::setVisibleName(const QString& name)
{
    d->df->desktopGroup().writeEntry( "Name", name );
}

bool KAutostart::isServiceRegistered(const QString& entryName)
{
    return QFile::exists(KStandardDirs::locate("autostart", entryName + ".desktop"));
}

QString KAutostart::commandToCheck() const
{
    return d->df->desktopGroup().readPathEntry( "TryExec", QString() );
}

void KAutostart::setCommandToCheck(const QString& exec)
{
    d->df->desktopGroup().writePathEntry( "TryExec", exec );
}

// do not specialize the readEntry template -
// http://connect.microsoft.com/VisualStudio/feedback/ViewFeedback.aspx?FeedbackID=100911
KAutostart::StartPhase readEntry(const KConfigGroup &group, const char* key, const KAutostart::StartPhase& aDefault)
{
    const QByteArray data = group.readEntry(key, QByteArray());

    if (data.isNull())
        return aDefault;

    if (data == "0" || data == "BaseDesktop")
        return KAutostart::BaseDesktop;
    else if (data == "1" || data == "DesktopServices")
        return KAutostart::DesktopServices;
    else if (data == "2" || data == "Applications")
        return KAutostart::Applications;

    return aDefault;
}

KAutostart::StartPhase KAutostart::startPhase() const
{
    return readEntry(d->df->desktopGroup(), "X-KDE-autostart-phase", Applications);
}

void KAutostart::setStartPhase(KAutostart::StartPhase phase)
{
    QByteArray data = "Applications";

    switch (phase) {
        case BaseDesktop:
            data = "BaseDesktop";
            break;
        case DesktopServices:
            data = "DesktopServices";
            break;
        case Applications: // This is the default
            break;
    }
    d->df->desktopGroup().writeEntry( "X-KDE-autostart-phase", data );
}

QStringList KAutostart::allowedEnvironments() const
{
    return d->df->desktopGroup().readXdgListEntry( "OnlyShowIn" );
}

void KAutostart::setAllowedEnvironments(const QStringList& environments)
{
    d->df->desktopGroup().writeXdgListEntry( "OnlyShowIn", environments );
}

void KAutostart::addToAllowedEnvironments(const QString& environment)
{
    QStringList envs = allowedEnvironments();

    if (envs.contains(environment))
    {
        return;
    }

    envs.append(environment);
    setAllowedEnvironments(envs);
}

void KAutostart::removeFromAllowedEnvironments(const QString& environment)
{
    QStringList envs = allowedEnvironments();
    int index = envs.indexOf(environment);

    if (index < 0)
    {
        return;
    }

    envs.removeAt(index);
    setAllowedEnvironments(envs);
}

QStringList KAutostart::excludedEnvironments() const
{
    return d->df->desktopGroup().readXdgListEntry("NotShowIn");
}

void KAutostart::setExcludedEnvironments(const QStringList& environments)
{
    d->df->desktopGroup().writeXdgListEntry("NotShowIn", environments);
}

void KAutostart::addToExcludedEnvironments(const QString& environment)
{
    QStringList envs = excludedEnvironments();

    if (envs.contains(environment))
    {
        return;
    }

    envs.append(environment);
    setExcludedEnvironments(envs);
}

void KAutostart::removeFromExcludedEnvironments(const QString& environment)
{
    QStringList envs = excludedEnvironments();
    int index = envs.indexOf(environment);

    if (index < 0)
    {
        return;
    }

    envs.removeAt(index);
    setExcludedEnvironments(envs);
}

#include "kautostart.moc"
