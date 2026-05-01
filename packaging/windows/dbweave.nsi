; DB-WEAVE Qt 6 port Windows installer (NSIS).
;
; Assumes dist/windeployqt/ has been populated by packaging/windows/
; build_installer.ps1, which runs cmake --install, copies the built
; executable, runs `windeployqt dbweave.exe`, and drops LICENSE +
; the two PDFs alongside.
;
; Build with:
;     makensis dbweave.nsi
;
; Output: dbweave_setup.exe in the current directory.

!define APPNAME    "DB-WEAVE"
!define COMPANY    "Brunold Software"
; APPVER is supplied on the makensis command line by
; build_installer.ps1 (which reads it out of the top-level
; CMakeLists.txt -- the single source of truth for the project
; version). The fallback below only fires when the .nsi is built
; by hand without the helper script.
!ifndef APPVER
    !define APPVER "0.0.0"
!endif
!define EXE        "dbweave.exe"
!define SRCDIR     "..\..\dist\windeployqt"

Unicode true
Name "${APPNAME}"
OutFile "dbweave_setup.exe"
InstallDir "$PROGRAMFILES64\${APPNAME}"
InstallDirRegKey HKLM "Software\${COMPANY}\${APPNAME}" "Install_Dir"
RequestExecutionLevel admin
SetCompressor /SOLID lzma
VIProductVersion "${APPVER}.0"
VIAddVersionKey "ProductName"     "${APPNAME}"
VIAddVersionKey "CompanyName"     "${COMPANY}"
VIAddVersionKey "FileDescription" "DB-WEAVE weaving pattern designer"
VIAddVersionKey "FileVersion"     "${APPVER}"
VIAddVersionKey "LegalCopyright"  "GPL v3 or later"

Page directory
Page instfiles
UninstPage uninstConfirm
UninstPage instfiles

; -------------------- install ------------------------------------

Section "${APPNAME}"
    SectionIn RO
    SetOutPath "$INSTDIR"
    ; Everything windeployqt assembled: the exe, its DLLs, the Qt
    ; plugins tree, plus docs.
    File /r "${SRCDIR}\*"

    ; Registry anchor so the next install finds the existing dir.
    WriteRegStr HKLM "Software\${COMPANY}\${APPNAME}" "Install_Dir" "$INSTDIR"

    ; .dbw file association.
    WriteRegStr HKCR ".dbw" "" "DBWEAVE.Pattern"
    WriteRegStr HKCR "DBWEAVE.Pattern" "" "DB-WEAVE weaving pattern"
    WriteRegStr HKCR "DBWEAVE.Pattern\DefaultIcon" "" "$INSTDIR\${EXE},0"
    WriteRegStr HKCR "DBWEAVE.Pattern\shell\open\command" "" '"$INSTDIR\${EXE}" "%1"'

    ; Add/Remove Programs registration.
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APPNAME}" "DisplayName"     "${APPNAME}"
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APPNAME}" "DisplayVersion"  "${APPVER}"
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APPNAME}" "Publisher"       "${COMPANY}"
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APPNAME}" "UninstallString" '"$INSTDIR\uninstall.exe"'
    WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APPNAME}" "NoModify" 1
    WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APPNAME}" "NoRepair" 1
    WriteUninstaller "$INSTDIR\uninstall.exe"
SectionEnd

Section "Start Menu Shortcuts"
    SetShellVarContext all
    CreateDirectory "$SMPROGRAMS\${APPNAME}"
    CreateShortCut  "$SMPROGRAMS\${APPNAME}\${APPNAME}.lnk"  "$INSTDIR\${EXE}"
    CreateShortCut  "$SMPROGRAMS\${APPNAME}\Uninstall.lnk"   "$INSTDIR\uninstall.exe"
    IfFileExists "$INSTDIR\LICENSE" 0 +2
    CreateShortCut  "$SMPROGRAMS\${APPNAME}\License.lnk"     "$INSTDIR\LICENSE"
    IfFileExists "$INSTDIR\dbw_manual.pdf" 0 +2
    CreateShortCut  "$SMPROGRAMS\${APPNAME}\Manual.lnk"      "$INSTDIR\dbw_manual.pdf"
    IfFileExists "$INSTDIR\dbw_handbuch.pdf" 0 +2
    CreateShortCut  "$SMPROGRAMS\${APPNAME}\Handbuch.lnk"    "$INSTDIR\dbw_handbuch.pdf"
SectionEnd

Section "Desktop Shortcut"
    SetShellVarContext all
    CreateShortCut "$DESKTOP\${APPNAME}.lnk" "$INSTDIR\${EXE}"
SectionEnd

; -------------------- uninstall ----------------------------------

Section "Uninstall"
    SetShellVarContext all
    DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APPNAME}"
    DeleteRegKey HKLM "Software\${COMPANY}\${APPNAME}"
    DeleteRegKey HKCR ".dbw"
    DeleteRegKey HKCR "DBWEAVE.Pattern"

    Delete "$DESKTOP\${APPNAME}.lnk"
    Delete "$SMPROGRAMS\${APPNAME}\*.lnk"
    RMDir  "$SMPROGRAMS\${APPNAME}"

    ; windeployqt wrote many files. Blow away the install dir
    ; recursively; the user was warned on the UninstConfirm page.
    RMDir /r "$INSTDIR"
SectionEnd
