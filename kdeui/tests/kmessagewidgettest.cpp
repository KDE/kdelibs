#include <QApplication>
#include <QWidget>
#include <QVBoxLayout>
#include <QFrame>
#include <QCheckBox>

#include <kmessagewidget.h>

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);
    QWidget* mainWindow = new QWidget();

    QVBoxLayout* l = new QVBoxLayout(mainWindow);

    KMessageWidget* mw = new KMessageWidget(mainWindow);
    mw->setWordWrap(true);
    mw->setText(
        "Test KMessageWidget is properly sized when word-wrap is enabled by default."
    );
    // A frame to materialize the end of the KMessageWidget
    QFrame* frame = new QFrame(mainWindow);
    frame->setFrameStyle(QFrame::Panel);
    frame->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    QCheckBox* cb = new QCheckBox("wordWrap", mainWindow);
    cb->setChecked(true);

    l->addWidget(cb);
    l->addWidget(mw);
    l->addWidget(frame);

    mainWindow->resize(400, 300);
    mainWindow->show();

    QObject::connect(cb, SIGNAL(toggled(bool)), mw, SLOT(setWordWrap(bool)));

    return app.exec();
    delete mainWindow;
}

// kate: replace-tabs on;
