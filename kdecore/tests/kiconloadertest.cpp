#include <kiconloader.h>
#include <qdatetime.h>
#include <stdio.h>
#include <kapp.h>
#include <stdlib.h>
#include <kdebug.h>

int main(int argc, char *argv[])
{
  KApplication app(argc,argv,"kiconloadertest"/*,false,false*/);

  KIconLoader * mpLoader = KGlobal::iconLoader();
  int mContext = KIcon::Application;
  QTime dt;
  dt.start();
  int count = 0;
  for ( int mGroup = 0; mGroup < KIcon::LastGroup ; ++mGroup )
  {
      kdDebug() << "queryIcons " << mGroup << "," << mContext << endl;
      QStringList filelist=mpLoader->queryIcons(mGroup, mContext);
      kdDebug() << " -> found " << filelist.count() << " icons." << endl;
      int i=0;
      for(QStringList::Iterator it = filelist.begin();
          it != filelist.end() /*&& i<10*/;
          ++it, ++i )
      {
          //kdDebug() << ( i==9 ? "..." : (*it) ) << endl;
          mpLoader->loadIcon( (*it), mGroup );
	  ++count;
      }
  }
  kdDebug() << "Loading " << count << " icons took " << (float)(dt.elapsed()) / 1000 << " seconds" << endl;
}

