;--------------------------------
;Installateur de qRezix pour Windows
;--
;Cet installateur prend en charge les installations suivante :
;   - installation des dlls utiles (qt, msvcr...)
;   - installation de qRezix.exe
;   - création du désinstalleur
;   - installation optionnelle des thèmes d'icônes, traduction...
;   - ...
;--------------------------------

;Définition de versions
  !define MUI_PRODUCT "qRezix"
  !define MUI_VERSION "v1.5.1"
  !define MUI_NAME "${MUI_PRODUCT} ${MUI_VERSION}"

;Inclusion de fichiers
  !include "MUI.nsh"
  !include "Sections.nsh"

;Pour pouvoir copier qt-mt331.dll dans l'installeur
  !define QTDIR "C:\Qt"
  !define SOURCESYSDIR "C:\windows\system32"
  !define QTDLL "qt-mt331.dll"

;Pour le cas ou on utilise VC++ pour compiler
  !define USE_MSVCR_DLL

  !ifdef USE_MSVCR_DLL
     !define MSVCR_DLL "msvcr71.dll"
  !endif

;--------------------------------
;Configuration

;Nom
  Name ${MUI_PRODUCT}
  Caption "Installation de ${MUI_NAME}"

;General
  OutFile "${MUI_NAME} - Installer.exe"
  SetCompressor lzma
  
  ShowInstDetails show
  ShowUninstDetails show
  
;Configuration de l'installation
  InstType "Complète - avec plug-ins"
  InstType "Complète - sans plug-ins"
  InstType "Minimum"

  InstallDir "$PROGRAMFILES\ReziX"
  

;--------------------------------
;Modern UI Configuration

;Configuration des pages
  !define MUI_HEADERIMAGE
  !define MUI_WELCOMEFINISHPAGE_BITMAP "win.bmp"
  !define MUI_WELCOMEPAGE
    !define MUI_WELCOMEPAGE_TITLE "Bienvenue à l'installation ${MUI_NAME}"
    !define MUI_WELCOMEPAGE_TEXT "Cet assistant va te guider durant l'installation de qRezix et de ces composants.\r\n\r\nInitié par les X2000,qRezix est un programme développé par le BR pour les X. Il permet de chatter à l'intérieur de l'X, de permettre un accès facile aux données partagées...\r\n\r\nL'utilisation de ce programme est indispensable à ton intégration sur le rézal.\r\n\r\n$_CLICK"
  !define MUI_DIRECTORYPAGE
  !define MUI_COMPONENTSPAGE
    !define MUI_COMPONENTSPAGE_SMALLDESC
  !define MUI_STARTMENUPAGE
    !define MUI_STARTMENUPAGE_DEFAULTFOLDER "${MUI_PRODUCT}"
  !define MUI_FINISHPAGE
    !define MUI_FINISHPAGE_RUN "$INSTDIR\qRezix.exe"
    !define MUI_FINISHPAGE_SHOWREADME "$INSTDIR\ReadMe.txt"
    !define MUI_FINISHPAGE_NOREBOOTSUPPORT
  !define MUI_ABORTWARNING

  !define MUI_UNINSTALLER
  !define MUI_UNCONFIRMPAGE

;variable pour le menu démarrer
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
  File "..\icons\themes\${THEME}\*.png"
!macroend

!macro INSTALL_XPLO_THEME THEME
  SetOutPath "$INSTDIR\plugins\themes"
  CreateDirectory "${THEME}"
  SetOutPath "$INSTDIR\plugins\themes\${THEME}"
  File "..\..\xplo\xploplugin2\src\themes\${THEME}\*.png"
!macroend

!macro INSTALL_SMILEY_THEME THEME
  SetOutPath "$INSTDIR\plugins\themes"
  CreateDirectory "${THEME}"
  SetOutPath "$INSTDIR\plugins\themes\${THEME}"
  File "..\..\smilix\themes\${THEME}\*.png"
!macroend

!macro INSTALL_SMILEY_IMAGES THEME
  SetOutPath "$INSTDIR\plugins\smiley"
  CreateDirectory "${THEME}"
  SetOutPath "$INSTDIR\plugins\smileys\${THEME}"
  File "..\..\smilix\smileys\${THEME}\*.png"
!macroend
  
;--------------------------------
;Languages
 
  !insertmacro MUI_LANGUAGE "French"  

;--------------------------------
;Language Strings

  ;Description
  LangString DESC_SecBase ${LANG_FRENCH} "Fichiers nécessaires pour ${MUI_PRODUCT}"
  LangString DESC_SecStartMenu ${LANG_FRENCH} "Création d'un groupe dans le menu Démarrer"
  LangString DESC_SecLaunchStartup ${LANG_FRENCH} `Création d'un raccourci dans "Menu Démarrer\Démarrage"`
  LangString DESC_SecIconDesktop ${LANG_FRENCH} `Ajout d'un raccourci sur le bureau`
  LangString DESC_SecThemeClassic ${LANG_FRENCH} "Thème d'icônes par défaut"
  LangString DESC_SecThemeKrystal ${LANG_FRENCH} "Un thème d'icônes sympa"
  LangString DESC_SecThemeNoia ${LANG_FRENCH} "Un autre thème d'icônes sympa"
  LangString DESC_SecThememS ${LANG_FRENCH} "L'invasion des pingouins !"
  LangString DESC_SecTransFrench ${LANG_FRENCH} `Traduction française de qRezix`
  LangString DESC_SecPiXplo ${LANG_FRENCH} "Plug-in de l'Xplo... pour la recherche de fichiers"
  LangString DESC_SecPiSmiley ${LANG_FRENCH} "Plug-in pour le chat qui ajoute converti les smileys en image"


;--------------------------------
;Installer Sections

Section "Fichiers de base de qRezix" SecBase
  SetDetailsPrint textonly
  DetailPrint "Base de qRezix"
  SetDetailsPrint listonly

  SectionIn 1 2 3 RO   ; Section toujours sélectionnée

  ;Installation des Dlls utilisées par qRezix
  SetOutPath "$SYSDIR"
  SetOverwrite off
  
  !ifdef USE_MSVCR_DLL
    File "${SOURCESYSDIR}\${MSVCR_DLL}"
    ifErrors "" +3
      Push "Impossible d'installer $SYSDIR\${MSVCR_DLL}.\nRelancez l'installation en tant qu'Administrateur."
      Call ShowAbort
    push "$SYSDIR\${MSVCR_DLL}"
    Call AddSharedDLL
  !endif

  File "${QTDIR}\bin\${QTDLL}"
  IfErrors "" +3
    Push "Impossible d'installer $SYSDIR\${QTDLL}.\nRelancez l'installation en tant qu'Administrateur."
    Call ShowAbort
  push "$SYSDIR\${QTDLL}"
  Call AddSharedDLL

  SetOverwrite on
  SetOutPath "$INSTDIR"
  File "..\qRezix\qRezix.exe"
  IfErrors "" +3
    Push "Impossible de remplacer $INSTDIR\qRezix.exe.$\nQuittez ${MUI_PRODUCT} avant de lancer la désinstallation."  
    Call ShowAbort

  File /oname=ReadMe.txt "..\README"

  ;Création des répertoires pour le stockages des données  
  CreateDirectory "themes"
  CreateDirectory "icones"
  CreateDirectory "log"
  CreateDirectory "translations"
  CreateDirectory "plugins"

  ;Install the shortcuts in start menu
  !insertmacro MUI_STARTMENU_WRITE_BEGIN "qRezix"
    CreateDirectory "$SMPROGRAMS\$STARTMENU_FOLDER"
    CreateShortCut "$SMPROGRAMS\$STARTMENU_FOLDER\${MUI_PRODUCT}.lnk" "$INSTDIR\qRezix.exe"
    CreateShortCut "$SMPROGRAMS\$STARTMENU_FOLDER\ReadMe.lnk" "$INSTDIR\ReadMe.txt"
    CreateShortCut "$SMPROGRAMS\$STARTMENU_FOLDER\Désinstaller ${MUI_PRODUCT}.lnk" "$INSTDIR\uninstall.exe"
  !insertmacro MUI_STARTMENU_WRITE_END

  ;Enregistrement du chemin de stockage
  WriteRegExpandStr HKLM "Software\qRezix" "InstDir" "$INSTDIR"
  
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

;Les icônes à installer
SubSection "Lancement et raccourcis" SecIcons

Section "Lancer au démarrage de Windows" SecLaunchStartup
  SetDetailsPrint textonly
  DetailPrint "Lancement au démarrage de Windows"
  SetDetailsPrint listonly

  SectionIn 1 2 3
  CreateShortCut "$SMSTARTUP\${MUI_PRODUCT}.lnk" "$INSTDIR\qRezix.exe"
SectionEnd

Section "Icône sur le Bureau" SecIconDesktop
  SetDetailsPrint textonly
  DetailPrint "Icône sur le bureau"
  SetDetailsPrint listonly

  SectionIn 1 2 3
  CreateShortCut "$DESKTOP\${MUI_PRODUCT}.lnk" "$INSTDIR\qRezix.exe"
SectionEnd

SubSectionEnd ; Raccourcis


; Les thèmes d'icônes
SubSection "Thèmes d'icônes" SecTheme

Section "Thème d'icônes 'classic'" SecThemeClassic
  SetDetailsPrint textonly
  DetailPrint "Thèmes d'icônes | classic"
  SetDetailsPrint listonly

  SectionIn 1 2 3 RO

  !insertmacro INSTALL_THEME "classic"
SectionEnd

Section "Thème d'icônes 'krystal'" SecThemeKrystal
  SetDetailsPrint textonly
  DetailPrint "Thèmes d'icônes | krystal"
  SetDetailsPrint listonly

  SectionIn 1 2

  !insertmacro INSTALL_THEME "krystal"
SectionEnd

Section "Thème d'icônes 'Noia'" SecThemeNoia
  SetDetailsPrint textonly
  DetailPrint "Thèmes d'icônes | NoiaWarmKDE"
  SetDetailsPrint listonly

  SectionIn 1 2

  !insertmacro INSTALL_THEME "NoiaWarmKDE"
SectionEnd

Section "Thème d'icônes 'mS'" SecThememS
  SetDetailsPrint textonly
  DetailPrint "Thèmes d'icônes | mS"
  SetDetailsPrint listonly

  SectionIn 1 2

  !insertmacro INSTALL_THEME "mS"
SectionEnd

SubSectionEnd ; Theme


;Traductions
SubSection "Traductions" SecTrans

Section "Traduction française" SecTransFrench
  SetDetailsPrint textonly
  DetailPrint "Traductions | Français"
  SetDetailsPrint listonly

  SectionIn 1 2 3

  SetOutPath "$INSTDIR\translations"
  File "..\qrezix\translations\qrezix_fr.qm"
  SetOutPath "$INSTDIR"
SectionEnd

SubSectionEnd ; Traductions


;Plug-ins
SubSection "Plug-ins" SecPlugIns

Section "Plug-in de l'Xplo" SecPiXplo
  SetDetailsPrint textonly
  DetailPrint "Plug-ins | Xplo"
  SetDetailsPrint listonly

  SectionIn 1

  SetOutPath "$INSTDIR\plugins"
  File "..\..\xplo\xploplugin2\bin\rzxpixplo.dll"
  CreateDirectory "themes"
  !insertmacro INSTALL_XPLO_THEME "classic"
  !insertmacro INSTALL_XPLO_THEME "krystal"
  SetOutPath "$INSTDIR"
SectionEnd

Section "Smilix, pour que le chat soit plus beau" SecPiSmiley
  SetDetailsPrint textonly
  DetailPrint "Plug-ins | Smilix"
  SetDetailsPrint listonly

  SectionIn 1

  SetOutPath "$INSTDIR\plugins"
  File "..\..\smilix\bin\rzxpismiley.dll"
  CreateDirectory "themes"
  CreateDirectory "smileys"
  !insertmacro INSTALL_SMILEY_THEME "classic"
  !insertmacro INSTALL_SMILEY_THEME "krystal"
  !insertmacro INSTALL_SMILEY_THEME "NoiaWarmKDE"

  !insertmacro INSTALL_SMILEY_IMAGES "basic"
  !insertmacro INSTALL_SMILEY_IMAGES "basic2"
  !insertmacro INSTALL_SMILEY_IMAGES "msnlike"

  SetOutPath "$INSTDIR"
SectionEnd

SubSectionEnd ; Plug-ins


; Juste pour s'assurer que le PWD soit bon à l'exécution de qrezix.exe
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
  !insertmacro MUI_DESCRIPTION_TEXT ${SecTheme} "Thèmes d'icônes pour qRezix"
  !insertmacro MUI_DESCRIPTION_TEXT ${SecThemeClassic} $(DESC_SecThemeClassic)
  !insertmacro MUI_DESCRIPTION_TEXT ${SecThemeKrystal} $(DESC_SecThemeKrystal)
  !insertmacro MUI_DESCRIPTION_TEXT ${SecThemeNoia} $(DESC_SecThemeNoia)
  !insertmacro MUI_DESCRIPTION_TEXT ${SecThememS} $(DESC_SecThemems)
  !insertmacro MUI_DESCRIPTION_TEXT ${SecTrans} "Traductions de qRezix"
  !insertmacro MUI_DESCRIPTION_TEXT ${SecTransFrench} $(DESC_SecTransFrench)
  !insertmacro MUI_DESCRIPTION_TEXT ${SecPlugIns} "Plug-ins pour qRezix"
  !insertmacro MUI_DESCRIPTION_TEXT ${SecPiXplo} $(DESC_SecPiXplo)
  !insertmacro MUI_DESCRIPTION_TEXT ${SecPiSmiley} $(DESC_SecPiSmiley)
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
  IfErrors "" +3
    Push "Impossible de supprimer $INSTDIR\qRezix.exe.$\nQuittez ${MUI_PRODUCT} avant de lancer la désinstallation."
    Call un.ShowAbort

  Delete "$INSTDIR\Uninstall.exe"
  Delete "$INSTDIR\ReadMe.txt"
  RMDir "$INSTDIR\translations"
  RMDir /r "$INSTDIR\plug-ins"
  RMDir "$INSTDIR"

  ;Suppression si nécessaire des DLL partagées
  !ifdef USE_MSVCR_DLL
    Push "$SYSDIR\${MSVCR_DLL}"
    Call un.RemoveSharedDLL
  !endif
  Push "$SYSDIR\${QTDLL}"
  Call un.RemoveSharedDLL

  ;Suppression des raccourcis
  !insertmacro MUI_STARTMENU_GETFOLDER "qRezix" $STARTMENU_FOLDER
  RMDir /r "$SMPROGRAMS\$STARTMENU_FOLDER"
  Delete "$SMSTARTUP\${MUI_PRODUCT}.lnk"
  Delete "$DESKTOP\${MUI_PRODUCT}.lnk"

  ;Suppression des informations de désinstallation    
  DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\qRezix" 
SectionEnd


;Suppression des préférences
Section /o "un.Supprimer les préférences" UninstPref
  SetDetailsPrint textonly
  DetailPrint "Suppression des préférences"
  SetDetailsPrint listonly

  Delete "$INSTDIR\*.*"
  RMDir /r "$INSTDIR"
  DeleteRegKey HKLM "Software\qRezix"
SectionEnd


;--------------------------------
;Uninstaller description

!insertmacro MUI_UNFUNCTION_DESCRIPTION_BEGIN
  !insertmacro MUI_DESCRIPTION_TEXT ${Uninstall} "Supprimer qRezix"
  !insertmacro MUI_DESCRIPTION_TEXT ${UninstPref} "Supprime les préférences, historiques des discussions... RAPPELEZ-VOUS DE VOTRE PASS POUR VOUS RECONNECTER AU xNet ULTERIEUREMENT"
!insertmacro MUI_UNFUNCTION_DESCRIPTION_END
