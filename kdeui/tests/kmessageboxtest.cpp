#include "kmessagebox.h"

#include <stdlib.h>

#include <stdio.h>

#include <kstatusbar.h>
#include <kapplication.h>

#include <kglobal.h>

void showResult(int test, int i)
{
    printf("%d. returned %d ", test, i);
    switch( i) {
    case KMessageBox::Ok : printf("(%s)\n", "Ok"); break;
    case KMessageBox::Cancel : printf("(%s)\n", "Cancel"); break;
    case KMessageBox::Yes : printf("(%s)\n", "Yes"); break;
    case KMessageBox::No : printf("(%s)\n", "No"); break;
    case KMessageBox::Continue : printf("(%s)\n", "Continue"); break;
    default: printf("(%s)\n", "ERROR!"); exit(1);
    }
}


int main( int argc, char *argv[] )
{
    int i, test;
    new KApplication( argc, argv, "MyApplication" );

    QStringList list; list.append("Hello"); list.append("World");

    for( test = 6; true; test++)
    {
       for( int count = 0; count < 3; count++)
       {
         switch(test)
         {
case 1:
  i = KMessageBox::questionYesNo(0, "Do you have a printer?");
  break;

case 2:
  i = KMessageBox::questionYesNo(0, "Do you have a printer?",
				 QString("Printer setup"), QString::null);
  break;

case 3:
  i = KMessageBox::questionYesNo(0,
                "Does your printer support color or only black and white?",
                "Printer setup", "&Color", "&Black & White");
  break;

case 4:
  i = KMessageBox::warningYesNo(0, "Your printer has been added.\n"
		"Do you want to update your configuration?");
  break;


case 5:
  i = KMessageBox::warningYesNo(0, "Your printer has been added.\n"
		"Do you want to update your configuration?",
                "Printer Setup");
  break;

case 6:
  i = KMessageBox::warningContinueCancel(0, 
                "You are about tp Print.\n"
		"Are you sure?",
                "Print", "&Print");
  break;
case 7:
  i = KMessageBox::warningContinueCancel(0, 
                "You are about tp Print.\n"
		"Are you sure?",
                "Print", "&Print", "dontask", false);
  break;

case 8:
  i = KMessageBox::warningYesNoCancel(0,
                "Your document contains unsaved changes.\n"
                "Do you want to save your changes?\n");
  break;

case 9:
  i = KMessageBox::warningYesNoCancel(0,
                "Your document contains unsaved changes.\n"
                "Do you want to save your changes?\n",
                "Close");
  break;

case 10:
  i = KMessageBox::warningYesNoCancel(0,
                "Your document contains unsaved changes.\n"
                "Do you want to save or discard your changes?\n",
                "Close", "&Save", "&Discard" );
  break;

case 11:
  i = KMessageBox::Ok;
  KMessageBox::error(0, "Oops, Your harddisk is unreadable.");
  break;

case 12:
  i = KMessageBox::Ok;
  KMessageBox::error(0, "Oops, Your harddisk is unreadable." , "Uh ooh");
  break;

case 13:
  i = KMessageBox::Ok;
  KMessageBox::sorry(0, "Sorry, Your harddisk appears to be empty.");
  break;

case 14:
  i = KMessageBox::Ok;
  KMessageBox::sorry(0, "Sorry, Your harddisk appears to be empty.", "Oops");
  break;

case 15:
  i = KMessageBox::Ok;
  KMessageBox::information(0, "You can enable the menubar again\n"
            "with the right mouse button menu.");
  break;

case 16:
  i = KMessageBox::Ok;
  KMessageBox::information(0, "You can enable the menubar again\n"
            "with the right mouse button menu.", "Menubar Info");
  break;

case 17:
  i = KMessageBox::Ok;
  KMessageBox::information(0, "You can enable the menubar again\nwith the right mouse button menu.", QString::null, "Enable_Menubar");
  break;

case 18:
  i = KMessageBox::Ok;
  KMessageBox::enableAllMessages();
  break;

case 19:
  i = KMessageBox::Ok;
  KMessageBox::information(0, "Return of the annoying popup message.", QString::null, "Enable_Menubar");
  break;
case 20:
  {
  QStringList strlist;
  strlist << "/dev/hda" << "/etc/inittab" << "/usr/somefile" << "/some/really/"
   "long/file/name/which/is/in/a/really/deep/directory/in/a/really/large/"
   "hard/disk/of/your/system" << "/and/another/one" ;
  i = KMessageBox::questionYesNoList(0, "Do you want to delete the following files ?",strlist);
  }
  break;
case 21:
  {
  QStringList strlist;
  printf("Filling StringList...\n");  
  for (int j=1;j<=6000;j++) strlist.append(QString("/tmp/tmp.%1").arg(j));
  printf("Completed...\n");  
  i = KMessageBox::questionYesNoList(0, "Do you want to delete the following files ?",strlist);
  }
  break;

default:
  return 0;
         } // Switch

         showResult(test, i);
      } // Count
    } // Test
}

