/*  This file is part of the KDE libraries
    Copyright (C) 2006 Michaël Larouche <michael.larouche@kdemail.net>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; version 2
    of the License.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/
#include "kmessagetest.h"

#include <QtCore/QString>
#include <QtGui/QMessageBox>
#include <QtGui/QPushButton>
#include <QtGui/QBoxLayout>

#include <kapplication.h>
#include <kcmdlineargs.h>
#include <kdebug.h>

#include <kmessage.h>
#include <kmessageboxmessagehandler.h>
#include <kpassivepopupmessagehandler.h>

KMessage_Test::KMessage_Test(QWidget *parent)
 : QWidget(parent)
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    QPushButton *buttonError = new QPushButton( QLatin1String("Show error"), this );
    connect(buttonError, SIGNAL(clicked()), this, SLOT(showError()));

    QPushButton *buttonFatal = new QPushButton( QLatin1String("Show fatal"), this );
    connect(buttonFatal, SIGNAL(clicked()), this, SLOT(showFatal()));

    QPushButton *buttonInformation = new QPushButton( QLatin1String("Show information"), this );
    connect(buttonInformation, SIGNAL(clicked()), this, SLOT(showInformation()));

    QPushButton *buttonSorry = new QPushButton( QLatin1String("Show sorry"), this );
    connect(buttonSorry, SIGNAL(clicked()), this, SLOT(showSorry()));

    QPushButton *buttonWarning = new QPushButton( QLatin1String("Show warning"), this );
    connect(buttonWarning, SIGNAL(clicked()), this, SLOT(showWarning()));

    mainLayout->addWidget( buttonError );
    mainLayout->addWidget( buttonFatal );
    mainLayout->addWidget( buttonInformation );
    mainLayout->addWidget( buttonSorry );
    mainLayout->addWidget( buttonWarning );
}

void KMessage_Test::showError()
{
    KMessage::message( KMessage::Error, QLatin1String("Error: Destruction of the Death Star failed."), QLatin1String("KMessage_Test") );
}

void KMessage_Test::showFatal()
{
    KMessage::message( KMessage::Fatal, QLatin1String("Fatal: You have turn to the dark side of the Force."), QLatin1String("KMessage_Test") );
}

void KMessage_Test::showInformation()
{
    KMessage::message( KMessage::Information, QLatin1String("Info: This is a demonstration of the new KMessage API for kdelibs. It abstract the display of message and you can develop custom mesage handle for your application"), QLatin1String("KMessage_Test") );
}

void KMessage_Test::showSorry()
{
    KMessage::message( KMessage::Sorry, QLatin1String("Sorry but our princess is in another castle."), QLatin1String("KMessage_Test") );
}

void KMessage_Test::showWarning()
{
    KMessage::message( KMessage::Warning, QLatin1String("Warning: Loading failed. Your user experience will be affected."), QLatin1String("KMessage_Test") );
}

int main(int argc, char **argv)
{
    KCmdLineArgs::init(argc, argv, "kmessagetest", 0, ki18n("KMessage_Test"), "version", ki18n("description"));

    KApplication app;
    app.setQuitOnLastWindowClosed( false );

    KMessage_Test *mainWidget = new KMessage_Test;
    mainWidget->setAttribute( static_cast<Qt::WidgetAttribute>(Qt::WA_DeleteOnClose | Qt::WA_QuitOnClose) );

    int i = QMessageBox::information(0, "Select", "Select type of MessageHandler",
                                 "KMessageBox", "KPassivePopup", "Default (stderr)");
    if(i == 0)
    {
        KMessage::setMessageHandler( new KMessageBoxMessageHandler(mainWidget) );
    }
    else if(i == 1)
    {
       KMessage::setMessageHandler( new KPassivePopupMessageHandler(mainWidget) );
    }

    mainWidget->show();

    return app.exec();
}

#include "kmessagetest.moc"
// kate: space-indent on; indent-width 4; encoding utf-8; replace-tabs on;
