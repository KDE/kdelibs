#include <kiconloader.h>
#include <stdio.h>
#include <kapp.h>
#include <stdlib.h>
#include <kdebug.h>

int main(int argc, char *argv[])
{
  KApplication app(argc,argv,"kiconloadertest",false,false);

  KIconLoader * mpLoader = KGlobal::iconLoader();
  int mContext = KIcon::Application;
  for ( int mGroup = 0; mGroup < KIcon::LastGroup ; ++mGroup )
  {
      kdDebug() << "queryIcons " << mGroup << "," << mContext << endl;
      QStringList filelist=mpLoader->queryIcons(mGroup, mContext);
      kdDebug() << " -> found " << filelist.count() << " icons." << endl;
      int i=0;
      for(QStringList::Iterator it = filelist.begin();
          it != filelist.end() && i<10;
          ++it, ++i )
          kdDebug() << ( i==9 ? "..." : (*it) ) << endl;
  }
}

