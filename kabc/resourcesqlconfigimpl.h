#ifndef RESOURCESQLCONFIGIMPL_H
#define RESOURCESQLCONFIGIMPL_H

#include "resourcesqlconfig.h"

class ResourceSqlConfigImpl : public ResourceSqlConfig
{
    Q_OBJECT
public:
    ResourceSqlConfigImpl( QWidget *parent = 0, const char *name = 0 );

public slots:
    void loadSettings( KConfig *config );
    void saveSettings( KConfig *config );
};

#endif
