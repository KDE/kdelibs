#ifndef BUILTIN_H
#define BUILTIN_H

#include "jstree.h"

void initBuiltin( JSScope *_scope );

class JSPrintFunction : public JSFunction
{
public:
    JSPrintFunction();
    ~JSPrintFunction() { }
    
    virtual int rightValue( JSScopeStack* _s, JSValue *rv, JSParameterListObject *_param );
};

#endif
