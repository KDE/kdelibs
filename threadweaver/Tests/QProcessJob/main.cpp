#include <QApplication>

#include "ThreadWeaver.h"
#include "QProcessJob.h"

int main ( int argc,  char ** argv )
{
    QApplication app ( argc,  argv );
    QProcessJob job;

    Weaver::instance()->enqueue ( &job );

    Weaver::instance()->finish();
    return 0;
}
