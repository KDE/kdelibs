#ifndef KDUALCOLORTEST_H
#define KDUALCOLORTEST_H

#include <kdualcolorbutton.h>

class QLabel;

class KDualColorWidget : public QWidget
{
  Q_OBJECT

  public:
    KDualColorWidget( QWidget *parent = 0 );

  protected Q_SLOTS:
    void slotForegroundColorChanged( const QColor &color );
    void slotBackgroundColorChanged( const QColor &color );
    void slotSelectionChanged( KDualColorButton::Selection selection );

  protected:
    QLabel *mLabel;
};

#endif
