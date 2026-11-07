@echo off
setlocal DisableDelayedExpansion
pushd "%~dp0"
if errorlevel 1 exit /b 1
powershell -NoProfile -File "%~dp0tools\github_push.ps1" %*
set "GITHUB_PUSH_EXIT=%ERRORLEVEL%"
popd
exit /b %GITHUB_PUSH_EXIT%
