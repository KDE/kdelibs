#include <QVBoxLayout>
#include <QTreeView>
#include <QStandardItemModel>
#include <QStandardItem>
#include <QEventLoop>

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

void LocalVariablesDock::clear()
{
    if (m_execModel)
        delete m_execModel;
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

