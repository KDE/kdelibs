::
:: win32 support file: allow running pythons scripts as binary 
:: 
@echo off

if not "%PYTHON%"=="" (
	assoc .py=Python.File
	ftype Python.File=%PYTHON%\python.exe "%%1" %%*
	set PATHEXT=%PATHEXT%;.py
	echo ...
	echo now you can start any python script from bksys with bksys\scriptname
) else (
	echo please set PYTHON environment variable to your python installation
)

