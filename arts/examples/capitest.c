#include <stdio.h>
#include "capi.h"

/* This is another artscat written with C Api */

/*
 * warning: the aRts C API is not yet finished and WILL CHANGE - this 
 * is here for experimental purposes ...
 */
int main()
{
	char buffer[1024];
	int bytes;

	if(arts_init() != 0)
	{
		printf("arts_init failed\n");
		exit(1);
	}
	if(arts_connect() != 0)
	{
		printf("arts_connect failed\n");
		exit(1);
	}
	while((bytes = fread(buffer,1,1024,stdin)) > 0)
		arts_write(buffer,bytes);
	/*
	if(arts_disconnect() != 0)
	{
		printf("arts_disconnect failed\n");
		exit(1);
	}
	if(arts_free() != 0)
	{
		printf("arts_free failed\n");
		exit(1);
	}
	*/
}
