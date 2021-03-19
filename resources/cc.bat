@echo off
REM
REM Note: This is mostly not needed due to sterling work over at Z88DK adding MGT Support
REM https://github.com/z88dk/z88dk/commit/fa1f1b45901e4412f190353647667192b4c2e61b)
REM but may still be useful
REM
REM Windows Batch file for launching ZCC to compile SAM Coupé executables and lauch them in SimCoupe via
REM an MGT disk image.
REM
REM https://github.com/dandoore/mgtman


set FILE=helloworld
set OUTPUT=auto.bin
set DOSFILE=samdos2
set EMUPATH="C:\Program Files (x86)\SimCoupe\simcoupe.exe"

echo Compiling %FILE%.c... 
zcc +sam %FILE%.c -lm -clib=ansi -pragma-define:ansicolumns=32
IF NOT %ERRORLEVEL% == 0  GOTO FAIL

echo Removing old %FILE%.mgt file if needed...
IF EXIST %FILE%.mgt del %FILE%.mgt 1> nul 2> nul
IF NOT %ERRORLEVEL% == 0  GOTO FAIL

echo Renaming BIN to %OUTPUT%...
rename a.bin %OUTPUT%
IF NOT %ERRORLEVEL% == 0  GOTO FAIL

echo Copying %DOSFILE% to %FILE%.mgt...
mgtman -w %FILE%.mgt %DOSFILE%
IF NOT %ERRORLEVEL% == 0  GOTO FAIL

echo Copying auto.bin to %FILE%.mgt...
mgtman -w %FILE%.mgt %OUTPUT% x
IF NOT %ERRORLEVEL% == 0  GOTO FAIL

echo Removing binary %OUTPUT%...
del %OUTPUT% 1> nul 2> nul
IF NOT %ERRORLEVEL% == 0  GOTO FAIL

echo Launching %FILE%.mgt with SimCoupe...
call %EMUPATH% %FILE%.mgt -autoload 1> nul 2> nul
IF NOT %ERRORLEVEL% == 0  GOTO FAIL

GOTO END

:FAIL
Echo  ^^ %FILE% failed, press a key.
pause 1> nul 2> nul
:END
