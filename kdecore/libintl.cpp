/* Combined sources of GNU gettext library
   Copyright (C) 1995 Software Foundation, Inc.

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2, or (at your option)
any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.  */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

/* Stephan Kulow: some defaults */
#ifndef GNULOCALEDIR
#define GNULOCALEDIR "/usr/share/locale/"
#endif

#ifndef LOCALE_ALIAS_PATH
#define LOCALE_ALIAS_PATH "/usr/share/local"
#endif

/* The following is from pathmax.h.  */
/* Non-POSIX BSD systems might have gcc's limits.h, which doesn't define
   PATH_MAX but might cause redefinition warnings when sys/param.h is
   later included (as on MORE/BSD 4.3).  */
#if defined(_POSIX_VERSION) || (defined(HAVE_LIMITS_H) && !defined(__GNUC__))
# include <limits.h>
#endif

#include <stdlib.h>
#include <string.h>

#ifdef HAVE_STRINGS_H
#include <strings.h>
#endif

#include <errno.h>

#include <sys/param.h>
#ifdef HAVE_ALLOCA_H
#include <alloca.h>
#endif

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#ifdef HAVE_LOCALE_H
# include <locale.h>
#endif

#ifdef HAVE_UNISTD_H
# include <unistd.h>
#endif

#ifdef HAVE_MMAP
# include <sys/mman.h>
#endif

#include <ctype.h>

/* Amount to increase buffer size by in each try.  */
#define PATH_INCR 32

#ifndef _POSIX_PATH_MAX
# define _POSIX_PATH_MAX 255
#endif

#if !defined(PATH_MAX) && defined(_PC_PATH_MAX)
# define PATH_MAX (pathconf ("/", _PC_PATH_MAX) < 1 ? 1024 : pathconf ("/", _PC_PATH_MAX))
#endif

/* Don't include sys/param.h if it already has been.  */
#if defined(HAVE_SYS_PARAM_H) && !defined(PATH_MAX) && !defined(MAXPATHLEN)
# include <sys/param.h>
#endif

#if defined(HAVE_ARGZ_H)
#ifdef __BEGIN_DECLS
#undef __BEGIN_DECLS
#undef __END_DECLS
#define __BEGIN_DECLS
#define __END_DECLS
#define _ARGZ_H_WORKAROUND_ 
extern "C" {
#endif
# include <argz.h>
#ifdef _ARGZ_H_WORKAROUND_
}
#endif
#undef __BEGIN_DECLS
#undef __END_DECLS
#ifdef HAVE_SYS_CDEFS_H
#include <sys/cdefs.h>
#endif
#endif

#if !defined(PATH_MAX) && defined(MAXPATHLEN)
# define PATH_MAX MAXPATHLEN
#endif

#ifndef PATH_MAX
# define PATH_MAX _POSIX_PATH_MAX
#endif

/* XPG3 defines the result of `setlocale (category, NULL)' as:
   ``Directs `setlocale()' to query `category' and return the current
     setting of `local'.''
   However it does not specify the exact format.  And even worse: POSIX
   defines this not at all.  So we can use this feature only on selected
   system (e.g. those using GNU C Library).  */
#ifdef _LIBC
# define HAVE_LOCALE_NULL
#endif

/* Encoding of locale name parts.  */
#define CEN_REVISION		1
#define CEN_SPONSOR		2
#define CEN_SPECIAL		4
#define XPG_NORM_CODESET	8
#define XPG_CODESET		16
#define TERRITORY		32
#define CEN_AUDIENCE		64
#define XPG_MODIFIER		128

#define CEN_SPECIFIC	(CEN_REVISION|CEN_SPONSOR|CEN_SPECIAL|CEN_AUDIENCE)
#define XPG_SPECIFIC	(XPG_CODESET|XPG_NORM_CODESET|XPG_MODIFIER)


struct loaded_l10nfile
{
  const char *filename;
  int decided;

  const void *data;

  struct loaded_l10nfile *next;
  struct loaded_l10nfile *successor[1];
};


static const char *_nl_normalize_codeset (const char *codeset,
				   size_t name_len);

static struct loaded_l10nfile *
_nl_make_l10nflist (struct loaded_l10nfile **l10nfile_list,
		    const char *dirlist, size_t dirlist_len, int mask,
		    const char *language, const char *territory,
		    const char *codeset,
		    const char *normalized_codeset,
		    const char *modifier, const char *special,
		    const char *sponsor, const char *revision,
		    const char *filename, int do_allocate);


static const char *_nl_expand_alias (const char *name);

static int _nl_explode_name (char *name, const char **language,
		      const char **modifier,
		      const char **territory,
		      const char **codeset,
		      const char **normalized_codeset,
		      const char **special,
		      const char **sponsor,
		      const char **revision);


#if !HAVE_LC_MESSAGES
/* This value determines the behaviour of the gettext() and dgettext()
   function.  But some system does not have this defined.  Define it
   to a default value.  */
# define LC_MESSAGES (-1)
#endif


/* Declarations for gettext-using-catgets interface.  Derived from
   Jim Meyering's libintl.h.  */
struct _msg_ent
{
  const char *_msg;
  int _msg_number;
};

/* For automatical extraction of messages sometimes no real
   translation is needed.  Instead the string itself is the result.  */
#define gettext_noop(Str) (Str)


/* The magic number of the GNU message catalog format.  */
#define _MAGIC 0x950412de
#define _MAGIC_SWAPPED 0xde120495

/* Revision number of the currently used .mo (binary) file format.  */
#define MO_REVISION_NUMBER 0

/* The following contortions are an attempt to use the C preprocessor
   to determine an unsigned integral type that is 32 bits wide.  An
   alternative approach is to use autoconf's AC_CHECK_SIZEOF macro, but
   doing that would require that the configure script compile and *run*
   the resulting executable.  Locally running cross-compiled executables
   is usually not possible.  */

#define UINT_MAX_32_BITS 4294967295U

/* If UINT_MAX isn't defined, assume it's a 32-bit type.
   This should be valid for all systems GNU cares about because
   that doesn't include 16-bit systems, and only modern systems
   (that certainly have <limits.h>) have 64+-bit integral types.  */

#ifndef UINT_MAX
# define UINT_MAX UINT_MAX_32_BITS
#endif

#if UINT_MAX == UINT_MAX_32_BITS
typedef unsigned nls_uint32;
#else
# if USHRT_MAX == UINT_MAX_32_BITS
typedef unsigned short nls_uint32;
# else
#  if ULONG_MAX == UINT_MAX_32_BITS
typedef unsigned long nls_uint32;
#  else
  /* The following line is intended to throw an error.  Using #error is
     not portable enough.  */
  "Cannot determine unsigned 32-bit data type."
#  endif
# endif
#endif


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

#ifndef W
# define W(flag, data) ((flag) ? SWAP (data) : (data))
#endif


static inline nls_uint32
SWAP (nls_uint32 i)
{
  return (i << 24) | ((i & 0xff00) << 8) | ((i >> 8) & 0xff00) | (i >> 24);
}

struct loaded_domain
{
  const char *data;
  int must_swap;
  nls_uint32 nstrings;
  struct string_desc *orig_tab;
  struct string_desc *trans_tab;
  nls_uint32 hash_size;
  nls_uint32 *hash_tab;
};

struct binding
{
  struct binding *next;
  char *domainname;
  char *dirname;
};

/* We need a sign, whether a new catalog was loaded, which can be associated
   with all translations.  This is important if the translations are
   cached by one of GCC's features.  */
int _nl_msg_cat_cntr;

/* Load the message catalogs specified by FILENAME.  If it is no valid
   message catalog do nothing.  */
static void
_nl_load_domain (struct loaded_l10nfile *domain_file)
{
  int fd;
  struct stat st;
  struct mo_file_header *data = (struct mo_file_header *) -1;
#if (defined HAVE_MMAP && defined HAVE_MUNMAP && !defined DISALLOW_MMAP)
  int use_mmap = 0;
#endif
  struct loaded_domain *domain;

  domain_file->decided = 1;
  domain_file->data = 0L;

  /* If the record does not represent a valid locale the FILENAME
     might be NULL.  This can happen when according to the given
     specification the locale file name is different for XPG and CEN
     syntax.  */
  if (domain_file->filename == 0L)
    return;

  /* Try to open the addressed file.  */
  fd = open (domain_file->filename, O_RDONLY);
  if (fd == -1)
    return;

  /* We must know about the size of the file.  */
  if (fstat (fd, &st) != 0
      && st.st_size < (off_t) sizeof (struct mo_file_header))
    {
      /* Something went wrong.  */
      close (fd);
      return;
    }

#if (defined HAVE_MMAP && defined HAVE_MUNMAP && !defined DISALLOW_MMAP)
  /* Now we are ready to load the file.  If mmap() is available we try
     this first.  If not available or it failed we try to load it.  */
  data = (struct mo_file_header *) mmap (0L, st.st_size, PROT_READ,
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
      if (data == 0L)
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
	munmap ((caddr_t) data, st.st_size);
      else
#endif
	free (data);
      return;
    }

  domain_file->data
    = (struct loaded_domain *) malloc (sizeof (struct loaded_domain));
  if (domain_file->data == 0L)
    return;

  domain = (struct loaded_domain *) domain_file->data;
  domain->data = (char *) data;
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
	munmap ((caddr_t) data, st.st_size);
      else
#endif
	free (data);
      free (domain);
      domain_file->data = 0L;
      return;
    }

  /* Show that one domain is changed.  This might make some cached
     translations invalid.  */
  ++_nl_msg_cat_cntr;
}

/* List of already loaded domains.  */
static struct loaded_l10nfile *_nl_loaded_domains;

/* Return a data structure describing the message catalog described by
   the DOMAINNAME and CATEGORY parameters with respect to the currently
   established bindings.  */
static struct loaded_l10nfile *
k_nl_find_domain (const char *dirname, char *locale, const char *domainname)
{
  struct loaded_l10nfile *retval;
  const char *language;
  const char *modifier;
  const char *territory;
  const char *codeset;
  const char *normalized_codeset;
  const char *special;
  const char *sponsor;
  const char *revision;
  const char *alias_value;
  int mask;

  /* LOCALE can consist of up to four recognized parts for the XPG syntax:

		language[_territory[.codeset]][@modifier]

     and six parts for the CEN syntax:

	language[_territory][+audience][+special][,sponsor][_revision]

     Beside the first all of them are allowed to be missing.  If the
     full specified locale is not found, the less specific one are
     looked for.  The various part will be stripped of according to
     the following order:
		(1) revision
		(2) sponsor
		(3) special
		(4) codeset
		(5) normalized codeset
		(6) territory
		(7) audience/modifier
   */

  /* If we have already tested for this locale entry there has to
     be one data set in the list of loaded domains.  */
  retval = _nl_make_l10nflist (&_nl_loaded_domains, dirname,
			       strlen (dirname) + 1, 0, locale, 0L, 0L,
			       0L, 0L, 0L, 0L, 0L, domainname, 0);
  if (retval != 0L)
    {
      /* We know something about this locale.  */
      int cnt;

      if (retval->decided == 0)
	_nl_load_domain (retval);

      if (retval->data != 0L)
	return retval;

      for (cnt = 0; retval->successor[cnt] != 0L; ++cnt)
	{
	  if (retval->successor[cnt]->decided == 0)
	    _nl_load_domain (retval->successor[cnt]);

	  if (retval->successor[cnt]->data != 0L)
	    break;
	}
      return cnt >= 0 ? retval : 0L;
      /* NOTREACHED */
    }

  /* See whether the locale value is an alias.  If yes its value
     *overwrites* the alias name.  No test for the original value is
     done.  */
  alias_value = _nl_expand_alias (locale);
  if (alias_value != 0L)
    {
      size_t len = strlen (alias_value) + 1;
      locale = (char *) malloc (len);
      if (locale == 0L)
	return 0L;

      memcpy (locale, alias_value, len);
    }

  /* Now we determine the single parts of the locale name.  First
     look for the language.  Termination symbols are `_' and `@' if
     we use XPG4 style, and `_', `+', and `,' if we use CEN syntax.  */
  mask = _nl_explode_name (locale, &language, &modifier, &territory,
			   &codeset, &normalized_codeset, &special,
			   &sponsor, &revision);

  /* Create all possible locale entries which might be interested in
     generalzation.  */
  retval = _nl_make_l10nflist (&_nl_loaded_domains, dirname,
			       strlen (dirname) + 1, mask, language, territory,
			       codeset, normalized_codeset, modifier, special,
			       sponsor, revision, domainname, 1);
  if (retval == 0L)
    /* This means we are out of core.  */
    return 0L;

  if (retval->decided == 0)
    _nl_load_domain (retval);
  if (retval->data == 0L)
    {
      int cnt;
      for (cnt = 0; retval->successor[cnt] != 0L; ++cnt)
	{
	  if (retval->successor[cnt]->decided == 0)
	    _nl_load_domain (retval->successor[cnt]);
	  if (retval->successor[cnt]->data != 0L)
	    break;
	}
    }

  /* The room for an alias was dynamically allocated.  Free it now.  */
  if (alias_value != 0L)
    free (locale);

  return retval;
}

static void _nl_load_domain (struct loaded_l10nfile *__domain);

//#line 22 "hash-string.h"

/* We assume to have `unsigned long int' value with at least 32 bits.  */
#define HASHWORDBITS 32


/* Defines the so called `hashpjw' function by P.J. Weinberger
   [see Aho/Sethi/Ullman, COMPILERS: Principles, Techniques and Tools,
   1986, 1987 Bell Telephone Laboratories, Inc.]  */

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

#ifndef HAVE_STPCPY
static char *
stpcpy (char *dest, const char *src)
{
  while ((*dest++ = *src++) != '\0')
    /* Do nothing. */ ;
  return dest - 1;
}
#endif


/* Name of the default domain used for gettext(3) prior any call to
   textdomain(3).  The default value for this is "messages".  */
const char _nl_default_default_domain[] = "messages";

/* Value used as the default domain for gettext(3).  */
const char *_nl_current_default_domain = _nl_default_default_domain;

/* Contains the default location of the message catalogs.  */
const char _nl_default_dirname[] = GNULOCALEDIR;

/* List with bindings of specific domains created by bindtextdomain()
   calls.  */
struct binding *_nl_domain_bindings;

/* Specify that the DOMAINNAME message catalog will be found
   in DIRNAME rather than in the system locale data base.  */
char *
k_bindtextdomain (const char *domainname, const char *dirname)
{
  struct binding *binding;

  /* Some sanity checks.  */
  if (domainname == 0L || domainname[0] == '\0')
    return 0L;

  for (binding = _nl_domain_bindings; binding != 0L; binding = binding->next)
    {
      int compare = strcmp (domainname, binding->domainname);
      if (compare == 0)
	/* We found it!  */
	break;
      if (compare < 0)
	{
	  /* It is not in the list.  */
	  binding = 0L;
	  break;
	}
    }

  if (dirname == 0L)
    /* The current binding has be to returned.  */
    return binding == 0L ? (char *) _nl_default_dirname : binding->dirname;

  if (binding != 0L)
    {
      /* The domain is already bound.  Replace the old binding.  */
      char *new_dirname;

      if (strcmp (dirname, _nl_default_dirname) == 0)
	new_dirname = (char *) _nl_default_dirname;
      else
	{
	  size_t len = strlen (dirname) + 1;
	  new_dirname = (char *) malloc (len);
	  if (new_dirname == 0L)
	    return 0L;

	  memcpy (new_dirname, dirname, len);
	}

      if (strcmp (binding->dirname, _nl_default_dirname) != 0)
        free (binding->dirname);

      binding->dirname = new_dirname;
    }
  else
    {
      /* We have to create a new binding.  */
      size_t len;
      struct binding *new_binding =
	(struct binding *) malloc (sizeof (*new_binding));

      if (new_binding == 0L)
	return 0L;

      len = strlen (domainname) + 1;
      new_binding->domainname = (char *) malloc (len);
      if (new_binding->domainname == 0L)
	  return 0L;
      memcpy (new_binding->domainname, domainname, len);

      if (strcmp (dirname, _nl_default_dirname) == 0)
	new_binding->dirname = (char *) _nl_default_dirname;
      else
	{
	  len = strlen (dirname) + 1;
	  new_binding->dirname = (char *) malloc (len);
	  if (new_binding->dirname == 0L)
	    return 0L;
	  memcpy (new_binding->dirname, dirname, len);
	}

      /* Now enqueue it.  */
      if (_nl_domain_bindings == 0L
	  || strcmp (domainname, _nl_domain_bindings->domainname) < 0)
	{
	  new_binding->next = _nl_domain_bindings;
	  _nl_domain_bindings = new_binding;
	}
      else
	{
	  binding = _nl_domain_bindings;
	  while (binding->next != 0L
		 && strcmp (domainname, binding->next->domainname) > 0)
	    binding = binding->next;

	  new_binding->next = binding->next;
	  binding->next = new_binding;
	}

      binding = new_binding;
    }

  return binding->dirname;
}

# if !defined HAVE_GETCWD
char *getwd (char *buf);
#  define getcwd(buf, max) getwd (buf)
# else
char *getcwd (char *buf, size_t size);
# endif

/* Prototypes for local functions.  */
static char *find_msg (struct loaded_l10nfile *domain_file,
		       const char *msgid);

/* For those loosing systems which don't have `alloca' we have to add
   some additional code emulating it.  */
#ifdef HAVE_ALLOCA
/* Nothing has to be done.  */
# define ADD_BLOCK(list, address) /* nothing */
# define FREE_BLOCKS(list) /* nothing */
#else
struct block_list
{
  void *address;
  struct block_list *next;
};
# define ADD_BLOCK(list, addr)						      \
  do {									      \
    struct block_list *newp = (struct block_list *) malloc (sizeof (*newp));  \
    /* If we cannot get a free block we cannot add the new element to	      \
       the list.  */							      \
    if (newp != 0L) {							      \
      newp->address = (addr);						      \
      newp->next = (list);						      \
      (list) = newp;							      \
    }									      \
  } while (0)
# define FREE_BLOCKS(list)						      \
  do {									      \
    while (list != 0L) {						      \
      struct block_list *old = list;					      \
      list = list->next;						      \
      free (old);							      \
    }									      \
  } while (0)
# undef alloca
# define alloca(size) (malloc (size))
#endif	/* have alloca */


/* Look up MSGID in the DOMAINNAME message catalog for the current CATEGORY
   locale.  */
char *
k_dcgettext (const char *domainname, const char *msgid, const char *categoryvalue)
{
#ifndef HAVE_ALLOCA
  struct block_list *block_list = 0L;
#endif
  struct loaded_l10nfile *domain;
  struct binding *binding;
  const char *categoryname;
  char *dirname, *xdomainname; ;
  char *single_locale;
  char *retval;
  int saved_errno = errno;
  const char *_domainname = (domainname == 0L) ? _nl_current_default_domain : domainname;

  /* If no real MSGID is given return 0L.  */
  if (msgid == 0L)
    return 0L;

  /* First find matching binding.  */
  for (binding = _nl_domain_bindings; binding != 0L; binding = binding->next)
    {
      int compare = strcmp (_domainname, binding->domainname);
      if (compare == 0)
	/* We found it!  */
	break;
      if (compare < 0)
	{
	  /* It is not in the list.  */
	  binding = 0L;
	  break;
	}
    }

  if (binding == 0L)
    dirname = (char *) _nl_default_dirname;
  else if (binding->dirname[0] == '/')
    dirname = binding->dirname;
  else
    {
      /* We have a relative path.  Make it absolute now.  */
      size_t dirname_len = strlen (binding->dirname) + 1;
      size_t path_max;
      char *ret;

      path_max = (unsigned) PATH_MAX;
      path_max += 2;		/* The getcwd docs say to do this.  */

      dirname = (char *) alloca (path_max + dirname_len);
      ADD_BLOCK (block_list, dirname);

      errno = 0;
      while ((ret = getcwd (dirname, path_max)) == 0L && errno == ERANGE)
	{
	  path_max += PATH_INCR;
	  dirname = (char *) alloca (path_max + dirname_len);
	  ADD_BLOCK (block_list, dirname);
	  errno = 0;
	}

      if (ret == 0L)
	{
	  /* We cannot get the current working directory.  Don't signal an
	     error but simply return the default string.  */
	  FREE_BLOCKS (block_list);
	  errno = saved_errno;
	  return (char *) msgid;
	}

      /* We don't want libintl.a to depend on any other library.  So
	 we avoid the non-standard function stpcpy.  In GNU C Library
	 this function is available, though.  Also allow the symbol
	 HAVE_STPCPY to be defined.  */
      stpcpy (stpcpy (strchr (dirname, '\0'), "/"), binding->dirname);
    }
  
  /* Now determine the symbolic name of CATEGORY and its value.  */
  categoryname = "LC_MESSAGES";
  xdomainname = (char *) alloca (strlen (categoryname)
                                 + strlen (_domainname) + 5);
  
  ADD_BLOCK (block_list, xdomainname);
  /* We don't want libintl.a to depend on any other library.  So we
     avoid the non-standard function stpcpy.  In GNU C Library this
     function is available, though.  Also allow the symbol HAVE_STPCPY
     to be defined.  */
  stpcpy (stpcpy (stpcpy (stpcpy (xdomainname, categoryname), "/"),
                  _domainname),
          ".mo");

  /* Creating working area.  */
  single_locale = (char *) alloca (strlen (categoryvalue) + 1);
  ADD_BLOCK (block_list, single_locale);

  /* Search for the given string.  This is a loop because we perhaps
     got an ordered list of languages to consider for th translation.  */
  while (1)
    {
      /* Make CATEGORYVALUE point to the next element of the list.  */
      while (categoryvalue[0] != '\0' && categoryvalue[0] == ':')
	++categoryvalue;
      if (categoryvalue[0] == '\0')
	{
	  /* The whole contents of CATEGORYVALUE has been searched but
	     no valid entry has been found.  We solve this situation
	     by implicitely appending a "C" entry, i.e. no translation
	     will take place.  */
	  single_locale[0] = 'C';
	  single_locale[1] = '\0';
	}
      else
	{
	  char *cp = single_locale;
	  while (categoryvalue[0] != '\0' && categoryvalue[0] != ':')
	    *cp++ = *categoryvalue++;
	  *cp = '\0';
	}

      /* If the current locale value is C (or POSIX) we don't load a
	 domain.  Return the MSGID.  */
      if (strcmp (single_locale, "C") == 0
	  || strcmp (single_locale, "POSIX") == 0)
	{
	  FREE_BLOCKS (block_list);
	  errno = saved_errno;
	  return (char *) msgid;
	}


      /* Find structure describing the message catalog matching the
	 DOMAINNAME and CATEGORY.  */
      domain = k_nl_find_domain (dirname, single_locale, xdomainname);

      if (domain != 0L)
	{
	  retval = find_msg (domain, msgid);

	  if (retval == 0L)
	    {
	      int cnt;

	      for (cnt = 0; domain->successor[cnt] != 0L; ++cnt)
		{
		  retval = find_msg (domain->successor[cnt], msgid);

		  if (retval != 0L)
		    break;
		}
	    }

	  if (retval != 0L)
	    {
	      FREE_BLOCKS (block_list);
	      errno = saved_errno;
	      return retval;
	    }
	}
    }
  /* NOTREACHED */
}

static char *
find_msg (struct loaded_l10nfile *domain_file, const char *msgid)
{
  size_t top, act=0, bottom;
  struct loaded_domain *domain;

  if (domain_file->decided == 0)
    _nl_load_domain (domain_file);

  if (domain_file->data == 0L)
    return 0L;

  domain = (struct loaded_domain *) domain_file->data;

  /* Locate the MSGID and its translation.  */
  if (domain->hash_size > 2 && domain->hash_tab != 0L)
    {
      /* Use the hashing table.  */
      nls_uint32 len = strlen (msgid);
      nls_uint32 hash_val = hash_string (msgid);
      nls_uint32 idx = hash_val % domain->hash_size;
      nls_uint32 incr = 1 + (hash_val % (domain->hash_size - 2));
      nls_uint32 nstr = W (domain->must_swap, domain->hash_tab[idx]);

      if (nstr == 0)
	/* Hash table entry is empty.  */
	return 0L;

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
	    return 0L;

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
  return bottom >= top ? 0L : (char *) domain->data
                                + W (domain->must_swap,
				     domain->trans_tab[act].offset);
}

/* Set the current default message catalog to DOMAINNAME.
   If DOMAINNAME is null, return the current default.
   If DOMAINNAME is "", reset to the default of "messages".  */
char *
k_textdomain (const char *domainname)
{
  char *old;

  /* A 0L pointer requests the current setting.  */
  if (domainname == 0L)
    return (char *) _nl_current_default_domain;

  old = (char *) _nl_current_default_domain;

  /* If domain name is the null string set to default domain "messages".  */
  if (domainname[0] == '\0'
      || strcmp (domainname, _nl_default_default_domain) == 0)
    _nl_current_default_domain = _nl_default_default_domain;
  else
    {
      /* If the following malloc fails `_nl_current_default_domain'
	 will be NULL.  This value will be returned and so signals we
	 are out of core.  */
      size_t len = strlen (domainname) + 1;
      char *cp = (char *) malloc (len);
      if (cp != 0L)
	memcpy (cp, domainname, len);
      _nl_current_default_domain = cp;
    }

  if (old != _nl_default_default_domain)
    free (old);

  return (char *) _nl_current_default_domain;
}

#if !defined HAVE___ARGZ_COUNT
/* Returns the number of strings in ARGZ.  */
 
static size_t
argz_count__ (const char *argz, size_t len)
{
  size_t count = 0;
  while (len > 0)
    {
      size_t part_len = strlen (argz);
      argz += part_len + 1;
      len -= part_len + 1;
      count++;
    }
  return count;
}
# undef __argz_count
# define __argz_count(argz, len) argz_count__ (argz, len)
#endif  /* !HAVE___ARGZ_COUNT */
 
#if !defined HAVE___ARGZ_STRINGIFY
/* Make '\0' separated arg vector ARGZ printable by converting all the '\0's
   except the last into the character SEP.  */
 
static void
argz_stringify__ (char *argz, size_t len, int sep)
{
  while (len > 0)
    {
      size_t part_len = strlen (argz);
      argz += part_len;
      len -= part_len + 1;
      if (len > 0)
        *argz++ = sep;
    }
}
# undef __argz_stringify
# define __argz_stringify(argz, len, sep) argz_stringify__ (argz, len, sep)
#endif  /* !HAVE___ARGZ_STRINGIFY */
 
#if !defined HAVE___ARGZ_NEXT
 
static char *
argz_next__ (char *argz, size_t argz_len, const char *entry)
{
  if (entry)
    {
      if (entry < argz + argz_len)
        entry = strchr (entry, '\0') + 1;
 
      return entry >= argz + argz_len ? 0L : (char *) entry;
    }
  else
    if (argz_len > 0)
      return argz;
    else
      return 0;
}
# undef __argz_next
# define __argz_next(argz, len, entry) argz_next__ (argz, len, entry)
#endif  /* !HAVE___ARGZ_NEXT */
 
 
/* Return number of bits set in X.  */
static inline int
pop (int x)
{
  /* We assume that no more than 16 bits are used.  */
  x = ((x & ~0x5555) >> 1) + (x & 0x5555);
  x = ((x & ~0x3333) >> 2) + (x & 0x3333);
  x = ((x >> 4) + x) & 0x0f0f;
  x = ((x >> 8) + x) & 0xff;
 
  return x;
}
 
static struct loaded_l10nfile *
_nl_make_l10nflist (struct loaded_l10nfile **l10nfile_list,
		    const char *dirlist, size_t dirlist_len,
		    int mask, const char *language,
                    const char *territory, const char *codeset,
		    const char *normalized_codeset, const char *modifier,
		    const char *special, const char *sponsor,
		    const char *revision, const char *filename,
		    int do_allocate)
{
  char *abs_filename;
  struct loaded_l10nfile *last = 0L;
  struct loaded_l10nfile *retval;
  char *cp;
  size_t entries;
  int cnt;
 
  /* Allocate room for the full file name.  */
  abs_filename = (char *) malloc (dirlist_len
                                  + strlen (language)
                                  + ((mask & TERRITORY) != 0
                                     ? strlen (territory) + 1 : 0)
                                  + ((mask & XPG_CODESET) != 0
                                     ? strlen (codeset) + 1 : 0)
                                  + ((mask & XPG_NORM_CODESET) != 0
                                     ? strlen (normalized_codeset) + 1 : 0)
                                  + (((mask & XPG_MODIFIER) != 0
                                      || (mask & CEN_AUDIENCE) != 0) ?
                                     strlen (modifier) + 1 : 0)
                                  + ((mask & CEN_SPECIAL) != 0
                                     ? strlen (special) + 1 : 0)
                                  + ((mask & CEN_SPONSOR) != 0
                                     ? strlen (sponsor) + 1 : 0)
                                  + ((mask & CEN_REVISION) != 0
                                     ? strlen (revision) + 1 : 0)
                                  + 1 + strlen (filename) + 1);
 
  if (abs_filename == 0L)
    return 0L;
 
  retval = 0L;
  last = 0L;
 
  /* Construct file name.  */
  memcpy (abs_filename, dirlist, dirlist_len);
  __argz_stringify (abs_filename, dirlist_len, ':');
  cp = abs_filename + (dirlist_len - 1);
  *cp++ = '/';
  cp = stpcpy (cp, language);
 
  if ((mask & TERRITORY) != 0)
    {
      *cp++ = '_';
      cp = stpcpy (cp, territory);
    }
  if ((mask & XPG_CODESET) != 0)
    {
      *cp++ = '.';
      cp = stpcpy (cp, codeset);
    }
  if ((mask & XPG_NORM_CODESET) != 0)
    {
      *cp++ = '.';
      cp = stpcpy (cp, normalized_codeset);
    }
  if ((mask & (XPG_MODIFIER | CEN_AUDIENCE)) != 0)
    {
      /* This component can be part of both syntaces but has different
         leading characters.  For CEN we use `+', else `@'.  */
      *cp++ = (mask & CEN_AUDIENCE) != 0 ? '+' : '@';
      cp = stpcpy (cp, modifier);
    }
  if ((mask & CEN_SPECIAL) != 0)
    {
      *cp++ = '+';
      cp = stpcpy (cp, special);
    }
  if ((mask & CEN_SPONSOR) != 0)
    {
      *cp++ = ',';
      cp = stpcpy (cp, sponsor);
    }
  if ((mask & CEN_REVISION) != 0)
    {
      *cp++ = '_';
      cp = stpcpy (cp, revision);
    }
 
  *cp++ = '/';
  stpcpy (cp, filename);
 
  /* Look in list of already loaded domains whether it is already
     available.  */
  last = 0L;
  for (retval = *l10nfile_list; retval != 0L; retval = retval->next)
    if (retval->filename != 0L)
      {
        int compare = strcmp (retval->filename, abs_filename);
        if (compare == 0)
          /* We found it!  */
          break;
        if (compare < 0)
          {
            /* It's not in the list.  */
            retval = 0L;
            break;
          }
 
        last = retval;
      }
 
  if (retval != 0L || do_allocate == 0)
    {
      free (abs_filename);
      return retval;
    }
 
  retval = (struct loaded_l10nfile *)
    malloc (sizeof (*retval) + (__argz_count (dirlist, dirlist_len)
                                * (1 << pop (mask))
                                * sizeof (struct loaded_l10nfile *)));
  if (retval == 0L)
    return 0L;
 
  retval->filename = abs_filename;
  retval->decided = (__argz_count (dirlist, dirlist_len) != 1
                     || ((mask & XPG_CODESET) != 0
                         && (mask & XPG_NORM_CODESET) != 0));
  retval->data = 0L;
 
  if (last == 0L)
    {
      retval->next = *l10nfile_list;
      *l10nfile_list = retval;
    }
  else
    {
      retval->next = last->next;
      last->next = retval;
    }
 
  entries = 0;
  /* If the DIRLIST is a real list the RETVAL entry correcponds not to
     a real file.  So we have to use the DIRLIST separation machanism
     of the inner loop.  */
  cnt = __argz_count (dirlist, dirlist_len) == 1 ? mask - 1 : mask;
  for (; cnt >= 0; --cnt)
    if ((cnt & ~mask) == 0
        && ((cnt & CEN_SPECIFIC) == 0 || (cnt & XPG_SPECIFIC) == 0)
        && ((cnt & XPG_CODESET) == 0 || (cnt & XPG_NORM_CODESET) == 0))
      {
        /* Iterate over all elements of the DIRLIST.  */
        char *dir = 0L;
        while ((dir = __argz_next ((char *) dirlist, dirlist_len, dir))
               != 0L)
          retval->successor[entries++]
            = _nl_make_l10nflist (l10nfile_list, dir, strlen (dir) + 1, cnt,
                                  language, territory, codeset,
                                  normalized_codeset, modifier, special,
                                  sponsor, revision, filename, 1);
      }
  retval->successor[entries] = 0L;
 
  return retval;
}

struct alias_map
{
  const char *alias;
  const char *value;
};
 
 
static struct alias_map *map;
static size_t nmap = 0;
static size_t maxmap = 0;

/* Prototypes for local functions.  */
static void extend_alias_table (void);

static int
alias_compare (const struct alias_map *map1, const struct alias_map *map2)
{
#if defined HAVE_STRCASECMP
  return strcasecmp (map1->alias, map2->alias);
#else
  const unsigned char *p1 = (const unsigned char *) map1->alias;
  const unsigned char *p2 = (const unsigned char *) map2->alias;
  unsigned char c1, c2;
 
  if (p1 == p2)
    return 0;
 
  do
    {
     /* I know this seems to be odd but the tolower() function in
         some systems libc cannot handle nonalpha characters.  */
      c1 = isupper (*p1) ? tolower (*p1) : *p1;
      c2 = isupper (*p2) ? tolower (*p2) : *p2;
      if (c1 == '\0')
        break;
      ++p1;
      ++p2;
    }
  while (c1 == c2);
 
  return c1 - c2;
#endif
}

static size_t
read_alias_file (const char *fname, int fname_len)
{
#ifndef HAVE_ALLOCA
  struct block_list *block_list = 0L;
#endif
  FILE *fp;
  char *full_fname;
  size_t added;
  static const char aliasfile[] = "/locale.alias";

  full_fname = (char *) alloca (fname_len + sizeof aliasfile);
  ADD_BLOCK (block_list, full_fname);
  memcpy (full_fname, fname, fname_len);
  memcpy (&full_fname[fname_len], aliasfile, sizeof aliasfile);

  fp = fopen (full_fname, "r");
  if (fp == 0L)
    {
      FREE_BLOCKS (block_list);
      return 0;
    }

  added = 0;
  while (!feof (fp))
    {
      /* It is a reasonable approach to use a fix buffer here because
	 a) we are only interested in the first two fields
	 b) these fields must be usable as file names and so must not
	    be that long
       */
      char buf[BUFSIZ];
      char *alias;
      char *value;
      char *cp;

      if (fgets (buf, BUFSIZ, fp) == 0L)
	/* EOF reached.  */
	break;

      cp = buf;
      /* Ignore leading white space.  */
      while (isspace (cp[0]))
	++cp;

      /* A leading '#' signals a comment line.  */
      if (cp[0] != '\0' && cp[0] != '#')
	{
	  alias = cp++;
	  while (cp[0] != '\0' && !isspace (cp[0]))
	    ++cp;
	  /* Terminate alias name.  */
	  if (cp[0] != '\0')
	    *cp++ = '\0';

	  /* Now look for the beginning of the value.  */
	  while (isspace (cp[0]))
	    ++cp;

	  if (cp[0] != '\0')
	    {
	      char *tp;
	      size_t len;

	      value = cp++;
	      while (cp[0] != '\0' && !isspace (cp[0]))
		++cp;
	      /* Terminate value.  */
	      if (cp[0] == '\n')
		{
		  /* This has to be done to make the following test
		     for the end of line possible.  We are looking for
		     the terminating '\n' which do not overwrite here.  */
		  *cp++ = '\0';
		  *cp = '\n';
		}
	      else if (cp[0] != '\0')
		*cp++ = '\0';

	      if (nmap >= maxmap)
		extend_alias_table ();

	      /* We cannot depend on strdup available in the libc.  Sigh!  */
	      len = strlen (alias) + 1;
	      tp = (char *) malloc (len);
	      if (tp == 0L)
		{
		  FREE_BLOCKS (block_list);
		  return added;
		}
	      memcpy (tp, alias, len);
	      map[nmap].alias = tp;

	      len = strlen (value) + 1;
	      tp = (char *) malloc (len);
	      if (tp == 0L)
		{
		  FREE_BLOCKS (block_list);
		  return added;
		}
	      memcpy (tp, value, len);
	      map[nmap].value = tp;

	      ++nmap;
	      ++added;
	    }
	}

      /* Possibily not the whole line fitted into the buffer.  Ignore
	 the rest of the line.  */
      while (strchr (cp, '\n') == 0L)
	{
	  cp = buf;
	  if (fgets (buf, BUFSIZ, fp) == 0L)
	    /* Make sure the inner loop will be left.  The outer loop
	       will exit at the `feof' test.  */
	    *cp = '\n';
	}
    }

  /* Should we test for ferror()?  I think we have to silently ignore
     errors.  --drepper  */
  fclose (fp);

  if (added > 0)
    qsort (map, nmap, sizeof (struct alias_map),
	   (int (*) (const void *, const void *)) alias_compare);

  FREE_BLOCKS (block_list);
  return added;
}

static const char *
_nl_expand_alias (const char *name)
{
  static const char *locale_alias_path = LOCALE_ALIAS_PATH;
  struct alias_map *retval;
  size_t added;

  do
    {
      struct alias_map item;

      item.alias = name;

      if (nmap > 0)
	retval = (struct alias_map *) bsearch (&item, map, nmap,
					       sizeof (struct alias_map),
					       (int (*) (const void *,
							 const void *)
						) alias_compare);
      else
	retval = 0L;

      /* We really found an alias.  Return the value.  */
      if (retval != 0L)
	return retval->value;

      /* Perhaps we can find another alias file.  */
      added = 0;
      while (added == 0 && locale_alias_path[0] != '\0')
	{
	  const char *start;

	  while (locale_alias_path[0] == ':')
	    ++locale_alias_path;
	  start = locale_alias_path;

	  while (locale_alias_path[0] != '\0' && locale_alias_path[0] != ':')
	    ++locale_alias_path;

	  if (start < locale_alias_path)
	    added = read_alias_file (start, locale_alias_path - start);
	}
    }
  while (added != 0);

  return 0L;
}

static void
extend_alias_table ()
{
  size_t new_size;
  struct alias_map *new_map;
 
  new_size = maxmap == 0 ? 100 : 2 * maxmap;
  new_map = (struct alias_map *) malloc (new_size
                                         * sizeof (struct alias_map));
  if (new_map == 0L)
    /* Simply don't extend: we don't have any more core.  */
    return;
 
  memcpy (new_map, map, nmap * sizeof (struct alias_map));
 
  if (maxmap != 0)
    free (map);
 
  map = new_map;
  maxmap = new_size;
}


static int
_nl_explode_name (char *name, const char **language,
		  const char **modifier, const char **territory,
		  const char **codeset, const char **normalized_codeset,
		  const char **special, const char **sponsor,
		  const char **revision)
{
  enum { undecided, xpg, cen } syntax;
  char *cp;
  int mask;

  *modifier = 0L;
  *territory = 0L;
  *codeset = 0L;
  *normalized_codeset = 0L;
  *special = 0L;
  *sponsor = 0L;
  *revision = 0L;

  /* Now we determine the single parts of the locale name.  First
     look for the language.  Termination symbols are `_' and `@' if
     we use XPG4 style, and `_', `+', and `,' if we use CEN syntax.  */
  mask = 0;
  syntax = undecided;
  *language = cp = name;
  while (cp[0] != '\0' && cp[0] != '_' && cp[0] != '@'
	 && cp[0] != '+' && cp[0] != ',')
    ++cp;

  if (*language == cp)
    /* This does not make sense: language has to be specified.  Use
       this entry as it is without exploding.  Perhaps it is an alias.  */
    cp = strchr (*language, '\0');
  else if (cp[0] == '_')
    {
      /* Next is the territory.  */
      cp[0] = '\0';
      *territory = ++cp;

      while (cp[0] != '\0' && cp[0] != '.' && cp[0] != '@'
	     && cp[0] != '+' && cp[0] != ',' && cp[0] != '_')
	++cp;

      mask |= TERRITORY;

      if (cp[0] == '.')
	{
	  /* Next is the codeset.  */
	  syntax = xpg;
	  cp[0] = '\0';
	  *codeset = ++cp;

	  while (cp[0] != '\0' && cp[0] != '@')
	    ++cp;

	  mask |= XPG_CODESET;

	  if (*codeset != cp && (*codeset)[0] != '\0')
	    {
	      *normalized_codeset = _nl_normalize_codeset (*codeset,
							   cp - *codeset);
	      if (strcmp (*codeset, *normalized_codeset) == 0)
		free ((char *) *normalized_codeset);
	      else
		mask |= XPG_NORM_CODESET;
	    }
	}
    }

  if (cp[0] == '@' || (syntax != xpg && cp[0] == '+'))
    {
      /* Next is the modifier.  */
      syntax = cp[0] == '@' ? xpg : cen;
      cp[0] = '\0';
      *modifier = ++cp;

      while (syntax == cen && cp[0] != '\0' && cp[0] != '+'
	     && cp[0] != ',' && cp[0] != '_')
	++cp;

      mask |= XPG_MODIFIER | CEN_AUDIENCE;
    }

  if (syntax != xpg && (cp[0] == '+' || cp[0] == ',' || cp[0] == '_'))
    {
      syntax = cen;

      if (cp[0] == '+')
	{
 	  /* Next is special application (CEN syntax).  */
	  cp[0] = '\0';
	  *special = ++cp;

	  while (cp[0] != '\0' && cp[0] != ',' && cp[0] != '_')
	    ++cp;

	  mask |= CEN_SPECIAL;
	}

      if (cp[0] == ',')
	{
 	  /* Next is sponsor (CEN syntax).  */
	  cp[0] = '\0';
	  *sponsor = ++cp;

	  while (cp[0] != '\0' && cp[0] != '_')
	    ++cp;

	  mask |= CEN_SPONSOR;
	}

      if (cp[0] == '_')
	{
 	  /* Next is revision (CEN syntax).  */
	  cp[0] = '\0';
	  *revision = ++cp;

	  mask |= CEN_REVISION;
	}
    }

  /* For CEN sytnax values it might be important to have the
     separator character in the file name, not for XPG syntax.  */
  if (syntax == xpg)
    {
      if (*territory != 0L && (*territory)[0] == '\0')
	mask &= ~TERRITORY;

      if (*codeset != 0L && (*codeset)[0] == '\0')
	mask &= ~XPG_CODESET;

      if (*modifier != 0L && (*modifier)[0] == '\0')
	mask &= ~XPG_MODIFIER;
    }

  return mask;
}

/* Normalize codeset name.  There is no standard for the codeset
   names.  Normalization allows the user to use any of the common
   names.  */
static const char *
_nl_normalize_codeset (const char *codeset, size_t name_len)
{
  int len = 0;
  int only_digit = 1;
  char *retval;
  char *wp;
  size_t cnt;

  for (cnt = 0; cnt < name_len; ++cnt)
    if (isalnum (codeset[cnt]))
      {
	++len;

	if (isalpha (codeset[cnt]))
	  only_digit = 0;
      }

  retval = (char *) malloc ((only_digit ? 3 : 0) + len + 1);

  if (retval != 0L)
    {
      if (only_digit)
	wp = stpcpy (retval, "iso");
      else
	wp = retval;

      for (cnt = 0; cnt < name_len; ++cnt)
	if (isalpha (codeset[cnt]))
	  *wp++ = tolower (codeset[cnt]);
	else if (isdigit (codeset[cnt]))
	  *wp++ = codeset[cnt];

      *wp = '\0';
    }

  return (const char *) retval;
}




