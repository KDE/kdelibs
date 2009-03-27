// From https://bugs.gentoo.org/show_bug.cgi?id=263823#c8
#include <X11/Xlib.h>
#include <fixx11h.h>

static Bool foo() { return True; }

#include <X11/Xdefs.h>

int main(int, char**)
{
        Bool b = foo();
        return b;
}

