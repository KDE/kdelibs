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
  Q_PROPERTY( bool editing READ isEditing WRITE setEditing )

public:

  /**
   * Constructs a key button widget.
   */
  KKeyButton( QWidget *parent=0, const char *name=0 );
  /**
   * Destructs the key button widget.
   */
  ~KKeyButton();
  /**
   * Reimplemented for internal purposes.
   */
  void setText( const QString& text );
  /**
   * Sets the widget into editing mode or not.
   * In editing mode, the widget has a different
   * look.
   */
  void setEditing(bool _editing);
  /**
   * @return whether the widget is in editing mode.
   */
  bool isEditing() const;

protected:
  /**
   * Reimplemented for internal reasons.
   */
  void drawButton( QPainter* _painter );

private:
  bool editing;

  class KKeyButtonPrivate;
  KKeyButtonPrivate *d;
};


#endif
