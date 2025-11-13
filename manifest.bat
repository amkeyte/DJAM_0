@echo off
setlocal

REM Folder where this batch file lives (your repo root)
set "ROOT=%~dp0"
REM Trim trailing backslash
set "ROOT=%ROOT:~0,-1%"

REM Path to Python script
set "SCRIPT=%ROOT%\manifest.py"

REM Target folder to scan: [this folder]\source
set "TARGET=%ROOT%\source"

REM Base URL for raw GitHub files
set "BASEURL=https://raw.githubusercontent.com/amkeyte/DJAM_0/master/source"

REM Output manifest.txt in [this folder]
set "OUTFILE=%ROOT%\manifest.txt"

echo Generating manifest.txt...
python "%SCRIPT%" "%TARGET%" "%BASEURL%" "%OUTFILE%"
echo Done.

endlocal
pause
