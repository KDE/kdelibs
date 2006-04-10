#ifndef KBETTERTHANKDIALOG_H
#define KBETTERTHANKDIALOG_H

#include <QDialog>

#include "ui_kbetterthankdialogbase.h"

class KBetterThanKDialog : public QDialog, private Ui_KBetterThanKDialogBase
{
  Q_OBJECT

  public:
    KBetterThanKDialog( QWidget *parent = 0 );

    void init();
    void setLabel( const QString &label );

  public Q_SLOTS:
    void clicked();

  protected Q_SLOTS:
    virtual void accept();
    virtual void reject();
};

#endif
