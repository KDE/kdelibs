#ifndef _KKEYBUTTON_H_
#define _KKEYBUTTON_H_
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
  ~KKeyButton();

  void setKey( uint key );
  uint getKey() const { return key; }
  /**
   * Reimplemented for internal purposes.
   */
  void setText( const QString& text );
  /**
   * Sets the widget into editing mode or not.
   * In editing mode, the widget has a different
   * look.
   */
  //void setEditing(bool _editing);
  void captureKey( bool bCapture );
  /**
   * @return whether the widget is in editing mode.
   */
  bool isCapturing() const	{ return editing; }

signals:
  void capturedKey( uint key );

public slots:
  void captureKey();

protected:
  virtual bool x11Event( XEvent *pEvent );
  void keyPressEventX( XEvent *pEvent );

protected:
  /**
   * Reimplemented for internal reasons.
   */
  void drawButton( QPainter* _painter );

private:
  bool editing;
  uint key;

  //class KKeyButtonPrivate;
  //KKeyButtonPrivate *d;
};


#endif
