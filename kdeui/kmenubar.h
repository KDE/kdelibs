//Revision 1.17  1999/03/06 18:03:34  ettrich
//the nifty "flat" feature of kmenubar/ktoolbar is now more visible:
//It has its own menu entry and reacts on simple LMP clicks.
#include <qmenubar.h>

class KMenuBar : public QMenuBar
{
    Q_OBJECT

public:
	KMenuBar( QWidget *parent=0, const char *name=0 );
    ~KMenuBar(); 
};
   bool highlight;
   bool transparent;
   bool buttonDownOnHandle;
 };

#endif
