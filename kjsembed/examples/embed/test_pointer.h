#ifndef TEST_POINTER_H
#define TEST_POINTER_H
#include <kjsembed/static_binding.h>

struct Line
{
    enum Limits { START = 1, END = 10};
    Line() { offset = 0;}
    QString name;
    int offset;
};

KJS_BINDING( TestPointer )

#endif
