#include "resourcesqlconfigimpl.h"

ResourceSqlConfigImpl::ResourceSqlConfigImpl( QWidget *parent, const char *name )
    : ResourceSqlConfig( parent, name )
{
}

void ResourceSqlConfigImpl::loadSettings( KConfig *config )
{
    user->setText( config->readEntry( "SqlUser" ) );
    password->setText( config->readEntry( "SqlPassword" ) );
    dbName->setText( config->readEntry( "SqlName" ) );
    host->setText( config->readEntry( "SqlHost" ) );
}

void ResourceSqlConfigImpl::saveSettings( KConfig *config )
{
    config->writeEntry( "SqlUser", user->text() );
    config->writeEntry( "SqlPassword", password->text() );
    config->writeEntry( "SqlName", dbName->text() );
    config->writeEntry( "SqlHost", host->text() );
}

#include "resourcesqlconfigimpl.moc"
