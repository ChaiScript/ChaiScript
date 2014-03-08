@ECHO OFF
:: Make Chaiscript for Windows.
ECHO.

:start
SET START_DIR="%cd%"
cd /D %~dp0
cd
goto :welcome

:welcome
echo:
echo ** Chaiscript Build Script **
echo:
echo this script was written by da2ce7
echo please feel free to report bugs on github.
echo:
goto :define

:define

set "ifErr=set foundErr=1&(if errorlevel 0 if not errorlevel 1 set foundErr=)&if defined foundErr"

SET MSVC_BAT="C:\Program Files (x86)\Microsoft Visual Studio 12.0\VC\vcvarsall.bat"
SET CMAKE_EXE=cmake.exe
SET NMAKE_EXE=nmake.exe
SET BUILD_DIR=build
SET INSTALL_DIR=install


SET C_GEN=-G "NMake Makefiles"
SET C_INSTALL_PATH=-DCMAKE_INSTALL_PREFIX:PATH="../%INSTALL_DIR%"
SET C_TYPE_DEF=-DCMAKE_BUILD_TYPE:STRING

goto :print_defines

:print_defines
echo:
echo **************** SET ****************
echo MSVC_BAT = %MSVC_BAT%
echo CMAKE_EXE = %CMAKE_EXE%
echo NMAKE_EXE = %NMAKE_EXE%
echo C_GEN = %C_GEN%
echo C_INSTALL_PATH = %C_INSTALL_PATH%
echo C_TYPE_DEF = %C_TYPE_DEF%
echo **************** END ****************
pause
goto :check_cmake

:check_cmake
echo:
echo: checking For cmake:
%CMAKE_EXE% --version >nul 2>&1 && (
    echo: found: %CMAKE_EXE%
) || (
    echo: cannot find: %CMAKE_EXE%
    echo: please install: %CMAKE_EXE%
    goto :finish
)
goto :clean

:clean
echo:
echo: cleanup old install files...
cd /D %~dp0
IF EXIST "%INSTALL_DIR%" RMDIR /S/Q "%INSTALL_DIR%"
goto :core

:core
echo:
echo: Build Chaiscript
echo:
for %%a IN (x86_amd64 x86) DO (
  for %%b IN (debug release) DO (
    echo:
    echo: Building Chaiscript for: %%a in %%b mode.
    echo:

    echo: running %MSVC_BAT% %%a
    %MSVC_BAT% %%a

    IF EXIST "%BUILD_DIR%" RMDIR /S/Q "%BUILD_DIR%"

    echo: "mkdir build && cd build"
    mkdir build && cd build

    echo: %CMAKE_EXE% %C_GEN% %C_INSTALL_PATH%  %C_TYPE_DEF%="%%b" ..
    %CMAKE_EXE% %C_GEN% %C_INSTALL_PATH%  %C_TYPE_DEF%="%%b" ..
    @ECHO OFF

    for %%c IN (ALL TEST INSTALL PACKAGE CLEAN) DO (

      IF NOT %%c == PACKAGE (
        %NMAKE_EXE% %%c
      )
      IF %%c == PACKAGE IF %%b == release IF %%a == x86 (
        %NMAKE_EXE% %%c
        echo: 
        echo: Package file Created!  Please copy from build folder.
        echo: Build Folder deleted in next step!
        echo: 
        pause
      )
    )
    cd /D %~dp0
  )
)

IF EXIST "%BUILD_DIR%" RMDIR /S/Q "%BUILD_DIR%"

goto :done


:done
ECHO:
ECHO DONE!
ECHO:
goto :finish

:finish
cd /D %START_DIR%
pause
goto :eof
