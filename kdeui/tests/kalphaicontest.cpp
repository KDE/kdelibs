#include "kalphaicontest.h"
#include <kapp.h>
#include <kcmdlineargs.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <qpainter.h>
#include <qdatetime.h>

KAlphaTest::KAlphaTest() : QWidget(0L, "")
{

  m_image.load("go.png");
  m_pixmap= KGlobal::iconLoader()->loadIcon("go", KIcon::Desktop,48);
//  m_pixmap.setOptimization(QPixmap::BestOptim);
//  m_pixmap.convertFromImage(m_image);

  { // Initialize the background image 
    QImage bg("testimage.png");
    QPixmap bgPixmap(2*bg.width(), 2*bg.height());
    QPainter p(&bgPixmap);
    p.drawImage(0,0,bg);
    p.drawImage(bg.width(),0,bg);
    p.drawImage(0,bg.height(),bg);
    p.drawImage(bg.width(),bg.height(),bg);
    m_bg=bgPixmap.convertToImage();
  } 

  resize(m_bg.width(), m_bg.height());
}

void KAlphaTest::paintEvent(QPaintEvent *)
{
  int begin, end;
  QPixmap dblBuffer(width(), height());
  QPainter p;
  if (m_useDblBuffer) p.begin(&dblBuffer);
  else p.begin(this);

  QImage bgTemp(m_bg);
  bgTemp.detach();  // Or we will paint over bg !!

  // Draw the background
  p.drawImage(0,0,bgTemp);

  QTime time;
  time.start();

  if (!m_usePixmap) // use Images
  {
    begin=time.elapsed();
    for (int y=0; y< m_bg.height()-m_image.height(); y+=40) 
      for (int x=0; x< m_bg.width()-m_image.width(); x+=40)
	KAlphaPainter::draw(&p,m_image,bgTemp,x,y,m_correctOverlapping);
    end=time.elapsed();
  }
  else // use Pixmaps
  {  
    begin=time.elapsed();
    for (int y=0; y< m_bg.height()-m_image.height(); y+=40) 
      for (int x=0; x< m_bg.width()-m_image.width(); x+=40)
	KAlphaPainter::draw(&p,m_pixmap,bgTemp,x,y,m_correctOverlapping);
    end=time.elapsed();
  }

  kdDebug() << "Time : " << end-begin << " ms.\n";

  p.end();
  if (m_useDblBuffer)
  {
    p.begin(this);
    p.drawPixmap(0,0,dblBuffer);
    p.end();
  }
}

bool KAlphaTest::m_useDblBuffer=true;
bool KAlphaTest::m_usePixmap=true;
bool KAlphaTest::m_correctOverlapping=true;

static KCmdLineOptions options[] =
{
  { "images", "Use QImage objects instead of QPixmap.", 0 }, 
  { "dblBuffer", "Use double buffer to paint in window.", 0 }, 
  { "correctOverlapping", "Paint shadows in background as well as in the window.", 0 }, 
  { 0, 0, 0 } // End of options.
}; 

int main(int argc, char **argv)
{
  KCmdLineArgs::init(argc, argv, "kalphaicontest", "Test for KAlphaIcon", "1.0");
  KCmdLineArgs::addCmdLineOptions( options );

  KApplication app;

  KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

  QString currentUsage("Using ");
  if (args->isSet("images")) 
  {
    KAlphaTest::m_usePixmap=false;
    currentUsage+="QImage,";
  } 
  else 
  {
    KAlphaTest::m_usePixmap=true; 
    currentUsage+="QPixmap,";
  }

  if (args->isSet("dblBuffer")) 
  {
    KAlphaTest::m_useDblBuffer=false; 
    currentUsage+="not Double Buffer,";
  }
  else 
  {
    KAlphaTest::m_useDblBuffer=true;
    currentUsage+="Double Buffer,";
  }

  if (args->isSet("correctOverlapping")) 
  {
    KAlphaTest::m_correctOverlapping=false; 
    currentUsage+="not correct Overlapping.";
  }
  else 
  {
    KAlphaTest::m_correctOverlapping=true;
    currentUsage+="correct Overlapping.";
  }
  
  kdDebug() << currentUsage << "\n";

  args->clear();

  KAlphaTest w;
  app.setMainWidget(&w);
  w.show();

  return(app.exec());
};
