#include "kmessagebox.h"

#include <stdlib.h>

#include <kstatusbar.h>
#include <kapp.h>

#include <kglobal.h>




int main( int argc, char *argv[] )
{
    int i, test;
    KApplication *myApp = new KApplication( argc, argv, "KMessageBoxTest" );

for( test = 1; true; test++)
{
for( int count = 0; count < 5; count++)
{
switch(test) {
case 1:
    i = KMessageBox::questionYesNo(0, "1. Is this a question?");
    
    printf("%d. returned %d ", test, i);
    switch( i) {
    case KMessageBox::Ok : printf("(%s)\n", "Ok"); break;
    case KMessageBox::Cancel : printf("(%s)\n", "Cancel"); break;
    case KMessageBox::Yes : printf("(%s)\n", "Yes"); break;
    case KMessageBox::No : printf("(%s)\n", "No"); break;
    default: printf("(%s)\n", "ERROR!"); exit(1);
    }
    break;

case 2:
    i = KMessageBox::warningYesNo(0, "2. Is this a question?");
    
    printf("%d. returned %d ", test, i);
    switch( i) {
    case KMessageBox::Ok : printf("(%s)\n", "Ok"); break;
    case KMessageBox::Cancel : printf("(%s)\n", "Cancel"); break;
    case KMessageBox::Yes : printf("(%s)\n", "Yes"); break;
    case KMessageBox::No : printf("(%s)\n", "No"); break;
    default: printf("(%s)\n", "ERROR!"); exit(1);
    }
    break;
case 3:
    i = KMessageBox::warningYesNoCancel(0, "3. Is this a question?");
    
    printf("%d. returned %d ", test, i);
    switch( i) {
    case KMessageBox::Ok : printf("(%s)\n", "Ok"); break;
    case KMessageBox::Cancel : printf("(%s)\n", "Cancel"); break;
    case KMessageBox::Yes : printf("(%s)\n", "Yes"); break;
    case KMessageBox::No : printf("(%s)\n", "No"); break;
    default: printf("(%s)\n", "ERROR!"); exit(1);
    }
    break;

default:
    return 0;
}
}
}
}

