#ifndef _DCOPOBJECT_H
#define _DCOPOBJECT_H

#include <qobject.h>
#include <qmap.h>
#include <qstring.h>

class DCOPObject
{
 public:
  DCOPObject(QObject *obj);
  DCOPObject(const QString &objId);
  virtual ~DCOPObject();

  QString id() const;

  /**
   * dispatch a message.
   */
  virtual bool process(const QString &fun, const QByteArray &data,
		       QByteArray &replyData);

  static bool hasObject(const QString &objId);
  static DCOPObject *find(const QString &objId);

 private:
  QString ident;
};

#endif
