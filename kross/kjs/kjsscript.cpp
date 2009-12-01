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
#include <kjs/PropertyNameArray.h>
//#include <kjs/array_instance.h>
#include <kjs/function_object.h>

// for KjsEmbed
#include <kjsembed/kjsembed.h>
#include <kjsembed/qobject_binding.h>
#include <kjsembed/variant_binding.h>
#include <kjsembed/slotproxy.h>

#include <QMetaObject>
#include <QMetaMethod>
#include <QPointer>
#include <QTextCodec>

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
        int lineno = -1;
        if( value && value->type() == KJS::ObjectType ) {
            KJS::JSValue* linevalue = value->getObject()->get(exec, "line");
            if( linevalue && linevalue->type() == KJS::NumberType )
                lineno = linevalue->toInt32(exec);
        }
        const QString message = QString("%1%2: %3").arg( type ).arg((lineno >= 0) ? QString(" line %1").arg(lineno) : "").arg(value ? value->toString(exec).qstring() : "NULL");

        ErrorInterface err;
        err.setError(message, QString(), lineno);
        return err;
    }

    /// \internal d-pointer class.
    class KjsScriptPrivate
    {
        public:
            /**
            * One engine per script to have them clean separated.
            */
            KJSEmbed::Engine* m_engine;

            /**
            * The KJS objects that got published.
            */
            QList< QPair<KJS::JSObject*, QPointer<QObject> > > m_publishedObjects;

            /**
            * List of QObject instances that should be
            * auto connected on execution.
            * \see ChildrenInterface::AutoConnectSignals
            */
            QList< QObject* > m_autoconnect;

            /**
            * The list of functionnames that are in the script per default.
            */
            QStringList m_defaultFunctionNames;

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
                            m_autoconnect.append( sender );
                        }
                    }
                }
            }

            /// Publish a QObject to a KJSEmbed::Engine.
            bool publishObject(KJS::ExecState* exec, const QString &name, QObject* object)
            {
                Q_UNUSED(exec);

                KJS::JSObject* obj = m_engine->addObject(object, name.isEmpty() ? object->objectName() : name);
                if( ! obj ) {
                    krosswarning( QString("Failed to publish the QObject name=\"%1\" objectName=\"%2\"").arg(name).arg(object ? object->objectName() : "NULL") );
                    return false;
                }
                m_publishedObjects << QPair<KJS::JSObject*, QPointer<QObject> >(obj, object);

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
                return true;
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
    if( d->m_engine )
        finalize(); // finalize before initialize
    clearError(); // clear previous errors.

    krossdebug( QString("KjsScript::initialize") );

    d->m_engine = new KJSEmbed::Engine();

    KJS::Interpreter* kjsinterpreter = d->m_engine->interpreter();
    kjsinterpreter->setShouldPrintExceptions(true);
    KJS::ExecState* exec = kjsinterpreter->globalExec();

    // publish our own action and the manager
    d->publishObject(exec, "self", action());
    d->publishObject(exec, "Kross", &Manager::self());

    d->m_defaultFunctionNames = functionNames();
    d->m_defaultFunctionNames << "Kross";

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
        for(KJS::PropertyNameArrayIterator it = props.begin(); it != props.end(); ++it)
            krossdebug( QString("  property name=%1").arg( it->ascii() ) );
    }
    */

    return true;
}

void KjsScript::finalize()
{
    KJS::Interpreter* kjsinterpreter = d->m_engine->interpreter();
    KJS::ExecState* exec = kjsinterpreter->globalExec();
    Q_UNUSED(exec);

    QList< QPair<KJS::JSObject*, QPointer<QObject> > >::Iterator it( d->m_publishedObjects.begin() );
    QList< QPair<KJS::JSObject*, QPointer<QObject> > >::Iterator end( d->m_publishedObjects.end() );
    for(; it != end; ++it) {
        QObject* obj = (*it).second;
        if( ! obj )
            continue;
        /*
        KJSEmbed::QObjectBinding *imp = KJSEmbed::extractBindingImp<KJSEmbed::QObjectBinding>(exec, kjsobj);
        Q_ASSERT(imp);
        QObject* obj = imp->object<QObject>();
        Q_ASSERT(obj);
        */

        // try to remove all pending slotproxy's the dirty way... please note, that we can't
        // do it using findChildren since the slotproxy's are handcraftet QObject's and don't
        // implement all of the QObject functionality. Also it seems KjsEmbed does some wired
        // things with the slotproxy's what prevents us from doing it another more nicer way.
        foreach( QObject* child, obj->children() )
            if( KJSEmbed::SlotProxy* proxy = dynamic_cast< KJSEmbed::SlotProxy* >(child) )
                delete proxy;

        /* the kjsobj-instance will be or got already deleted by KJS and we don't need to care
        KJS::JSObject* kjsobj = (*it).first;
        krossdebug(QString("KjsScript::finalize published object=%1").arg( kjsobj->className().ascii() ));
        delete kjsobj;
        */
    }
    d->m_publishedObjects.clear();

    d->m_autoconnect.clear();
    d->m_defaultFunctionNames.clear();

    delete d->m_engine;
    d->m_engine = 0;
}

void KjsScript::execute()
{
    if(! initialize()) {
        krosswarning( QString("KjsScript::execute aborted cause initialize failed.") );
        return;
    }

    QByteArray code = action()->code();
    if(code.startsWith("#!")) // krazy:exclude=strings
        code.remove(0, code.indexOf('\n')); // remove optional shebang-line

    QTextCodec *codec = QTextCodec::codecForLocale();
    KJS::UString c = codec ? KJS::UString(codec->toUnicode(code)) : KJS::UString(code.data(), code.size());
    //krossdebug( QString("KjsScript::execute code=\n%1").arg(c.qstring()) );
    KJSEmbed::Engine::ExitStatus exitstatus = d->m_engine->execute(c);

    KJS::Completion completion = d->m_engine->completion();
    KJS::Interpreter* kjsinterpreter = d->m_engine->interpreter();
    KJS::ExecState* exec = kjsinterpreter->globalExec();

    if(exitstatus != KJSEmbed::Engine::Success) {
        ErrorInterface error = extractError(completion, exec);
        setError(&error);
        return;
    }

    KJS::JSObject* kjsglobal = kjsinterpreter->globalObject();
    if( exec->hadException() ) {
        ErrorInterface error = extractError(d->m_engine->completion(), exec);
        krossdebug(QString("KjsScript::execute() failed: %1").arg(error.errorMessage()));
        setError(&error);
        //exec->clearException();
        return;
    }

    foreach(QObject* object, d->m_autoconnect) {
        const QMetaObject* metaobject = object->metaObject();
        const int count = metaobject->methodCount();
        for(int i = 0; i < count; ++i) {
            QMetaMethod metamethod = metaobject->method(i);
            if( metamethod.methodType() == QMetaMethod::Signal ) {
                const QString signature = metamethod.signature();
                const QByteArray name = signature.left(signature.indexOf('(')).toLatin1();
                krossdebug( QString("KjsScript::execute function=%1").arg(name.data()) );

                KJS::Identifier id = KJS::Identifier( KJS::UString(name.data()) );
                KJS::JSValue *functionvalue = kjsglobal->get(exec, id);
                if( ! functionvalue->isObject() )
                    continue;
                KJS::JSObject *function = functionvalue->toObject(exec);
                Q_ASSERT( ! exec->hadException() );
                if( exec->hadException() )
                    continue;
                if ( function && function->implementsCall() ) {
                    krossdebug( QString("KjsScript::execute connect function=%1 with signal=%2").arg(name.data()).arg(signature) );

                    QByteArray sendersignal = QString("2%1").arg(signature).toLatin1();
                    QByteArray receiverslot = QString("1%1").arg(signature).toLatin1();
                    KJSEmbed::SlotProxy* receiver = new KJSEmbed::SlotProxy(kjsglobal, exec->dynamicInterpreter(), object, signature.toLatin1());

                    if( connect(object, sendersignal, receiver, receiverslot) ) {
                        krossdebug( QString("KjsScript::execute connected function=%1 with object=%2 signal=%3").arg(name.data()).arg(object->objectName()).arg(signature) );
                    }
                    else {
                        krosswarning( QString("KjsScript::execute failed to connect object=%1 signal=%2").arg(object->objectName()).arg(signature) );
                    }

                }
            }
        }

    }
}

QStringList KjsScript::functionNames()
{
    KJS::Interpreter* kjsinterpreter = d->m_engine->interpreter();
    KJS::ExecState* exec = kjsinterpreter->globalExec();
    KJS::JSObject* kjsglobal = kjsinterpreter->globalObject();
    if( exec->hadException() ) {
        return QStringList();
    }

    KJS::PropertyNameArray props;
    kjsglobal->getPropertyNames(exec, props);

    QStringList list;
    for(KJS::PropertyNameArrayIterator it = props.begin(); it != props.end(); ++it) {
        const char* name = it->ascii();
        KJS::Identifier id = KJS::Identifier(name);
        KJS::JSValue *value = kjsglobal->get(exec, id);
        if( ! value || ! value->isObject() )
            continue;
        KJS::JSObject *obj = value->toObject(exec);
        if( ! obj || ! obj->implementsCall() || ! obj->implementsConstruct() || ! obj->classInfo() )
            continue;
        if( d->m_defaultFunctionNames.contains(name) )
            continue;
        list << name;
    }

    Q_ASSERT( ! exec->hadException() );
    return list;
}

QVariant KjsScript::callFunction(const QString& name, const QVariantList& args)
{
    //if( hadError() ) return QVariant(); // check if we had a prev error and abort if that's the case

    KJS::Interpreter* kjsinterpreter = d->m_engine->interpreter();
    KJS::ExecState* exec = kjsinterpreter->globalExec();
    KJS::JSObject* kjsglobal = kjsinterpreter->globalObject();
    if( exec->hadException() ) {
        ErrorInterface error = extractError(d->m_engine->completion(), exec);
        //setError(&error);
        krossdebug(QString("KjsScript::callFunction(\"%1\") Prev error: %2").arg(name).arg(error.errorMessage()));
        return QVariant();
    }

    KJS::Identifier id = KJS::Identifier( KJS::UString(name.toLatin1().data()) );
    KJS::JSValue *functionvalue = kjsglobal->get(exec, id);
    Q_ASSERT( ! exec->hadException() );

    KJS::JSObject *function = functionvalue->toObject(exec);
    if ( ! function || ! function->implementsCall() ) {
        krossdebug(QString("KjsScript::callFunction(\"%1\") No such function").arg(name));
        setError(QString("No such function \"%1\"").arg(name));
        return QVariant();
    }

    KJS::List kjsargs;
    foreach(const QVariant &variant, args) {
        if( qVariantCanConvert< QWidget* >(variant) ) {
            if( QWidget* widget = qvariant_cast< QWidget* >(variant) ) {
                kjsargs.append( KJSEmbed::createQObject(exec, widget, KJSEmbed::ObjectBinding::QObjOwned) );
                Q_ASSERT( ! exec->hadException() );
                continue;
            }
        }
        if( qVariantCanConvert< QObject* >(variant) ) {
            if( QObject* obj = qvariant_cast< QObject* >(variant) ) {
                kjsargs.append( KJSEmbed::createQObject(exec, obj, KJSEmbed::ObjectBinding::QObjOwned) );
                Q_ASSERT( ! exec->hadException() );
                continue;
            }
        }
        KJS::JSValue* jsvalue = KJSEmbed::convertToValue(exec, variant);
        Q_ASSERT( ! exec->hadException() );
        kjsargs.append( jsvalue );
    }

    KJS::JSValue *retValue = function->call(exec, kjsglobal, kjsargs);
    if( exec->hadException() ) {
        ErrorInterface error = extractError(d->m_engine->completion(), exec);
        //exec->clearException();
        krossdebug(QString("KjsScript::callFunction(\"%1\") Call failed: %2").arg(name).arg(error.errorMessage()));
        setError(&error);
        return QVariant();
    }

    QVariant result = retValue ? KJSEmbed::convertToVariant(exec, retValue) : QVariant();
    Q_ASSERT( ! exec->hadException() );
    return result;
}

QVariant KjsScript::evaluate(const QByteArray& code)
{
    QTextCodec *codec = QTextCodec::codecForLocale();
    KJS::UString c = codec ? KJS::UString(codec->toUnicode(code)) : KJS::UString(code.data(), code.size());

    KJSEmbed::Engine::ExitStatus exitstatus = d->m_engine->execute(c);

    KJS::Completion completion = d->m_engine->completion();
    KJS::Interpreter* kjsinterpreter = d->m_engine->interpreter();
    KJS::ExecState* exec = kjsinterpreter->globalExec();

    if(exitstatus != KJSEmbed::Engine::Success) {
        ErrorInterface error = extractError(completion, exec);
        setError(&error);
        return QVariant();
    }

    KJS::JSValue *retValue = completion.value();
    QVariant result = retValue ? KJSEmbed::convertToVariant(exec, retValue) : QVariant();
    Q_ASSERT( ! exec->hadException() );
    return result;
}
