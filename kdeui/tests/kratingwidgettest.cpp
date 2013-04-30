#include <QApplication>
#include <QVBoxLayout>

#include <kratingwidget.h>

int main( int argc, char **argv )
{
    QApplication app(argc, argv);

    QWidget window;
    window.setLayout(new QVBoxLayout);

    KRatingWidget *enabled = new KRatingWidget(&window);
    window.layout()->addWidget(enabled);

    KRatingWidget *disabled = new KRatingWidget(&window);
    disabled->setEnabled(false);
    window.layout()->addWidget(disabled);

    window.show();

    return app.exec();
}

/* vim: et sw=4
 */
