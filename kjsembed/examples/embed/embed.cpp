#include <QDebug>
#include <QApplication>

#include <kjs/interpreter.h>
#include <kjs/ustring.h>

#include <kjsembed/kjsembed.h>
#include <kjsembed/object_binding.h>

#include "test_object.h"
#include "test_pointer.h"

int main( int argc, char **argv )
{
    QApplication app(argc, argv);

    Line obj;
    obj.name = "C++ Object";

    KJSEmbed::Engine *kernel = new KJSEmbed::Engine();
    KJS::Interpreter *js = kernel->interpreter();
    KJS::JSObject *globalObject = js->globalObject();
    KJS::ExecState *exec = js->globalExec();

    KJSEmbed::StaticConstructor::add( exec, globalObject, TestObject::constructor() ); // Ctor
    KJSEmbed::StaticConstructor::add( exec, globalObject, TestPointer::constructor() ); // Ctor

    KJS::JSValue *val = KJSEmbed::createObject(exec, "Line", &obj);
    globalObject->put( exec, "Test", val ); // Static object

    KJS::JSObject *appobj = kernel->addObject( &app, "Application" );

    KJSEmbed::Engine::ExitStatus result = kernel->runFile( "test.js" );
    if ( result != KJSEmbed::Engine::Success )
    {
        KJS::Completion jsres = kernel->completion();
        qDebug() << jsres.value()->toString(exec).qstring();
        return 0;
    }

    return result;
}

