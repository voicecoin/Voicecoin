; -- Example1.iss --
; Demonstrates copying 3 files and creating an icon.

; SEE THE DOCUMENTATION FOR DETAILS ON CREATING .ISS SCRIPT FILES!

[Setup]
AppName=voicecoin
AppVersion=1.0
DefaultDirName={pf}\voicecoin
DefaultGroupName=voicecoin
UninstallDisplayIcon={app}\voicecoin-qt.exe
Compression=lzma2
SolidCompression=yes
OutputDir=./

[Files]
Source: "voicecoin-cli.exe"; DestDir: "{app}"
Source: "voicecoin-tx.exe"; DestDir: "{app}"
Source: "voicecoin-tx.exe"; DestDir: "{app}"
Source: "voicecoin-qt.exe"; DestDir: "{app}"

[Icons]
Name: "{group}\voicecoin"; Filename: "{app}\voicecoin-qt.exe"
