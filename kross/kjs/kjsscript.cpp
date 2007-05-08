/***************************************************************************
 * kjsscript.cpp
 * This file is part of the KDE project
 * copyright (C)2004-2006 by Sebastian Sauer (mail@dipe.org)
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

#include "kjsscript.h"
#include "../core/action.h"
#include "../core/manager.h"
#include "../core/interpreter.h"

// for Kjs
#include <kjs/interpreter.h>
#include <kjs/ustring.h>
#include <kjs/object.h>
//#include <kjs/PropertyNameArray.h>

// for KjsEmbed
#include <kjsembed/kjsembed.h>
#include <kjsembed/qobject_binding.h>
#include <kjsembed/slotproxy.h>

#include <QMetaObject>
#include <QMetaMethod>

using namespace Kross;

namespace Kross {

    /// Extract an errormessage from a KJS::Completion object.
    static ErrorInterface extractError(const KJS::Completion& completion, KJS::ExecState* exec)
    {
        QString type;
        switch( completion.complType() ) {
            case KJS::Normal: type = "Normal"; break;
            case KJS::Break: type = "Break"; break;
            case KJS::Continue: type = "Continue"; break;
            case KJS::ReturnValue: type = "ReturnValue"; break;
            case KJS::Throw: {
                type = "Throw";
            } break;
            case KJS::Interrupted: type = "Interrupted"; break;
            default: type = "Unknown"; break;
        }

        KJS::JSValue* value = completion.value();
        Q_ASSERT(value);
        int lineno = -1;
        if( value->type() == KJS::ObjectType ) {
            KJS::JSValue* linevalue = value->getObject()->get(exec, "line");
            if( linevalue && linevalue->type() == KJS::NumberType )
                lineno = linevalue->toInt32(exec);
        }
        const QString message = QString("%1%2: %3").arg( type ).arg((lineno >= 0) ? QString(" line %1").arg(lineno) : "").arg(value->toString(exec).qstring());

        ErrorInterface err;
        err.setError(message, QString(), lineno);
        return err;
    }

    /// \internal d-pointer class.
    class KjsScriptPrivate
    {
        public:
            /// One engine per script to have them clean separated.
            KJSEmbed::Engine* m_engine;

            /**
            * List of QObject instances that should be
            * auto connected on execution.
            * \see ChildrenInterface::AutoConnectSignals
            */
            QHash< QObject* , KJS::JSObject* > m_autoconnect;

            /**
            * This method does walk through the list of children the \a ChildrenInterface
            * has - \a Action and \a Manager are implementing this interface - and
            * fills the m_functions hash which got used to connect signals with
            * Kjs scripting functions.
            */
            void addFunctions(ChildrenInterface* children)
            {
                QHashIterator< QString, ChildrenInterface::Options > it( children->objectOptions() );
                while(it.hasNext()) {
                    it.next();
                    if( it.value() & ChildrenInterface::AutoConnectSignals ) {
                        QObject* sender = children->object( it.key() );
                        if( sender ) {
                            krossdebug( QString("KjsScript::addFunctions sender name=%1 className=%2").arg(sender->objectName()).arg(sender->metaObject()->className()) );
                            m_autoconnect.insert( sender, 0 );
                        }
                    }
                }
            }

            /// Publish a QObject to a KJSEmbed::Engine.
            void publishObject(KJS::ExecState* exec, const QString &name, QObject* object)
            {
                Q_UNUSED(exec);

                KJS::JSObject* obj = m_engine->addObject(object, name.isEmpty() ? object->objectName() : name);
                if( ! obj ) {
                    krosswarning( QString("Failed to publish the QObject name=\"%1\" objectName=\"%2\"").arg(name).arg(object ? object->objectName() : "NULL") );
                    return;
                }

                if( m_autoconnect.contains( object ) ) {
                    m_autoconnect[object] = obj;
                }

                /*
                bool restricted = interpreter()->interpreterInfo()->optionValue("restricted", true).toBool();
                if( restricted ) {
                    KJSEmbed::QObjectBinding* objImp = KJSEmbed::extractBindingImp<KJSEmbed::QObjectBinding>(exec, obj);
                    objImp->setAccess(
                        KJSEmbed::QObjectBinding::ScriptableSlots |
                        KJSEmbed::QObjectBinding::NonScriptableSlots |
                        KJSEmbed::QObjectBinding::PublicSlots |
                        KJSEmbed::QObjectBinding::ScriptableSignals |
                        KJSEmbed::QObjectBinding::NonScriptableSignals |
                        KJSEmbed::QObjectBinding::PublicSignals |
                        KJSEmbed::QObjectBinding::ScriptableProperties |
                        KJSEmbed::QObjectBinding::NonScriptableProperties |
                        KJSEmbed::QObjectBinding::GetParentObject |
                        KJSEmbed::QObjectBinding::ChildObjects
                    );
                }
                */
            }

    };

}

KjsScript::KjsScript(Interpreter* interpreter, Action* action)
    : Script(interpreter, action)
    , d(new KjsScriptPrivate())
{
    krossdebug( QString("KjsScript::KjsScript") );
    d->m_engine = 0;

    d->addFunctions( &Manager::self() );
    d->addFunctions( action );
}

KjsScript::~KjsScript()
{
    krossdebug( QString("KjsScript::~KjsScript") );
    finalize();
    delete d;
}

bool KjsScript::initialize()
{
    finalize(); // finalize before initialize
    clearError(); // clear previous errors.

    krossdebug( QString("KjsScript::initialize") );

    d->m_engine = new KJSEmbed::Engine();

    KJS::Interpreter* kjsinterpreter = d->m_engine->interpreter();
    KJS::ExecState* exec = kjsinterpreter->globalExec();

    { // publish the global objects.
        QHash< QString, QObject* > objects = Manager::self().objects();
        QHash< QString, QObject* >::Iterator it(objects.begin()), end(objects.end());
        for(; it != end; ++it)
            d->publishObject(exec, it.key(), it.value());
    }

    { // publish the local objects.
        QHash< QString, QObject* > objects = action()->objects();
        QHash< QString, QObject* >::Iterator it(objects.begin()), end(objects.end());
        for(; it != end; ++it)
            d->publishObject(exec, it.key(), it.value());
    }

    /*
    { // some debugging
        krossdebug( QString("Global object") );
        KJS::JSObject* obj = kjsinterpreter->globalObject();
        KJS::ExecState* exec = kjsinterpreter->globalExec();
        KJS::PropertyNameArray props;
        obj->getPropertyNames(exec, props);
        for(KJS::PropertyNameArrayIterator it = props.begin(); it != props.end(); it++)
            krossdebug( QString("  property name=%1").arg( it->ascii() ) );
    }
    */

    return true;
}

void KjsScript::finalize()
{
    d->m_autoconnect.clear();
    delete d->m_engine;
    d->m_engine = 0;
}

void KjsScript::execute()
{
    if(! initialize()) {
        krosswarning( QString("KjsScript::execute aborted cause initialize failed.") );
        return;
    }

    QString code = action()->code();
    if(code.startsWith("#!")) // remove optional shebang-line
        code.remove(0, code.indexOf('\n'));

    //krossdebug( QString("KjsScript::execute code=\n%1").arg(code.qstring()) );
    KJSEmbed::Engine::ExitStatus exitstatus = d->m_engine->execute( KJS::UString(code) );

    KJS::Completion completion = d->m_engine->completion();
    KJS::Interpreter* kjsinterpreter = d->m_engine->interpreter();
    KJS::ExecState* exec = kjsinterpreter->globalExec();

    if(exitstatus != KJSEmbed::Engine::Success) {
        ErrorInterface error = extractError(completion, exec);
        setError(&error);
        return;
    }


    //JSObject* globalobject = kjsinterpreter->globalObject();

krossdebug( QString("#################################### 1") );
    QHash< QObject* , KJS::JSObject* >::Iterator it( d->m_autoconnect.begin() ), end( d->m_autoconnect.end() );
    for( ; it != end; ++it) {
krossdebug( QString("#################################### 2") );
        KJS::JSObject* jsobj = it.value();
        if( ! jsobj ) {
            krosswarning( QString("KjsScript::execute jsobj is NULL. Skipping.") );
            continue;
        }
        /*
        KJSEmbed::QObjectBinding *jsobjImp = KJSEmbed::extractBindingImp<KJSEmbed::QObjectBinding>(exec, jsobj);
        if( ! jsobjImp ) {
            //KJS::throwError(exec, KJS::GeneralError, i18n("Invalid QObject."));
            krosswarning( QString("KjsScript::execute jsobjImp is NULL. Skipping.") );
            continue;
        }
        */

krossdebug( QString("#################################### 3") );
        const QMetaObject* metaobject = it.key()->metaObject();
        const int count = metaobject->methodCount();
        for(int i = 0; i < count; ++i) {
krossdebug( QString("#################################### 4") );
            QMetaMethod metamethod = metaobject->method(i);
            if( metamethod.methodType() == QMetaMethod::Signal ) {
krossdebug( QString("#################################### 5") );
                const QString signature = metamethod.signature();
                const QByteArray name = signature.left(signature.indexOf('(')).toLatin1();
                KJS::Identifier id = KJS::Identifier( KJS::UString(name.data()) );
                KJS::JSObject *function = jsobj->get(exec, id)->toObject(exec);
                if ( function && function->implementsCall() ) {
krossdebug( QString("#################################### 6") );
                    /*
                    QByteArray sendersignal = QString("2%1").arg(signature).toLatin1();
                    QByteArray receiverslot = QString("1%1").arg(signature).toLatin1();
                    QObject* receiver = new KJSEmbed::SlotProxy(jsobj, exec->dynamicInterpreter(), it.key(), signature.toLatin1());
                    if( ! connect(it.key(), sendersignal, receiver, receiverslot) ) {
                        krosswarning( QString("KjsScript::execute failed to connect object=%1 signal=%2").arg(it.key()->objectName()).arg(signature) );
                    }
                    */
                }

//m_functions.insert( name, QPair< QObject* , QString >(sender, signature) );
            }
        }

    }
krossdebug( QString("#################################### 9") );
}

QStringList KjsScript::functionNames()
{
    //TODO
    return QStringList();
}

QVariant KjsScript::callFunction(const QString& name, const QVariantList& args)
{
    //TODO
    Q_UNUSED(name);
    Q_UNUSED(args);
    return QVariant();
}

