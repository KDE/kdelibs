/***************************************************************************
 * interpreter.cpp
 * This file is part of the KDE project
 * copyright (C)2007 by Sebastian Sauer (mail@dipe.org)
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

#include "interpreter.h"

#include <QScriptEngine>
#include <QScriptValueIterator>

#include <kapplication.h>

// The in krossconfig.h defined KROSS_EXPORT_INTERPRETER macro defines an
// exported C function used as factory for Kross::EcmaInterpreter instances.
KROSS_EXPORT_INTERPRETER( Kross::EcmaInterpreter )

using namespace Kross;

/******************************************************************************************
 * EcmaScript
 */

namespace Kross {

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

                m_engine->importExtension("kross");
                if( m_engine->hasUncaughtException() ) {
                    handleException();
                    delete m_engine;
                    m_engine = 0;
                    return false;
                }

                QScriptValue global = m_engine->globalObject();
                m_kross = global.property("Kross");
                Q_ASSERT( m_kross.isValid() );
                Q_ASSERT( m_kross.isObject() );
                Q_ASSERT( m_kross.isQObject() );

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
                    for(; it != end; ++it)
                        global.setProperty(it.key(), m_engine->newQObject( it.value() ) );
                }

                return ! m_engine->hasUncaughtException();
            }

            void handleException() {
                const QString err = m_engine->uncaughtException().toString();
                const int linenr = m_engine->uncaughtExceptionLineNumber();
                const QString trace = m_engine->uncaughtExceptionBacktrace().join("\n");
                krossdebug( QString("%1, line:%2, backtrace:\n%3").arg(err).arg(linenr).arg(trace) );
                m_script->action()->setError(err, trace, linenr);
            }

            void addObject(QObject* object, const QString& name = QString()) {
                Q_ASSERT( m_engine );
                QScriptValue global = m_engine->globalObject();
                QScriptValue value = m_engine->newQObject(object);
                global.setProperty(name.isEmpty() ? object->objectName() : name, value);
            }

    };

}

EcmaScript::EcmaScript(Interpreter* interpreter, Action* action) : Script(interpreter, action), d(new Private(this))
{
    krossdebug( QString("EcmaScript::EcmaScript") );
}

EcmaScript::~EcmaScript()
{
    krossdebug( QString("EcmaScript::~EcmaScript") );
    delete d;
}

void EcmaScript::execute()
{
    if( ! d->init() ) {
        d->handleException();
        return;
    }

    QString scriptCode = action()->code();
    if( scriptCode.startsWith("#!") ) // remove optional shebang-line
        scriptCode.remove(0, scriptCode.indexOf('\n'));

    const QString fileName = action()->file().isEmpty() ? action()->name() : action()->file();

    //krossdebug( QString("EcmaScript::execute fileName=%1 scriptCode=\n%2").arg(fileName).arg(scriptCode) );

    Q_ASSERT( d->m_engine );
    d->m_engine->evaluate( scriptCode, fileName );
    if( d->m_engine->hasUncaughtException() ) {
        d->handleException();
        //return;
    }
}

QStringList EcmaScript::functionNames()
{
    if( ! d->m_engine && ! d->init() ) {
        d->handleException();
        return QStringList();
    }
    QStringList names;
    QScriptValueIterator it( d->m_self );
    while( it.hasNext() ) {
        it.next();
        if( it.value().isFunction() )
            names << it.name();
    }
    return names;
}

QVariant EcmaScript::callFunction(const QString& name, const QVariantList& args)
{
    if( ! d->m_engine && ! d->init() ) {
        d->handleException();
        return QVariant();
    }

    QScriptValue function = d->m_self.property(name);
    if( ! function.isFunction() ) {
        krosswarning( QString("EcmaScript::callFunction No such function '%1'").arg(name) );
        return QVariant();
    }

    QScriptValueList arguments;
    foreach(QVariant v, args)
        arguments << d->m_engine->newVariant(v);
    QScriptValue result = function.call(d->m_self, arguments);
    return result.toVariant();
}

/******************************************************************************************
 * EcmaInterpreter
 */

namespace Kross {

    class EcmaInterpreter::Private
    {
        public:
    };

}

EcmaInterpreter::EcmaInterpreter(Kross::InterpreterInfo* info) : Kross::Interpreter(info), d(new Private())
{
    krossdebug( QString("EcmaInterpreter::EcmaInterpreter") );
}

EcmaInterpreter::~EcmaInterpreter()
{
    krossdebug( QString("EcmaInterpreter::~EcmaInterpreter") );
    delete d;
}

Kross::Script* EcmaInterpreter::createScript(Kross::Action* action)
{
    return new EcmaScript(this, action);
}

