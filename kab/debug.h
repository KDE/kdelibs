/** Source file for removing Nana macros.
  * Mirko Sucker, 1998.
  * $Id$
  */

/* The following is provided to remove Nana-assertions from the
 * sourcecode. I strongly recommend using Nana if you are sear-
 * ching for bugs  because the assertions will catch mostly all
 * forbidden program states. Additionally the checks for object
 * invariants using the 
 *         bool invariant();
 * element function are used that watch the state of each part-
 * icular object. Nana is available at 
 *         http://wattle.cs.ntu.edu.au/homepages/pjm/nana-home/
 * For compiling final binaries  Nana  is no more needed due to 
 * this defines. The macro REMOVE_NANA should be defined in the
 * Makefile when compiling a final binary.
 */

#ifndef KAB_DEBUG_H
#define	KAB_DEBUG_H

#include <stdio.h> // Nana needs this
#include <errno.h>
#include <kapp.h>

// use version without Nana per default:
#ifndef KAB_DEBUG
#define REMOVE_NANA
#endif

#if defined REMOVE_NANA

#define EIFFEL_CHECK CHECK_NO 
#define L_LEVEL 0 
#define I_LEVEL 0
#define L(x...)
#define LG(x...)
// There is a problem with the ID(..) function in ktoolbar.h:
#if !defined ID_already_declared
#define ID(a...)
#endif
#define CHECK(a...)
#define I(a...)
#define REQUIRE(a...)
#define ENSURE(a...)

#else 

#define EIFFEL_CHECK CHECK_ALL
#include <nana.h>
#include <eiffel.h>
#include <Qstl.h>

#endif

// general defines:

// taken from kapp.h:
#ifndef i18n
#define i18n(X) KApplication::getKApplication()->getLocale()->translate(X)
#endif

#endif // KAB_DEBUG_H
