#include "format.h"

#include "resourcefileconfigimpl.h"

ResourceFileConfigImpl::ResourceFileConfigImpl( QWidget *parent, const char *name )
    : ResourceFileConfig( parent, name )
{
    formatBox->insertItem( i18n( "VCard" ), FORMAT_VCARD );
    formatBox->insertItem( i18n( "Binary" ), FORMAT_BINARY );
}

void ResourceFileConfigImpl::loadSettings( KConfig *config )
{
    uint format = config->readNumEntry( "FileFormat", FORMAT_VCARD );
    formatBox->setCurrentItem( format );

    fileNameEdit->setURL( config->readEntry( "FileName" ) );    
}

void ResourceFileConfigImpl::saveSettings( KConfig *config )
{
    config->writeEntry( "FileFormat", formatBox->currentItem() );
    config->writeEntry( "FileName", fileNameEdit->url() );
}

#include "resourcefileconfigimpl.moc"
