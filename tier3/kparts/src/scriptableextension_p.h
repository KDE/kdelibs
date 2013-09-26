/* This file is part of the KDE project
   Copyright (C) 2010 Maksim Orlovich <maksim@kde.org>

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

#ifndef kparts_scriptableextension_p_h
#define kparts_scriptableextension_p_h

#include "browserextension.h"

namespace KParts {

// LiveConnectExtension -> ScriptableExtension adapter. 
class ScriptableLiveConnectExtension: public ScriptableExtension
{
    Q_OBJECT
public:
    ScriptableLiveConnectExtension(QObject* parent, LiveConnectExtension* old);

    QVariant rootObject();
    // enclosingObject: not applicable, plugins wouldn't have children

    // callAsFunction: we only have function rereferences.
    QVariant callFunctionReference(ScriptableExtension* callerPrincipal, quint64 objId,
                                   const QString& f, const ArgList& args);

    // callAsConstructor: unsupported by LC

    bool hasProperty(ScriptableExtension* callerPrincipal, quint64 objId, const QString& propName);

    QVariant get(ScriptableExtension* callerPrincipal, quint64 objId, const QString& propName);

    bool put(ScriptableExtension* callerPrincipal, quint64 objId, const QString& propName, const QVariant& value);

    // removeProperty: unsupported by LC
    // enumerateProperties: unsupported by LC
    // setException: unsupported by LC
    // evaluateScript: unsupported by LC, though we have to
    //                 route LC evaluation requests to our parent
    //                 as appropriate

    void acquire(quint64 objid);
    void release(quint64 objid);
private:
    // LC uses 0-1 refcounting, we use arbitrary, so we need to call
    // unregister when done.
    QHash<quint64, int>     refCounts;
    LiveConnectExtension* wrapee;

    // also registers when needed
    QVariant fromLC(const QString& name, LiveConnectExtension::Type type,
                    unsigned long objId, const QString& value);

    QString toLC(const QVariant& in, bool* ok);
public Q_SLOTS:
    void liveConnectEvent(const unsigned long, const QString&, const KParts::LiveConnectExtension::ArgList&);
};


} // namespace KParts

#endif
// kate: indent-width 4; replace-tabs on; tab-width 4; space-indent on;
