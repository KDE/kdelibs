#ifndef __kregexpeditorfactory_h
#define __kregexpeditorfactory_h
#include <klibloader.h>

class KRegExpEditorFactory :public KLibFactory
{
public:
  virtual KRegExpEditor* create( QWidget* parent, const char* name )
  {
  }
  
protected:
  KRegExpEditorFactory( QWidget* parent, const char* name )
    : KLibFactory( parent, name )
  {
  }
};


#endif // regexpeditorfactory_h
