#include "resourceconfigwidget.h"

ResourceConfigWidget::ResourceConfigWidget( QWidget *parent, const char *name )
    : QWidget( parent, name )
{
}

void ResourceConfigWidget::loadSettings( KConfig * )
{
}

void ResourceConfigWidget::saveSettings( KConfig * )
{
}

#include "resourceconfigwidget.moc"
