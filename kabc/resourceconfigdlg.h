#ifndef RESOURCECONFIGDLG_H
#define RESOURCECONFIGDLG_H

#include <qcheckbox.h>
#include <qdialog.h>

#include <kbuttonbox.h>
#include <kconfig.h>
#include <klineedit.h>

#include "resourceconfigwidget.h"

class ResourceConfigDlg : QDialog
{
    Q_OBJECT
public:
    ResourceConfigDlg( QWidget *parent, const QString& type,
	    KConfig *config, const char *name = 0);

    KButtonBox *buttonBox;
    KLineEdit *resourceName;
    QCheckBox *resourceIsReadOnly;
    QCheckBox *resourceIsFast;

public slots:
    int exec();

protected slots:
    void accept();

private:
    ResourceConfigWidget *mConfigWidget;
    KConfig *mConfig;
};

#endif
