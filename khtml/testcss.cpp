// programm to test the CSS implementation

#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include "css_stylesheetimpl.h"
#include "dom_string.h"

int main(int argc, char *argv[])
{
    char buf[40000];
    int fd; 

    if(argc == 2)
	fd = open(argv[1], O_RDONLY);
    else
	fd = open("/home/kde/test.css", O_RDONLY);
	
    if (fd < 0)
    {
        printf("Couldn't open file\n");
        return 0;
    }

    int len = read(fd, buf, 40000);

    DOM::DOMString str(buf);

    close(fd);

    DOM::CSSStyleSheetImpl *sheet = new DOM::CSSStyleSheetImpl((DOM::CSSStyleSheetImpl *)0);
    sheet->parseString( str );
    return 0;
}
