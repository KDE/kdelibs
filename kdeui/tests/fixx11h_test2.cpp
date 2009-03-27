// Test the case where Xdefs.h is first
#include <X11/Xdefs.h>

static Bool foo() { return 1; } // Xdefs doesn't define True!

#include <X11/Xlib.h>
#include <fixx11h.h>

int main(int, char**)
{
        Bool b = foo();
        return b;
}

