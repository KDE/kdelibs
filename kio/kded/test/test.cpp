#include "test.h"

TestModule::TestModule(const QCString &obj) : KDEDModule(obj)
{
  // Do stuff here
}

QString TestModule::world()
{
  return "Hello World!";  
}

extern "C" { 
  KDEDModule *create_test(const QCString &obj)
  {
     return new TestModule(obj);
  }
};

#include "test.moc"
