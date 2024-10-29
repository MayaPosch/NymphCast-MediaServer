@echo off & setlocal enableextensions enabledelayedexpansion
::
:: Setup prerequisites and build NymphCast Media Server command line client (MSVC).
::
:: Created 23 January 2022.
:: Copyright (c) 2021 Nyanko.ws
::
:: Usage: Setup-NMake-vcpkg [LIBNYMPHCAST_ROOT=path/to/lib] [NYMPHRPC_ROOT=path/to/lib] [POCO_ROOT=path/to/lib] [target]
::

:: Install vcpkg tool:
:: > git clone https://github.com/microsoft/vcpkg /path/to/vcpkg-folder
:: > .\vcpkg-folder\bootstrap-vcpkg.bat -disableMetrics
:: > set VCPKG_ROOT=/path/to/vcpkg-folder
::

:: Note: For most flexibility, "quote on use" is generally used and quotes in variable assignment avoided.
:: For example:
:: - `set var=val ue` // Note: no spaces around `=`
:: - `echo "%var%"`

echo.

set INSTALL_PREFIX=D:\Programs\NymphCastMediaServer

:: Note: static building does not yet work.
set NC_STATIC=0
:: set NC_STATIC=1

set NC_CONFIG=Release
:: set NC_CONFIG=Debug

set NCS_TGT_BITS=64
set NCS_TGT_ARCH=x%NCS_TGT_BITS%

set NC_LNKCRT=-MD
set VCPKG_TRIPLET=x64-windows

if [%NC_STATIC%] == [1] (
    set NC_LNKCRT=-MT
    set VCPKG_TRIPLET=x64-windows-static
    echo [Setup NCMS: static build does not yet work. Continuing.]
)

:: Check for 64-bit Native Tools Command Prompt

if not [%VSCMD_ARG_TGT_ARCH%] == [%NCS_TGT_ARCH%] (
    echo [Setup NCMS: Make sure to run these commands in a '64-bit Native Tools Command Prompt'; expecting 'x64', got '%VSCMD_ARG_TGT_ARCH%'. Bailing out.]
    endlocal & goto :EOF
)

:: Check for vcpkg:

set vcpkg=%VCPKG_ROOT%\vcpkg.exe

if "[%VCPKG_ROOT%"] == [""] (
    echo [Setup NCMS: Make sure environment variable 'VCPKG_ROOT' points to your vcpkg installation; it's empty or does not exist. Bailing out.]
    endlocal & goto :EOF
)

:: NymphRPC and LibNymphCast libraries:

if ["%NYMPHRPC_ROOT%"] == [""] (
    set NYMPHRPC_ROOT=D:\Libraries\NymphRPC
)

if ["%LIBNYMPHCAST_ROOT%"] == [""] (
    set LIBNYMPHCAST_ROOT=D:\Libraries\LibNymphCast
)

:: Make sure NymphRPC and LibNymphCast will be build with the same Poco version:

:: TODO check for proper lib, using NC_LNKCRT_MT (to be added above): mt or no mt

:: Poco[core]:

if exist "%VCPKG_ROOT%\installed\%VCPKG_TRIPLET%\include\Poco" (
    echo Setup NCMS: Poco is already installed at "%VCPKG_ROOT%\installed\%VCPKG_TRIPLET%\include\Poco".
) else (
    echo Installing vcpkg Poco; please be patient, this may take about 10 minutes...
    "%vcpkg%" install --triplet %VCPKG_TRIPLET% poco
)

echo Setup NCMS: Using POCO_ROOT=%VCPKG_ROOT%\installed\%VCPKG_TRIPLET%

set POCO_ROOT=%VCPKG_ROOT%\installed\%VCPKG_TRIPLET%

:: NymphRPC - Download and build NymphRPC dependency:

if exist "%NYMPHRPC_ROOT%\include\nymph\nymph.h" (
    echo Setup NCMS: NymphRPC has been installed at "%NYMPHRPC_ROOT%".
) else (
    echo Setup NCMS: NymphRPC not found at "%NYMPHRPC_ROOT%".

    git clone --depth 1 https://github.com/MayaPosch/NymphRPC.git

    cd NymphRPC & call Setup-NMake-vcpkg.bat & cd ..

    rmdir /s /q NymphRPC
)

:: LibNymphCast - Download and build LibNymphCast dependency:

if exist "%LIBNYMPHCAST_ROOT%\include\nymphcast_client.h" (
    echo Setup NCMS: LibNymphCast has been installed at "%LIBNYMPHCAST_ROOT%".
) else (
    echo Setup NCMS: LibNymphCast not found at "%LIBNYMPHCAST_ROOT%".

    git clone --depth 1 https://github.com/MayaPosch/LibNymphCast.git

    cd LibNymphCast & call Setup-NMake-vcpkg.bat & cd ..

    rmdir /s /q LibNymphCast
)

:: Finally, build NymphCast Media Server:

nmake -nologo -f NMakefile ^
         NC_STATIC=%NC_STATIC% ^
         NC_CONFIG=%NC_CONFIG% ^
         NC_LNKCRT=%NC_LNKCRT% ^
         POCO_ROOT="%POCO_ROOT%" ^
     NYMPHRPC_ROOT="%NYMPHRPC_ROOT%" ^
 LIBNYMPHCAST_ROOT="%LIBNYMPHCAST_ROOT%" ^
    INSTALL_PREFIX="%INSTALL_PREFIX%" ^
        %*

echo.

endlocal

:: End of file
