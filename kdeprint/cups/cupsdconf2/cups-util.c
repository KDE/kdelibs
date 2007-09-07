#include <cups/ipp.h>
#include <cups/http.h>
#include <cups/cups.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

#include "cups-util.h"

static ipp_status_t last_error;

const char* cupsGetConf(void);
int cupsPutConf(const char*);

const char *    /* O - Filename for PPD file */
cupsGetConf(void)
{
    int  fd;   /* PPD file */
    http_status_t status;   /* HTTP status from server */
    http_t *cups_server;
    static char filename[HTTP_MAX_URI]; /* Local filename */
    char          fqdn[ HTTP_MAX_URI ];   /* Server name buffer */


    /*
     * Connect to the correct server as needed...
     */

    cups_server = httpConnectEncrypt(cupsServer(), ippPort(), cupsEncryption());
    if (cups_server == NULL) {
        last_error = IPP_SERVICE_UNAVAILABLE;
        return (NULL);
    }

    /*
     * Get a temp file...
     */

    fd = cupsTempFd(filename, sizeof(filename));
    if (fd < 0) {
        /*
         * Can't open file; close the server connection and return NULL...
         */

        httpFlush(cups_server);
        httpClose(cups_server);
        cups_server = NULL;
        return (NULL);
    }

    /*
     * And send a request to the HTTP server...
     */
    status = cupsGetFd(cups_server, "/admin/conf/cupsd.conf", fd);

    close(fd);
    httpFlush(cups_server);
    httpClose(cups_server);
    cups_server = NULL;

    if (status != HTTP_OK) {
        unlink(filename);
        return (NULL);
    } else {
        return (filename);
    }
}

int     /* O - Status of operation */
cupsPutConf(const char *name)  /* I - Name of the config file to send */
{
    int  fd;   /* PPD file */
    http_status_t status;   /* HTTP status from server */
    http_t *cups_server;

    if (name == NULL)
        return 0;

    /*
     * Connect to the correct server as needed...
     */

    cups_server = httpConnectEncrypt(cupsServer(), ippPort(), cupsEncryption());
    if (cups_server == NULL) {
        last_error = IPP_SERVICE_UNAVAILABLE;
        return 0;
    }

    /*
     * Open the local config file...
     */

    fd = open(name, O_RDONLY);
    if (fd < 0) {
        /*
         * Can't open file; close the server connection and return NULL...
         */

        httpFlush(cups_server);
        httpClose(cups_server);
        cups_server = NULL;
        return 0;
    }

    /*
     * And send a request to the HTTP server...
     */

    status = cupsPutFd(cups_server, "/admin/conf/cupsd.conf", fd);

    httpFlush(cups_server);
    httpClose(cups_server);
    cups_server = NULL;
    close(fd);

    /*
     * See if we actually got the file or an error...
     */

    if (status != HTTP_CREATED)
        return 0;
    else
        return 1;
}
