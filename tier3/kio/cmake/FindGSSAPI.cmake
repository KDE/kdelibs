# - Try to detect the GSSAPI support
# Once done this will define
#
#  GSSAPI_FOUND - system supports GSSAPI
#  GSSAPI_INCS - the GSSAPI include directory
#  GSSAPI_LIBS - the libraries needed to use GSSAPI
#  GSSAPI_FLAVOR - the type of API - MIT or HEIMDAL

# Copyright (c) 2006, Pino Toscano, <toscano.pino@tiscali.it>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.


if(GSSAPI_LIBS AND GSSAPI_FLAVOR)

  # in cache already
  set(GSSAPI_FOUND TRUE)

else(GSSAPI_LIBS AND GSSAPI_FLAVOR)

  find_program(KRB5_CONFIG NAMES krb5-config PATHS
     /opt/local/bin
     ONLY_CMAKE_FIND_ROOT_PATH               # this is required when cross compiling with cmake 2.6 and ignored with cmake 2.4, Alex
  )
  mark_as_advanced(KRB5_CONFIG)
  
  #reset vars
  set(GSSAPI_INCS)
  set(GSSAPI_LIBS)
  set(GSSAPI_FLAVOR)
  
  if(KRB5_CONFIG)
  
    set(HAVE_KRB5_GSSAPI TRUE)
    exec_program(${KRB5_CONFIG} ARGS --libs gssapi RETURN_VALUE _return_VALUE OUTPUT_VARIABLE GSSAPI_LIBS)
    if(_return_VALUE)
      message(STATUS "GSSAPI configure check failed.")
      set(HAVE_KRB5_GSSAPI FALSE)
    endif(_return_VALUE)
  
    exec_program(${KRB5_CONFIG} ARGS --cflags gssapi RETURN_VALUE _return_VALUE OUTPUT_VARIABLE GSSAPI_INCS)
    string(REGEX REPLACE "(\r?\n)+$" "" GSSAPI_INCS "${GSSAPI_INCS}")
    string(REGEX REPLACE " *-I" ";" GSSAPI_INCS "${GSSAPI_INCS}")

    exec_program(${KRB5_CONFIG} ARGS --vendor RETURN_VALUE _return_VALUE OUTPUT_VARIABLE gssapi_flavor_tmp)
    set(GSSAPI_FLAVOR_MIT)
    if(gssapi_flavor_tmp MATCHES ".*Massachusetts.*")
      set(GSSAPI_FLAVOR "MIT")
    else(gssapi_flavor_tmp MATCHES ".*Massachusetts.*")
      set(GSSAPI_FLAVOR "HEIMDAL")
    endif(gssapi_flavor_tmp MATCHES ".*Massachusetts.*")
  
    if(NOT HAVE_KRB5_GSSAPI)
      if (gssapi_flavor_tmp MATCHES "Sun Microsystems.*")
         message(STATUS "Solaris Kerberos does not have GSSAPI; this is normal.")
         set(GSSAPI_LIBS)
         set(GSSAPI_INCS)
      else(gssapi_flavor_tmp MATCHES "Sun Microsystems.*")
         message(WARNING "${KRB5_CONFIG} failed unexpectedly.")
      endif(gssapi_flavor_tmp MATCHES "Sun Microsystems.*")
    endif(NOT HAVE_KRB5_GSSAPI)

    if(GSSAPI_LIBS) # GSSAPI_INCS can be also empty, so don't rely on that
      set(GSSAPI_FOUND TRUE)
      message(STATUS "Found GSSAPI: ${GSSAPI_LIBS}")

      set(GSSAPI_INCS ${GSSAPI_INCS})
      set(GSSAPI_LIBS ${GSSAPI_LIBS})
      set(GSSAPI_FLAVOR ${GSSAPI_FLAVOR})

      mark_as_advanced(GSSAPI_INCS GSSAPI_LIBS GSSAPI_FLAVOR)

    endif(GSSAPI_LIBS)
  
  endif(KRB5_CONFIG)

endif(GSSAPI_LIBS AND GSSAPI_FLAVOR)
