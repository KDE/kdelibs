
#include <kapp.h>
#include <knotifyclient.h>

int main(int argc, char **argv)
{
  KApplication app(argc, argv, "kmixclient");

  KNotifyClient::userEvent("Bleh", KNotifyClient::Messagebox);

}
