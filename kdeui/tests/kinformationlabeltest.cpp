/*  This file is part of the KDE libraries
    Copyright (C) 2007 Michaël Larouche <larouche@kde.org>

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
#include "kinformationlabeltest.h"

#include <QtGui/QVBoxLayout>
#include <QtGui/QHBoxLayout>
#include <QtGui/QLabel>
#include <QtGui/QSpinBox>
#include <QtGui/QPushButton>
#include <QtGui/QComboBox>
#include <QtGui/QLineEdit>

#include <kinformationlabel.h>
#include <kapplication.h>
#include <kcmdlineargs.h>
#include <kicon.h>

KInformationLabel_Test::KInformationLabel_Test(QWidget *parent)
 : QWidget(parent)
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    iconComboBox = new QComboBox(this);
    iconComboBox->addItem( "Information" );
    iconComboBox->addItem( "Error" );
    iconComboBox->addItem( "Warning" );
    iconComboBox->addItem( "Custom" );
    mainLayout->addWidget( iconComboBox );

    QHBoxLayout *iconNameLayout = new QHBoxLayout(this);

    QLabel *labelIconName = new QLabel("Icon name:", this);
    iconNameLayout->addWidget(labelIconName);

    lineIconName = new QLineEdit(this);
    iconNameLayout->addWidget(lineIconName);

    mainLayout->addLayout(iconNameLayout);

    QHBoxLayout *timeoutLayout = new QHBoxLayout(this);

    QLabel *labelTimeout = new QLabel("Autohide timeout:", this);
    timeoutLayout->addWidget(labelTimeout);

    timeoutSpinBox = new QSpinBox(this);
    timeoutSpinBox->setRange(0, 10000);
    timeoutLayout->addWidget(timeoutSpinBox);

    mainLayout->addLayout(timeoutLayout);

    // Create main layout
    QPushButton *buttonTest = new QPushButton("Show message", this);
    connect(buttonTest, SIGNAL(clicked()), this, SLOT(buttonTestClicked()));
    mainLayout->addWidget(buttonTest);

    testLabel = new KInformationLabel(this);
    mainLayout->addWidget(testLabel);
    mainLayout->addStretch();
}

void KInformationLabel_Test::buttonTestClicked()
{
    testLabel->setIconType( static_cast<KInformationLabel::Icon>( iconComboBox->currentIndex() ) );
    testLabel->setIcon( KIcon(lineIconName->text()) );
    testLabel->setAutoHideTimeout( timeoutSpinBox->value() );
    testLabel->setText( "This is a test message" );
}

int main(int argc, char **argv)
{
    KCmdLineArgs::init(argc, argv, "kinformationlabeltest", "KInformationLabel_Test", "description", "version");

    KApplication app;

    KInformationLabel_Test *mainWidget = new KInformationLabel_Test;
    mainWidget->setAttribute( static_cast<Qt::WidgetAttribute>(Qt::WA_DeleteOnClose | Qt::WA_QuitOnClose) );
    mainWidget->show();

    return app.exec();
}

#include "kinformationlabeltest.moc"

// kate: space-indent on; indent-width 4; encoding utf-8; replace-tabs on;
