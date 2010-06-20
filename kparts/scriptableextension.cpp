/* This file is part of the KDE project
   Copyright (C) 2010 Maksim Orlovich <maksim@kde.org>
   Copyright (C) 2002, 2004 Koos Vriezen <koos.vriezen@gmail.com>

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
#include "scriptableextension_p.h"
#include <kglobal.h>
#include <kdebug.h>

namespace KParts {

struct ScriptableExtensionPrivate {
    ScriptableExtension* hostContext;

    ScriptableExtensionPrivate(): hostContext(0)
    {}
};

ScriptableExtension::ScriptableExtension(QObject* parent):
    QObject(parent), d(new ScriptableExtensionPrivate)
{}

ScriptableExtension::~ScriptableExtension()
{
    delete d;
}

ScriptableExtension* ScriptableExtension::childObject(QObject* obj)
{
    return KGlobal::findDirectChild<KParts::ScriptableExtension*>(obj);
}

ScriptableExtension* ScriptableExtension::adapterFromLiveConnect(QObject* parentObj,
                                                                 LiveConnectExtension* oldApi)
{
    return new ScriptableLiveConnectExtension(parentObj, oldApi);
}

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

QVariant ScriptableExtension::enclosingObject()
{
    if (d->hostContext)
        return d->hostContext->encloserForKid(this);
    else
        return QVariant::fromValue(Null());
}

QVariant ScriptableExtension::encloserForKid(KParts::ScriptableExtension* kid)
{
    Q_UNUSED(kid);
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

QVariant ScriptableExtension::callFunctionReference(ScriptableExtension* callerPrincipal,
                                                    quint64 objId, const QString& f,
                                                    const ArgList& args)
{
    Q_UNUSED(callerPrincipal);
    Q_UNUSED(objId);
    Q_UNUSED(args);
    Q_UNUSED(f);
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
                                            ScriptLanguage language)
{
    Q_UNUSED(callerPrincipal);
    Q_UNUSED(contextObjectId);
    Q_UNUSED(code);
    Q_UNUSED(language);
    return unimplemented();
}

bool ScriptableExtension::isScriptLanguageSupported(ScriptLanguage lang) const
{
    Q_UNUSED(lang);
    return false;
}

void ScriptableExtension::acquire(quint64 objId)
{
    Q_UNUSED(objId);
}

QVariant ScriptableExtension::acquireValue(const QVariant& v)
{
    if (v.canConvert<Object>()) {
        Object o = v.value<Object>();
        o.owner->acquire(o.objId);
    } else if (v.canConvert<FunctionRef>()) {
        FunctionRef fr = v.value<FunctionRef>();
        fr.base.owner->acquire(fr.base.objId);
    }
    return v;
}

void ScriptableExtension::release(quint64 objId)
{
    Q_UNUSED(objId);
}

QVariant ScriptableExtension::releaseValue(const QVariant& v)
{
    if (v.canConvert<Object>()) {
        Object o = v.value<Object>();
        o.owner->release(o.objId);
    } else if (v.canConvert<FunctionRef>()) {
        FunctionRef fr = v.value<FunctionRef>();
        fr.base.owner->release(fr.base.objId);
    }
    return v;
}

// LiveConnectExtension -> ScriptableExtension adapter. We use
// lc object IDs as our own object IDs.
// ----------------------------------------------------------------------------
ScriptableLiveConnectExtension::ScriptableLiveConnectExtension(QObject* p, LiveConnectExtension* old):
        ScriptableExtension(p), wrapee(old)
{
    connect(wrapee,
            SIGNAL(partEvent(const unsigned long, const QString &, const KParts::LiveConnectExtension::ArgList &)),
            this,
            SLOT(liveConnectEvent(const unsigned long, const QString&, const KParts::LiveConnectExtension::ArgList &)));
}

QVariant ScriptableLiveConnectExtension::rootObject()
{
    // Plugin root is always LC object #0.
    return acquireValue(QVariant::fromValue(ScriptableExtension::Object(this, 0)));
}

bool ScriptableLiveConnectExtension::hasProperty(ScriptableExtension*, quint64 objId, const QString& propName)
{
    QVariant val = get(0, objId, propName);
    bool ok = !val.canConvert<ScriptableExtension::Exception>();
    releaseValue(val);
    return ok;
}

// Note that since we wrap around a plugin, and do not implement the browser,
// we do not perform XSS checks ourselves.
QVariant ScriptableLiveConnectExtension::callFunctionReference(ScriptableExtension*,
                        quint64 o, const QString& f, const ScriptableExtension::ArgList& a)
{
    QStringList qargs;
    // Convert args to strings for LC use.
    for (int i = 0; i < a.size(); ++i) {
        bool ok;
        qargs.append(toLC(a[i], &ok));
        if (!ok)
            return unimplemented();
    }

    LiveConnectExtension::Type retType;
    unsigned long              retObjId;
    QString                    retVal;
    if (wrapee->call((unsigned long)o, f, qargs, retType, retObjId, retVal)) {
        return acquireValue(fromLC(QString(), retType, retObjId, retVal));
    } else {
        return unimplemented();
    }
}

QVariant ScriptableLiveConnectExtension::get(ScriptableExtension*,
                                             quint64 objId, const QString& propName)
{
    LiveConnectExtension::Type retType;
    unsigned long              retObjId;
    QString                    retVal;
    if (wrapee->get((unsigned long)objId, propName, retType, retObjId, retVal)) {
        return acquireValue(fromLC(propName, retType, retObjId, retVal));
    } else {
        // exception signals failure. ### inellegant
        return unimplemented();
    }
}

bool ScriptableLiveConnectExtension::put(ScriptableExtension*, quint64 objId,
                                         const QString& propName, const QVariant& value)
{
    bool ok;
    QString val = toLC(value, &ok);
    if (!ok)
        return false;

    return wrapee->put((unsigned long)objId, propName, val);
}

QVariant ScriptableLiveConnectExtension::fromLC(const QString& name,
                                                LiveConnectExtension::Type type,
                                                unsigned long objId,
                                                const QString& value)
{
    switch (type) {
    case KParts::LiveConnectExtension::TypeBool: {
        bool ok;
        int i = value.toInt(&ok);
        if (ok)
            return QVariant(bool(i));
        return QVariant(value.toLower() == QLatin1String("true"));
    }
    case KParts::LiveConnectExtension::TypeObject:
    case KParts::LiveConnectExtension::TypeFunction: {
        if (!refCounts.contains(objId))
            refCounts[objId] = 0;

        Object o = ScriptableExtension::Object(this, objId);
        if (type == KParts::LiveConnectExtension::TypeObject)
            return QVariant::fromValue(o);
        else
            return QVariant::fromValue(FunctionRef(o, name));
    }

    case KParts::LiveConnectExtension::TypeNumber:
        return QVariant(value.toDouble());

    case KParts::LiveConnectExtension::TypeString:
        return QVariant(value);

    case KParts::LiveConnectExtension::TypeVoid:
    default:
        return QVariant::fromValue(ScriptableExtension::Undefined());
    }
}

QString ScriptableLiveConnectExtension::toLC(const QVariant& in, bool* ok)
{
    *ok = true; // most of the time.

    // Objects (or exceptions) can't be converted
    if (in.canConvert<ScriptableExtension::Object>() ||
        in.canConvert<ScriptableExtension::Exception>() ||
        in.canConvert<ScriptableExtension::FunctionRef>()) {

        *ok = false;
        return QString();
    }

    // Convert null and undefined to appropriate strings
    // ### this matches old KHTML behavior, but is this sensible?
    if (in.canConvert<ScriptableExtension::Null>())
        return QString::fromLatin1("null");

    if (in.canConvert<ScriptableExtension::Undefined>())
        return QString::fromLatin1("undefined");

    if (in.type() == QVariant::Bool)
        return in.toBool() ? QString::fromLatin1("true") : QString::fromLatin1("false");

    // Just stringify everything else, makes sense for nums as well.
    if (in.canConvert<QString>())
        return in.toString();

    // something really icky...
    *ok = false;
    return QString();
}

void ScriptableLiveConnectExtension::acquire(quint64 objId)
{
    ++refCounts[objId];
}

void ScriptableLiveConnectExtension::release(quint64 objId)
{
    int newRC = --refCounts[objId];
    if (!newRC) {
        if (objId != 0)
            wrapee->unregister((unsigned long)objId);
        refCounts.remove(objId);
    }
}

void ScriptableLiveConnectExtension::liveConnectEvent(const unsigned long, const QString& event,
                                                      const LiveConnectExtension::ArgList& args)
{
    // We want to evaluate in the enclosure's context.
    QVariant enclosure = enclosingObject();
    if (!enclosure.canConvert<Object>()) {
        releaseValue(enclosure);
        kDebug(1000) << "No enclosure, can't evaluate";
        return;
    }

    Object enclosureObj = enclosure.value<Object>();    

    if (!host()->isScriptLanguageSupported(ECMAScript)) {
        releaseValue(enclosure);    
        kDebug(1000) << "Host can't evaluate ECMAScript";
    }

    // Compute a string to evaluate. We ned to escape a lot of stuff
    // since we're composing a bunch of strings into one.
    QString script;
    script.sprintf("%s(", event.toLatin1().constData());

    LiveConnectExtension::ArgList::const_iterator i = args.begin();
    const LiveConnectExtension::ArgList::const_iterator argsBegin = i;
    const LiveConnectExtension::ArgList::const_iterator argsEnd = args.end();

    for ( ; i != argsEnd; ++i) {
        if (i != argsBegin)
            script += ",";
        if ((*i).first == KParts::LiveConnectExtension::TypeString) {
            script += "\"";
            script += QString((*i).second).replace('\\', "\\\\").replace('"', "\\\"");
            script += "\"";
        } else
            script += (*i).second;
    }
    script += ")";

    kDebug(1000) << script;

    // Ask host to evaluate.. (unfortunately, we can't do anything with the result,
    // but anything that uses this interface isn't expective one in the first place)
    QVariant result = host()->evaluateScript(this, enclosureObj.objId, script);

    releaseValue(result);
    releaseValue(enclosure);
}

// hash functions
// ----------------------------------------------------------------------------

unsigned int qHash(const KParts::ScriptableExtension::Object& o)
{
    return qHash(qMakePair(o.owner, o.objId));
}

unsigned int qHash(const KParts::ScriptableExtension::FunctionRef& f)
{
    return qHash(qMakePair(f.base, f.field));
}

} // namespace KParts



#include "scriptableextension.moc"
#include "scriptableextension_p.moc"
// kate: indent-width 4; replace-tabs on; tab-width 4; space-indent on;

