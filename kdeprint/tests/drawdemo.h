#ifndef DRAWDEMO_H
#define DRAWDEMO_H

//
// DrawView has installable draw routines, just add a function pointer
// and a text in the table above.
//

#ifdef __GNUC__
#warning rename class or remove from global namespace
#endif
class DrawView : public QWidget
{
    Q_OBJECT
public:
    DrawView();
    ~DrawView();
public Q_SLOTS:
    void   updateIt( int );
    void   printIt();
protected:
    void   drawIt( QPainter * );
    void   paintEvent( QPaintEvent * );
    void   resizeEvent( QResizeEvent * );
private:
    KPrinter	 *printer;
	KButtonGroup *bgroup;
    QPushButton	 *print;
    int		  drawindex;
    int		  maxindex;
};

#endif
