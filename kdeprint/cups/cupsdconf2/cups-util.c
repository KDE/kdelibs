#include <cups/ipp.h>
#include <cups/http.h>
#include <cups/cups.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

#define CUPS_SERVERROOT	"/etc/cups"
static http_t		*cups_server;
static ipp_status_t	last_error;
static char		authstring[HTTP_MAX_VALUE];
static char		pwdstring[33];
static int cups_local_auth(http_t *http);

const char *				/* O - Filename for PPD file */
cupsGetConf()
{
  int		fd;			/* PPD file */
  int		bytes;			/* Number of bytes read */
  char		buffer[8192];		/* Buffer for file */
  char		resource[HTTP_MAX_URI];	/* Resource name */
  const char	*password;		/* Password string */
  char		realm[HTTP_MAX_VALUE],	/* realm="xyz" string */
		nonce[HTTP_MAX_VALUE],	/* nonce="xyz" string */
		plain[255],		/* Plaintext username:password */
		encode[512];		/* Encoded username:password */
  http_status_t	status;			/* HTTP status from server */
  char		prompt[1024];		/* Prompt string */
  int		digest_tries;		/* Number of tries with Digest */
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

  snprintf(resource, sizeof(resource), "/admin/conf/cupsd.conf");

  digest_tries = 0;

  do
  {
    httpClearFields(cups_server);
    httpSetField(cups_server, HTTP_FIELD_HOST, cupsServer());
    httpSetField(cups_server, HTTP_FIELD_AUTHORIZATION, authstring);

    if (httpGet(cups_server, resource))
    {
      if (httpReconnect(cups_server))
      {
        status = HTTP_ERROR;
	break;
      }
      else
      {
        status = HTTP_UNAUTHORIZED;
        continue;
      }
    }

    while ((status = httpUpdate(cups_server)) == HTTP_CONTINUE);

    if (status == HTTP_UNAUTHORIZED)
    {
      fprintf(stderr,"cupsGetConf: unauthorized...\n");

     /*
      * Flush any error message...
      */

      httpFlush(cups_server);

     /*
      * See if we can do local authentication...
      */

      if (cups_local_auth(cups_server))
        continue;

     /*
      * See if we should retry the current digest password...
      */

      if (strncmp(cups_server->fields[HTTP_FIELD_WWW_AUTHENTICATE], "Basic", 5) == 0 ||
          digest_tries > 1 || !pwdstring[0])
      {
       /*
	* Nope - get a password from the user...
	*/

	snprintf(prompt, sizeof(prompt), "Password for %s on %s? ", cupsUser(),
        	 cups_server->hostname);

        if ((password = cupsGetPassword(prompt)) == NULL)
	  break;
	if (!password[0])
	  break;

        strncpy(pwdstring, password, sizeof(pwdstring) - 1);
	pwdstring[sizeof(pwdstring) - 1] = '\0';

        digest_tries = 0;
      }
      else
        digest_tries ++;

     /*
      * Got a password; encode it for the server...
      */

      if (strncmp(cups_server->fields[HTTP_FIELD_WWW_AUTHENTICATE], "Basic", 5) == 0)
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

        httpGetSubField(cups_server, HTTP_FIELD_WWW_AUTHENTICATE, "realm", realm);
        httpGetSubField(cups_server, HTTP_FIELD_WWW_AUTHENTICATE, "nonce", nonce);

	httpMD5(cupsUser(), realm, pwdstring, encode);
	httpMD5Final(nonce, "GET", resource, encode);
	snprintf(authstring, sizeof(authstring),
	         "Digest username=\"%s\", realm=\"%s\", nonce=\"%s\", "
	         "response=\"%s\"", cupsUser(), realm, nonce, encode);
      }

      continue;
    }
#ifdef HAVE_LIBSSL
    else if (status == HTTP_UPGRADE_REQUIRED)
    {
     /*
      * Flush any error message...
      */

      httpFlush(cups_server);

     /*
      * Upgrade with encryption...
      */

      httpEncryption(cups_server, HTTP_ENCRYPT_REQUIRED);

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

  if (status != HTTP_OK)
  {
    close(fd);
    unlink(filename);
    httpFlush(cups_server);
    httpClose(cups_server);
    cups_server = NULL;
    return (NULL);
  }

 /*
  * OK, we need to copy the file...
  */

  while ((bytes = httpRead(cups_server, buffer, sizeof(buffer))) > 0)
  {
    write(fd, buffer, bytes);
  }

  close(fd);

  return (filename);
}

int					/* O - Status of operation */
cupsPutConf(const char *name)		/* I - Name of the config file to send */
{
  int		fd;			/* PPD file */
  int		bytes;			/* Number of bytes read */
  char		buffer[8192];		/* Buffer for file */
  char		resource[HTTP_MAX_URI];	/* Resource name */
  const char	*password;		/* Password string */
  char		realm[HTTP_MAX_VALUE],	/* realm="xyz" string */
		nonce[HTTP_MAX_VALUE],	/* nonce="xyz" string */
		plain[255],		/* Plaintext username:password */
		encode[512];		/* Encoded username:password */
  http_status_t	status;			/* HTTP status from server */
  char		prompt[1024];		/* Prompt string */
  int		digest_tries;		/* Number of tries with Digest */

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

  strncpy(resource, "/admin/conf/cupsd.conf", sizeof(resource));

  digest_tries = 0;

  do
  {
    httpClearFields(cups_server);
    httpSetField(cups_server, HTTP_FIELD_HOST, cupsServer());
    httpSetField(cups_server, HTTP_FIELD_AUTHORIZATION, authstring);
    httpSetField(cups_server, HTTP_FIELD_TRANSFER_ENCODING, "chunked");

    if (httpPut(cups_server, resource))
    {
      if (httpReconnect(cups_server))
      {
        status = HTTP_ERROR;
	break;
      }
      else
      {
        status = HTTP_UNAUTHORIZED;
        continue;
      }
    }

    /* send the file now */
    lseek(fd, 0, SEEK_SET);
    status = HTTP_CONTINUE;
    while ((bytes = read(fd, buffer, sizeof(buffer))) > 0)
        if (httpCheck(cups_server))
	{
	    if ((status = httpUpdate(cups_server)) != HTTP_CONTINUE)
	        break;
	}
	else
	    httpWrite(cups_server, buffer, bytes);

    if (status == HTTP_CONTINUE)
    {
        httpWrite(cups_server, buffer, 0);
	while ((status = httpUpdate(cups_server)) == HTTP_CONTINUE);
    }

    if (status == HTTP_UNAUTHORIZED)
    {
      fprintf(stderr,"cupsPutConf: unauthorized...");

     /*
      * Flush any error message...
      */

      httpFlush(cups_server);

     /*
      * See if we can do local authentication...
      */

      if (cups_local_auth(cups_server))
        continue;

     /*
      * See if we should retry the current digest password...
      */

      if (strncmp(cups_server->fields[HTTP_FIELD_WWW_AUTHENTICATE], "Basic", 5) == 0 ||
          digest_tries > 1 || !pwdstring[0])
      {
       /*
	* Nope - get a password from the user...
	*/

	snprintf(prompt, sizeof(prompt), "Password for %s on %s? ", cupsUser(),
        	 cups_server->hostname);

        if ((password = cupsGetPassword(prompt)) == NULL)
	  break;
	if (!password[0])
	  break;

        strncpy(pwdstring, password, sizeof(pwdstring) - 1);
	pwdstring[sizeof(pwdstring) - 1] = '\0';

        digest_tries = 0;
      }
      else
        digest_tries ++;

     /*
      * Got a password; encode it for the server...
      */

      if (strncmp(cups_server->fields[HTTP_FIELD_WWW_AUTHENTICATE], "Basic", 5) == 0)
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

        httpGetSubField(cups_server, HTTP_FIELD_WWW_AUTHENTICATE, "realm", realm);
        httpGetSubField(cups_server, HTTP_FIELD_WWW_AUTHENTICATE, "nonce", nonce);

	httpMD5(cupsUser(), realm, pwdstring, encode);
	httpMD5Final(nonce, "GET", resource, encode);
	snprintf(authstring, sizeof(authstring),
	         "Digest username=\"%s\", realm=\"%s\", nonce=\"%s\", "
	         "response=\"%s\"", cupsUser(), realm, nonce, encode);
      }

      continue;
    }
#ifdef HAVE_LIBSSL
    else if (status == HTTP_UPGRADE_REQUIRED)
    {
     /*
      * Flush any error message...
      */

      httpFlush(cups_server);

     /*
      * Upgrade with encryption...
      */

      httpEncryption(cups_server, HTTP_ENCRYPT_REQUIRED);

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

  if (status != HTTP_CREATED)
  {
    httpFlush(cups_server);
    httpClose(cups_server);
    cups_server = NULL;
    close(fd);
    return 0;
  }

  close(fd);

  return 1;
}

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
#if CUPS_VERSION_MAJOR == 1 && CUPS_VERSION_MINOR >= 2
    if (ntohl(*(int*)&http->_hostaddr.sin_addr) != 0x7f000001 &&
      strcasecmp(http->hostname, "localhost") != 0)
#else
    if (ntohl(*(int*)&http->hostaddr.sin_addr) != 0x7f000001 &&
      strcasecmp(http->hostname, "localhost") != 0)
#endif
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
