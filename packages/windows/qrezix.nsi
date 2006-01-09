;--------------------------------
;Installateur de qRezix pour Windows
;--
;Cet installateur prend en charge les installations suivante :
;   - installation des dlls utiles (qt, msvcr...)
;   - installation de qRezix.exe
;   - cr�ation du d�sinstalleur
;   - installation optionnelle des th�mes d'ic�nes, traduction...
;   - ...
;--------------------------------

;D�finition de versions
  !define MUI_PRODUCT "qRezix"
  !define MUI_VERSION "v2.0"
  !define MUI_COMPLETEVERSION "v2.0.0-beta1"
  !define MUI_NAME "${MUI_PRODUCT} ${MUI_VERSION}"
  !define MUI_COMPLETENAME "${MUI_PRODUCT} ${MUI_COMPLETEVERSION}"

;Inclusion de fichiers
  !include "MUI.nsh"
  !include "Sections.nsh"

;Informations sur la compilation du programme
;  !define RZX_DEBUG
  !define USE_MSVCR_DLL

;Pour pouvoir copier les dll dans l'installeur
  !define QTDIR "C:\Qt\4.1.0"
  !define SOURCESYSDIR "C:\windows\system32"
  !ifdef RZX_DEBUG
     !define QTCOREDLL "QtCored4.dll"
     !define QTGUIDLL "QtGuid4.dll"
     !define QTNETDLL "QtNetworkd4.dll"
  !else
     !define QTCOREDLL "QtCore4.dll"
     !define QTGUIDLL "QtGui4.dll"
     !define QTNETDLL "QtNetwork4.dll"
  !endif

;Pour le cas ou on utilise VC++ pour compiler
  !ifdef USE_MSVCR_DLL
     !ifdef RZX_DEBUG
        !define MSVCR_DLL "msvcr71d.dll"
        !define MSVCP_DLL "msvcp71d.dll"
     !else       
        !define MSVCR_DLL "msvcr71.dll"
        !define MSVCP_DLL "msvcp71.dll"
     !endif
  !endif

;--------------------------------
;Configuration

;Nom
  Name ${MUI_PRODUCT}
  Caption "Installation de ${MUI_COMPLETENAME}"

;General
  OutFile "${MUI_COMPLETENAME} - Installer.exe"
  SetCompressor lzma
  
  ShowInstDetails show
  ShowUninstDetails show
  
;Configuration de l'installation
  InstType "Compl�te - avec plug-ins"
  InstType "Compl�te - sans plug-ins"
  InstType "Minimum"

  InstallDir "$PROGRAMFILES\qRezix"
  

;--------------------------------
;Modern UI Configuration

;Configuration des pages
  !define MUI_HEADERIMAGE
  !define MUI_WELCOMEFINISHPAGE_BITMAP "win.bmp"
  !define MUI_WELCOMEPAGE
    !define MUI_WELCOMEPAGE_TITLE "Bienvenue � l'installation ${MUI_NAME}"
    !define MUI_WELCOMEPAGE_TEXT "Cet assistant va te guider durant l'installation de qRezix et de ces composants.\r\n\r\nIniti� par les X2000,qRezix est un programme d�velopp� par le BR pour les X. Il permet de chatter � l'int�rieur de l'X, de permettre un acc�s facile aux donn�es partag�es...\r\n\r\nL'utilisation de ce programme est indispensable � ton int�gration sur le r�zal.\r\n\r\n$_CLICK"
  !define MUI_DIRECTORYPAGE
  !define MUI_COMPONENTSPAGE
    !define MUI_COMPONENTSPAGE_SMALLDESC
  !define MUI_STARTMENUPAGE
    !define MUI_STARTMENUPAGE_DEFAULTFOLDER "${MUI_PRODUCT}"
  !define MUI_FINISHPAGE
	;!define MUI_FINISHPAGE_NOAUTOCLOSE   			;Pour debugger
    !define MUI_FINISHPAGE_RUN "$INSTDIR\qRezix.exe"
    !define MUI_FINISHPAGE_SHOWREADME "$INSTDIR\ReadMe.txt"
    !define MUI_FINISHPAGE_NOREBOOTSUPPORT
  !define MUI_ABORTWARNING

  !define MUI_UNINSTALLER
  !define MUI_UNCONFIRMPAGE
  ;!define MUI_UNFINISHPAGE_NOAUTOCLOSE   			;Pour debugger

;variable pour le menu d�marrer
  var STARTMENU_FOLDER  

;Pages for the installer
  !insertmacro MUI_PAGE_WELCOME
  !insertmacro MUI_PAGE_DIRECTORY
  !insertmacro MUI_PAGE_COMPONENTS
  !insertmacro MUI_PAGE_STARTMENU "qRezix" $STARTMENU_FOLDER
  !insertmacro MUI_PAGE_INSTFILES
  !insertmacro MUI_PAGE_FINISH

;Pages for the un-installer
  !insertmacro MUI_UNPAGE_CONFIRM
  !insertmacro MUI_UNPAGE_COMPONENTS
  !insertmacro MUI_UNPAGE_INSTFILES


;--------------------------------
;Functions

Function ShowAbort
  Pop $0
  MessageBox MB_OK|MB_ICONSTOP $0
  Abort $0
FunctionEnd
  

Function un.ShowAbort
  Pop $0
  MessageBox MB_OK|MB_ICONSTOP $0
  Abort $0
FunctionEnd

Function AddSharedDLL
  Exch $R1
  Push $R0
  ReadRegDword $R0 HKLM Software\Microsoft\Windows\CurrentVersion\SharedDLLs $R1
  IntOp $R0 $R0 + 1
  WriteRegDWORD HKLM Software\Microsoft\Windows\CurrentVersion\SharedDLLs $R1 $R0
  Pop $R0
  Pop $R1
FunctionEnd

Function un.RemoveSharedDLL
  Exch $R1
  Push $R0
  ReadRegDword $R0 HKLM Software\Microsoft\Windows\CurrentVersion\SharedDLLs $R1
  StrCmp $R0 "" remove
    IntOp $R0 $R0 - 1
    IntCmp $R0 0 rk rk uk
    rk:
      DeleteRegValue HKLM Software\Microsoft\Windows\CurrentVersion\SharedDLLs $R1
    goto Remove
    uk:
      WriteRegDWORD HKLM Software\Microsoft\Windows\CurrentVersion\SharedDLLs $R1 $R0
    Goto noremove
  remove:
    Delete /REBOOTOK $R1
  noremove:
  Pop $R0
  Pop $R1
FunctionEnd

!macro INSTALL_THEME THEME
  SetOutPath "$INSTDIR\themes"
  CreateDirectory "${THEME}"
  SetOutPath "$INSTDIR\themes\${THEME}"
  File "..\..\resources\themes\${THEME}\*.png"
!macroend

;!macro INSTALL_XPLO_THEME THEME
;  SetOutPath "$INSTDIR\plugins\themes"
;  CreateDirectory "${THEME}"
;  SetOutPath "$INSTDIR\plugins\themes\${THEME}"
;  File "..\..\qrezix-plugins\xplo\src\themes\${THEME}\*.png"
;!macroend

;!macro INSTALL_SMILEY_THEME THEME
;  SetOutPath "$INSTDIR\plugins\themes"
;  CreateDirectory "${THEME}"
;  SetOutPath "$INSTDIR\plugins\themes\${THEME}"
;  File "..\..\resources\smilix\themes\${THEME}\*.png"
;!macroend



;!macro INSTALL_SMILEY_IMAGES THEME
;  SetOutPath "$INSTDIR\plugins\smiley"
;  CreateDirectory "${THEME}"
;  SetOutPath "$INSTDIR\plugins\smileys\${THEME}"
;  File "..\..\resources\smileys\${THEME}\*.png"
;!macroend
  
;--------------------------------
;Languages
 
  !insertmacro MUI_LANGUAGE "French"  

;--------------------------------
;Language Strings

  ;Description
  LangString DESC_SecBase ${LANG_FRENCH} "Fichiers n�cessaires pour ${MUI_PRODUCT}"
  LangString DESC_SecStartMenu ${LANG_FRENCH} "Cr�ation d'un groupe dans le menu D�marrer"
  LangString DESC_SecLaunchStartup ${LANG_FRENCH} `Cr�ation d'un raccourci dans "Menu D�marrer\D�marrage"`
  LangString DESC_SecIconDesktop ${LANG_FRENCH} `Ajout d'un raccourci sur le bureau`
  LangString DESC_SecThemeClassic ${LANG_FRENCH} "Th�me d'ic�nes par d�faut"
  LangString DESC_SecThemeKrystal ${LANG_FRENCH} "Un th�me d'ic�nes sympa"
  LangString DESC_SecThemeNoia ${LANG_FRENCH} "Un autre th�me d'ic�nes sympa"
  LangString DESC_SecThememS ${LANG_FRENCH} "L'invasion des pingouins !"
  LangString DESC_SecThemeMacOSX ${LANG_FRENCH} "Ressemble un peu � MacOS X"
  LangString DESC_SecThemeKids ${LANG_FRENCH} "Th�me d'ic�ne un peu enfantin... sympa"
  LangString DESC_SecThemeNuvola ${LANG_FRENCH} "Toujours un th�me d'ic�nes sympa"
  LangString DESC_SecTransFrench ${LANG_FRENCH} `Traduction fran�aise de qRezix`
  ;LangString DESC_SecPiXplo ${LANG_FRENCH} "Plug-in de l'Xplo... pour la recherche de fichiers"
  ;LangString DESC_SecPiSmiley ${LANG_FRENCH} "Plug-in pour le chat qui ajoute converti les smileys en image"


;--------------------------------
;Installer Sections

Section "Fichiers de base de qRezix" SecBase
  SetDetailsPrint textonly
  DetailPrint "Base de qRezix"
  SetDetailsPrint listonly

  SectionIn 1 2 3 RO   ; Section toujours s�lectionn�e

  ;Installation des Dlls utilis�es par qRezix
  SetOutPath "$SYSDIR"
  SetOverwrite off
  
  !ifdef USE_MSVCR_DLL
    File "${SOURCESYSDIR}\${MSVCR_DLL}"
    ifErrors "" +3
      Push "Impossible d'installer $SYSDIR\${MSVCR_DLL}.\nRelancez l'installation en tant qu'Administrateur."
      Call ShowAbort
    push "$SYSDIR\${MSVCR_DLL}"
    Call AddSharedDLL

    File "${SOURCESYSDIR}\${MSVCP_DLL}"
    ifErrors "" +3
      Push "Impossible d'installer $SYSDIR\${MSVCP_DLL}.\nRelancez l'installation en tant qu'Administrateur."
      Call ShowAbort
    push "$SYSDIR\${MSVCP_DLL}"
    Call AddSharedDLL
  !endif

  SetOverwrite on
  SetOutPath "$INSTDIR"
  File "${QTDIR}\bin\${QTCOREDLL}"
  IfErrors "" +3
    Push "Impossible d'installer $SYSDIR\${QTCOREDLL}.\nRelancez l'installation en tant qu'Administrateur."
    Call ShowAbort

  SetOutPath "$INSTDIR"
  File "${QTDIR}\bin\${QTGUIDLL}"
  IfErrors "" +3
    Push "Impossible d'installer $SYSDIR\${QTGUIDLL}.\nRelancez l'installation en tant qu'Administrateur."
    Call ShowAbort
	
  SetOutPath "$INSTDIR"
  File "${QTDIR}\bin\${QTNETDLL}"
  IfErrors "" +3
    Push "Impossible d'installer $SYSDIR\${QTNETDLL}.\nRelancez l'installation en tant qu'Administrateur."
    Call ShowAbort

  SetOutPath "$INSTDIR"
  File "..\..\qRezix.exe"
  IfErrors "" +3
    Push "Impossible de remplacer $INSTDIR\qRezix.exe.$\nQuittez ${MUI_PRODUCT} avant de lancer la d�sinstallation."  
    Call ShowAbort

  SetOutPath "$INSTDIR"
  File "..\..\qrezix2.dll"
  IfErrors "" +3
    Push "Impossible de remplacer $INSTDIR\qrezix2.dll.$\nQuittez ${MUI_PRODUCT} avant de lancer la d�sinstallation."
    Call ShowAbort		

  File /oname=ReadMe.txt "..\..\README"

  ;Cr�ation des r�pertoires pour le stockages des donn�es  
  CreateDirectory "themes"
  CreateDirectory "icones"
  CreateDirectory "log"
  CreateDirectory "translations"

  CreateDirectory "maps"

  SetOutPath "$INSTDIR"
  File /r /x .svn "..\..\resources\smileys"
  File /r /x .svn "..\..\resources\maps"
  File "..\..\core\subnet.ini"
  File "..\..\rezals\map\map.ini"
  !insertmacro INSTALL_THEME "none"
  
  ;Install the shortcuts in start menu
  !insertmacro MUI_STARTMENU_WRITE_BEGIN "qRezix"
    CreateDirectory "$SMPROGRAMS\$STARTMENU_FOLDER"
    CreateShortCut "$SMPROGRAMS\$STARTMENU_FOLDER\${MUI_PRODUCT}.lnk" "$INSTDIR\qRezix.exe"
    CreateShortCut "$SMPROGRAMS\$STARTMENU_FOLDER\ReadMe.lnk" "$INSTDIR\ReadMe.txt"
    CreateShortCut "$SMPROGRAMS\$STARTMENU_FOLDER\D�sinstaller ${MUI_PRODUCT}.lnk" "$INSTDIR\uninstall.exe"
  !insertmacro MUI_STARTMENU_WRITE_END

  ;Enregistrement du chemin de stockage
  DeleteRegKey HKLM "Software\BR\qRezix\InstDir"
  WriteRegStr HKLM "Software\BR\qRezix" "InstDir" "$INSTDIR"
  
  ;Create uninstaller
  WriteRegExpandStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\qRezix" "UninstallString" '"$INSTDIR\Uninstall.exe"'
  WriteRegExpandStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\qRezix" "InstallLocation" "$INSTDIR"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\qRezix" "DisplayName" "${MUI_NAME}"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\qRezix" "DisplayIcon" "$INSTDIR\qRezix.exe,0"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\qRezix" "DisplayVersion" "${MUI_VERSION}"
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\qRezix" "NoModify" "1"
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\qRezix" "NoRepair" "1"
  WriteUninstaller "$INSTDIR\Uninstall.exe"
SectionEnd

;Les ic�nes � installer
SubSection "Lancement et raccourcis" SecIcons

Section "Lancer au d�marrage de Windows" SecLaunchStartup
  SetDetailsPrint textonly
  DetailPrint "Lancement au d�marrage de Windows"
  SetDetailsPrint listonly

  SectionIn 1 2 3
  CreateShortCut "$SMSTARTUP\${MUI_PRODUCT}.lnk" "$INSTDIR\qRezix.exe"
SectionEnd

Section "Ic�ne sur le Bureau" SecIconDesktop
  SetDetailsPrint textonly
  DetailPrint "Ic�ne sur le bureau"
  SetDetailsPrint listonly

  SectionIn 1 2 3
  CreateShortCut "$DESKTOP\${MUI_PRODUCT}.lnk" "$INSTDIR\qRezix.exe"
SectionEnd

SubSectionEnd ; Raccourcis


; Les th�mes d'ic�nes
SubSection "Th�mes d'ic�nes" SecTheme

Section "Th�me d'ic�nes 'Classic'" SecThemeClassic
  SetDetailsPrint textonly
  DetailPrint "Th�mes d'ic�nes | Classic"
  SetDetailsPrint listonly

  SectionIn 1 2 3 RO

  !insertmacro INSTALL_THEME "Classic"
SectionEnd

Section "Th�me d'ic�nes 'Krystal'" SecThemeKrystal
  SetDetailsPrint textonly
  DetailPrint "Th�mes d'ic�nes | Krystal"
  SetDetailsPrint listonly

  SectionIn 1 2

  !insertmacro INSTALL_THEME "Krystal"
SectionEnd

Section "Th�me d'ic�nes 'Noia'" SecThemeNoia
  SetDetailsPrint textonly
  DetailPrint "Th�mes d'ic�nes | NoiaWarmKDE"
  SetDetailsPrint listonly

  SectionIn 1 2

  !insertmacro INSTALL_THEME "NoiaWarmKDE"
SectionEnd

Section "Th�me d'ic�ne 'MacOS X'" SecThemeMacOSX
  SetDetailsPrint textonly
  DetailPrint "Th�mes d'ic�nes | MacOS X"
  SetDetailsPrint listonly

  SectionIn 1 2
  
  !insertmacro INSTALL_THEME "MacOSX"
SectionEnd

Section "Th�me d'ic�ne 'Kids'" SecThemeKids
  SetDetailsPrint textonly
  DetailPrint "Th�mes d'ic�nes | Kids"
  SetDetailsPrint listonly

  SectionIn 1 2

  !insertmacro INSTALL_THEME "Kids"
SectionEnd

Section "Th�me d'ic�ne 'Nuvola'" SecThemeNuvola
  SetDetailsPrint textonly
  DetailPrint "Th�mes d'ic�nes | Nuvola"
  SetDetailsPrint listonly

  SectionIn 1 2
  
  !insertmacro INSTALL_THEME "Nuvola"
SectionEnd



; Sauf si quelqu'un le compl�te, le th�me mS est mort
;Section "Th�me d'ic�nes 'mS'" SecThememS
;  SetDetailsPrint textonly
;  DetailPrint "Th�mes d'ic�nes | mS"
;  SetDetailsPrint listonly
;
;  SectionIn 1 2
;
;  !insertmacro INSTALL_THEME "mS"
;SectionEnd

SubSectionEnd ; Theme


;Traductions
SubSection "Traductions" SecTrans

Section "Traduction fran�aise" SecTransFrench
  SetDetailsPrint textonly
  DetailPrint "Traductions | Fran�ais"
  SetDetailsPrint listonly

  SectionIn 1 2 3

  SetOutPath "$INSTDIR\translations"
  
  File "..\..\resources\translations\*.qm"
  SetOutPath "$INSTDIR"
SectionEnd

SubSectionEnd ; Traductions

; pas de plugin pour l'instant
;Plug-ins
;SubSection "Plug-ins" SecPlugIns


;Section "Plug-in de l'Xplo" SecPiXplo
;  SetDetailsPrint textonly
;  DetailPrint "Plug-ins | Xplo"
;  SetDetailsPrint listonly
;
;  SectionIn 1
;
;  SetOutPath "$INSTDIR\plugins"
;  File "..\..\qrezix-plugins\xplo\bin\rzxpixplo.dll"
;  CreateDirectory "themes"
;  !insertmacro INSTALL_XPLO_THEME "classic"
;  !insertmacro INSTALL_XPLO_THEME "krystal"
;  !insertmacro INSTALL_XPLO_THEME "NoiaWarmKDE"
;  !insertmacro INSTALL_XPLO_THEME "MacOS X"
;  SetOutPath "$INSTDIR"
;SectionEnd

;Section "Smilix, pour que le chat soit plus beau" SecPiSmiley
;  SetDetailsPrint textonly
;  DetailPrint "Plug-ins | Smilix"
;  SetDetailsPrint listonly
;
;  SectionIn 1
;
;  SetOutPath "$INSTDIR\plugins"
;  File "..\..\qrezix-plugins\smilix\bin\rzxpismiley.dll"
;  CreateDirectory "themes"
;  CreateDirectory "smileys"
;  !insertmacro INSTALL_SMILEY_THEME "classic"
;  !insertmacro INSTALL_SMILEY_THEME "krystal"
;  !insertmacro INSTALL_SMILEY_THEME "NoiaWarmKDE"
;  !inser;tmacro INSTALL_SMILEY_THEME "MacoOS X"
;
;  !insertmacro INSTALL_SMILEY_IMAGES "basic"
;  !insertmacro INSTALL_SMILEY_IMAGES "basic2"
;  !insertmacro INSTALL_SMILEY_IMAGES "msnlike"
;  !insertmacro INSTALL_SMILEY_IMAGES "pingu"
;
;  SetOutPath "$INSTDIR"
;SectionEnd

;SubSectionEnd ; Plug-ins


; Juste pour s'assurer que le PWD soit bon � l'ex�cution de qrezix.exe
Section "-Post"
  SetOutPath "$INSTDIR"
SectionEnd


;--------------------------------
;Descriptions

!insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
  !insertmacro MUI_DESCRIPTION_TEXT ${SecBase} $(DESC_SecBase)
  !insertmacro MUI_DESCRIPTION_TEXT ${SecIcons} "Raccourcis et lancement"
  !insertmacro MUI_DESCRIPTION_TEXT ${SecLaunchStartup} $(DESC_SecLaunchStartup)
  !insertmacro MUI_DESCRIPTION_TEXT ${SecIconDesktop} $(DESC_SecIconDesktop)
  !insertmacro MUI_DESCRIPTION_TEXT ${SecTheme} "Th�mes d'ic�nes pour qRezix"
  !insertmacro MUI_DESCRIPTION_TEXT ${SecThemeClassic} $(DESC_SecThemeClassic)
  !insertmacro MUI_DESCRIPTION_TEXT ${SecThemeKrystal} $(DESC_SecThemeKrystal)
  !insertmacro MUI_DESCRIPTION_TEXT ${SecThemeNoia} $(DESC_SecThemeNoia)
  !insertmacro MUI_DESCRIPTION_TEXT ${SecThemeMacOSX} $(DESC_SecThemeMacOSX)
  !insertmacro MUI_DESCRIPTION_TEXT ${SecThemeKids} $(DESC_SecThemeKids)
  !insertmacro MUI_DESCRIPTION_TEXT ${SecThemeNuvola} $(DESC_SecThemeNuvola)
;  !insertmacro MUI_DESCRIPTION_TEXT ${SecThememS} $(DESC_SecThemems)
  !insertmacro MUI_DESCRIPTION_TEXT ${SecTrans} "Traductions de qRezix"
  !insertmacro MUI_DESCRIPTION_TEXT ${SecTransFrench} $(DESC_SecTransFrench)
  ;!insertmacro MUI_DESCRIPTION_TEXT ${SecPlugIns} "Plug-ins pour qRezix"
  ;!insertmacro MUI_DESCRIPTION_TEXT ${SecPiXplo} $(DESC_SecPiXplo)
  ;!insertmacro MUI_DESCRIPTION_TEXT ${SecPiSmiley} $(DESC_SecPiSmiley)
!insertmacro MUI_FUNCTION_DESCRIPTION_END


;--------------------------------
;Uninstaller Sections

;Suppression du programme principale
Section "un.Suppression de qRezix" Uninstall
  SetDetailsPrint textonly
  DetailPrint "Supression de qRezix"
  SetDetailsPrint listonly

  SectionIn RO

  Delete "$INSTDIR\qRezix.exe"
  Delete "$INSTDIR\qrezix2.dll"
  IfErrors "" +3
    Push "Impossible de supprimer $INSTDIR\qRezix.exe.$\nQuittez ${MUI_PRODUCT} avant de lancer la d�sinstallation."
    Call un.ShowAbort

  Delete "$INSTDIR\Uninstall.exe"
  Delete "$INSTDIR\ReadMe.txt"
  RMDir /r "$INSTDIR\translations"
  RMDir /r "$INSTDIR\smileys"
  RMDir /r "$INSTDIR\themes"
  RMDir /r "$INSTDIR\maps"
  ;RMDir /r "$INSTDIR\plugins"
  Delete "$INSTDIR\*.*"
  RMDir "$INSTDIR"

  ;Suppression si n�cessaire des DLL partag�es
  !ifdef USE_MSVCR_DLL
    Push "$SYSDIR\${MSVCR_DLL}"
    Call un.RemoveSharedDLL
  !endif
  Push "$INSTDIR\${QTCOREDLL}"
  Call un.RemoveSharedDLL
  Push "$INSTDIR\${QTNETDLL}"
  Call un.RemoveSharedDLL
  Push "$INSTDIR\${QTGUIDLL}"
  Call un.RemoveSharedDLL

  ;Suppression des raccourcis
  !insertmacro MUI_STARTMENU_GETFOLDER "qRezix" $STARTMENU_FOLDER
  RMDir /r "$SMPROGRAMS\$STARTMENU_FOLDER"
  Delete "$SMSTARTUP\${MUI_PRODUCT}.lnk"
  Delete "$DESKTOP\${MUI_PRODUCT}.lnk"

  ;Suppression des informations de d�sinstallation    
  DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\qRezix" 
SectionEnd


;Suppression des pr�f�rences
Section /o "un.Supprimer les pr�f�rences" UninstPref
  SetDetailsPrint textonly
  DetailPrint "Suppression des pr�f�rences"
  SetDetailsPrint listonly

  Delete "$INSTDIR\*.*"
  RMDir /r "$INSTDIR"
  DeleteRegKey HKLM "Software\BR\qRezix"
SectionEnd


;--------------------------------
;Uninstaller description

!insertmacro MUI_UNFUNCTION_DESCRIPTION_BEGIN
  !insertmacro MUI_DESCRIPTION_TEXT ${Uninstall} "Supprimer qRezix"
  !insertmacro MUI_DESCRIPTION_TEXT ${UninstPref} "Supprime les pr�f�rences, historiques des discussions... RAPPELEZ-VOUS DE VOTRE PASS POUR VOUS RECONNECTER AU xNet ULTERIEUREMENT"
!insertmacro MUI_UNFUNCTION_DESCRIPTION_END
