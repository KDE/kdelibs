#include "test.h"

class TestObject : public KShared
{
public:
   TestObject(const QCString &_app) : app(_app)
     { qWarning("Creating TestObject belonging to '%s'", app.data()); }
   ~TestObject() 
     { qWarning("Destructing TestObject belonging to '%s'", app.data()); }
protected:
   QCString app;
};

TestModule::TestModule(const QCString &obj) : KDEDModule(obj)
{
  // Do stuff here
  setIdleTimeout(15); // 15 seconds idle timeout.
}

QString TestModule::world()
{
  return "Hello World!";  
}

void TestModule::idle()
{
   qWarning("TestModule is idle.");
}

void TestModule::registerMe(const QCString &app)
{
   insert(app, "test", new TestObject(app));
   // When 'app' unregisters with DCOP, the TestObject will get deleted.
}

extern "C" { 
  KDE_EXPORT KDEDModule *create_test(const QCString &obj)
  {
     return new TestModule(obj);
  }
};

#include "test.moc"
