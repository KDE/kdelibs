#ifndef KBETTERTHANKDIALOG_H
#define KBETTERTHANKDIALOG_H

#include <QtGui/QDialog>

#include "ui_kbetterthankdialogbase.h"

class KBetterThanKDialog : public QDialog, private Ui_KBetterThanKDialogBase
{
  Q_OBJECT

  public:
    KBetterThanKDialog( QWidget *parent = 0 );

    void init();
    void setLabel( const QString &label );

  protected Q_SLOTS:
    virtual void accept();
    virtual void reject();
  
  private Q_SLOTS:
    void allowOnceClicked();
    void allowAlwaysClicked();
    void denyClicked();
    void denyForeverClicked();
};

#endif
