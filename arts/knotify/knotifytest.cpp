#include <string>
#include <stdio.h>
#include <kapplication.h>
#include <knotifyclient.h>

int main(int argc, char **argv)
{
  KApplication app(argc, argv, "knotifytest");

//
  while (1) {
      char inp = getc(stdin);

      if ( inp=='q' || inp==27 ) break;
      if ( inp=='1' ) KNotifyClient::userEvent( "Foo", KNotifyClient::Sound, KNotifyClient::Default, "KDE_Window_DeIconify.wav" );
      if ( inp=='2' ) KNotifyClient::userEvent( "MessageBox Event", KNotifyClient::Messagebox );
      if ( inp=='3' ) KNotifyClient::userEvent( "Stderr Event", KNotifyClient::Stderr );
  }
}
