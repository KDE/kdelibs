/* This file is part of the KDE project
   Copyright (C) 2010 Maksim Orlovich <maksim@kde.org>
   Copyright (C) 2002 Koos Vriezen <koos.vriezen@gmail.com>

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
#include "scriptableextension.h"
#include <kglobal.h>
#include <QDBusMetaType>

namespace KParts {

struct ScriptableExtensionPrivate {
    ScriptableExtension* hostContext;

    ScriptableExtensionPrivate(): hostContext(0)
    {}
};

ScriptableExtension::ScriptableExtension(QObject* parent):
    QObject(parent), d(new ScriptableExtensionPrivate)
{
    registerDBusTypes();
}

ScriptableExtension::~ScriptableExtension()
{
    delete d;
}

ScriptableExtension* ScriptableExtension::childObject(QObject* obj)
{
    return KGlobal::findDirectChild<KParts::ScriptableExtension*>(obj);
}

// ###  static ScriptableExtension* adapterFromLiveConnect(LiveConnectExtension* oldApi);

void ScriptableExtension::setHost(ScriptableExtension* host)
{
    d->hostContext = host;
}

ScriptableExtension* ScriptableExtension::host() const
{
    return d->hostContext;
}

QVariant ScriptableExtension::rootObject()
{
    return QVariant::fromValue(Null());
}

QVariant ScriptableExtension::enclosingObject(KParts::ReadOnlyPart* childPart)
{
    Q_UNUSED(childPart);
    return QVariant::fromValue(Null());
}

static QVariant unimplemented()
{
    ScriptableExtension::Exception except(QString::fromLatin1("[unimplemented]"));
    return QVariant::fromValue(except);
}

QVariant ScriptableExtension::callAsFunction(ScriptableExtension* callerPrincipal,
                                             quint64 objId, const ArgList& args)
{
    Q_UNUSED(callerPrincipal);
    Q_UNUSED(objId);
    Q_UNUSED(args);
    return unimplemented();
}

QVariant ScriptableExtension::callAsConstructor(ScriptableExtension* callerPrincipal,
                                                quint64 objId, const ArgList& args)
{
    Q_UNUSED(callerPrincipal);
    Q_UNUSED(objId);
    Q_UNUSED(args);
    return unimplemented();
}

bool ScriptableExtension::hasProperty(ScriptableExtension* callerPrincipal,
                                      quint64 objId, const QString& propName)
{
    Q_UNUSED(callerPrincipal);
    Q_UNUSED(objId);
    Q_UNUSED(propName);
    return false;
}

QVariant ScriptableExtension::get(ScriptableExtension* callerPrincipal,
                                  quint64 objId, const QString& propName)
{
    Q_UNUSED(callerPrincipal);
    Q_UNUSED(objId);
    Q_UNUSED(propName);
    return unimplemented();
}

bool ScriptableExtension::put(ScriptableExtension* callerPrincipal, quint64 objId,
                              const QString& propName, const QVariant& value)
{
    Q_UNUSED(callerPrincipal);
    Q_UNUSED(objId);
    Q_UNUSED(propName);
    Q_UNUSED(value);
    return false;
}

bool ScriptableExtension::removeProperty(ScriptableExtension* callerPrincipal,
                                         quint64 objId, const QString& propName)
{
    Q_UNUSED(callerPrincipal);
    Q_UNUSED(objId);
    Q_UNUSED(propName);
    return false;
}

bool ScriptableExtension::enumerateProperties(ScriptableExtension* callerPrincipal,
                                              quint64 objId, QStringList* result)
{
    Q_UNUSED(callerPrincipal);
    Q_UNUSED(objId);
    Q_UNUSED(result);
    return false;
}

bool ScriptableExtension::setException(ScriptableExtension* callerPrincipal,
                                       const QString& message)
{
    Q_UNUSED(callerPrincipal);
    Q_UNUSED(message);
    return false;
}

QVariant ScriptableExtension::evaluateScript(ScriptableExtension* callerPrincipal,
                                            quint64 contextObjectId,
                                            const QString& code,
                                            const QString& language)
{
    Q_UNUSED(callerPrincipal);
    Q_UNUSED(contextObjectId);
    Q_UNUSED(code);
    Q_UNUSED(language);
    return unimplemented();
}

void ScriptableExtension::acquire(quint64 objId)
{
    Q_UNUSED(objId);
}

void ScriptableExtension::release(quint64 objId)
{
    Q_UNUSED(objId);
}

} // namespace KParts


// DBus stuff
// ----------------------------------------------------------------------------
void KParts::ScriptableExtension::registerDBusTypes()
{
    qDBusRegisterMetaType<Null>();
    qDBusRegisterMetaType<Undefined>();
    qDBusRegisterMetaType<Exception>();
}

const QDBusArgument& KPARTS_EXPORT operator<<(QDBusArgument& arg,
                                              const KParts::ScriptableExtension::Null&)
{
    arg.beginStructure();
    arg.endStructure();
    return arg;
}

const QDBusArgument& KPARTS_EXPORT operator>>(const QDBusArgument& arg,
                                              KParts::ScriptableExtension::Null&)
{
    arg.beginStructure();
    arg.endStructure();
    return arg;
}

const QDBusArgument& KPARTS_EXPORT operator<<(QDBusArgument& arg,
                                              const KParts::ScriptableExtension::Undefined&)
{
    arg.beginStructure();
    arg.endStructure();
    return arg;
}

const QDBusArgument& KPARTS_EXPORT operator>>(const QDBusArgument& arg,
                                              KParts::ScriptableExtension::Undefined&)
{
    arg.beginStructure();
    arg.endStructure();
    return arg;
}

const QDBusArgument& KPARTS_EXPORT operator<<(QDBusArgument& arg,
                                              const KParts::ScriptableExtension::Exception& e)
{
    arg.beginStructure();
    arg << e.message;
    arg.endStructure();
    return arg;
}

const QDBusArgument& KPARTS_EXPORT operator>>(const QDBusArgument& arg,
                                              KParts::ScriptableExtension::Exception& e)
{
    arg.beginStructure();
    arg >> e.message;
    arg.endStructure();
    return arg;
}

#include "scriptableextension.moc"
// kate: indent-width 4; replace-tabs on; tab-width 4; space-indent on;
