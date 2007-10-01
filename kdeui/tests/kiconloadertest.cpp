#include <kiconloader.h>
#include <kaboutdata.h>
#include <kcmdlineargs.h>
#include <kapplication.h>
#include <kdebug.h>

#include <QtCore/QDate>
#include <QtGui/QPixmap>

#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
  KAboutData about("kiconloadertest", 0, ki18n("kiconloadertest"), "version");
  KCmdLineArgs::init(argc, argv, &about);

  KApplication app;

  KIconLoader * mpLoader = KIconLoader::global();
  KIconLoader::Context mContext = KIconLoader::Application;
  QTime dt;
  dt.start();
  int count = 0;
  for ( int mGroup = 0; mGroup < KIconLoader::LastGroup ; ++mGroup )
  {
      kDebug() << "queryIcons " << mGroup << "," << mContext;
      const QStringList filelist = mpLoader->queryIcons(mGroup, mContext);
      kDebug() << " -> found " << filelist.count() << " icons.";
      int i=0;
      for(QStringList::ConstIterator it = filelist.begin();
          it != filelist.end() /*&& i<10*/;
          ++it, ++i )
      {
          //kDebug() << ( i==9 ? "..." : (*it) );
          mpLoader->loadIcon( (*it), (KIconLoader::Group)mGroup );
          ++count;
      }
  }
  kDebug() << "Loading " << count << " icons took " << (float)(dt.elapsed()) / 1000 << " seconds";
}

