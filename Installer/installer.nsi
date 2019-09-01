!define APPNAME "COMScales"
!define COMPANYNAME "ДАВСофт-Эксперт"
!define DESCRIPTION "Компонента для подключения весового терминала"
# These three must be integers
!define VERSIONMAJOR 1
!define VERSIONMINOR 0
SetCompressor /SOLID lzma
RequestExecutionLevel admin
InstallDir "$PROGRAMFILES\${COMPANYNAME}\${APPNAME}"
Name "${COMPANYNAME} - ${APPNAME}"
# rtf or txt file - remember if it is txt, it must be in the DOS text format (\r\n)
LicenseData "LicenseNotice.txt"

# name installer
OutFile "COMScalesInstall.exe"
!include LogicLib.nsh
page license
page directory
Page instfiles
 
!macro VerifyUserIsAdmin
UserInfo::GetAccountType
pop $0
${If} $0 != "admin" ;Require admin rights on NT4+
        messageBox mb_iconstop "Запустите программу с правами администратора!"
        setErrorLevel 740 ;ERROR_ELEVATION_REQUIRED
        quit
${EndIf}
!macroend

Function .onInit
	setShellVarContext all
	!insertmacro VerifyUserIsAdmin
FunctionEnd

# default section start
Section
    Call Install
SectionEnd

Function Install	
	SetOutPath $INSTDIR
	# Files added here should be removed by the uninstaller (see section "uninstall")
	File "COMScales.dll"
	File "register.cmd"
	File "manual.pdf"
	File "license_kemerovo.pdf"
	File "license_tomsk.pdf"
	File "Demo.epf"
	File "DemoMF.epf"
	# File "Лицензия.pdf"
	# Add any other files for the install directory (license files, app data, etc) here
 
	# Uninstaller - See function un.onInit and section "uninstall" for configuration	
	WriteUninstaller "$INSTDIR\uninstall.exe"
	# Start Menu
	CreateDirectory "$SMPROGRAMS\${COMPANYNAME}\${APPNAME}"
	CreateShortCut "$SMPROGRAMS\${COMPANYNAME}\${APPNAME}\Руководство пользователя.lnk" "$INSTDIR\manual.pdf"
	CreateShortCut "$SMPROGRAMS\${COMPANYNAME}\${APPNAME}\Лицензия Кемерово.lnk" "$INSTDIR\license_kemerovo.pdf"
	CreateShortCut "$SMPROGRAMS\${COMPANYNAME}\${APPNAME}\Лицензия Томск.lnk" "$INSTDIR\license_tomsk.pdf"
	CreateShortCut "$SMPROGRAMS\${COMPANYNAME}\${APPNAME}\Примеры.lnk" "$INSTDIR"
    Exec "$INSTDIR\register.cmd"
FunctionEnd

# Uninstaller
 
Function un.onInit
	SetShellVarContext all
 
	#Verify the uninstaller - last chance to back out
	MessageBox MB_OKCANCEL "Удалить ${APPNAME}?" IDOK next
		Abort
	next:
	!insertmacro VerifyUserIsAdmin
FunctionEnd
 
Section "uninstall"
 
	# Remove Start Menu launcher
	Delete "$SMPROGRAMS\${COMPANYNAME}\${APPNAME}\Руководство пользователя.lnk"
	Delete "$SMPROGRAMS\${COMPANYNAME}\${APPNAME}\Лицензия.lnk"
	# Try to remove the Start Menu folder - this will only happen if it is empty
	RmDir "$SMPROGRAMS\${COMPANYNAME}\${APPNAME}"
 
	# Remove files
	Delete $INSTDIR\COMScales.dll
	Delete $INSTDIR\logo.ico
 
	# Always delete uninstaller as the last action
	Delete $INSTDIR\uninstall.exe
 
	# Try to remove the install directory - this will only happen if it is empty
	rmDir $INSTDIR
 
	# Remove uninstaller information from the registry
	DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${COMPANYNAME} ${APPNAME}"
SectionEnd