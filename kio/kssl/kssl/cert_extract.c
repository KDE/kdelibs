/*
**  cert_extract.c -- Extract CA Certs out of Netscape certN.db files
**
**  Originally written and released under the GPL by 
**  Ariel Glenn from the AcIS R&D group at Columbia
**  as the two sources findoffset.c and dblist.c. See under
**  http://www.columbia.edu/~ariel/good-certs/ for more details.
**
**  Merged into one single program in August 1998 
**  by Ralf S. Engelschall for use in the mod_ssl project.
**  See under http://www.engelschall.com/sw/mod_ssl/ for more details.
**
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <ctype.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/time.h>

#include <db1/db.h>

#include "openssl/asn1.h"
#include "openssl/x509.h"

int findoffset(char *dbname);

int findoffset(char *dbname)
{
    DB *db;
    DBT dkey, dvalue;
    int result;
    int offset = 0;
    char *p;
    int ptag = 0, pclass, plen;
    X509 *mycert;

    if ((db = dbopen(dbname, O_RDONLY, 0, DB_HASH, NULL)) == NULL) {
        fprintf(stderr, "Failed to open DB file '%s': %s\n", dbname, strerror(errno));
        exit(1);
    }
    while ((result = (db->seq(db, &dkey, &dvalue, R_NEXT))) == 0) {
        if ((dvalue.size) > 520) {
            while (offset < dvalue.size) {
                p = (char *)dvalue.data + offset - 1;
                ASN1_get_object((unsigned char **)&p, (long *)&plen, &ptag, &pclass, dvalue.size);
                if (ptag == V_ASN1_SEQUENCE) {  /* ok, it might be a cert then. */
                    /* include length of object header junk */
                    plen += p - ((char *) dvalue.data + offset - 1);
                    mycert = NULL;
                    p = (char *) dvalue.data + offset - 1;
                    d2i_X509(&mycert, (unsigned char **) &p, plen);
                    if (mycert == NULL) {       /* must be garbage after all */
                        offset++;
                        continue;
                    }
                    break;
                }
                else
                    offset++;
            }
            if (offset > 0)
                break;          /* found it, let's quit */
        }
    }
    db->close(db);
    return (offset);
}

int main(int argc, char **argv)
{
    char *dbname;
    DB *db;
    int j;
    int offset;
    DBT dkey, dvalue;
    int result;
    char oname[40];
    int fout;
    int find;
    char *p;
    int ptag = 0, pclass, plen;
    X509 *mycert;
    char *shortname;
    char byte1, byte2;

    if (argc != 2) {
        fprintf(stderr, "usage: %s /path/to/netscape/cert.db\n", argv[0]);
        exit(1);
    }

    dbname = argv[1];
    offset = findoffset(dbname);
    if (offset == 0) {
        fprintf(stderr, "Couldn't determine cert offset in DB file '%s'\n", dbname);
        exit(1);
    }
    else {
        fprintf(stderr, "Ok: certificates are at offset %d\n", offset);
    }

    if ((db = dbopen(dbname, O_RDONLY, 0, DB_HASH, NULL)) == NULL) {
        fprintf(stderr, "Failed to open DB file '%s': %s\n", dbname, strerror(errno));
        exit(1);
    }
    if ((find = open("cert.index", O_WRONLY | O_CREAT | O_TRUNC, 0755)) == -1) {
        fprintf(stderr, "Failed to open Index file '%s': %s\n", "cert-index", strerror(errno));
        exit(1);
    }
    j = 0;
    byte1 = -1;
    byte2 = -1;
    while ((result = (db->seq(db, &dkey, &dvalue, R_NEXT))) == 0) {
        if (dvalue.size > offset && ((dvalue.size) - offset) > 500) {
            p = (char *)dvalue.data + offset - 1;
            if (byte1 != -1 && byte2 != -1)
                if (byte1 != p[0] || byte2 != p[1])
                    continue;
            ASN1_get_object((unsigned char **)&p, (long *)&plen, &ptag, &pclass, dvalue.size);
            if (ptag == V_ASN1_SEQUENCE) {      /* ok, it might be a cert then. */
                if (byte1 == -1 && byte2 == -1) {
                    byte1 = p[0];
                    byte2 = p[1];
                }
                /* include length of object header junk */
                plen += p - ((char *) dvalue.data + offset - 1);
                mycert = NULL;
                p = (char *) dvalue.data + offset - 1;
                d2i_X509(&mycert, (unsigned char **) &p, plen);
                if (mycert == NULL) {   /* must be garbage after all */
                    continue;
                }
                j++;
                sprintf(oname, "cert.%02d.der", j);
                if ((fout = open(oname, O_WRONLY | O_CREAT | O_TRUNC, 0755)) == -1) {
                    fprintf(stderr, "couldn't open %s\n", oname);
                    continue;
                }
                write(fout, (char *) dvalue.data + offset - 1, plen);
                close(fout);
                write(find, oname, strlen(oname));
                write(find, ": ", 2);
                shortname = (char *) dvalue.data + offset - 1 + plen;
                write(find, shortname, dvalue.size - plen - offset);
                write(find, "\n", 1);
                fprintf(stderr, "Extracted: %s (", oname);
                write(fileno(stderr), shortname, dvalue.size - plen - offset);
                fprintf(stderr, ")\n");
            }
            else {
                /* fprintf(stderr, "Hmmm... ptag is %d, plen is %d\n", ptag, plen); */
            }
        }
    }
    close(find);
    db->close(db);

    return (0);
}

