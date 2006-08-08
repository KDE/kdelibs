#include <QVBoxLayout>
#include <QListWidget>
#include <QTreeView>

#include <kjs/interpreter.h>
#include <kjs/PropertyNameArray.h>
#include <kjs/context.h>
#include <kjs/scope_chain.h>
#include <kjs/object.h>
#include <kdebug.h>

#include <typeinfo>

//#include "jsobjectmodel.h"
#include "objectmodel.h"
#include "localvariabledock.h"
#include "localvariabledock.moc"

LocalVariablesDock::LocalVariablesDock(QWidget *parent)
    : QDockWidget("Local Variables", parent)
{
//    m_widget = new QTreeWidget;
    m_widget = new QTreeView;
    setWidget(m_widget);
}

LocalVariablesDock::~LocalVariablesDock()
{
}

void LocalVariablesDock::display(KJS::ExecState *exec)
{
    // kDebug("::display(..) called for interpreter: %p", interpreter);

/*
    if (!m_model)
    {
        m_model = new ObjectModel(interpreter, this);
        m_widget->setModel(m_model);
    }

    m_model->update(interpreter);
*/
//    m_widget->clear();

    kDebug() << "Doing a full ScopeChain dump:" << endl;

//    KJS::ExecState *exec  = interpreter->globalExec();
    KJS::Context* context = exec->context();
    if (!context) {
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

        kDebug() << typeid(*object).name() << endl;

        QString name = object->toString(exec).qstring();
        kDebug() << "scope list object: " << name << endl;

        KJS::PropertyNameArray props;
        object->getPropertyNames(exec, props);
        for( KJS::PropertyNameArrayIterator ref = props.begin(); ref != props.end(); ref++)
        {
            KJS::Identifier id = *ref;
            QString refName = id.qstring();
            // KJS::JSValue* instance = object->get(exec, id);

            kDebug() << "    refrence list object: " << refName << endl;
        }
    }
/*

    KJS::JSObject *parentInstance = interpreter->globalObject();

    VariableItem *childItem = 0;
    KJS::ExecState *exec = interpreter->globalExec();

    int idx = 0;
    KJS::ReferenceList props = parentInstance->propList(exec);
    for( KJS::ReferenceListIterator ref = props.begin(); ref != props.end(); ref++)
    {
        QByteArray name = ref->getPropertyName(exec).ascii();
        KJS::JSObject *object = parentInstance->get( exec, name.constData() )->toObject(exec);
        childItem = new VariableItem(name, object);
        m_widget->addTopLevelItem(childItem);
    }
*/
}




// void LocalVariablesDock::setInterpreter(KJS::Interpreter *interpreter)
// {
//     if (interpreter)
//     {
//         if (!m_model)
//         {
//             m_model = new JSObjectModel(interpreter, this);
//             m_widget->setModel(m_model);
//         }
// 
//         KJS::JSObject *obj = interpreter->globalObject();
//         m_model->updateModel(interpreter, obj);
//     }
// }
