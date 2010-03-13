;;
;;  english.nsh
;;
;;  Default language strings for the Windows SMC NSIS installer.
;;  Windows Code page: 1252
;;
;;  Version 1
;;  Note: If translating this file, replace '!insertmacro SMC_MACRO_DEFAULT_STRING'
;;  with '!define'.

; Make sure to update the SMC_MACRO_LANGUAGEFILE_END macro in
; langmacros.nsh when updating this file

; Startup checks
!insertmacro SMC_MACRO_DEFAULT_STRING SMC_INSTALLER_IS_RUNNING	"The ${PRODUCT_NAME} installer is already running."
!insertmacro SMC_MACRO_DEFAULT_STRING SMC_ALREADY_INSTALLED		"${PRODUCT_NAME} is already installed and should be removed.$\nInstalling over an already installed Version is not supported and can result in unexpected behavior.$\n$\nRemove the previous version before installing ${PRODUCT_VERSION} ?"
!insertmacro SMC_MACRO_DEFAULT_STRING SMC_IS_RUNNING			"An instance of SMC is currently running. Please exit SMC and try again."

; Components Page
!insertmacro SMC_MACRO_DEFAULT_STRING SMC_CORE_SECTION_TITLE				"Core (required)"
!insertmacro SMC_MACRO_DEFAULT_STRING SMC_CORE_SECTION_DESCRIPTION			"SMC core and data"
!insertmacro SMC_MACRO_DEFAULT_STRING SMC_SOURCE_CODE_SECTION_TITLE			"Source Code"
!insertmacro SMC_MACRO_DEFAULT_STRING SMC_SOURCE_CODE_SECTION_DESCRIPTION	"SMC Source Code which is intended for C++ Developers"
!insertmacro SMC_MACRO_DEFAULT_STRING SMC_STARTMENU_SHORTCUT_SECTION_TITLE	"Start Menu Shortcuts"
!insertmacro SMC_MACRO_DEFAULT_STRING SMC_STARTMENU_SHORTCUT_DESC			"Create a Start Menu entry for SMC"
!insertmacro SMC_MACRO_DEFAULT_STRING SMC_DESKTOP_SHORTCUT_SECTION_TITLE	"Desktop Shortcut"
!insertmacro SMC_MACRO_DEFAULT_STRING SMC_DESKTOP_SHORTCUT_DESC				"Create a shortcut to SMC on the Desktop"

; Installer Finish Page
!insertmacro SMC_MACRO_DEFAULT_STRING SMC_FINISH_WEB_PAGE_INFO			"Checkout if you have the latest SMC Release on the Web Page (www.secretmaryo.org), learn how to use the Editor or talk with other players in the Forum. Please donate to support the development :)"
!insertmacro SMC_MACRO_DEFAULT_STRING SMC_FINISH_VEW_CONTROLS			"View Controls"
!insertmacro SMC_MACRO_DEFAULT_STRING SMC_FINISH_VEW_CONTROLS_INFO		"View the default controls for all game actions."
!insertmacro SMC_MACRO_DEFAULT_STRING SMC_FINISH_DONATE_INFO			"Donate to support the development :)"

; Shortcuts
!insertmacro SMC_MACRO_DEFAULT_STRING SMC_SHORTCUT_WEB_PAGE		"Web Page"
!insertmacro SMC_MACRO_DEFAULT_STRING SMC_SHORTCUT_CONTROLS		"Controls"
!insertmacro SMC_MACRO_DEFAULT_STRING SMC_SHORTCUT_DONATE		"Donate"
!insertmacro SMC_MACRO_DEFAULT_STRING SMC_SHORTCUT_UNINSTALL	"Uninstall"

; Uninstall Section Prompts
!insertmacro SMC_MACRO_DEFAULT_STRING un.SMC_KEEP_USER_DATA		"Keep User Level, World, Savegame and Screenshot data ?"

