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

#include <typeinfo>

//#include "jsobjectmodel.h"
#include "objectmodel.h"
#include "localvariabledock.h"
#include "localvariabledock.moc"

LocalVariablesDock::LocalVariablesDock(QWidget *parent)
    : QDockWidget("Local Variables", parent)
{
    m_view = new QTreeView;
    m_model = new ObjectModel;
    m_view->setModel(m_model);

    setWidget(m_view);
}

LocalVariablesDock::~LocalVariablesDock()
{
}

void LocalVariablesDock::display(KJS::ExecState *exec)
{
    m_model->update(exec);
    m_view->reset();

/*
    KJS::Context* context = exec->context();
    if (!context)
    {
        kDebug() << "nothing running!" << endl;
        return;
    }

    m_model->clear();

    KJS::ScopeChain chain = context->scopeChain();
    for( KJS::ScopeChainIterator obj = chain.begin();
         obj != chain.end();
         ++obj)
    {
        KJS::JSObject *object = (*obj);
        if (!object)
            break;

        if (object->isActivation())         // hack check to see if we're in local scope
        {
            QString name = object->toString(exec).qstring();
            kDebug() << "scope list object: " << name << endl;

            KJS::PropertyNameArray props;
            object->getPropertyNames(exec, props);
            for(KJS::PropertyNameArrayIterator ref = props.begin();
                ref != props.end();
                ref++)
            {
                KJS::Identifier id = *ref;
                QString refName = id.qstring();
                KJS::JSValue *value = object->get(exec, id);

                QStandardItem *item = new QStandardItem(refName);

                // Should we check for these?
//                 bool isUndefined () const
//                 bool isNull () const
//                 bool isUndefinedOrNull () const

                // First lets check if its a primitive type
                if (value->isBoolean())
                {
                    item->setColumnCount(3);
                    kDebug() << "Boolean!" << endl << endl << endl << endl << endl << endl;
                    QStandardItem *typeName = new QStandardItem("bool");
                    QStandardItem *valueName = new QStandardItem(value->toBoolean(exec));
                    QList<QStandardItem*> items;
                    items << typeName << valueName;
                    item->appendColumn(items);
                }
                else if (value->isNumber())
                {
                    item->setColumnCount(3);
                    kDebug() << "Number!" << endl << endl << endl << endl << endl << endl;
                    QStandardItem *typeName = new QStandardItem("number");
                    QStandardItem *valueName = new QStandardItem(QString::number(value->toNumber(exec)));
                    QList<QStandardItem*> items;
                    items << typeName << valueName;
                    item->appendColumn(items);
                }
                else if (value->isString())
                {
                    item->setColumnCount(3);
                    kDebug() << "String!" << endl << endl << endl << endl << endl << endl;
                    QStandardItem *typeName = new QStandardItem("string");
                    QStandardItem *valueName = new QStandardItem(value->toString(exec).qstring());
                    QList<QStandardItem*> items;
                    items << typeName << valueName;
                    item->appendColumn(items);
                }
                else if (value->isObject())
                {
                    kDebug() << "Object!" << endl << endl << endl << endl << endl << endl;
                    KJS::JSObject *tmpObject = value->toObject(exec);
                    if(tmpObject->implementsConstruct())
                    {
                        item->setIcon(QIcon(":/images/class.png"));
                        item->setData(Qt::TextColorRole, "blue");
                    }
                    else if(tmpObject->implementsCall())
                    {
                        item->setIcon(QIcon(":/images/method.png"));
                        item->setData(Qt::TextColorRole, "green");
                    }
                    else
                    {
                        item->setIcon(QIcon(":/images/property.png"));
                        item->setData(Qt::TextColorRole, "black");
                    }
                }

                m_model->appendRow(item);
            }
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
