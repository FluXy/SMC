; ##########################################
; # Secret Maryo Chronicles NSIS Installer #
; # http://nsis.sourceforge.net            #
; ##########################################

!define PRODUCT_NAME "Secret Maryo Chronicles"
!define PRODUCT_VERSION "2.0"
!define PRODUCT_PUBLISHER "Florian Richter"
!define PRODUCT_WEB_PAGE "http://www.secretmaryo.org"
!define PRODUCT_DIR_REGKEY "Software\secretmaryo"
!define PRODUCT_UNINST_KEY "Software\Microsoft\Windows\CurrentVersion\Uninstall\secretmaryo"
!define PRODUCT_UNINST_ROOT_KEY "HKLM"

;----------- Configuration ------------------------------------------

; Setup Filename
OutFile "SMC_${PRODUCT_VERSION}_win32.exe"
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

; default installation directory
InstallDir "$PROGRAMFILES\${PRODUCT_NAME}"

; Registry key to check for directory ( if you install again, it overwrites the old one automatically )
InstallDirRegKey HKLM "${PRODUCT_DIR_REGKEY}" "install_dir"

; Request admin privileges for Windows Vista
RequestExecutionLevel admin

;----------- Includes ------------------------------------------

;Modern UI
!include "MUI.nsh"
;nsDialogs for custom pages
!include nsDialogs.nsh
!include LogicLib.nsh

;------------ Interface Settings ------------------------------------------

!define MUI_ICON ${NSISDIR}\Contrib\Graphics\Icons\orange-install.ico
!define MUI_UNICON ${NSISDIR}\Contrib\Graphics\Icons\orange-uninstall.ico
!define MUI_ABORTWARNING
!define MUI_HEADERIMAGE
!define MUI_HEADERIMAGE_BITMAP "installer_logo.bmp" ; optional

; Language
!define MUI_LANGDLL_ALWAYSSHOW
!define MUI_LANGDLL_REGISTRY_ROOT         "HKLM"
!define MUI_LANGDLL_REGISTRY_KEY          ${PRODUCT_DIR_REGKEY}
!define MUI_LANGDLL_REGISTRY_VALUENAME    "installer_language"
 
; Components
!define MUI_COMPONENTSPAGE_SMALLDESC
!define MUI_COMPONENTSPAGE_CHECKBITMAP ${NSISDIR}\Contrib\Graphics\Checks\simple-round2.bmp

;------------- Reserve Files ------------------------------------------

;These files should be inserted before other files in the data block
;Keep these lines before any File command
;Only for solid compression (by default, solid compression is enabled for BZIP2 and LZMA)

;Language selection dialog
!insertmacro MUI_RESERVEFILE_LANGDLL

;-------------- Variables ------------------------------------------

Var Custom_Page_Dialog
Var Custom_Page_Label_Webpage
Var Custom_Page_Label_Controls
Var Custom_Page_Checkbox_Controls
Var Checkbox_State

;---------- Pages ------------------------------------------

!insertmacro MUI_PAGE_LICENSE "..\..\..\docs\license.txt"
!insertmacro MUI_PAGE_COMPONENTS
!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_INSTFILES
Page custom Custom_Page_Enter Custom_Page_Leave

!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES

;----------- Languages ------------------------------------------
; first language is the default language

!insertmacro MUI_LANGUAGE "English"
!insertmacro MUI_LANGUAGE "German"

;----------- Translations ------------------------------------------

!define SMC_DEFAULT_LANGFILE "translations\English.nsh"
!include "langmacros.nsh"

!insertmacro SMC_MACRO_INCLUDE_LANGFILE "ENGLISH"	"translations\english.nsh"
!insertmacro SMC_MACRO_INCLUDE_LANGFILE "GERMAN"	"translations\german.nsh"
  
;---------- Functions ------------------------------------------

; on init
Function .onInit
	Call Check_If_Installer_Runs
	Call Check_If_Already_Installed
 
	; display language selection dialog
	!insertmacro MUI_LANGDLL_DISPLAY
FunctionEnd


Function Check_If_Installer_Runs
	; create mutex to check if only only one installer runs
	System::Call 'kernel32::CreateMutexA(i 0, i 0, t "SMC_Mutex") i .r1 ?e'
	Pop $R0
 
	StrCmp $R0 0 +3
		MessageBox MB_OK|MB_ICONEXCLAMATION $(SMC_INSTALLER_IS_RUNNING)
		Abort
FunctionEnd


Function Check_If_Already_Installed
	ReadRegStr $R0 ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "UninstallString"
	
	;Run the uninstaller
	StrCmp $R0 "" skip_uninstall
		MessageBox MB_YESNO|MB_ICONEXCLAMATION $(SMC_ALREADY_INSTALLED) IDNO skip_uninstall
		ClearErrors
		;Don't copy the uninstaller to a temp file
		ExecWait '$R0 _?=$INSTDIR'
	skip_uninstall:
FunctionEnd


Function Custom_Page_Enter
	; Init
	nsDialogs::Create 1018
	; Assign
	Pop $Custom_Page_Dialog
	; Check if valid
	${If} $Custom_Page_Dialog == error
		Abort
	${EndIf}
	
	; Create the dialog
	
	; Webpage Label
	${NSD_CreateLabel} 0 0 100% 24u "$(SMC_FINISH_WEB_PAGE_INFO)"
	Pop $Custom_Page_Label_Webpage

	; Controls Label
	${NSD_CreateLabel} 0 50u 100% 10u "$(SMC_FINISH_VEW_CONTROLS_INFO)"
	Pop $Custom_Page_Label_Controls
	; Controls Checkbox
	${NSD_CreateCheckbox} 0 61u 100% 10u "$(SMC_FINISH_VEW_CONTROLS)"
	Pop $Custom_Page_Checkbox_Controls
	; Checked
	${NSD_SetState} $Custom_Page_Checkbox_Controls ${BST_CHECKED}

	; Show
	nsDialogs::Show
FunctionEnd


Function Custom_Page_Leave
	; Open Controls if checked
	${NSD_GetState} $Custom_Page_Checkbox_Controls $Checkbox_State
	${If} $Checkbox_State == ${BST_CHECKED}
		ExecShell "open" "docs\controls.html"
	${EndIf}
	
	; Visit donate webpage if online
	Push $R0
	ClearErrors
	Dialer::GetConnectedState
	; if error IE3 is not installed
	IfErrors not_connected
	
    Pop $R0
    StrCmp $R0 "online" connected
		Goto not_connected
	
    connected:
		ExecShell "open" "${PRODUCT_WEB_PAGE}/?page=donate"
			
  	not_connected:
	
	Pop $R0
	
FunctionEnd


; on uninstaller init
Function un.onInit
	!insertmacro MUI_UNGETLANGUAGE
FunctionEnd

;---------- install types ------------------------------------------

InstType "Default"

;---------- The stuff to install ------------------------------------------

; Main Game
Section $(SMC_CORE_SECTION_TITLE) Sec_SMC

	; required
	SectionIn 1 RO

	; Set output path to the installation directory.
	SetOutPath "$INSTDIR"

	; Install VC++ 9 SP1 runtime
	File "..\vcredist_2008_SP1_x86.exe"
	ExecWait "$INSTDIR\vcredist_2008_SP1_x86.exe /q"
	Delete "$INSTDIR\vcredist_2008_SP1_x86.exe"

	; Create Directories

	; Installation Directories
	File /r "..\..\..\data"
	File /r "..\..\..\docs"
	; Installation Files
	File "..\..\..\Secret Maryo Chronicles.exe"
	File /r "..\..\..\*.dll"
	File /r "..\..\..\*.xsd"

	; Write the installation path into the registry
	WriteRegStr HKLM "${PRODUCT_DIR_REGKEY}" "install_dir" "$INSTDIR"
	WriteRegStr HKLM "${PRODUCT_DIR_REGKEY}" "version" "${PRODUCT_VERSION}"

	; Write the uninstall keys for Windows
	WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "DisplayName" "${PRODUCT_NAME}"
	WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "UninstallString" '"$INSTDIR\uninstall.exe"'
	WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "DisplayIcon" "$INSTDIR\Secret Maryo Chronicles.exe"
	WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "DisplayVersion" "${PRODUCT_VERSION}"
	WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "URLInfoAbout" "${PRODUCT_WEB_PAGE}"
	WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "Publisher" "${PRODUCT_PUBLISHER}"
	WriteRegDWORD ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "NoModify" 1
	WriteRegDWORD ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "NoRepair" 1
	WriteUninstaller "uninstall.exe"

SectionEnd

; Source Files
Section $(SMC_SOURCE_CODE_SECTION_TITLE) Sec_Source

	; Set output path to the installation directory.
	SetOutPath "$INSTDIR"

	; Installation Directories
	File /r "..\..\..\makefiles"
	File /r "..\..\..\src"
  
SectionEnd

; Optional Start Menu Shortcuts
Section $(SMC_STARTMENU_SHORTCUT_SECTION_TITLE) Sec_Menu_Shortcut

	; default is enabled
	SectionIn 1

	CreateDirectory "$SMPROGRAMS\${PRODUCT_NAME}"
	CreateShortCut "$SMPROGRAMS\${PRODUCT_NAME}\$(SMC_SHORTCUT_UNINSTALL).lnk" "$INSTDIR\uninstall.exe" "" "$INSTDIR\uninstall.exe" 0 SW_SHOWNORMAL "" "Uninstall ${PRODUCT_NAME}"
	CreateShortCut "$SMPROGRAMS\${PRODUCT_NAME}\${PRODUCT_NAME}.lnk" "$INSTDIR\Secret Maryo Chronicles.exe" "" "$INSTDIR\Secret Maryo Chronicles.exe" 0 SW_SHOWNORMAL "" "Start ${PRODUCT_NAME}"
	CreateShortCut "$SMPROGRAMS\${PRODUCT_NAME}\$(SMC_SHORTCUT_CONTROLS).lnk" "$INSTDIR\docs\controls.html" "" "" 0 SW_SHOWNORMAL "" "$(SMC_FINISH_VEW_CONTROLS_INFO)"
	CreateShortCut "$SMPROGRAMS\${PRODUCT_NAME}\$(SMC_SHORTCUT_WEB_PAGE).lnk" "${PRODUCT_WEB_PAGE}/" "" "$WINDIR\system32\shell32.dll" 14 SW_SHOWNORMAL "" "$(SMC_FINISH_WEB_PAGE_INFO)"
	CreateShortCut "$SMPROGRAMS\${PRODUCT_NAME}\$(SMC_SHORTCUT_DONATE).lnk" "${PRODUCT_WEB_PAGE}/?page=donate" "" "$WINDIR\system32\shell32.dll" 14 SW_SHOWNORMAL "" "$(SMC_FINISH_DONATE_INFO)"

SectionEnd

; Optional Desktop Shortcut
Section $(SMC_DESKTOP_SHORTCUT_SECTION_TITLE) Sec_Desktop_Shortcut

	; default is enabled
	SectionIn 1

	CreateShortCut "$DESKTOP\${PRODUCT_NAME}.lnk" "$INSTDIR\Secret Maryo Chronicles.exe" "" "$INSTDIR\Secret Maryo Chronicles.exe" 0 SW_SHOWNORMAL "" "Start ${PRODUCT_NAME}"

SectionEnd

;------------- Descriptions ------------------------------------------

;Assign language strings to sections
!insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
	!insertmacro MUI_DESCRIPTION_TEXT ${Sec_SMC} $(SMC_CORE_SECTION_DESCRIPTION)
	!insertmacro MUI_DESCRIPTION_TEXT ${Sec_Source} $(SMC_SOURCE_CODE_SECTION_DESCRIPTION)
	!insertmacro MUI_DESCRIPTION_TEXT ${Sec_Menu_Shortcut} $(SMC_STARTMENU_SHORTCUT_DESC)
	!insertmacro MUI_DESCRIPTION_TEXT ${Sec_Desktop_Shortcut} $(SMC_DESKTOP_SHORTCUT_DESC)
!insertmacro MUI_FUNCTION_DESCRIPTION_END

;------------ Uninstaller ------------------------------------------


Section "Uninstall"
  
	MessageBox MB_YESNO $(un.SMC_KEEP_USER_DATA) IDYES true IDNO false
	true:
		;nothing
	Goto next
	false:
		; user directory
		Delete "$APPDATA\smc\config.xml"
		RMDir /r "$APPDATA\smc\savegames"
		RMDir /r "$APPDATA\smc\screenshots"
		RMDir /r "$APPDATA\smc\levels"
		RMDir /r "$APPDATA\smc\worlds"
		RMDir /r "$APPDATA\smc\campaign"
		RMDir "$APPDATA\smc"
	next:

	; Remove registry keys
	DeleteRegKey ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}"
	DeleteRegValue HKLM "${PRODUCT_DIR_REGKEY}" "version"

	; Remove main directoy files
	Delete "$INSTDIR\Secret Maryo Chronicles.exe"
	Delete "$INSTDIR\configure"
	Delete "$INSTDIR\*.ac"
	Delete "$INSTDIR\*.am"
	Delete "$INSTDIR\*.sh"
	Delete "$INSTDIR\*.log"
	Delete "$INSTDIR\stdout.txt"
	Delete "$INSTDIR\stderr.txt"
	Delete "$INSTDIR\*.manifest"
	Delete "$INSTDIR\*.dll"
	Delete "$INSTDIR\*.xsd"

	; Remove data files
	RMDir /r "$INSTDIR\data\campaign"
	RMDir /r "$INSTDIR\data\editor"
	RMDir /r "$INSTDIR\data\gui"
	RMDir /r "$INSTDIR\data\icon"
	RMDir /r "$INSTDIR\data\levels"
	RMDir /r "$INSTDIR\data\pixmaps"
	RMDir /r "$INSTDIR\data\schema"
	RMDir /r "$INSTDIR\data\sounds"
	RMDir /r "$INSTDIR\data\translations"
	RMDir /r "$INSTDIR\data\world"
	Delete "$INSTDIR\data\Makefile.am"
	; Remove documents
	RMDir /r "$INSTDIR\docs"
	; Remove source
	RMDir /r "$INSTDIR\src"
	RMDir /r "$INSTDIR\makefiles"
	
	; Remove directories if empty
	RMDir "$INSTDIR\data\music"
	RMDir "$INSTDIR\data"

	; Remove uninstaller
	Delete $INSTDIR\uninstall.exe

	; Remove installation directories if empty
	RMDir "$INSTDIR"
	; Remove startmenu directory
	RMDir /r "$SMPROGRAMS\${PRODUCT_NAME}"
	; Remove desktop shortcut
	Delete "$DESKTOP\${PRODUCT_NAME}.lnk"

	SetAutoClose true

SectionEnd
