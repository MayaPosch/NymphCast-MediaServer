; Inno Setup script for NymphCast Media Server.
;
; Created 23 January 2022.
; Copyright (c) 2021 Nyanko.ws
;

#define MyAppName           "NymphCast Media Server"
#define MyAppNameNoSpace    "NymphCastMediaServer"
#define MyAppPublisher      "Nyanko"
#define MyAppPublisherURL   "http://www.nyanko.ws/"
#define MyAppContact        "info@nyanko.ws"
#define MyAppCopyright      "Copyright (C) 2019-2022, Nyanko"
#define MyAppURL            "http://nyanko.ws/nymphcast.php"
#define MyAppSupportURL     "https://github.com/MayaPosch/NymphCast-MediaServer/issues"
#define MyAppUpdatesURL     "https://github.com/MayaPosch/NymphCast-MediaServer/releases"
#define MyAppComments       "Media server for the NymphCast ecosystem."

#define MyAppBinFolder      "../bin/x86_64-w64-msvc/release/"

#define MyAppBaseName        MyAppNameNoSpace
#define MyAppExeName         MyAppBaseName + ".exe"
#define MyAppIconName        MyAppBaseName + ".ico"
#define MyAppHomeShortcut    MyAppBaseName + ".url"
#define MyAppExeSrcPath      MyAppBinFolder + MyAppExeName
#define MyAppExeDestName     MyAppNameNoSpace + ".exe"

#define MyAppReadme         "Readme.txt"
#define MyAppChanges        "Changes.txt"
#define MyAppLicenseFile    "Copying.txt"
#define MyAppInfoBeforeFile "InfoBefore.txt"
#define MyAppInfoAfterFile  "InfoAfter.txt"

#define NcFolderConfig      "folders.ini"

; Product version string is expected from a definition on
; the iscc commandline like: `-DMyAppVersion="v0.1[.2][-rc0-yyyymmdd]"`.

#ifndef MyAppVersion
#define MyAppVersion         "vx.x.x"
#endif

; Tasks:

#define NcConfigAutorunTask "Autorun NymphCast Media Server with Default Configuration"

; Paths for DLLs of dependencies to include:

#define VcpkgRoot            GetEnv('VCPKG_ROOT')
#define VcpkgDllFolder      "installed\x64-windows\bin"

; Download, extract VC redistributable if required:

; Paths for VC redistributable required:

#define VcRedistFile        "vc_redist.x64.exe"
#define VcRedistUrl         "https://aka.ms/vs/17/release/" + VcRedistFile
#define VcRedistMsg         "Installing Microsoft Visual C++ 14.1 RunTime..."

; Tool wget Expected in {NymphCast}/tools/
; wget.exe: https://eternallybored.org/misc/wget/

#define ToolPath            "../../NymphCast/tools/"

#define Wget                "wget.exe"
#define WgetPath             ToolPath + Wget
#define WgetMsgRedist       "Downloading VC redistributable"

[Setup]

; Require Windows 7 or newer:

MinVersion = 0,6.1

; The source paths are relative to the directory with this script.
; Place Setup-NymphCast-MediaServer-{version}.exe in directory with this script.
; Note: outputdir is relative to sourcedir:

SourceDir          = ./
OutputDir          = ./

; Application information:

; NOTE: The value of AppId uniquely identifies this application.
; Do not use the same AppId value in installers for other applications.
; (To generate a new GUID, click Tools | Generate GUID inside the IDE.)

AppId              = {{92521A58-BAB6-4C4C-8236-580D1E69DBDC}
AppName            = {#MyAppName}
AppVersion         = {#MyAppVersion}
;AppVerName        = {#MyAppName} {#MyAppVersion}
AppPublisher       = {#MyAppPublisher}
AppPublisherURL    = {#MyAppPublisherURL}
AppSupportURL      = {#MyAppSupportURL}
AppUpdatesURL      = {#MyAppUpdatesURL}
AppCopyright       = {#MyAppCopyright}

DefaultDirName     = {pf}\{#MyAppBaseName}
DefaultGroupName   = {#MyAppBaseName}
OutputBaseFilename = Setup-{#MyAppBaseName}-{#MyAppVersion}-dll

; Show welcome page [, license, pre- and postinstall information]:

DisableWelcomePage  = no
;LicenseFile        = {#MyAppLicenseFile}
;InfoBeforeFile     = {#MyAppInfoBeforeFile}
InfoAfterFile       = {#MyAppInfoAfterFile}

; Other:

DirExistsWarning    = yes
PrivilegesRequired  = none
Compression         = lzma
SolidCompression    = yes
ChangesAssociations = no
ChangesEnvironment  = no
; No update of other applications (explorer) needed

; Ensure 64-bit install:

ArchitecturesAllowed            = x64
ArchitecturesInstallIn64BitMode = x64

; Cosmetic:

;SetupIconFile        = {#MyAppIconName}
;UninstallDisplayIcon = {#MyAppIconName}
ShowComponentSizes    = yes
;ShowTasksTreeLines   = yes

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

[Tasks]
Name: "Autorun"        ; Description: "Autorun server"            ; GroupDescription: "{#NcConfigAutorunTask}"; Flags: unchecked
Name: "desktopicon"    ; Description: "{cm:CreateDesktopIcon}"    ; GroupDescription: "{cm:AdditionalIcons}"  ; Flags: unchecked
Name: "quicklaunchicon"; Description: "{cm:CreateQuickLaunchIcon}"; GroupDescription: "{cm:AdditionalIcons}"  ; Flags: unchecked; OnlyBelowVersion: 0,6.1

[Dirs]
Name: "{app}/bin"
Name: "{app}/config"

[Files]
; NOTE: Don't use "Flags: ignoreversion" on any shared system files

; Tools:
Source: "{#WgetPath}"; DestDir: "{tmp}"; Flags: deleteafterinstall;

; Program and DLLs of dependencies:

Source: ".\folders.ini"     ; DestDir: "{app}\config"; Flags: ignoreversion
Source: "{#MyAppExeSrcPath}"; DestDir: "{app}\bin"   ; DestName: "{#MyAppExeDestName}"; Flags: ignoreversion

Source: "{#VcpkgRoot}/{#VcpkgDllFolder}/libexpat.dll"      ; DestDir: "{app}/bin"   ; Flags: ignoreversion
Source: "{#VcpkgRoot}/{#VcpkgDllFolder}/libpng16.dll"      ; DestDir: "{app}/bin"   ; Flags: ignoreversion
Source: "{#VcpkgRoot}/{#VcpkgDllFolder}/pcre.dll"          ; DestDir: "{app}/bin"   ; Flags: ignoreversion
Source: "{#VcpkgRoot}/{#VcpkgDllFolder}/zlib1.dll"         ; DestDir: "{app}/bin"   ; Flags: ignoreversion
Source: "{#VcpkgRoot}/{#VcpkgDllFolder}/PocoFoundation.dll"; DestDir: "{app}/bin"   ; Flags: ignoreversion
Source: "{#VcpkgRoot}/{#VcpkgDllFolder}/PocoJSON.dll"      ; DestDir: "{app}/bin"   ; Flags: ignoreversion
Source: "{#VcpkgRoot}/{#VcpkgDllFolder}/PocoNet.dll"       ; DestDir: "{app}/bin"   ; Flags: ignoreversion
Source: "{#VcpkgRoot}/{#VcpkgDllFolder}/PocoUtil.dll"      ; DestDir: "{app}/bin"   ; Flags: ignoreversion
Source: "{#VcpkgRoot}/{#VcpkgDllFolder}/PocoXML.dll"       ; DestDir: "{app}/bin"   ; Flags: ignoreversion

[Icons]
Name: "{group}\{#NcFolderConfig}"    ; Filename: "{app}/config/{#NcFolderConfig}"
Name: "{group}\{#MyAppName} - Config"; Filename: "{%COMSPEC}"; Parameters: "/k """"{app}\bin\{#MyAppExeDestName}"" --folders ""{app}/config/{#NcFolderConfig}""" ; WorkingDir: "{autodocs}"; Comment: "Run NymphCast Media Server with config/folder.ini configuration.";
Name: "{commondesktop}\{#MyAppName}" ; Filename: "{%COMSPEC}"; Parameters: "/k """"{app}\bin\{#MyAppExeDestName}"" --folders ""{app}/config/{#NcFolderConfig}""" ; WorkingDir: "{autodocs}"; Comment: "Run NymphCast Media Server with config/folder.ini configuration."; Tasks: desktopicon
Name: "{userappdata}\Microsoft\Internet Explorer\Quick Launch\{#MyAppName}"; Filename: "{%COMSPEC}"; Parameters: "/k """"{app}\bin\{#MyAppExeDestName}"" --folders ""{app}/config/{#NcFolderConfig}""" ; WorkingDir: "{autodocs}"; Comment: "Run NymphCast Media Server with config/folder.ini configuration."; Tasks: quicklaunchicon

; {userstartup}, or {commonstartup}:
Name: "{userstartup}\{#MyAppName}"   ; Filename: "{%COMSPEC}"; Parameters: "/k """"{app}\bin\{#MyAppExeDestName}"" --folders ""{app}/config/{#NcFolderConfig}""" ; WorkingDir: "{autodocs}"; Comment: "Run NymphCast Media Server with config/folder.ini configuration."; Tasks: Autorun

[Run]

; If needed, download and install the Visual C++ runtime:
Filename: "{tmp}/{#Wget}"        ; Parameters: """{#VcRedistUrl}"""; WorkingDir: "{tmp}"; StatusMsg: "{#WgetMsgRedist}"; Check: IsWin64 and not VCinstalled
Filename: "{tmp}/{#VcRedistFile}"; Parameters: "/install /passive" ; WorkingDir: "{tmp}"; StatusMsg: "{#VcRedistMsg}"  ; Check: IsWin64 and not VCinstalled

; If requested, run NymphCast Media Server with [folder.ini] configuration:
Filename: "{%COMSPEC}"; Parameters: "/k """"{app}\bin\{#MyAppExeDestName}"" --folders ""{app}/config/{#NcFolderConfig}""" ; WorkingDir: "{autodocs}"; Description: "{cm:LaunchProgram,{#StringChange(MyAppName, '&', '&&')}}"; Flags: nowait postinstall skipifsilent

; Code to determine if installation of VC14.1 (VS2017) runtime is needed.
; From: http://stackoverflow.com/questions/11137424/how-to-make-vcredist-x86-reinstall-only-if-not-yet-installed/11172939#11172939
;       https://stackoverflow.com/a/45979466/437272

[Code]

function VCinstalled: Boolean;
 // Function for Inno Setup Compiler
 // Returns True if same or later Microsoft Visual C++ 2017 Redistributable is installed, otherwise False.
 var
  major: Cardinal;
  minor: Cardinal;
  bld: Cardinal;
  rbld: Cardinal;
  key: String;
 begin
  Result := False;
  key := 'SOFTWARE\Microsoft\VisualStudio\14.0\VC\Runtimes\x64';
  if RegQueryDWordValue(HKEY_LOCAL_MACHINE, key, 'Major', major) then begin
    if RegQueryDWordValue(HKEY_LOCAL_MACHINE, key, 'Minor', minor) then begin
      if RegQueryDWordValue(HKEY_LOCAL_MACHINE, key, 'Bld', bld) then begin
        if RegQueryDWordValue(HKEY_LOCAL_MACHINE, key, 'RBld', rbld) then begin
            Log('VC 2017 Redist Major is: ' + IntToStr(major) + ' Minor is: ' + IntToStr(minor) + ' Bld is: ' + IntToStr(bld) + ' Rbld is: ' + IntToStr(rbld));
            // Version info was found. Return true if later or equal to our 14.31.30818.00 redistributable
            // Note brackets required because of weird operator precendence
            Result := (major >= 14) and (minor >= 31) and (bld >= 30818) and (rbld >= 0)
        end;
      end;
    end;
  end;
 end;

(* End of file *)
