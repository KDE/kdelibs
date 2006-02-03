#include "builtins.h"
#include "static_binding.h"

#include <QApplication>
#include <QDebug>

using namespace KJSEmbed;

KJS::JSValue *callExec( KJS::ExecState *exec, KJS::JSObject *self, const KJS::List &args )
{
    return KJS::Boolean( QApplication::exec() );
}

KJS::JSValue *callDump( KJS::ExecState *exec, KJS::JSObject *self, const KJS::List &args )
{
    Q_UNUSED(self);
    if( args.size() == 1)
    {
        KJS::JSObject *object = args[0]->toObject(exec);
    }
    return KJS::Null();
}

const Method BuiltinsFactory::BuiltinMethods[] =
{
    {"exec", 0, KJS::DontDelete|KJS::ReadOnly, &callExec },
    {"dump", 1, KJS::DontDelete|KJS::ReadOnly, &callDump },
    {0, 0, 0, 0 }
};
