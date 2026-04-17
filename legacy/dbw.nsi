; dbw.nsi

;--------------------------------

; The name of the installer
Name "DB-WEAVE"

LoadLanguageFile "${NSISDIR}\Contrib\Language files\English.nlf"
LoadLanguageFile "${NSISDIR}\Contrib\Language files\German.nlf"

; The file to write
OutFile "dbweave_setup.exe"

; The default installation directory
InstallDir $PROGRAMFILES\DB-WEAVE

; Registry key to check for directory (so if you install again, it will 
; overwrite the old one automatically)
InstallDirRegKey HKLM "Software\DBW" "Install_Dir"

;--------------------------------

; Pages

Page directory
Page instfiles

UninstPage uninstConfirm
UninstPage instfiles

;--------------------------------

; The stuff to install
Section "DB-WEAVE"

  SectionIn RO
  
  ; Set output path to the installation directory.
  SetOutPath $INSTDIR
  
  ; Put file there
  File dbw.exe
  File license.txt
  File manual.pdf
  File handbuch.pdf
  File source.zip
  
  ; Write the installation path into the registry
  WriteRegStr HKLM SOFTWARE\DBWEAVE "Install_Dir" "$INSTDIR"
  
  ; Write the file type registration
  WriteRegStr HKCR .dbw "" "DBWEAVE"
  WriteRegStr HKCR "DBWEAVE\shell\open\command" "" '"$INSTDIR\dbw.exe" "%1"'
  WriteRegStr HKCR "DBWEAVE\shell\print\command" "" '"$INSTDIR\dbw.exe" "/p" "%1"'

  ; Write the uninstall keys for Windows
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\DBWEAVE" "DisplayName" "DB-WEAVE"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\DBWEAVE" "UninstallString" '"$INSTDIR\uninstall.exe"'
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\DBWEAVE" "NoModify" 1
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\DBWEAVE" "NoRepair" 1
  WriteUninstaller "uninstall.exe"
  
SectionEnd

; Optional section (can be disabled by the user)
Section "Start Menu Shortcuts"

  SetShellVarContext current

  CreateDirectory "$SMPROGRAMS\DB-WEAVE"
  CreateShortCut "$SMPROGRAMS\DB-WEAVE\Uninstall.lnk" "$INSTDIR\uninstall.exe" "" "$INSTDIR\uninstall.exe" 0
  CreateShortCut "$SMPROGRAMS\DB-WEAVE\License.lnk" "$INSTDIR\license.txt" "" "$INSTDIR\license.txt" 0
  CreateShortCut "$SMPROGRAMS\DB-WEAVE\Manual.lnk" "$INSTDIR\manual.pdf" "" "$INSTDIR\manual.pdf" 0
  CreateShortCut "$SMPROGRAMS\DB-WEAVE\Handbuch.lnk" "$INSTDIR\handbuch.pdf" "" "$INSTDIR\handbuch.pdf" 0
  CreateShortCut "$SMPROGRAMS\DB-WEAVE\DB-WEAVE.lnk" "$INSTDIR\dbw.exe" "" "$INSTDIR\dbw.exe" 0
  CreateShortCut "$DESKTOP\DB-WEAVE.lnk" "$INSTDIR\dbw.exe" "" "$INSTDIR\dbw.exe" 0
  
SectionEnd

;--------------------------------

; Uninstaller

Section "Uninstall"
  
  SetShellVarContext current

  ; Remove registry keys
  DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\DBWEAVE"
  DeleteRegKey HKLM SOFTWARE\DBWEAVE
  DeleteRegKey HKCR ".dbw"
  DeleteRegKey HKCR "DBWEAVE"

  ; Remove files and uninstaller
  Delete $INSTDIR\dbw.exe

  Delete $INSTDIR\handbuch.pdf

  Delete $INSTDIR\manual.pdf
  Delete $INSTDIR\license.txt
  Delete $INSTDIR\source.zip
  Delete $INSTDIR\uninstall.exe

  ; Remove shortcuts, if any
  Delete "$SMPROGRAMS\DB-WEAVE\Uninstall.lnk"
  Delete "$SMPROGRAMS\DB-WEAVE\License.lnk"
  Delete "$SMPROGRAMS\DB-WEAVE\Manual.lnk"
  Delete "$SMPROGRAMS\DB-WEAVE\Handbuch.lnk"
  Delete "$SMPROGRAMS\DB-WEAVE\DB-WEAVE.lnk"
  Delete "$DESKTOP\DB-WEAVE.lnk"

  ; Remove directories used
  RMDir "$SMPROGRAMS\DB-WEAVE"
  RMDir "$INSTDIR"

SectionEnd
