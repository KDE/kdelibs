#include <kdebug.h>
#include <qwidget.h>
#include <qimage.h>
#include <qpixmap.h>
#include <kimageeffect.h>
#include <kalphapainter.h>

class KAlphaTest : public QWidget
{
  public:
    KAlphaTest();
  protected:
    void paintEvent(QPaintEvent *);
  private:
    QImage m_image;
    QPixmap m_pixmap;
    QImage m_bg;

  public:
    static bool m_useDblBuffer;
    static bool m_usePixmap;
    static bool m_correctOverlapping;
};

