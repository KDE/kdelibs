#include <QMainWindow>

class Window : public QMainWindow
{
    Q_OBJECT
public:
    Window();
};

#include <QtCore/QDir>

#include <kde_terminal_interface.h>
#include <kparts/part.h>
#include <kservice.h>
#include <QApplication>

Window::Window()
{
    this->resize(800,600);

    KService::Ptr service = KService::serviceByDesktopName("konsolepart");
    Q_ASSERT(service);

    KParts::ReadOnlyPart* part = service->createInstance<KParts::ReadOnlyPart>(this, this, QVariantList());
    Q_ASSERT(part);

    setCentralWidget( part->widget() );

    TerminalInterface* interface = qobject_cast<TerminalInterface*>(part);
    interface->showShellInDir(QDir::home().path());

    connect(part, SIGNAL(destroyed()), this, SLOT(deleteLater()));
}

int main( int argc, char** argv )
{
    QApplication::setApplicationName("tetest");
    QApplication app(argc, argv);
    Window* window = new Window();
    window->show();
    return app.exec();
};

#include "terminal_test.moc"
