#include <QtDebug>
#include <QCoreApplication>

#include <Weaver/ThreadWeaver.h>
#include <Weaver/Lambda.h>

using namespace ThreadWeaver;

int main(int argc, char** argv)
{
    QCoreApplication app(argc, argv);
    Lambda l( []() { qDebug() << "Hello World!"; } );
    Weaver::instance()->enqueueRaw(&l);
    Weaver::instance()->finish();
}

