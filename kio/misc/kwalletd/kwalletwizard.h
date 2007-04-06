#ifndef KWALLETWIZARD_H
#define KWALLETWIZARD_H

#include <QtGui/QWizard>

class PageIntro;
class PagePassword;

class KWalletWizard : public QWizard
{
  Q_OBJECT
  public:

        enum WizardType
        {
            Basic,
            Advanced
        };

        static const int PageIntroId = 0;
        static const int PagePasswordId = 1;
        static const int PageOptionsId = 2;
        static const int PageExplanationId = 3;

    KWalletWizard( QWidget *parent = 0 );

        WizardType wizardType() const;

    protected:
        virtual void initializePage(int id);

  protected Q_SLOTS:
    void passwordPageUpdate();

    private:
        PageIntro *m_pageIntro;
        PagePassword *m_pagePasswd;
};

#endif
