#include <qapplication.h>
#include <qstring.h>
#include <qcstring.h>

#include <qresolver.h>
#include <qsocketaddress.h>

ADDON_USE

class Test : public QObject
{
  Q_OBJECT

public:
  Test(QString host, QString service)
  {
    int flags = 0;
    int families = QResolver::AnyFamily;
    QResolver::resolveAsync(this, SLOT(results(QResolverResults)), host, service, flags, families);
  }

public slots:
  void results(QResolverResults r)
  {
    if (r.errorCode() == 0)
      {
	QResolverResults::const_iterator it = r.begin();
	while (it != r.end())
	  {
	    qDebug("%s | %s", (*it).address().nodeName().local8Bit().data(), (*it).address().serviceName().local8Bit().data());
	    ++it;
	  }
      }
    else
      qFatal("Could not resolve: %s", QResolver::strError(r.errorCode(), r.systemError()).local8Bit().data());

    QApplication::exit();
  }

};

int main(int argc, char **argv)
{
  QApplication a(argc, argv);

  if (argc < 2)
    return 1;

  Test test(QString::fromLocal8Bit(argv[1]), QString::fromLocal8Bit(argv[2]));

  return a.exec();
}

#include "resolution.moc"

