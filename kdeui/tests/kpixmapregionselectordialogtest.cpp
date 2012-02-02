#include "kpixmapregionselectordialog.h"
#include <QPixmap>
#include <QImage>
#include <kapplication.h>
#include <kcmdlineargs.h>
#include <iostream>

int main(int argc, char**argv)
{
  KCmdLineOptions options;
  options.add("+file", qi18n("The image file to open"));

  KCmdLineArgs::init(argc, argv, "test", 0, qi18n("test"), "1.0", qi18n("test"));
  KCmdLineArgs::addCmdLineOptions( options );
  KApplication app("test");

  KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
  if (args->count()!=1)
  {
    std::cout << "Usage: kpixmapregionselectordialogtest <imageFile>" << std::endl;
    return 1;
  }

  QImage image=
     KPixmapRegionSelectorDialog::getSelectedImage(QPixmap(args->arg(0)),100,100);

  image.save("output.png", "PNG");

  return 0;
}
