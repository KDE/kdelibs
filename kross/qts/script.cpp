/***************************************************************************
 * script.cpp
 * This file is part of the KDE project
 * copyright (C)2007-2008 by Sebastian Sauer (mail@dipe.org)
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 * You should have received a copy of the GNU Library General Public License
 * along with this program; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 ***************************************************************************/

#include "script.h"

#include <QMetaObject>
#include <QMetaMethod>
#include <QScriptEngine>
#include <QScriptValueIterator>

#include <kapplication.h>

using namespace Kross;

namespace Kross {

    /// \internal private d-pointer class.
    class EcmaScript::Private
    {
        public:
            EcmaScript* m_script;
            QScriptEngine* m_engine;
            QScriptValue m_kross;
            QScriptValue m_self;

            explicit Private(EcmaScript* script) : m_script(script), m_engine(0) {}
            ~Private() { delete m_engine; }

            bool init() {
                if( m_script->action()->hadError() )
                    m_script->action()->clearError();

                delete m_engine;
                m_engine = new QScriptEngine();

                // load the Kross QScriptExtensionPlugin plugin that provides
                // us a bridge between Kross and QtScript. See here plugin.h
                m_engine->importExtension("kross");
                if( m_engine->hasUncaughtException() ) {
                    handleException();
                    delete m_engine;
                    m_engine = 0;
                    return false;
                }

                // the Kross QScriptExtensionPlugin exports the "Kross" property.
                QScriptValue global = m_engine->globalObject();
                m_kross = global.property("Kross");
                Q_ASSERT( m_kross.isQObject() );
                Q_ASSERT( ! m_engine->hasUncaughtException() );

                // Attach our Kross::Action instance to be able to access it in
                // scripts. Just like at the Kjs-backend we publish our own
                // action as "self".
                m_self = m_engine->newQObject( m_script->action() );
                global.setProperty("self", m_self, QScriptValue::ReadOnly|QScriptValue::Undeletable);

                { // publish the global objects.
                    QHash< QString, QObject* > objects = Manager::self().objects();
                    QHash< QString, QObject* >::Iterator it(objects.begin()), end(objects.end());
                    for(; it != end; ++it)
                        global.setProperty(it.key(), m_engine->newQObject( it.value() ) );
                }

                { // publish the local objects.
                    QHash< QString, QObject* > objects = m_script->action()->objects();
                    QHash< QString, QObject* >::Iterator it(objects.begin()), end(objects.end());
                    for(; it != end; ++it) {
                        copyEnumsToProperties( it.value() );
                        global.setProperty(it.key(), m_engine->newQObject( it.value() ) );
                    }
                }

                return ! m_engine->hasUncaughtException();
            }

            void copyEnumsToProperties(QObject* object) {
                const QMetaObject* meta = object->metaObject();
                for (int i = 0; i < meta->enumeratorCount(); ++i) {
                    QMetaEnum metaenum = meta->enumerator(i);
                    for (int j = 0; j < metaenum.keyCount(); ++j) {
                        object->setProperty(metaenum.key(j), metaenum.value(j));
                    }
                }
            }

            void handleException() {
                Q_ASSERT( m_engine );
                Q_ASSERT( m_engine->hasUncaughtException() );
                const QString err = m_engine->uncaughtException().toString();
                const int linenr = m_engine->uncaughtExceptionLineNumber();
                const QString trace = m_engine->uncaughtExceptionBacktrace().join("\n");
                krossdebug( QString("%1, line:%2, backtrace:\n%3").arg(err).arg(linenr).arg(trace) );
                m_script->action()->setError(err, trace, linenr);
                m_engine->clearExceptions();
            }

            void addObject(QObject* object, const QString& name = QString()) {
                Q_ASSERT( m_engine );
                Q_ASSERT( ! m_engine->hasUncaughtException() );
                QScriptValue global = m_engine->globalObject();
                QScriptValue value = m_engine->newQObject(object);
                global.setProperty(name.isEmpty() ? object->objectName() : name, value);
            }

            void connectFunctions(ChildrenInterface* children) {
                Q_ASSERT( m_engine );
                Q_ASSERT( ! m_engine->hasUncaughtException() );
                QString eval;
                QScriptValue global = m_engine->globalObject();
                QHashIterator< QString, ChildrenInterface::Options > it( children->objectOptions() );
                while(it.hasNext()) {
                    it.next();
                    if( it.value() & ChildrenInterface::AutoConnectSignals ) {
                        QObject* sender = children->object(it.key());
                        if( ! sender )
                            continue;
                        QScriptValue obj = m_engine->globalObject().property(it.key());
                        if( ! obj.isQObject() )
                            continue;
                        const QMetaObject* mo = sender->metaObject();
                        const int count = mo->methodCount();
                        for(int i = 0; i < count; ++i) {
                            QMetaMethod mm = mo->method(i);
                            const QString signature = mm.signature();
                            const QString name = signature.left(signature.indexOf('('));
                            if( mm.methodType() == QMetaMethod::Signal ) {
                                QScriptValue func = global.property(name);
                                if( ! func.isFunction() ) {
                                    //krossdebug( QString("EcmaScript::connectFunctions No function to connect with %1.%2").arg(it.key()).arg(name) );
                                    continue;
                                }
                                krossdebug( QString("EcmaScript::connectFunctions Connecting with %1.%2").arg(it.key()).arg(name) );
                                eval += QString("try { %1.%2.connect(%3); } catch(e) { print(e); }\n").arg(it.key()).arg(name).arg(name);
                            }
                        }
                    }
                }
                Q_ASSERT( ! m_engine->hasUncaughtException() );
                if( ! eval.isNull() ) {
                    m_engine->evaluate(eval);
                    Q_ASSERT( ! m_engine->hasUncaughtException() );
                }
            }

    };

}

EcmaScript::EcmaScript(Interpreter* interpreter, Action* action) : Script(interpreter, action), d(new Private(this))
{
    //krossdebug( QString("EcmaScript::EcmaScript") );
}

EcmaScript::~EcmaScript()
{
    //krossdebug( QString("EcmaScript::~EcmaScript") );
    delete d;
}

void EcmaScript::execute()
{
    if( ! d->init() ) {
        d->handleException();
        return;
    }

    QString scriptCode = action()->code();
    if( scriptCode.startsWith(QLatin1String("#!")) ) // remove optional shebang-line
        scriptCode.remove(0, scriptCode.indexOf('\n'));

    const QString fileName = action()->file().isEmpty() ? action()->name() : action()->file();

    //krossdebug( QString("EcmaScript::execute fileName=%1 scriptCode=\n%2").arg(fileName).arg(scriptCode) );

    Q_ASSERT( d->m_engine );

    if( d->m_engine->hasUncaughtException() ) {
        d->m_engine->clearExceptions();
    }

    d->m_engine->evaluate( scriptCode, fileName );

    if( d->m_engine->hasUncaughtException() ) {
        d->handleException();
        return;
    }

    //d->connectFunctions( &Manager::self() );
    d->connectFunctions( action() );
}

QStringList EcmaScript::functionNames()
{
    if( ! d->m_engine && ! d->init() ) {
        d->handleException();
        return QStringList();
    }
    QStringList names;
    QScriptValueIterator it( d->m_engine->globalObject() );
    while( it.hasNext() ) {
        it.next();
        if( it.value().isFunction() ) {
            names << it.name();
        }
    }
    return names;
}

QVariant EcmaScript::callFunction(const QString& name, const QVariantList& args)
{
    if( ! d->m_engine && ! d->init() ) {
        d->handleException();
        return QVariant();
    }

    QScriptValue obj = d->m_engine->globalObject();
    QScriptValue function = obj.property(name);
    if( ! function.isFunction() ) {
        QString err = QString("No such function '%1'").arg(name);
        krosswarning( QString("EcmaScript::callFunction %1").arg(err) );
        setError(err);
        return QVariant();
    }

    QScriptValueList arguments;
    foreach(const QVariant &v, args)
        arguments << d->m_engine->toScriptValue(v);
    QScriptValue result = function.call(obj, arguments);
    if( d->m_engine->hasUncaughtException() ) {
        d->handleException();
        return QVariant();
    }
    return result.toVariant();
}

QVariant EcmaScript::evaluate(const QByteArray& code)
{
    if( ! d->m_engine && ! d->init() ) {
        d->handleException();
        return QVariant();
    }

    QScriptValue result = d->m_engine->evaluate(code);
    if( d->m_engine->hasUncaughtException() ) {
        d->handleException();
        return QVariant();
    }
    return result.toVariant();
}

QObject* EcmaScript::engine() const
{
    return d->m_engine;
}

#include "script.moc"
