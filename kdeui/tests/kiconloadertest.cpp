/*
 * Test for KIconLoaderDialog.
 */

#include <qstring.h>
#include <kapp.h>
#include <kicontheme.h>
#include "kiconloaderdialog.h"

int main(int argc, char **argv)
{
    KApplication app(argc, argv, "test");
    KIconLoaderButton *but = new KIconLoaderButton();
    app.setMainWidget(but);
    but->show();
    return app.exec();
}
