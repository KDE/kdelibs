#ifndef _DCOPOBJECT_H
#define _DCOPOBJECT_H

#include <qobject.h>
#include <qmap.h>
#include <qstring.h>

class DCOPObject
{
 public:
  DCOPObject(QObject *obj);
  DCOPObject(const QCString &objId);
  virtual ~DCOPObject();

  QCString id() const;

  /**
   * dispatch a message.
   */
  virtual bool process(const QCString &fun, const QByteArray &data,
		       QByteArray &replyData);

  static bool hasObject(const QCString &objId);
  static DCOPObject *find(const QCString &objId);

 private:
  QCString ident;
};

#endif
