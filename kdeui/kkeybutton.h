#ifndef _KKEYBUTTON_H_
#define _KKEYBUTTON_H_

#include <qpushbutton.h>
#include <kshortcut.h>	// For class KShortcut

/**
 *  A push button that looks like a keyboard key.
 *  @short A push button that looks like a keyboard key.
 *  @author Mark Donohoe <donohoe@kde.org>
 *  @internal
 */
class KKeyButton: public QPushButton
{
  Q_OBJECT

public:

  /**
   * Constructs a key button widget.
   */
  KKeyButton( QWidget *parent=0, const char *name=0 );
  /**
   * Destructs the key button widget.
   */
  virtual ~KKeyButton();

  void setShortcut( const KShortcut& cut );
  KShortcut getShortcut() const
    { return m_cut; }

  /**
   * Reimplemented for internal purposes.
   */
  void setText( const QString& text );
  /**
   * Sets the widget into editing mode or not.
   * In editing mode, the widget has a different
   * look.
   */
  void captureShortcut( bool bCapture );
  /**
   * @return whether the widget is in editing mode.
   */
  bool isCapturing() const	{ return m_bEditing; }

signals:
  void capturedShortcut( KShortcut );

public slots:
  void slotCaptureShortcut();

protected:
#ifdef Q_WS_X11
  virtual bool x11Event( XEvent *pEvent );
  void x11EventKeyPress( XEvent *pEvent );
#endif

protected:
  bool m_bEditing;
  KShortcut m_cut;

  /**
   * Reimplemented for internal reasons.
   */
  void drawButton( QPainter* _painter );

private:
  class KKeyButtonPrivate* d;
};


#endif
