; ########################################################
; # Secret Maryo Chronicles Addon : Music NSIS Installer #
; # http://nsis.sourceforge.net                          #
; ########################################################

!define PRODUCT_NAME "Secret Maryo Chronicles Music Pack"
!define PRODUCT_VERSION "5.0"
!define PRODUCT_PUBLISHER "Florian Richter"
!define PRODUCT_WEB_PAGE "http://www.secretmaryo.org"
!define PRODUCT_DIR_REGKEY "Software\secretmaryo"
!define PRODUCT_UNINST_KEY "Software\Microsoft\Windows\CurrentVersion\Uninstall\secretmaryo_music"
!define PRODUCT_UNINST_ROOT_KEY "HKLM"

;----------- Configuration ---------------------

; Setup Filename
OutFile "SMC_Music_${PRODUCT_VERSION}_high_win32.exe"
; Installer Name
Name "${PRODUCT_NAME} ${PRODUCT_VERSION}"

; Should be like X.X.X.X
VIProductVersion        "${PRODUCT_VERSION}.0.0"
VIAddVersionKey         "FileDescription"       "${PRODUCT_NAME} Setup"
VIAddVersionKey         "ProductName"           "${PRODUCT_NAME}"
VIAddVersionKey         "FileVersion"           "${PRODUCT_VERSION}"
VIAddVersionKey         "ProductVersion"        "${PRODUCT_VERSION}"
VIAddVersionKey         "LegalCopyright"        "(C) The SMC Team"

; Set Compression
SetCompressor lzma
; Set Icon
Icon "icon.ico"

; default music installation directory
InstallDir "$PROGRAMFILES\Secret Maryo Chronicles"

; Registry key to check for the SMC directory
InstallDirRegKey HKLM "${PRODUCT_DIR_REGKEY}" "install_dir"

;----------- Include Modern UI ---------------------

!include "MUI.nsh"

;------------ Interface Settings --------------------

!define MUI_ICON ${NSISDIR}\Contrib\Graphics\Icons\orange-install.ico
!define MUI_UNICON ${NSISDIR}\Contrib\Graphics\Icons\orange-uninstall.ico
!define MUI_ABORTWARNING
!define MUI_HEADERIMAGE
!define MUI_HEADERIMAGE_BITMAP "installer_logo.bmp" ; optional

; Components
!define MUI_COMPONENTSPAGE_SMALLDESC
!define MUI_COMPONENTSPAGE_CHECKBITMAP ${NSISDIR}\Contrib\Graphics\Checks\simple-round2.bmp

;---------- Pages ----------------------

!insertmacro MUI_PAGE_LICENSE "..\..\..\docs\license.txt"
!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_INSTFILES

!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES

;----------- Languages ---------------------
; first language is the default language

!insertmacro MUI_LANGUAGE "English"

;---------- Functions ----------------------

; on init
Function .onInit
	Call Check_If_Installer_Runs
FunctionEnd


Function Check_If_Installer_Runs
	; create mutex to check if only only one installer runs
	System::Call 'kernel32::CreateMutexA(i 0, i 0, t "SMC_Mutex") i .r1 ?e'
	Pop $R0
 
	StrCmp $R0 0 +3
		MessageBox MB_OK|MB_ICONEXCLAMATION "Secret Maryo Chronicles installer is already running."
		Abort
FunctionEnd

;---------- install types ----------------------

InstType "Default"

;---------- The stuff to install ----------------------

; Music Addon
Section "!Secret Maryo Chronicles Addon : Music" Sec_Music

  	; required
  	SectionIn 1 RO
  
  	; Set output path to the installation data directory
  	SetOutPath "$INSTDIR\data"

  	; Installation Directories
  	File /r "..\..\..\data\music"

 	; Set output path to the installation docs directory
	SetOutPath "$INSTDIR\docs"
  	File "Addon - Music Readme.txt"

	; Set output path to the installation directory.
	SetOutPath "$INSTDIR"

  	; Write the installation path into the registry
  	WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} SOFTWARE\secretmaryo\music "install_dir" "$INSTDIR"
	WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} SOFTWARE\secretmaryo\music "version" "${PRODUCT_VERSION}"
  
  	; Write the uninstall keys for Windows
  	WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "DisplayName" "${PRODUCT_NAME}"
  	WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "UninstallString" '"$INSTDIR\uninstall_music.exe"'
	WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "DisplayIcon" "$INSTDIR\Secret Maryo Chronicles.exe"
	WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "DisplayVersion" "${PRODUCT_VERSION}"
	WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "URLInfoAbout" "${PRODUCT_WEB_PAGE}"
	WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "Publisher" "${PRODUCT_PUBLISHER}"
  	WriteRegDWORD ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "NoModify" 1
  	WriteRegDWORD ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "NoRepair" 1
  	WriteUninstaller "uninstall_music.exe"

SectionEnd

;------------- Descriptions -------------------

;Assign language strings to sections
!insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
	!insertmacro MUI_DESCRIPTION_TEXT ${Sec_Music} "Music files (required)"
!insertmacro MUI_FUNCTION_DESCRIPTION_END

;------------ Uninstaller --------------------


Section "Uninstall"
  
	; Delete Music
	Delete "$INSTDIR\data\music\game\*.ogg"
	Delete "$INSTDIR\data\music\game\*.txt"
	Delete "$INSTDIR\data\music\land\*.ogg"
	Delete "$INSTDIR\data\music\land\*.txt"
	Delete "$INSTDIR\data\music\overworld\*.ogg"
	Delete "$INSTDIR\data\music\overworld\*.txt"
	Delete "$INSTDIR\data\music\story\*.ogg"
	Delete "$INSTDIR\data\music\story\*.txt"
	RMDir "$INSTDIR\data\music\game"
	RMDir "$INSTDIR\data\music\land"
	RMDir "$INSTDIR\data\music\overworld"
	RMDir "$INSTDIR\data\music\story"
	RMDir "$INSTDIR\data\music"

	; Remove registry keys
	DeleteRegKey ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}"
	DeleteRegKey ${PRODUCT_UNINST_ROOT_KEY} SOFTWARE\secretmaryo\music

	; Delete Music Readme
	Delete "$INSTDIR\docs\Addon - Music Readme.txt"

	; Remove uninstaller
	Delete $INSTDIR\uninstall_music.exe

	SetAutoClose true
  
SectionEnd
