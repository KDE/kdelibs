#include <klocale.h>

#include <qgroupbox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qpushbutton.h>

#include "resourcefactory.h"
#include "resourceconfigdlg.h"

ResourceConfigDlg::ResourceConfigDlg( QWidget *parent, const QString& type,
	KConfig *config, const char *name )
    : QDialog( parent, name, true ), mConfig( config )
{
    KABC::ResourceFactory *factory = KABC::ResourceFactory::self();
    mConfigWidget = factory->configWidget( type, this );
    if ( mConfigWidget && mConfig )
	mConfigWidget->loadSettings( mConfig );

    setCaption( i18n( "Resource Configuration" ) );

    QVBoxLayout *mainLayout = new QVBoxLayout( this );
    
    QGroupBox *groupBox = new QGroupBox( 2, Qt::Horizontal,  this );
    groupBox->setTitle( i18n( "General Settings" ) );

    QLabel *label = new QLabel( i18n( "Name:" ), groupBox );

    resourceName = new KLineEdit( groupBox );

    resourceIsReadOnly = new QCheckBox( i18n( "Read-Only" ), groupBox );

    resourceIsFast = new QCheckBox( i18n( "Fast resource" ), groupBox );

    mainLayout->addWidget( groupBox );

    if ( mConfigWidget ) {
	mainLayout->addSpacing( 10 );
	mainLayout->addWidget( mConfigWidget );
	mConfigWidget->show();
    }
    mainLayout->addSpacing( 10 );

    buttonBox = new KButtonBox( this );

    buttonBox->addStretch();    
    buttonBox->addButton( i18n( "&Apply" ), this, SLOT( accept() ) );
    buttonBox->addButton( i18n( "&Cancel" ), this, SLOT( reject() ) );
    buttonBox->layout();

    mainLayout->addWidget( buttonBox );
}

int ResourceConfigDlg::exec()
{
    return QDialog::exec();
}

void ResourceConfigDlg::accept()
{
    if ( mConfigWidget && mConfig )
	mConfigWidget->saveSettings( mConfig );

    QDialog::accept();
}

#include "resourceconfigdlg.moc"
