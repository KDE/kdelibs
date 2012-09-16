#include "main.h"

#include <QtCore/QDir>

#include <kde_terminal_interface.h>
#include <KParts/Part>
#include <KService>
#include <KAboutData>
#include <KCmdLineArgs>
#include <KApplication>

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
    KAboutData* about = new KAboutData( "tetest", 0, ki18n("TETest"), "0.1" );
    KCmdLineArgs::init( argc, argv, about );
    KApplication app;
    Window* window = new Window();
    window->show();
    return app.exec();
};

