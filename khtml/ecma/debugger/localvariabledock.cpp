#include <QVBoxLayout>
#include <QTreeView>
#include <QStandardItemModel>
#include <QStandardItem>

#include <kjs/interpreter.h>
#include <kjs/PropertyNameArray.h>
#include <kjs/context.h>
#include <kjs/scope_chain.h>
#include <kjs/object.h>
#include <kdebug.h>

#include "objectmodel.h"
#include "execstatemodel.h"
#include "localvariabledock.h"
#include "localvariabledock.moc"

LocalVariablesDock::LocalVariablesDock(QWidget *parent)
    : QDockWidget("Local Variables", parent)
{
    m_view = new QTreeView;
//    m_model = new ObjectModel;
//    m_view->setModel(m_model);

    setWidget(m_view);
}

LocalVariablesDock::~LocalVariablesDock()
{
}


void getValues3(KJS::ExecState *exec, KJS::JSObject *object)
{
    KJS::PropertyNameArray props;
    object->getPropertyNames(exec, props);
    for(KJS::PropertyNameArrayIterator ref = props.begin();
        ref != props.end();
        ref++)
    {
        KJS::Identifier id = *ref;
        QString refName = id.qstring();
        KJS::JSValue *jsvalue = object->get(exec, id);

        // Should we check for these?
        // bool isUndefined () const
        // bool isNull () const
        // bool isUndefinedOrNull () const

        QString type;
        QVariant value;
        // First lets check if its a primitive type
        if (jsvalue->isBoolean())
        {
            type = "bool";
            value = jsvalue->toBoolean(exec);
        }
        else if (jsvalue->isNumber())
        {
            type = "number";
            value = jsvalue->toNumber(exec);
        }
        else if (jsvalue->isString())
        {
            type = "string";
            value = jsvalue->toString(exec).qstring();
        }
        else if (jsvalue->isObject())
        {
            type = "object";
            value = "[object data]";
        }
        kDebug() << "getValues3: " << refName << "\t" << type << "\t" << value << endl;
    }
}


void getValues2(KJS::ExecState *exec, KJS::JSObject *object)
{
    KJS::PropertyNameArray props;
    object->getPropertyNames(exec, props);
    for(KJS::PropertyNameArrayIterator ref = props.begin();
        ref != props.end();
        ref++)
    {
        KJS::Identifier id = *ref;
        QString refName = id.qstring();
        KJS::JSValue *jsvalue = object->get(exec, id);

        // Should we check for these?
        // bool isUndefined () const
        // bool isNull () const
        // bool isUndefinedOrNull () const

        QString type;
        QVariant value;
        // First lets check if its a primitive type
        if (jsvalue->isBoolean())
        {
            type = "bool";
            value = jsvalue->toBoolean(exec);
        }
        else if (jsvalue->isNumber())
        {
            type = "number";
            value = jsvalue->toNumber(exec);
        }
        else if (jsvalue->isString())
        {
            type = "string";
            value = jsvalue->toString(exec).qstring();
        }
        else if (jsvalue->isObject())
        {
            type = "object";
            value = "[object data]";

            KJS::JSObject *newObject = jsvalue->toObject(exec);
            getValues3(exec, newObject);
        }
        kDebug() << "getValues2: " << refName << "\t" << type << "\t" << value << endl;
    }
}



void getValues(KJS::ExecState *exec, KJS::JSObject *object)
{
    KJS::PropertyNameArray props;
    object->getPropertyNames(exec, props);
    for(KJS::PropertyNameArrayIterator ref = props.begin();
        ref != props.end();
        ref++)
    {
        KJS::Identifier id = *ref;
        QString refName = id.qstring();
        KJS::JSValue *jsvalue = object->get(exec, id);

        // Should we check for these?
        // bool isUndefined () const
        // bool isNull () const
        // bool isUndefinedOrNull () const

        QString type;
        QVariant value;
        // First lets check if its a primitive type
        if (jsvalue->isBoolean())
        {
            type = "bool";
            value = jsvalue->toBoolean(exec);
        }
        else if (jsvalue->isNumber())
        {
            type = "number";
            value = jsvalue->toNumber(exec);
        }
        else if (jsvalue->isString())
        {
            type = "string";
            value = jsvalue->toString(exec).qstring();
        }
        else if (jsvalue->isObject())
        {
            type = "object";
            value = "[object data]";

            KJS::JSObject *newObject = jsvalue->toObject(exec);
            getValues2(exec, newObject);
        }
        kDebug() << "getValues: " << refName << "\t" << type << "\t" << value << endl;
    }
}


void LocalVariablesDock::display(KJS::ExecState *exec)
{
    if (m_execModel)
        delete m_execModel;

    m_execModel = new ExecStateModel(exec);
    m_view->setModel(m_execModel);
//    m_view->reset();

/*
    KJS::Context* context = exec->context();
    if (!context)
    {
        kDebug() << "nothing running!" << endl;
        return;
    }

    KJS::ScopeChain chain = context->scopeChain();
    for( KJS::ScopeChainIterator obj = chain.begin();
         obj != chain.end();
         ++obj)
    {
        KJS::JSObject *object = (*obj);
        if (!object)
            break;

        if (object->isActivation())         // hack check to see if we're in local scope
            getValues(exec, object);
    }
*/
}

