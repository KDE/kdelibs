#ifndef KLISTBOX_H
#define KLISTBOX_H

#include <qcursor.h>
#include <qlistbox.h>

class KListBox : public QListBox
{
    Q_OBJECT

public:
  KListBox( QWidget *parent = 0, const char *name = 0, WFlags f = 0 );

signals:
  void executed( QListBoxItem *item );
  void executed( QListBoxItem *item, const QPoint &pos );
  void doubleClicked( QListBoxItem *item, const QPoint &pos );

protected slots:
  void slotOnItem( QListBoxItem *item );
  void slotOnViewport();

  void slotSettingsChanged(int);

  /**
   * Auto selection happend.
   */
  void slotAutoSelect();

protected:
  void emitExecute( QListBoxItem *item, const QPoint &pos );

  virtual void keyPressEvent(QKeyEvent *e);
  virtual void focusOutEvent( QFocusEvent *fe );
  virtual void leaveEvent( QEvent *e );
  virtual void contentsMousePressEvent( QMouseEvent *e );
  virtual void contentsMouseDoubleClickEvent ( QMouseEvent *e );

  QCursor oldCursor;
  bool m_bUseSingle;
  bool m_bChangeCursorOverItem;

  QListBoxItem* m_pCurrentItem;

  QTimer* m_pAutoSelect;
  int m_autoSelectDelay;

private slots:
  void slotMouseButtonClicked( int btn, QListBoxItem *item, const QPoint &pos );

private:
  class KListBoxPrivate;
  KListBoxPrivate *d;
};

#endif
