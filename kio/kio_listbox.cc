#include <kiconloader.h>
#include <kapp.h>

#include "kio_listbox.h"

#define SIZE_OPERATION        100
#define SIZE_LOCAL_FILENAME   160
#define SIZE_RESUME           40
#define SIZE_COUNT            60
#define SIZE_PROGRESS         100
#define SIZE_SPEED            70
#define SIZE_REMAINING_TIME   70
#define SIZE_ADDRESS          450

KIOListBox::KIOListBox (QWidget *parent, const char *name, 
			      int columns, WFlags f)
  : KTabListBox (parent, name, columns, f)
{
  setColumn(TB_OPERATION, i18n("Op."), SIZE_OPERATION);
  setColumn(TB_LOCAL_FILENAME, i18n("Local Filename"), SIZE_LOCAL_FILENAME);
  setColumn(TB_RESUME, i18n("Res."), SIZE_RESUME);
  setColumn(TB_COUNT, i18n("Count"), SIZE_COUNT);
  setColumn(TB_PROGRESS, i18n("Size"), SIZE_PROGRESS);
  setColumn(TB_SPEED, i18n("Speed"), SIZE_SPEED);
  setColumn(TB_REMAINING_TIME, i18n("Rem. Time"), SIZE_REMAINING_TIME);
  setColumn(TB_ADDRESS, i18n("Address ( URL )"), SIZE_ADDRESS);

  readConfig();

   if ( columnWidth( TB_OPERATION ) == 100 ) {
    setColumnWidth( TB_OPERATION, SIZE_OPERATION );
    setColumnWidth( TB_LOCAL_FILENAME, SIZE_LOCAL_FILENAME );
    setColumnWidth( TB_RESUME, SIZE_RESUME );
    setColumnWidth( TB_COUNT, SIZE_COUNT );
    setColumnWidth( TB_PROGRESS, SIZE_PROGRESS );
    setColumnWidth( TB_SPEED, SIZE_SPEED );
    setColumnWidth( TB_REMAINING_TIME, SIZE_REMAINING_TIME );
    setColumnWidth( TB_ADDRESS, SIZE_ADDRESS );
  }
}



KIOListBox::~KIOListBox()
{
  writeConfig();
}


#include "kio_listbox.moc"
