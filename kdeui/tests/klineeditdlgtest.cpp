#include <kapplication.h>
#include <klineeditdlg.h>

#include <qstring.h>
#include <qtextview.h>

int main(int argc, char** argv)
{
  KApplication app(argc, argv, "klineedittest");
  KLineEditDlg dialog( "_text", "_value", 0L );
  if(dialog.exec())
    {
      qDebug("Accepted.");
    } else {
      qDebug("Rejected.");
    }
  return 0;
}

