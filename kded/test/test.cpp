#include "test.h"

#if 0
class TestObject : public KShared
{
public:
   TestObject(const DCOPCString &_app) : app(_app)
     { qWarning("Creating TestObject belonging to '%s'", app.data()); }
   ~TestObject() 
     { qWarning("Destructing TestObject belonging to '%s'", app.data()); }
protected:
   DCOPCString app;
};
#endif

TestModule::TestModule(const DCOPCString &obj) : KDEDModule(obj)
{
#if 0
  // Do stuff here
  setIdleTimeout(15); // 15 seconds idle timeout.
#endif
}

QString TestModule::world()
{
  return "Hello World!";  
}

void TestModule::idle()
{
   qWarning("TestModule is idle.");
}

void TestModule::registerMe(const DCOPCString &app)
{
#if 0
   insert(app, "test", new TestObject(app));
   // When 'app' unregisters with DCOP, the TestObject will get deleted.
#endif
}

extern "C" { 
  KDE_EXPORT KDEDModule *create_test(const DCOPCString &obj)
  {
     return new TestModule(obj);
  }
};

#include "test.moc"
