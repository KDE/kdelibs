#include <stdlib.h>

#include <qapp.h>
#include <qdialog.h>
#include <qpushbt.h>
#include <qlabel.h>
#include <qmsgbox.h>
#include <qdatetm.h>

#include "kdatepicktestdlg.h"
#include "kdatepicktestdlg.moc"

#include "kdatepik.h"

int getRand(int nLower, int nUpper);

CMainDlg::CMainDlg(QWidget *parent, const char *name)
       :QDialog(parent,name)
{
   initMetaObject();
   
   setCaption("KDatePicker Demonstration");
   setFixedSize(2*285,2*152);

   // buttons
   QPushButton *pb;

   pb = new QPushButton("Quit",this);
   pb->setGeometry(2*195,2*130,2*50,2*14);
   connect(pb, SIGNAL(clicked()), SLOT(quitapp()));

   pb = new QPushButton("About",this);
   pb->setGeometry(2*30,2*130,2*50,2*14);
   connect(pb,SIGNAL(clicked()),SLOT(about()));

   KDatePicker *dp;
   dp = new KDatePicker(this);
   dp->setGeometry(10,10,222,200);
   
   m_lab = new QLabel(this);
   m_lab->setGeometry(270, 110, 150, 28);
   
   connect(dp, SIGNAL(dateSelected(QDate)), SLOT(printDate(QDate)));
   
}

void CMainDlg::quitapp()
{
   qApp->quit();
}

void CMainDlg::about()
{
   QMessageBox mb;
   mb.setText("Send comments/suggestions/bugs to\nTim Gilman  <tdgilman@best.com>");
   mb.show();
}

void CMainDlg::printDate(QDate dt)
{
   m_lab->setText((const char*)dt.toString());
}
