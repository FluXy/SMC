;;
;;  german.nsh
;;
;;  German language strings for the Windows SMC NSIS installer.
;;  Windows Code page: 1252
;;
;;  Version 1
;;  Author: FluXy
;;




; Startup checks
!define SMC_INSTALLER_IS_RUNNING	"Der ${PRODUCT_NAME} Installer läuft schon."
!define SMC_ALREADY_INSTALLED		"${PRODUCT_NAME} ist schon installiert und sollte entfernt werden.$\nÜber eine schon installierte Version zu installieren wird nicht unterstützt und kann unvorhergesehenes verhalten verursachen.$\n$\nEntferne die alte Version bevor ${PRODUCT_VERSION} installiert wird ?"
!define SMC_IS_RUNNING				"Eine Instanz von SMC läuft momentan. Beenden Sie SMC und versuchen Sie es nochmal."
 
; Components Page
!define SMC_CORE_SECTION_TITLE					"Kern (erforderlich)"
!define SMC_CORE_SECTION_DESCRIPTION			"SMC Kern und daten"
!define SMC_SOURCE_CODE_SECTION_TITLE			"Quellcode"
!define SMC_SOURCE_CODE_SECTION_DESCRIPTION		"SMC Quellcode welcher für C++ Entwickler gedacht ist"
!define SMC_STARTMENU_SHORTCUT_SECTION_TITLE	"Startmenü Verknüpfungen"
!define SMC_STARTMENU_SHORTCUT_DESC				"Erstellt einen Eintrag für SMC im Startmenü"
!define SMC_DESKTOP_SHORTCUT_SECTION_TITLE		"Desktop Verknüpfung"
!define SMC_DESKTOP_SHORTCUT_DESC				"Erstellt eine Verknüpfung zu SMC auf dem Desktop"

; Installer Finish Page
!define SMC_FINISH_WEB_PAGE_INFO		"Auf der Webseite (www.secretmaryo.org) kannst du nachschauen ob du die aktuellste SMC Version hast, lernen wie man den Editor benutzt oder spreche mit anderen Spielern im Forum. Mit einer Spende kannst du auch die Entwicklung von SMC zu unterstützen :)"
!define SMC_FINISH_VEW_CONTROLS			"Zeige Tastenzuordnung"
!define SMC_FINISH_VEW_CONTROLS_INFO	"Zeige die Standard-Tastenzuordnung für alle Spielaktionen."
!define SMC_FINISH_DONATE_INFO			"Spende um die Entwicklung zu unterstützen :)"

; Shortcuts
!insertmacro SMC_MACRO_DEFAULT_STRING SMC_SHORTCUT_WEB_PAGE		"Webseite"
!insertmacro SMC_MACRO_DEFAULT_STRING SMC_SHORTCUT_CONTROLS		"Tastenzuordnung"
!insertmacro SMC_MACRO_DEFAULT_STRING SMC_SHORTCUT_DONATE		"Spenden"
!insertmacro SMC_MACRO_DEFAULT_STRING SMC_SHORTCUT_UNINSTALL	"Deinstallieren"


; Uninstall Section Prompts
!define un.SMC_KEEP_USER_DATA		"Behalte Benutzer Level, Welten, Spielstände und Screenshot Daten ?"
