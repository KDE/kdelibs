#ifndef RESOURCECONFIGWIDGET_H
#define RESOURCECONFIGWIDGET_H

#include <qwidget.h>

#include <kconfig.h>

class ResourceConfigWidget : public QWidget
{
    Q_OBJECT
public:
    ResourceConfigWidget( QWidget *parent = 0, const char *name = 0 );

public slots:
    virtual void loadSettings( KConfig *config );
    virtual void saveSettings( KConfig *config );
};

#endif
