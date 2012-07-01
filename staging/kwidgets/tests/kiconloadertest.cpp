#include <kiconloader.h>
#include <qapplication.h>

#include <QDebug>
#include <QtCore/QDate>
#include <QPixmap>

#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
  QApplication app(argc, argv);

  KIconLoader * mpLoader = KIconLoader::global();
  KIconLoader::Context mContext = KIconLoader::Application;
  QTime dt;
  dt.start();
  int count = 0;
  for ( int mGroup = 0; mGroup < KIconLoader::LastGroup ; ++mGroup )
  {
      qDebug() << "queryIcons " << mGroup << "," << mContext;
      const QStringList filelist = mpLoader->queryIcons(mGroup, mContext);
      qDebug() << " -> found " << filelist.count() << " icons.";
      int i=0;
      for(QStringList::ConstIterator it = filelist.begin();
          it != filelist.end() /*&& i<10*/;
          ++it, ++i )
      {
          //qDebug() << ( i==9 ? "..." : (*it) );
          mpLoader->loadIcon( (*it), (KIconLoader::Group)mGroup );
          ++count;
      }
  }
  qDebug() << "Loading " << count << " icons took " << (float)(dt.elapsed()) / 1000 << " seconds";
}

