#include <kapp.h>
#include <kstddirs.h>

int main(int argc, char **argv)
{
  KApplication a(argc, argv);
  KStandardDirs t("test");
  t.addPrefix("/home/coolo/prod/KDE/");
  t.addPrefix("/home/coolo/.kde/");
  t.addResourceType("icon", "share/icons/");
  t.addResourceType("icon", "share/icons/mini/");
  t.addResourceDir("icon", "/home/coolo/.kde/share/myicons/");

  QString s = t.findResource("icon", "xv.xpm");
  debug(s);
}
