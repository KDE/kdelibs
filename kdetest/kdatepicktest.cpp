#include <qapp.h>
#include <qpixmap.h>
#include "kdatepicktestdlg.h"
#include "kdatepik.h"

int main(int argc, char **argv)
{
   QApplication a(argc, argv);
//   a.setStyle(WindowsStyle); // comment out for Motif
   
//   CMainDlg *w = new CMainDlg;
   KDatePicker *w = new KDatePicker;

   a.setMainWidget(w);
   w->resize(220,200);
   w->show();
   return a.exec();
}
   
