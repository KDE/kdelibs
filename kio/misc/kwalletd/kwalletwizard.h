#ifndef KWALLETWIZARD_H
#define KWAlLETWIZARD_H

#include "ui_kwalletwizardbase.h"

class KWalletWizard : public Q3Wizard, public Ui_KWalletWizardBase
{
  public:
    KWalletWizard( QWidget *parent = 0 );

    void passwordPageUpdate();
    void init();
    void setAdvanced();
    void setBasic();
    void destroy();
};

#endif
