#include "kpixmapregionselectordialog.h"
#include <QPixmap>
#include <QImage>
#include <QFile>
#include <QApplication>
#include <iostream>

int main(int argc, char**argv)
{
  QApplication app(argc, argv);

  if (argc <= 1) {
    std::cerr << "Usage: kpixmapregionselectordialogtest <imageFile>" << std::endl;
    return 1;
  }

  QPixmap pix(QFile::decodeName(argv[1]));
  QImage image = KPixmapRegionSelectorDialog::getSelectedImage(pix, 100, 100);
  image.save("output.png", "PNG");

  return 0;
}
