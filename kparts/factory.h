#ifndef __kparts_factory_h__
#define __kparts_factory_h__

#include <klibloader.h>

class QWidget;

namespace KParts
{

class Part;

class Factory : public KLibFactory
{
  Q_OBJECT
public:
  Factory( QObject *parent = 0, const char *name = 0 );
  virtual ~Factory();

  virtual Part *createPart( QWidget *parentWidget = 0, const char *widgetName = 0, QObject *parent = 0, const char *name = 0, const char *classname = "KParts::Part", const QStringList &args = QStringList() ) = 0;

  virtual QObject *create( QObject *parent = 0, const char *name = 0, const char *classname = "QObject", const QStringList &args = QStringList() );

};

};

#endif
