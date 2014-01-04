@echo off
REM bzr_script.cmd
REM 
REM PROJECT SETTING !!
REM define the remote location and ftp login (don't put the password here please!!!)
SET LOCATION="ftp://wxWebcamDB%%40burri-web.org@burri-web.org/"

REM USER PREFERENCES
REM define the Viewer to use for Diff Output
SET VIEWER="notepad.exe"
REM define the temp file for e.g. the Diff output
SET TEMPFILE=%TEMP%\BZtmp.txt
REM This overwrites the default editor if needed
SET BZR_EDITOR=%VIEWER%

REM the Reference Directory extension  i.e. Project is    foo   RefDir is then  ..\foo_Ref
SET REFDIR=_Ref

REM Arguments:
REM %1  the project name (case insensitive for Win)
REM %2 the command
REM %3 an optional filename

REM NO MORE CHANGES AFTER THIS POINT PLEASE

REM sanity checks
if "%1" == "" goto errout
if not exist ..\%1%REFDIR% goto errout_NoRef

if "%2" == "" goto bINFO

REM command dispatcher
if /i %2 == INFO		goto bINFO
if /i %2 == STAT		goto bSTAT
if /i %2 == LOG 		goto bLOG
if /i %2 == ADD_F       goto bADD_F
if /i %2 == DIFF_F	    goto bDIFF_F
if /i %2 == COMMIT_F	goto bCOMMIT_F
if /i %2 == COMMIT_ALL	goto bCOMMIT_ALL
if /i %2 == MERGE       goto bMERGE
if /i %2 == RESOLVE     goto bRESOLVE
if /i %2 == PUSH		goto bPUSH

if /i %2 == RLOG 		goto bRLOG
if /i %2 == PUSH_REF	goto bPUSH_REF
if /i %2 == PULL_REF	goto bPULL_REF
goto errout

:bINFO
REM Information about the local branch
@echo on
bzr info > %TEMPFILE%
@echo off
%VIEWER% %TEMPFILE%
goto :eof

:bSTAT
REM Status of the local branch
@echo on
bzr stat
@echo off
goto :eof

:bLOG
REM Logs the most recent things from the local branch
@echo on
bzr log --forward > %TEMPFILE%
@echo off
%VIEWER% %TEMPFILE%
goto :eof

:bADD_F
REM Adds a file to the branch
REM a filename is required
if "%2" == "" goto errout
@echo on
bzr add %2
@echo off
goto :eof

:bDIFF_F
REM Makes a diff of the most recent changes
REM a filename is required
if "%2" == "" goto errout
@echo on
bzr diff %2 > %TEMPFILE%
@echo off
%VIEWER% %TEMPFILE%
goto :eof

:bCOMMIT_F
REM Commits a file into the local branch
REM a filename is required
if "%2" == "" goto errout
@echo on
call bzr commit  -m "via Win-CB script" %2
@echo off
goto :eof

:bCOMMIT_ALL
REM Commits all into the local branch
@echo on
call bzr commit -m "via Win-CB script"
@echo off
goto :eof

:bMERGE
REM Merges the _Ref content into the project content
@echo on
bzr merge ..\%1%REFDIR% 
@echo off
goto :eof

:bRESOLVE
REM Resolves all conflicts
@echo on
bzr resolve
@echo off
goto :eof

:bPUSH
REM This pushes the content to the _Ref directory
@echo on
bzr push ..\%1%REFDIR%
@echo off
goto :eof


REM SERVER TARGETED COMMANDS

:bRLOG
REM Logs the most recent things from the remote server
@echo on
bzr log --forward %LOCATION%
@echo off
goto :eof

:bPUSH_REF
REM This pushes the content to the Server
@echo on
pushd ..\%1%REFDIR%
bzr push %LOCATION%
@echo off
popd
goto :eof

:bPULL_REF
REM Pull the Server into Project_Ref
@echo on
pushd ..\%1%REFDIR%
bzr pull --overwrite %LOCATION% 
@echo off
popd
goto :eof

REM IF SOMETHING FAILED ...
:errout_NoRef
ECHO "%1_Ref directory does not exist!!"
goto :eof

:errout
ECHO "Parameter wrong or missing"
goto :eof

