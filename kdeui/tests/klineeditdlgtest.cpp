#include <kapp.h>
#include <klineeditdlg.h>

#include <qstring.h>
#include <qtextview.h>

int main(int argc, char** argv)
{
  KApplication app(argc, argv, "klineedittest");
  KLineEditDlg dialog( "_text", "_value", 0L, true);;
  if(dialog.exec())
    {
      debug("Accepted.");
    } else {
      debug("Rejected.");
    }
  return 0;
}

