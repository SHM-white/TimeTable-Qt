; Compile after windeployqt. All paths and the application version are supplied
; by the build; this script never references a developer's machine.
#ifndef AppVersion
  #error AppVersion must be supplied with /DAppVersion=major.minor.patch
#endif
#ifndef SourceDir
  #error SourceDir must point to the deployed application directory
#endif
#ifndef OutputDir
  #error OutputDir must point to the installer output directory
#endif
#ifdef Qt5Legacy
  #define QtMajor "5"
#else
  #define QtMajor "6"
#endif
#define AppName "TimeTable Next"
#define AppExeName "TimeTableNext.exe"
#if !FileExists(SourceDir + "\\" + AppExeName)
  #error SourceDir does not contain TimeTableNext.exe; deploy the runtime first
#endif
#if !FileExists(SourceDir + "\\Qt" + QtMajor + "Core.dll")
  #error SourceDir does not contain the selected Qt runtime; run windeployqt first
#endif
#if !FileExists(SourceDir + "\\platforms\\qwindows.dll")
  #error SourceDir does not contain the Windows platform plugin
#endif

[Setup]
; Both Qt builds upgrade the same modern application, separate from legacy v3.
AppId={{6B09981C-BF0E-4B25-A33F-736E463E8C44}
AppName={#AppName}
AppVersion={#AppVersion}
AppPublisher=TimeTable-Qt
AppPublisherURL=https://github.com/SHM-white/TimeTable-Qt
VersionInfoVersion={#AppVersion}
DefaultDirName={localappdata}\Programs\TimeTableNext
DefaultGroupName={#AppName}
DisableProgramGroupPage=yes
PrivilegesRequired=lowest
ArchitecturesAllowed=x64
ArchitecturesInstallIn64BitMode=x64
#ifdef Qt5Legacy
; Runtime compatibility remains a Win7 SP1 candidate, not a verified guarantee.
MinVersion=6.1sp1
#else
; Qt 6.8 supports Windows 10 1809 and newer.
MinVersion=10.0.17763
#endif
OutputDir={#OutputDir}
OutputBaseFilename=TimeTableNext-{#AppVersion}-Qt{#QtMajor}-Windows-x64-Setup
Compression=lzma2
SolidCompression=yes
WizardStyle=modern
CloseApplications=yes
RestartApplications=no
SetupLogging=yes

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked
Name: "startup"; Description: "{cm:StartAtLogon}"; Flags: unchecked

[CustomMessages]
english.StartAtLogon=Start TimeTable Next when I sign in to Windows

[Files]
Source: "{#SourceDir}\*"; DestDir: "{app}"; Flags: ignoreversion recursesubdirs createallsubdirs

[Icons]
Name: "{autoprograms}\{#AppName}"; Filename: "{app}\{#AppExeName}"
Name: "{autodesktop}\{#AppName}"; Filename: "{app}\{#AppExeName}"; Tasks: desktopicon

[Registry]
; Match the application's own HKCU Run entry. Uninstall cleanup below checks
; ownership, so switching to a portable executable does not lose its Run entry.
Root: HKCU; Subkey: "Software\Microsoft\Windows\CurrentVersion\Run"; ValueType: string; ValueName: "TimeTableNext"; ValueData: """{app}\{#AppExeName}"""; Tasks: startup

[Run]
Filename: "{app}\{#AppExeName}"; Description: "{cm:LaunchProgram,{#StringChange(AppName, '&', '&&')}}"; Flags: nowait postinstall skipifsilent

[Code]
procedure CurUninstallStepChanged(CurUninstallStep: TUninstallStep);
var
  StartupValue: String;
begin
  if CurUninstallStep = usUninstall then
  begin
    if RegQueryStringValue(HKCU,
      'Software\Microsoft\Windows\CurrentVersion\Run',
      'TimeTableNext', StartupValue) then
    begin
      if CompareText(StartupValue,
        '"' + ExpandConstant('{app}\{#AppExeName}') + '"') = 0 then
        RegDeleteValue(HKCU,
          'Software\Microsoft\Windows\CurrentVersion\Run', 'TimeTableNext');
    end;
  end;
end;

; No user-data files or QSettings registry keys are removed on uninstall.
