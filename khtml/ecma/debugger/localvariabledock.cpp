#include <QVBoxLayout>
#include <QListWidget>
#include <QTreeView>

#include <kjs/interpreter.h>
#include <kjs/reference.h>
#include <kjs/reference_list.h>
#include <kjs/scope_chain.h>
#include <kjs/object.h>

//#include "jsobjectmodel.h"
#include "localvariabledock.h"
#include "localvariabledock.moc"

LocalVariablesDock::LocalVariablesDock(QWidget *parent)
    : QDockWidget("Local Variables", parent)
{
    m_widget = new QTreeWidget;
    setWidget(m_widget);
}

LocalVariablesDock::~LocalVariablesDock()
{
}

void LocalVariablesDock::display(KJS::Interpreter *interpreter)
{
    m_widget->clear();

    KJS::ExecState *exec = interpreter->globalExec();
    KJS::Context context = exec->context();
    KJS::ScopeChain chain = context.scopeChain();

/*
    for( KJS::ScopeChainIterator obj = chain.begin();
         obj != chain.end();
         ++obj)
    {
        QString name = (*obj)->className().qstring();
        KJS::JSObject *object = (*obj);

        VariableItem *child = new VariableItem(name, object);
        m_widget->addTopLevelItem(child);
    }
*/
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
