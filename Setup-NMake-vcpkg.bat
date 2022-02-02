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
:: > git clone https://github.com/microsoft/vcpkg
:: > .\vcpkg\bootstrap-vcpkg.bat -disableMetrics
::

echo.

set NC_LNKCRT=-MD
::set NC_LNKCRT=-MT

set NCS_TGT_BITS=64
set NCS_TGT_ARCH=x%NCS_TGT_BITS%

:: Select static/dynamic linking

:: Check for 64-bit Native Tools Command Prompt

if not [%VSCMD_ARG_TGT_ARCH%] == [x64] (
    echo [Make sure to run these commands in a '64-bit Native Tools Command Prompt'; expecting 'x64', got '%VSCMD_ARG_TGT_ARCH%'. Bailing out.]
    endlocal & goto :EOF
)

if [%VCPKG_ROOT%] == [] (
    echo [Make sure to environment variable 'VCPKG_ROOT' point to you vcpkg installation; it's empty or does not exist. Bailing out.]
    endlocal & goto :EOF
)

if [%NYMPHRPC_ROOT%] == [] (
    set NYMPHRPC_ROOT=D:\Libraries\NymphRPC
)

if [%LIBNYMPHCAST_ROOT%] == [] (
    set LIBNYMPHCAST_ROOT=D:\Libraries\LibNymphCast
)

:: Make sure NymphRPC and LibNymphCast will be build with the same Poco version:

set VCPKG_TRIPLET=x64-windows
:: set VCPKG_TRIPLET=x64-windows-static

:: TODO check for proper lib, using NC_LNKCRT_MT (to be added above): mt or no mt

:: Poco[core]:

if exist "%VCPKG_ROOT%\installed\%VCPKG_TRIPLET%\include\Poco" (
    echo Setup NCMS: Poco is already installed at "%VCPKG_ROOT%\installed\%VCPKG_TRIPLET%\include\Poco".
) else (
    echo Installing vcpkg Poco; please be patient, this may take about 10 minutes...
    vcpkg install --triplet %VCPKG_TRIPLET% poco
)

echo Setup NCMS: Using POCO_ROOT=%VCPKG_ROOT%\installed\%VCPKG_TRIPLET%

set POCO_ROOT=%VCPKG_ROOT%\installed\%VCPKG_TRIPLET%

:: NymphRPC - Download and build NymphRPC dependency:

if exist "%NYMPHRPC_ROOT%\include\nymph\nymph.h" (
    echo Setup NCMS: NymphRPC has been installed at "%NYMPHRPC_ROOT%".
) else (
    echo Setup NCMS: NymphRPC not found at "%NYMPHRPC_ROOT%".

    git clone --depth 1 https://github.com/MayaPosch/NymphRPC.git

    :: TODO temporary: Setup for NympRPC not yet in repository.
    if exist "D:\Own\Martin" (
        xcopy /y ..\MartinMoene\NymphCast-Scenarios\code\NMake\NymphRPC\NMakefile NymphRPC
        xcopy /y ..\MartinMoene\NymphCast-Scenarios\code\NMake\NymphRPC\Setup-NMake-vcpkg.bat NymphRPC
    )

    cd NymphRPC & call Setup-NMake-vcpkg.bat & cd ..

    rmdir /s /q NymphRPC
)

:: LibNymphCast - Download and build LibNymphCast dependency:

if exist "%LIBNYMPHCAST_ROOT%\include\nymphcast_client.h" (
    echo Setup NCMS: LibNymphCast has been installed at "%LIBNYMPHCAST_ROOT%".
) else (
    echo Setup NCMS: LibNymphCast not found at "%LIBNYMPHCAST_ROOT%".

    git clone --depth 1 https://github.com/MayaPosch/LibNymphCast.git

    :: TODO temporary: Setup for LibNymphCast not yet in repository.
    if exist "D:\Own\Martin" (
        xcopy /y ..\MartinMoene\NymphCast-Scenarios\code\NMake\LibNymphCast\NMakefile LibNymphCast
        xcopy /y ..\MartinMoene\NymphCast-Scenarios\code\NMake\LibNymphCast\Setup-NMake-vcpkg.bat LibNymphCast
    )

    cd LibNymphCast & call Setup-NMake-vcpkg.bat & cd ..

    rmdir /s /q LibNymphCast
)

:: Finally, build NymphCast Media Server:

nmake -nologo -f NMakefile ^
         NC_LNKCRT=%NC_LNKCRT% ^
         POCO_ROOT=%POCO_ROOT% ^
     NYMPHRPC_ROOT=%NYMPHRPC_ROOT% ^
 LIBNYMPHCAST_ROOT=%LIBNYMPHCAST_ROOT% ^
        %*

echo.

endlocal

:: End of file
