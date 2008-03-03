#include "ratingpaintertestwidget.h"

#include <QApplication>
#include <kcomponentdata.h>


int main( int argc, char** argv )
{
    QApplication app( argc, argv );
    KComponentData data( "NepomukRatingPainterTest" );
    RatingPainterTestWidget ratingW;
    ratingW.show();
    return app.exec();
}
