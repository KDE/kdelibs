#include <qapplication.h>
#include <qthread.h>

#include <qsynchronizing.cpp>

void voidFunction()
{
  qDebug("voidFunction");
}

void intFunction(int val)
{
  qDebug("intFunction(%d)", val);
}

class TestObject
{
public:
  mutable int * refcount;
  TestObject()
    : refcount(new int)
  {
    *refcount = 1;
    qDebug("TestObject::TestObject(), this = %p, refcount = %p (%d)",
	   (void*)this, (void*)refcount, *refcount);
  }

  TestObject(const TestObject& other)
    : refcount(other.refcount)
  {
    (*refcount)++;
    qDebug("TestObject::TestObject(const TestObject&)(@%p), this = %p, refcount = %p (%d)",
	   (void*)&other, (void*)this, (void*)refcount, *refcount);
  }

  ~TestObject()
  {
    (*refcount)--;
    qDebug("TestObject::~TestObject(), this = %p, refcount = %p (%d)",
	   (void*)this, (void*)refcount, *refcount);
    if (*refcount == 0)
      delete refcount;
  }

  TestObject& operator=(const TestObject& other)
  {
    qDebug("TestObject::operator==(const TestObject&)(@%p), this = %p, this->refcount = %p (%d), "
	   "other.refcount = %p (%d)",
	   (void*)&other, (void*)this, (void*)refcount, *refcount, (void*)other.refcount, *other.refcount);
    (*other.refcount)++;
    (*this->refcount)--;
    if (*this->refcount == 0)
      delete refcount;
    refcount = other.refcount;

    return *this;
  }

  void member()
  {
    qDebug("TestObject::member(void), this = %p", (void*)this);
  }

  void member(double val)
  {
    qDebug("TestObject::member(double)(%g), this = %p", val, (void*)this);
  }
};

class Test : public QObject, public QThread, virtual public QSynchronizing
{
  Q_OBJECT

  TestObject obj;

public:
  Test()
  {
    QObject::connect(this, SIGNAL(intSignal(int)), this, SLOT(intSlot(int)));
    QObject::connect(this, SIGNAL(intSignal(int)), this, SLOT(voidSlot()));
  }

  void byVal(TestObject obj)
  {
    qDebug("Test::byVal(TestObject)({refcount = %p (%d)}), this = %p",
	   (void*)obj.refcount, *obj.refcount, (void*)this);
  }

  void byRef(TestObject& obj)
  {
    qDebug("Test::byRef(TestObject&)(@%p {refcount = %p (%d)}), this = %p",
	   (void*)&obj, (void*)obj.refcount, *obj.refcount, (void*)this);
  }

  void byPtr(TestObject* obj)
  {
    qDebug("Test::byPtr(TestObject*)(%p {refcount = %p (%d)}), this = %p",
	   (void*)obj, (void*)obj->refcount, *obj->refcount, (void*)this);
  }
	   
  void run()
  {
    qDebug("Calling voidFunction");
    synchronize(voidFunction);

    qDebug("Calling intFunction(1)");
    synchronize(intFunction, 1);

    qDebug("Calling obj->voidMember, obj = %p", (void*)&obj);
    synchronize(obj, &TestObject::member);

    qDebug("Calling obj->doubleMember(42.0), obj = %p", (void*)&obj);
    synchronize(obj, &TestObject::member, (double)42);

    qDebug("Calling slot voidSlot, this = %p", (void*)this);
    synchronize(this, &Test::voidSlot);

    qDebug("Calling slot intSlot(2), this = %p", (void*)this);
    synchronize(this, &Test::intSlot, 2);

    qDebug("Emitting signal intSignal(3), this = %p", (void*)this);
    emit synchronize(this, &Test::intSignal, 3);

    qDebug("Calling byVal({refcount = %p (%d)}), this = %p",
	   (void*)obj.refcount, *obj.refcount, (void*)this);
    synchronize(this, &Test::byVal, obj);

    qDebug("Calling byRef(@%p {refcount = %p (%d)}), this = %p",
	   (void*)&obj, (void*)obj.refcount, *obj.refcount, (void*)this);
    synchronize(this, &Test::byRef, obj);

    qDebug("Calling byPtr(%p {refcount = %p (%d)}), this = %p",
	   (void*)&obj, (void*)obj.refcount, *obj.refcount, (void*)this);
    synchronize(this, &Test::byPtr, &obj);
  }


public slots:
  void intSlot(int val)
  {
    qDebug("Test::intSlot(%d)", val);
  }

  void voidSlot()
  {
    qDebug("Test::voidSlot");
  }

  void complexSlot(const Test& param)
  {
    qDebug("Test::complexSlot(@%p), this = %p",
	   (void*)&param, (void*)this);
  }

signals:
  void intSignal(int);

};

int main(int argc, char **argv)
{
  QApplication a(argc, argv);

  if (argc < 2)
    return 1;

  Test test;
  test.start();
  return a.exec();
}

#include "synchro.moc"
