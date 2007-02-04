#ifndef KWALLETWIZARD_H
#define KWALLETWIZARD_H

#include "ui_kwalletwizardbase.h"

class KWalletWizard : public Q3Wizard, public Ui_KWalletWizardBase
{
  Q_OBJECT
  public:
    KWalletWizard( QWidget *parent = 0 );
    void init();
    void destroy();
  protected Q_SLOTS:
    void passwordPageUpdate();
    void setAdvanced();
    void setBasic();
};

#endif
