!define MUI_PRODUCT "qRezix"
!define MUI_VERSION "v1.3"
!define MUI_NAME "${MUI_PRODUCT} ${MUI_VERSION}"

!include "MUI.nsh"

; Pour pouvoir copier qt-mt331.dll dans l'installeur
!define QTDIR "C:\Qt"


;--------------------------------
;Configuration

  ;General
  OutFile "Installer_qRezix.exe"

  ; A laisser, pour réutiliser la conf & cie de l'ancienne version. (celle d'avant l'ancienne version en fait)
  InstallDir "$PROGRAMFILES\ReziX"
  
  ShowInstDetails show
  ShowUninstDetails show
  
  InstType "Normal"

;--------------------------------
;Modern UI Configuration

  !define MUI_WELCOMEPAGE
  !define MUI_COMPONENTSPAGE
  !define MUI_DIRECTORYPAGE
  !define MUI_FINISHPAGE
    !define MUI_FINISHPAGE_RUN "$INSTDIR\qRezix.exe"
    !define MUI_FINISHPAGE_SHOWREADME "$INSTDIR\ReadMe.txt"
    !define MUI_FINISHPAGE_NOREBOOTSUPPORT    
      
  !define MUI_ABORTWARNING
  
  !define MUI_UNINSTALLER
  !define MUI_UNCONFIRMPAGE
  
  ;Modern UI System
;  !insertmacro MUI_SYSTEM ;parce que ne veut pas se compiler


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


!macro INSTALL_THEME THEME
  SetOutPath "$INSTDIR\themes"
  CreateDirectory "${THEME}"
  SetOutPath "$INSTDIR\themes\${THEME}"
  File "..\qrezix\themes\${THEME}\*"
!macroend


!macro SECTION_THEME THEME
Section "Thème d'icones '${THEME}'" SecTheme${THEME}
  SectionIn 1
  !insertmacro INSTALL_THEME "${THEME}"
SectionEnd
!macroend

  
;--------------------------------
;Languages
 
  !insertmacro MUI_LANGUAGE "French"

  !define MUI_TEXT_WELCOME_INFO_TEXT "Cet assistant vous aidera à installer ${MUI_NAME}"
  

;--------------------------------
;Language Strings

  ;Description
  LangString DESC_SecBase ${LANG_FRENCH} "Fichiers nécessaires pour ${MUI_PRODUCT}"
  LangString DESC_SecStartMenu ${LANG_FRENCH} "Création d'un groupe dans le menu Démarrer"
  LangString DESC_SecLaunchStartup ${LANG_FRENCH} `Création d'un raccourci dans "Menu Démarrer\Démarrage"`
  LangString DESC_SecIconDesktop ${LANG_FRENCH} `Ajout d'un raccourci sur le bureau`


;--------------------------------
;Installer Sections

Section "!Base" SecBase
  SectionIn 1 RO   ; Section toujours sélectionnée
  
  SetOutPath "$SYSDIR"
  IfFileExists "qt-mt331.dll" dll_ok
  File "${QTDIR}\bin\qt-mt331.dll"
  IfErrors "" +3
    Push "Impossible d'installer $SYSDIR\qt-mt331.dll.$\nRelancez l'installation en tant qu'Administrateur."
    Call ShowAbort

dll_ok:  

  SetOutPath "$INSTDIR"
  File "..\qRezix\qRezix.exe"
  IfErrors "" +3
    Push "Impossible de remplacer $INSTDIR\qRezix.exe.$\nQuittez ${MUI_PRODUCT} avant de lancer la désinstallation."  
    Call ShowAbort
    
  File /oname=ReadMe.txt "..\README"
  
;  File /oname=qrezix.qm "..\qRezix\translations\qrezix.qm"
  File /oname=qrezix_fr.qm "..\qRezix\translations\qrezix_fr.qm"

  CreateDirectory "themes"
  CreateDirectory "icones"
  CreateDirectory "log"
  
  ;Create uninstaller
  WriteUninstaller "$INSTDIR\Uninstall.exe"

SectionEnd


Section "Icônes dans le Menu Démarrer" SecStartMenu
  SectionIn 1
  CreateDirectory "$SMPROGRAMS\${MUI_PRODUCT}"
  CreateShortCut "$SMPROGRAMS\${MUI_PRODUCT}\${MUI_PRODUCT}.lnk" "$INSTDIR\qRezix.exe"
  CreateShortCut "$SMPROGRAMS\${MUI_PRODUCT}\Désinstaller ${MUI_PRODUCT}.lnk" "$INSTDIR\uninstall.exe"
SectionEnd


Section "Lancer au démarrage de Windows" SecLaunchStartup
  SectionIn 1
  CreateShortCut "$SMSTARTUP\${MUI_PRODUCT}.lnk" "$INSTDIR\qRezix.exe"
SectionEnd


Section "Icône sur le Bureau" SecIconDesktop
  CreateShortCut "$DESKTOP\${MUI_PRODUCT}.lnk" "$INSTDIR\qRezix.exe"
SectionEnd


; Les thèmes d'icônes
Section "Thème d'icônes 'classic'" SecThemeClassic
  SectionIn 1 RO
  !insertmacro INSTALL_THEME "classic"
SectionEnd

!insertmacro SECTION_THEME "krystal"
!insertmacro SECTION_THEME "Noia_Warm_KDE"
!insertmacro SECTION_THEME "mS"


; Juste pour s'assurer que le PWD soit bon à l'exécution de qrezix.exe
Section "-Post"
  SetOutPath "$INSTDIR"
SectionEnd


;Display the Finish header
;Insert this macro after the sections if you are not using a finish page
;!insertmacro MUI_SECTIONS_FINISHHEADER


;--------------------------------
;Descriptions

!insertmacro MUI_FUNCTIONS_DESCRIPTION_BEGIN
  !insertmacro MUI_DESCRIPTION_TEXT ${SecBase} $(DESC_SecBase)
  !insertmacro MUI_DESCRIPTION_TEXT ${SecStartMenu} $(DESC_SecStartMenu)
  !insertmacro MUI_DESCRIPTION_TEXT ${SecLaunchStartup} $(DESC_SecLaunchStartup)
  !insertmacro MUI_DESCRIPTION_TEXT ${SecIconDesktop} $(DESC_SecIconDesktop)
  !insertmacro MUI_DESCRIPTION_TEXT ${SecThemeClassic} "Thème d'icônes par défaut"
  !insertmacro MUI_DESCRIPTION_TEXT ${SecThemeKrystal} "Un thème d'icônes sympa"
  !insertmacro MUI_DESCRIPTION_TEXT ${SecThemeNoia_Warm_KDE} "Un autre thème d'icônes sympa"
  !insertmacro MUI_DESCRIPTION_TEXT ${SecThememS} "L'invasion des pingouins !"
!insertmacro MUI_FUNCTIONS_DESCRIPTION_END
 

;--------------------------------
;Uninstaller Section

;UninstallText "Ceci désinstallera ${MUI_NAME}. Appuyez sur suivant pour continuer."

Section "Uninstall"

  Delete "$INSTDIR\qRezix.exe"
  IfErrors "" +3
    Push "Impossible de supprimer $INSTDIR\qRezix.exe.$\nQuittez ${MUI_PRODUCT} avant de lancer la désinstallation."
    Call un.ShowAbort

  Delete "$INSTDIR\Uninstall.exe"  
  Delete "$INSTDIR\ReadMe.txt"
  Delete "$INSTDIR\qrezix.qm"
  Delete "$INSTDIR\qrezix_fr.qm"


  RMDir "$INSTDIR"

  RMDir /r "$SMPROGRAMS\${MUI_PRODUCT}"
  Delete "$SMSTARTUP\${MUI_PRODUCT}.lnk"
  Delete "$DESKTOP\${MUI_PRODUCT}.lnk"

  MessageBox MB_YESNO "Supprimer tout le contenu du répertoire $INSTDIR ?$\nCeci effacera la configuration, les favoris, les historiques, ..." IDNO normal_clean
  
  Delete "$INSTDIR\*.*"
  RMDir /r "$INSTDIR"
  
normal_clean:  
  
  ;Display the Finish header
 ; !insertmacro MUI_UNFINISHHEADER

SectionEnd
