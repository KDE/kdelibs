#include <kapp.h>
#include <config.h>
#include <stdio.h>

int main() {
    if (!strcmp(KDE_VERSION_STRING, VERSION))
        return 0;
    else {
        fprintf(stderr, "KDE_VERSION_STRING '%s' does not match VERSION '%s' !!!\n", KDE_VERSION_STRING, VERSION);
        return 1;
    }
}
