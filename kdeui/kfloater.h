#ifndef _KFLOATER_H
#define _KFLOATER_H

// KFloater - QWidget graber
// This file is part of KDE project
// (c) Sven Radej 1997, sven.radej@iname.com


/** New top-level frames for old widgets. */
class KFloater : public QWidget
{
  Q_OBJECT
     
public:
  /** Constructor.
	*  This is all you need. KrollUp will reparent your widget to himself
    *  and size himself to the widget. He handles himself.
    *  Usage:
    *  QWidget *myWidget;
    *
    *  floater = new KFloater ("I'm floatin'!")
    *  connect (floater, SIGNAL(killed()), this SLOT(slotFloatKilled()));
    *  floater->grabWidget(myWidget);
    *  . . .
    *
    *  floater->releaseWidget (myWidget);
    *  delete floater; // he returns myWidget to it's true parent
    *
    *  You can delete floater without releaseWidget(). floater's destructor
    *  will return WmyWidget to it's true parent and to it's true position in
    *  that widget
    *
    *  If argument in releaseWidget is not the widget you grabbed, nothin happens
    */
    
  KFloater (const char *_title = 0, const char *name = 0);

  /** Grabs widget
    *  Reparents widget to itself, and sizes itself to the widget
    *  You can call this only once. Second call will do nothing
    */
   
  void grabWidget (QWidget *);

  /** Releases widget
    *  Returns widget to it's true parent, and to it's
    *  previous location in parent. Hides himself.
    *  Blocks signal killed ()
    */
  void releaseWidget (QWidget *);

  /** Destructor
    *  Returns widget to it's parent, to it's original
    *  location in parent. Will not emit signal killed ()
    */
   
  ~KFloater ();

  signals:

  /** Connect to this signal to know if KFloater gets killed
	*  by WM-buttons or if you delete KFloater without previous
	*  release
	*/
       
  void killed ();
    
protected:
  virtual void resizeEvent (QResizeEvent * );
  virtual void paintEvent (QPaintEvent *);
  virtual void closeEvent (QCloseEvent *);
   
private:
  const char *title;      // title
  bool opened;
  bool widgetIsMine;
   
  QWidget *element;
  QWidget *Parent;
  int oldX;
  int oldY;

  bool noSize;
  bool noPaint;

};
   
#endif
