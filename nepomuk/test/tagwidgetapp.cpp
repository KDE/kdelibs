#include "tagwidgettest.h"

#include <QApplication>
#include <kcomponentdata.h>


int main( int argc, char** argv )
{
    QApplication app( argc, argv );
    KComponentData data( "TagWidgetApp" );
    TagWidgetTest tw;
    tw.show();
    return app.exec();
}
