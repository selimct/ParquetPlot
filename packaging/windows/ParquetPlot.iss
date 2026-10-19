; Compile with ISCC /DAppVersion=<version> /DAppSourceDir=<bundle> /O<output> ParquetPlot.iss
#ifndef AppVersion
  #error AppVersion must be supplied to the compiler
#endif
#ifndef AppSourceDir
  #error AppSourceDir must point to the bundled application directory
#endif

[Setup]
; Keep this ID stable so future installers upgrade the existing installation.
AppId={{FEFE138C-03CA-4777-94C7-06CD60A57628}
AppName=ParquetPlot
AppVersion={#AppVersion}
DefaultDirName={autopf}\ParquetPlot
PrivilegesRequired=admin
ArchitecturesAllowed=x64compatible
ArchitecturesInstallIn64BitMode=x64compatible
MinVersion=10.0
DisableProgramGroupPage=yes
UninstallDisplayName=ParquetPlot
UninstallDisplayIcon={app}\ParquetPlot.exe
LicenseFile={#AppSourceDir}\LICENSE
OutputBaseFilename=ParquetPlot-Windows-x64-Setup
Compression=lzma2
SolidCompression=yes
WizardStyle=modern
ChangesAssociations=yes

[Tasks]
Name: "desktopicon"; Description: "Create a &desktop shortcut"; GroupDescription: "Shortcuts:"; Flags: unchecked

[Files]
Source: "{#AppSourceDir}\*"; DestDir: "{app}"; Flags: ignoreversion recursesubdirs createallsubdirs

[Icons]
Name: "{commonprograms}\ParquetPlot"; Filename: "{app}\ParquetPlot.exe"; WorkingDir: "{app}"
Name: "{commondesktop}\ParquetPlot"; Filename: "{app}\ParquetPlot.exe"; WorkingDir: "{app}"; Tasks: desktopicon

[Registry]
; Register as an Open With option without taking over the user's default app.
Root: HKLM; Subkey: "Software\Classes\.parquet\OpenWithProgids"; ValueType: string; ValueName: "ParquetPlot.Parquet"; ValueData: ""; Flags: uninsdeletevalue uninsdeletekeyifempty
Root: HKLM; Subkey: "Software\Classes\ParquetPlot.Parquet"; ValueType: string; ValueData: "Parquet data file"; Flags: uninsdeletekey
Root: HKLM; Subkey: "Software\Classes\ParquetPlot.Parquet\DefaultIcon"; ValueType: string; ValueData: """{app}\ParquetPlot.exe"",0"
Root: HKLM; Subkey: "Software\Classes\ParquetPlot.Parquet\shell\open\command"; ValueType: string; ValueData: """{app}\ParquetPlot.exe"" ""%1"""
Root: HKLM; Subkey: "Software\Microsoft\Windows\CurrentVersion\App Paths\ParquetPlot.exe"; ValueType: string; ValueData: "{app}\ParquetPlot.exe"; Flags: uninsdeletekey

[Run]
Filename: "{app}\ParquetPlot.exe"; Description: "Launch ParquetPlot"; Flags: nowait postinstall skipifsilent runasoriginaluser
