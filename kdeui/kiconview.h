#ifndef KICONVIEW_H
#define KICONVIEW_H

#include <qcursor.h>
#include <qiconview.h>

#include <kdebug.h>

/**
 * This Widget extends the functionality of QIconView to honor the system
 * wide settings for Single Click/Double Click mode, Auto Selection and
 * Change Cursor over Link.
 *
 * There is a new signal executed(). It gets connected to either 
 * QIconView::clicked() or QIconView::doubleClicked() depending on the KDE 
 * wide Single Click/Double Click settings. It is strongly recomended that 
 * you use this signal instead of the above mentioned. This way you don´t 
 * need to care about the current settings.
 * If you want to get informed when the user selects something connect to the 
 * QIconView::selectionChanged() signal.
 **/
class KIconView : public QIconView
{
  Q_OBJECT
      
public:
  KIconView( QWidget *parent = 0, const char *name = 0, WFlags f = 0 );

signals:
  
  /**
   * Gets emitted when the execute operation has been performed.
   */
  void executed( QIconViewItem *item );

protected slots:
 void slotOnItem( QIconViewItem *item );
 void slotOnViewport();

 /**
  * Auto selection happend.
  */
 void slotAutoSelect();
 void slotExecute( QIconViewItem *item );

protected:
 /**
  * Check wether settings changed.
  */
  void checkSettings();
 
  QCursor oldCursor;
  bool m_bUseSingle;
  bool m_bChangeCursorOverItem;

  QIconViewItem* m_pCurrentItem;

  QTimer* m_pAutoSelect;
  int m_autoSelectDelay;
};

#endif
