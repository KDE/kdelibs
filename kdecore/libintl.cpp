/* libintl.cpp -- gettext related functions from glibc-2.0.5
   Copyright (C) 1995 Software Foundation, Inc.

This file is part of the KDE libraries, but it's derived work out
of glibc. The master sources can be found in

      bindtextdom.c
      dcgettext.c
      dgettext.c
      explodename.c
      finddomain.c
      gettext.c
      gettext.h
      gettextP.h
      hash-string.h
      l10nflist.c
      libintl.h
      loadinfo.h
      loadmsgcat.c
      localealias.c
      textdomain.c

which are part of glibc. The license is the same as in GLIBC, which
is the GNU Library General Public License. See COPYING.LIB for more
details.

*/

/* gettext.c -- implementation of gettext(3) function
   Copyright (C) 1995 Software Foundation, Inc.

This file is part of the GNU C Library.  Its master source is NOT part of
the C library, however.  The master source lives in /gd/gnu/lib.

The GNU C Library is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

The GNU C Library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with the GNU C Library; see the file COPYING.LIB.  If
not, write to the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
Cambridge, MA 02110-1301, USA.  */

#include "kdelibs_export.h"
#include "kde_file.h"
#include <config.h>

#include <qglobal.h>

#include <stdlib.h>

#if defined HAVE_STRING_H
# include <string.h>
#else
# include <strings.h>
#endif

#include <sys/types.h>
#include <fcntl.h>
#include <sys/stat.h>

#if defined HAVE_UNISTD_H
# include <unistd.h>
#endif

#if (defined HAVE_MMAP && defined HAVE_MUNMAP)
# include <sys/mman.h>
#endif

#ifndef W
# define W(flag, data) ((flag) ? SWAP (data) : (data))
#endif

typedef quint32 nls_uint32;

struct loaded_domain
{
  const char *data;
#if (defined HAVE_MMAP && defined HAVE_MUNMAP && !defined DISALLOW_MMAP)
  int use_mmap;
  size_t mmap_size;
#endif
  int must_swap;
  nls_uint32 nstrings;
  struct string_desc *orig_tab;
  struct string_desc *trans_tab;
  nls_uint32 hash_size;
  nls_uint32 *hash_tab;
};

struct kde_loaded_l10nfile
{
  const char *filename;
  int decided;

  const void *data;

  kde_loaded_l10nfile() : filename(0), decided(0), data(0) {}
};

void k_nl_load_domain(struct kde_loaded_l10nfile *__domain);

static inline nls_uint32
SWAP (nls_uint32  i)
{
  return (i << 24) | ((i & 0xff00) << 8) | ((i >> 8) & 0xff00) | (i >> 24);
}

/* @@ end of prolog @@ */

/* The magic number of the GNU message catalog format.  */
#define _MAGIC 0x950412de
#define _MAGIC_SWAPPED 0xde120495

/* Revision number of the currently used .mo (binary) file format.  */
#define MO_REVISION_NUMBER 0


/* Defines the so called `hashpjw' function by P.J. Weinberger
   [see Aho/Sethi/Ullman, COMPILERS: Principles, Techniques and Tools,
   1986, 1987 Bell Telephone Laboratories, Inc.]  */
static inline unsigned long hash_string  (const char *__str_param);

/* @@ end of prolog @@ */

/* Header for binary .mo file format.  */
struct mo_file_header
{
  /* The magic number.  */
  nls_uint32 magic;
  /* The revision number of the file format.  */
  nls_uint32 revision;
  /* The number of strings pairs.  */
  nls_uint32 nstrings;
  /* Offset of table with start offsets of original strings.  */
  nls_uint32 orig_tab_offset;
  /* Offset of table with start offsets of translation strings.  */
  nls_uint32 trans_tab_offset;
  /* Size of hashing table.  */
  nls_uint32 hash_tab_size;
  /* Offset of first hashing entry.  */
  nls_uint32 hash_tab_offset;
};

struct string_desc
{
  /* Length of addressed string.  */
  nls_uint32 length;
  /* Offset of string in file.  */
  nls_uint32 offset;
};

/* Prototypes for local functions.  */
char *k_nl_find_msg  (struct kde_loaded_l10nfile *domain_file,
			const char *msgid);

char *
k_nl_find_msg (struct kde_loaded_l10nfile *domain_file, const char *msgid)
{
  size_t top, act, bottom;
  struct loaded_domain *domain;

  if (domain_file->decided == 0)
    k_nl_load_domain (domain_file);

  if (domain_file->data == NULL)
    return NULL;

  domain = (struct loaded_domain *) domain_file->data;

  /* Locate the MSGID and its translation.  */
  if (domain->hash_size > 2 && domain->hash_tab != NULL)
    {
      /* Use the hashing table.  */
      nls_uint32 len = strlen (msgid);
      nls_uint32 hash_val = hash_string (msgid);
      nls_uint32 idx = hash_val % domain->hash_size;
      nls_uint32 incr = 1 + (hash_val % (domain->hash_size - 2));
      nls_uint32 nstr = W (domain->must_swap, domain->hash_tab[idx]);

      if (nstr == 0)
	/* Hash table entry is empty.  */
	return NULL;

      if (W (domain->must_swap, domain->orig_tab[nstr - 1].length) == len
	  && strcmp (msgid,
		     domain->data + W (domain->must_swap,
				       domain->orig_tab[nstr - 1].offset)) == 0)
	return (char *) domain->data + W (domain->must_swap,
					  domain->trans_tab[nstr - 1].offset);

      while (1)
	{
	  if (idx >= domain->hash_size - incr)
	    idx -= domain->hash_size - incr;
	  else
	    idx += incr;

	  nstr = W (domain->must_swap, domain->hash_tab[idx]);
	  if (nstr == 0)
	    /* Hash table entry is empty.  */
	    return NULL;

	  if (W (domain->must_swap, domain->orig_tab[nstr - 1].length) == len
	      && strcmp (msgid,
			 domain->data + W (domain->must_swap,
					   domain->orig_tab[nstr - 1].offset))
	         == 0)
	    return (char *) domain->data
	      + W (domain->must_swap, domain->trans_tab[nstr - 1].offset);
	}
      /* NOTREACHED */
    }

  /* Now we try the default method:  binary search in the sorted
     array of messages.  */
  bottom = 0;
  top = domain->nstrings;
  act = top;
  while (bottom < top)
    {
      int cmp_val;

      act = (bottom + top) / 2;
      cmp_val = strcmp (msgid, domain->data
			       + W (domain->must_swap,
				    domain->orig_tab[act].offset));
      if (cmp_val < 0)
	top = act;
      else if (cmp_val > 0)
	bottom = act + 1;
      else
	break;
    }

  /* If an translation is found return this.  */
  return bottom >= top ? NULL : (char *) domain->data
                                + W (domain->must_swap,
				     domain->trans_tab[act].offset);
}

/* @@ begin of epilog @@ */
/* We assume to have `unsigned long int' value with at least 32 bits.  */
#define HASHWORDBITS 32

static inline unsigned long
hash_string (const char *str_param)
{
  unsigned long int hval, g;
  const char *str = str_param;

  /* Compute the hash value for the given string.  */
  hval = 0;
  while (*str != '\0')
    {
      hval <<= 4;
      hval += (unsigned long) *str++;
      g = hval & ((unsigned long) 0xf << (HASHWORDBITS - 4));
      if (g != 0)
	{
	  hval ^= g >> (HASHWORDBITS - 8);
	  hval ^= g;
	}
    }
  return hval;
}

/* Load the message catalogs specified by FILENAME.  If it is no valid
   message catalog do nothing.  */
void
k_nl_load_domain (struct kde_loaded_l10nfile *domain_file)
{
  int fd;
  struct stat st;
  struct mo_file_header *data = (struct mo_file_header *) -1;
#if (defined HAVE_MMAP && defined HAVE_MUNMAP && !defined DISALLOW_MMAP)
  int use_mmap = 0;
#endif
  struct loaded_domain *domain;

  domain_file->decided = 1;
  domain_file->data = NULL;

  /* If the record does not represent a valid locale the FILENAME
     might be NULL.  This can happen when according to the given
     specification the locale file name is different for XPG and CEN
     syntax.  */
  if (domain_file->filename == NULL)
    return;

  /* Try to open the addressed file.  */
  fd = KDE_open (domain_file->filename, O_RDONLY);
  if (fd == -1)
    return;

  /* We must know about the size of the file.  */
  if (fstat (fd, &st) != 0
      || st.st_size < (off_t) sizeof (struct mo_file_header))
    {
      /* Something went wrong.  */
      close (fd);
      return;
    }

#if (defined HAVE_MMAP && defined HAVE_MUNMAP && !defined DISALLOW_MMAP)
  /* Now we are ready to load the file.  If mmap() is available we try
     this first.  If not available or it failed we try to load it.  */
  data = (struct mo_file_header *) mmap (NULL, st.st_size, PROT_READ,
					 MAP_PRIVATE, fd, 0);

  if (data != (struct mo_file_header *) -1)
    {
      /* mmap() call was successful.  */
      close (fd);
      use_mmap = 1;
    }
#endif

  /* If the data is not yet available (i.e. mmap'ed) we try to load
     it manually.  */
  if (data == (struct mo_file_header *) -1)
    {
      off_t to_read;
      char *read_ptr;

      data = (struct mo_file_header *) malloc (st.st_size);
      if (data == NULL)
	return;

      to_read = st.st_size;
      read_ptr = (char *) data;
      do
	{
	  long int nb = (long int) read (fd, read_ptr, to_read);
	  if (nb == -1)
	    {
	      close (fd);
	      return;
	    }

	  read_ptr += nb;
	  to_read -= nb;
	}
      while (to_read > 0);

      close (fd);
    }

  /* Using the magic number we can test whether it really is a message
     catalog file.  */
  if (data->magic != _MAGIC && data->magic != _MAGIC_SWAPPED)
    {
      /* The magic number is wrong: not a message catalog file.  */
#if (defined HAVE_MMAP && defined HAVE_MUNMAP && !defined DISALLOW_MMAP)
      if (use_mmap)
	munmap ((char *) data, st.st_size);
      else
#endif
	free (data);
      return;
    }

  domain_file->data
    = (struct loaded_domain *) malloc (sizeof (struct loaded_domain));
  if (domain_file->data == NULL)
    return;

  domain = (struct loaded_domain *) domain_file->data;
  domain->data = (char *) data;
#if (defined HAVE_MMAP && defined HAVE_MUNMAP && !defined DISALLOW_MMAP)
  domain->use_mmap = use_mmap;
  domain->mmap_size = st.st_size;
#endif
  domain->must_swap = data->magic != _MAGIC;

  /* Fill in the information about the available tables.  */
  switch (W (domain->must_swap, data->revision))
    {
    case 0:
      domain->nstrings = W (domain->must_swap, data->nstrings);
      domain->orig_tab = (struct string_desc *)
	((char *) data + W (domain->must_swap, data->orig_tab_offset));
      domain->trans_tab = (struct string_desc *)
	((char *) data + W (domain->must_swap, data->trans_tab_offset));
      domain->hash_size = W (domain->must_swap, data->hash_tab_size);
      domain->hash_tab = (nls_uint32 *)
	((char *) data + W (domain->must_swap, data->hash_tab_offset));
      break;
    default:
      /* This is an illegal revision.  */
#if (defined HAVE_MMAP && defined HAVE_MUNMAP && !defined DISALLOW_MMAP)
      if (use_mmap)
	munmap ((char *) data, st.st_size);
      else
#endif
	free (data);
      free (domain);
      domain_file->data = NULL;
      return;
    }
}

void
k_nl_unload_domain (struct loaded_domain *domain)
{
#if (defined HAVE_MMAP && defined HAVE_MUNMAP && !defined DISALLOW_MMAP)
  if (domain->use_mmap)
    munmap ((caddr_t) domain->data, domain->mmap_size);
  else
# endif
    free ((void *) domain->data);

  free (domain);
}
