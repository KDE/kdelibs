#ifndef _KIO_ListBox_h
#define _KIO_ListBox_h

#include <ktablistbox.h>

// Field constants
#define TB_OPERATION       0
#define TB_LOCAL_FILENAME  1
#define TB_RESUME          2
#define TB_COUNT           3
#define TB_PROGRESS        4
#define TB_SPEED           6
#define TB_REMAINING_TIME  7
#define TB_ADDRESS         8


class KIOListBox : public KTabListBox
{
  Q_OBJECT

public:

  KIOListBox (QWidget *parent=0, const char *name=0, 
	      int columns=1, WFlags f=0);
  
  virtual ~KIOListBox();

};


#endif
