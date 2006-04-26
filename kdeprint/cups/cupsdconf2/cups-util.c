#include <cups/ipp.h>
#include <cups/http.h>
#include <cups/cups.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

#include "cups-util.h"

#define CUPS_SERVERROOT	"/etc/cups"

static ipp_status_t	last_error;

#if CUPS_VERSION_MAJOR == 1 && CUPS_VERSION_MINOR >= 2
#warning drop the code from else section if we drop support for CUPS pre v1.2
#else
static char		pwdstring[33];
static  int		digest_tries;		/* Number of tries with Digest */
static char		authstring[HTTP_MAX_VALUE];

static int			/* O - 1 if available, 0 if not */
cups_local_auth(http_t *http)	/* I - Connection */
{
	int		pid;		/* Current process ID */
	FILE		*fp;		/* Certificate file */
	char		filename[1024],	/* Certificate filename */
			certificate[33];/* Certificate string */
	const char	*root;		/* Server root directory */


	/*
	* See if we are accessing localhost...
	*/
	if (ntohl(*(int*)&http->hostaddr.sin_addr) != 0x7f000001 &&
		strcasecmp(http->hostname, "localhost") != 0)
		return (0);

	/*
	* Try opening a certificate file for this PID.  If that fails,
	* try the root certificate...
	*/

	if ((root = getenv("CUPS_SERVERROOT")) == NULL)
		root = CUPS_SERVERROOT;

	pid = getpid();
	snprintf(filename, sizeof(filename), "%s/certs/%d", root, pid);
	if ((fp = fopen(filename, "r")) == NULL && pid > 0)
	{
		snprintf(filename, sizeof(filename), "%s/certs/0", root);
		fp = fopen(filename, "r");
	}
	
	if (fp == NULL)
		return (0);

	/*
	* Read the certificate from the file...
	*/
	
	fgets(certificate, sizeof(certificate), fp);
	fclose(fp);

	/*
	* Set the authorization string and return...
	*/

	snprintf(authstring, sizeof(authstring), "Local %s", certificate);
	
	return (1);
}

int
cupsDoAuthentication(http_t *http,
        const char *method, const char *resource)
{
	const char	*password;		/* Password string */
	char		realm[HTTP_MAX_VALUE],	/* realm="xyz" string */
			nonce[HTTP_MAX_VALUE],	/* nonce="xyz" string */
			plain[255],		/* Plaintext username:password */
			encode[512];		/* Encoded username:password */
	char		prompt[1024];		/* Prompt string */

	/*
	* See if we can do local authentication...
	*/

	if (cups_local_auth(http))
		return 0;

	/*
	* See if we should retry the current digest password...
	*/

	if (strncmp(http->fields[HTTP_FIELD_WWW_AUTHENTICATE], "Basic", 5) == 0 ||
		digest_tries > 1 || !pwdstring[0])
	{
		/*
		* Nope - get a password from the user...
		*/
		
		snprintf(prompt, sizeof(prompt), "Password for %s on %s? ", cupsUser(),
			http->hostname);

		if ((password = cupsGetPassword(prompt)) == NULL)
			return -1;
		if (!password[0])
			return -1;

		strncpy(pwdstring, password, sizeof(pwdstring) - 1);
		pwdstring[sizeof(pwdstring) - 1] = '\0';

		digest_tries = 0;
	}
	else
		digest_tries ++;

     /*
      * Got a password; encode it for the server...
      */

      if (strncmp(http->fields[HTTP_FIELD_WWW_AUTHENTICATE], "Basic", 5) == 0)
      {
       /*
	* Basic authentication...
	*/

	snprintf(plain, sizeof(plain), "%s:%s", cupsUser(), pwdstring);
	httpEncode64(encode, plain);
	snprintf(authstring, sizeof(authstring), "Basic %s", encode);
      }
      else
      {
       /*
	* Digest authentication...
	*/

        httpGetSubField(http, HTTP_FIELD_WWW_AUTHENTICATE, "realm", realm);
        httpGetSubField(http, HTTP_FIELD_WWW_AUTHENTICATE, "nonce", nonce);

	httpMD5(cupsUser(), realm, pwdstring, encode);
	httpMD5Final(nonce, method, resource, encode);
	snprintf(authstring, sizeof(authstring),
	         "Digest username=\"%s\", realm=\"%s\", nonce=\"%s\", "
	         "response=\"%s\"", cupsUser(), realm, nonce, encode);
      }

      return 0;
}

http_status_t
cupsGetFd(http_t *http, const char *resource, int fd)
{
  int		bytes;			/* Number of bytes read */
  char		buffer[8192];		/* Buffer for file */
  http_status_t	status;			/* HTTP status from server */

  do
  {
    httpClearFields(http);
    httpSetField(http, HTTP_FIELD_HOST, cupsServer());
    httpSetField(http, HTTP_FIELD_AUTHORIZATION, authstring);

    if (httpGet(http, resource))
    {
      if (httpReconnect(http))
      {
        status = HTTP_ERROR;
        break;
      }
      status = HTTP_CONTINUE;
      continue;
    }

    while ((status = httpUpdate(http)) == HTTP_CONTINUE);

    if (status == HTTP_UNAUTHORIZED)
    {
      fprintf(stderr, "cupsGetConf: unauthorized...\n");

     /*
      * Flush any error message...
      */

      httpFlush(http);

     /*
      * See if we can do authentication...
      */

      if(cupsDoAuthentication(http, "GET", resource))
        break;

     /*
      * Try again, we passed authentification...
      */

      continue;
    }
#ifdef HAVE_LIBSSL
    else if (status == HTTP_UPGRADE_REQUIRED)
    {
     /*
      * Flush any error message...
      */

      httpFlush(http);

     /*
      * Upgrade with encryption...
      */

      httpEncryption(http, HTTP_ENCRYPT_REQUIRED);

     /*
      * Try again, this time with encryption enabled...
      */

      continue;
    }
#endif /* HAVE_LIBSSL */
  }
  while (status == HTTP_UNAUTHORIZED || status == HTTP_UPGRADE_REQUIRED);

 /*
  * See if we actually got the file or an error...
  */

  if (status == HTTP_OK)
  {
   /*
   * OK, we need to copy the file...
   */
    while ((bytes = httpRead(http, buffer, sizeof(buffer))) > 0)
    {
      write(fd, buffer, bytes);
    }
  }

  return status;
}

http_status_t
cupsPutFd(http_t *http, const char *resource, int fd)
{
  int		bytes;			/* Number of bytes read */
  char		buffer[8192];		/* Buffer for file */
  http_status_t	status;			/* HTTP status from server */

  do
  {
    httpClearFields(http);
    httpSetField(http, HTTP_FIELD_HOST, cupsServer());
    httpSetField(http, HTTP_FIELD_AUTHORIZATION, authstring);
    httpSetField(http, HTTP_FIELD_TRANSFER_ENCODING, "chunked");

    if (httpPut(http, resource))
    {
      if (httpReconnect(http))
      {
        status = HTTP_ERROR;
        break;
      }
      status = HTTP_CONTINUE;
      continue;
    }

    /* send the file now */
    lseek(fd, 0, SEEK_SET);
    status = HTTP_CONTINUE;
    while ((bytes = read(fd, buffer, sizeof(buffer))) > 0)
    {
        if (httpCheck(http))
	{
	    if ((status = httpUpdate(http)) != HTTP_CONTINUE)
	        break;
	}
	else
	    httpWrite(http, buffer, bytes);
    }

    if (status == HTTP_CONTINUE)
    {
        httpWrite(http, buffer, 0);
	while ((status = httpUpdate(http)) == HTTP_CONTINUE);
    }

    if (status == HTTP_UNAUTHORIZED)
    {
      fprintf(stderr, "cupsPutConf: unauthorized...");

     /*
      * Flush any error message...
      */

      httpFlush(http);

     /*
      * See if we can do authentication...
      */
#warning original code makes authentification for GET, typo?
      if(cupsDoAuthentication(http, "PUT", resource))
        break;

     /*
      * Try again, we passed authentification...
      */

      continue;
    }
#ifdef HAVE_LIBSSL
    else if (status == HTTP_UPGRADE_REQUIRED)
    {
     /*
      * Flush any error message...
      */

      httpFlush(http);

     /*
      * Upgrade with encryption...
      */

      httpEncryption(http, HTTP_ENCRYPT_REQUIRED);

     /*
      * Try again, this time with encryption enabled...
      */

      continue;
    }
#endif /* HAVE_LIBSSL */
  }
  while (status == HTTP_UNAUTHORIZED || status == HTTP_UPGRADE_REQUIRED);

  return status;
}

#endif /* CUPS_VERSION_MAJOR == 1 && CUPS_VERSION_MINOR >= 2 */

const char *				/* O - Filename for PPD file */
cupsGetConf(void)
{
  int		fd;			/* PPD file */
  http_status_t	status;			/* HTTP status from server */
  http_t	*cups_server;
  static char	filename[HTTP_MAX_URI];	/* Local filename */


 /*
  * Connect to the correct server as needed...
  */

  if ((cups_server = httpConnectEncrypt(cupsServer(), ippPort(),
                                          cupsEncryption())) == NULL)
  {
    last_error = IPP_SERVICE_UNAVAILABLE;
    return (NULL);
  }

 /*
  * Get a temp file...
  */

  if ((fd = cupsTempFd(filename, sizeof(filename))) < 0)
  {
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

  if (status != HTTP_OK)
  {
    unlink(filename);
    return (NULL);
  }
  else
  {
    return (filename);
  }
}

int					/* O - Status of operation */
cupsPutConf(const char *name)		/* I - Name of the config file to send */
{
  int		fd;			/* PPD file */
  http_status_t	status;			/* HTTP status from server */
  http_t	*cups_server;

  if (name == NULL)
    return 0;

 /*
  * Connect to the correct server as needed...
  */

  if ((cups_server = httpConnectEncrypt(cupsServer(), ippPort(),
                                          cupsEncryption())) == NULL)
  {
    last_error = IPP_SERVICE_UNAVAILABLE;
    return 0;
  }

 /*
  * Open the local config file...
  */

  if ((fd = open(name, O_RDONLY)) < 0)
  {
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
