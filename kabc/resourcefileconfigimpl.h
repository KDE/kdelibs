#ifndef RESOURCEFILECONFIGIMPL_H
#define RESOURCEFILECONFIGIMPL_H

#include "resourcefileconfig.h"

class ResourceFileConfigImpl : public ResourceFileConfig
{
    Q_OBJECT
public:
    ResourceFileConfigImpl( QWidget *parent = 0, const char *name = 0 );

public slots:
    void loadSettings( KConfig *config );
    void saveSettings( KConfig *config );
};

#endif
