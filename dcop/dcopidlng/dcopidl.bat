@echo off
::
:: dcopidl wrapper for win32 
:: 
:: TODO: (rh) replace CYGWIN perl by native win32 perl
:: 

set LIBDIR=
if "%1" == "--srcdir" (
	shift
	set LIBDIR=%2
	shift
) 

if "%LIBDIR%" == "" (
	if "%KDECONFIG%" == "" (
		set KDECONFIG=kde-config
  )
  echo "TODO: implement -> LIBDIR='`$KDECONFIG --install data --expandvars`/dcopidl'
  rem set LIBDIR="`$KDECONFIG --install data --expandvars`/dcopidl"
)
%CYGWIN%\bin\perl  -I%LIBDIR% %LIBDIR%\\kalyptus --quiet --allow_k_dcop_accessors -f dcopidl %1 >%2
set RET=ERRORLEVEL
if %RET%==0 (
	rem
) else (
   del %2
)

exit %RET%
