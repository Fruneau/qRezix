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
  !define MUI_VERSION "v2.0"
  !define MUI_COMPLETEVERSION "v2.0.2"
  !define MUI_NAME "${MUI_PRODUCT} ${MUI_VERSION}"
  !define MUI_COMPLETENAME "${MUI_PRODUCT} ${MUI_COMPLETEVERSION}"

;Inclusion de fichiers
  !include "MUI.nsh"
  !include "Sections.nsh"

;Informations sur la compilation du programme
;Utilisation de ces flags :
; - RZX_DEBUG : indique qu'il faut installer les dll de debug ==> qRezix a été compilé avec CONFIG=debug (cf rzxglobal.pri)
; - USE_MSVCR_DLL : indique que qRezix a été compilé avec Visual C++
; - RZX_MODULES : indique que qRezix a été compilé intégralement sous forme de modules ==> installation modulaire envisageable
  !define RZX_DEBUG
  !define USE_MSVCR_DLL
;  !define RZX_MODULES

;Pour pouvoir copier les dll dans l'installeur
  !define QTDIR "G:\Qt\4.1.4"
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
;==> actuellement prévu pour Visual C++ .NET
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
!ifdef RZX_MODULES
  InstType "Complète"
  InstType "Discrète"
  InstType "Minimale"
!else
  InstType "Complète"
  InstType "Minimale"
!endif

  InstallDir "$PROGRAMFILES\qRezix"
  

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
	;!define MUI_FINISHPAGE_NOAUTOCLOSE   			;Pour debugger
    !define MUI_FINISHPAGE_RUN "$INSTDIR\qRezix.exe"
    !define MUI_FINISHPAGE_SHOWREADME "$INSTDIR\ReadMe.txt"
    !define MUI_FINISHPAGE_NOREBOOTSUPPORT
  !define MUI_ABORTWARNING

  !define MUI_UNINSTALLER
  !define MUI_UNCONFIRMPAGE
  ;!define MUI_UNFINISHPAGE_NOAUTOCLOSE   			;Pour debugger

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
  LangString DESC_SecBase ${LANG_FRENCH} "Fichiers nécessaires pour ${MUI_PRODUCT}"
  LangString DESC_SecStartMenu ${LANG_FRENCH} "Création d'un groupe dans le menu Démarrer"
  LangString DESC_SecLaunchStartup ${LANG_FRENCH} `Création d'un raccourci dans "Menu Démarrer\Démarrage"`
  LangString DESC_SecIconDesktop ${LANG_FRENCH} `Ajout d'un raccourci sur le bureau`
  LangString DESC_SecThemeClassic ${LANG_FRENCH} "Thème d'icônes par défaut"
  LangString DESC_SecThemeKrystal ${LANG_FRENCH} "Un thème d'icônes sympa"
  LangString DESC_SecThemeNoia ${LANG_FRENCH} "Un autre thème d'icônes sympa"
  LangString DESC_SecThememS ${LANG_FRENCH} "L'invasion des pingouins !"
  LangString DESC_SecThemeMacOSX ${LANG_FRENCH} "Ressemble un peu à MacOS X"
  LangString DESC_SecThemeKids ${LANG_FRENCH} "Thème d'icône un peu enfantin... sympa"
  LangString DESC_SecThemeNuvola ${LANG_FRENCH} "Toujours un thème d'icônes sympa"
  LangString DESC_SecTransFrench ${LANG_FRENCH} `Traduction française de qRezix`
  ;LangString DESC_SecPiXplo ${LANG_FRENCH} "Plug-in de l'Xplo... pour la recherche de fichiers"
  ;LangString DESC_SecPiSmiley ${LANG_FRENCH} "Plug-in pour le chat qui ajoute converti les smileys en image"


;--------------------------------
;Installer Sections

Section "Fichiers de base de qRezix" SecBase
  SetDetailsPrint textonly
  DetailPrint "Base de qRezix"
  SetDetailsPrint listonly

!ifdef RZX_MODULES
  SectionIn 1 2 3 RO   ; Section toujours sélectionnée
!else
  SectionIn 1 2 RO
!endif

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
    Push "Impossible de remplacer $INSTDIR\qRezix.exe.$\nQuittez ${MUI_PRODUCT} avant de lancer la désinstallation."  
    Call ShowAbort

  SetOutPath "$INSTDIR"
  File "..\..\qrezix2.dll"
  IfErrors "" +3
    Push "Impossible de remplacer $INSTDIR\qrezix2.dll.$\nQuittez ${MUI_PRODUCT} avant de lancer la désinstallation."
    Call ShowAbort		

  File /oname=ReadMe.txt "..\..\README"

  ;Création des répertoires pour le stockages des données  
  CreateDirectory "themes"
  CreateDirectory "icones"
  CreateDirectory "log"
  CreateDirectory "translations"
  File "..\..\core\subnet.ini"
  
!ifndef RZX_MODULES
  SetOutPath "$INSTDIR"
  File /r /x .svn "..\..\resources\smileys"
  File /r /x .svn "..\..\resources\maps"
  File "..\..\rezals\map\map.ini"
!else
  CreateDirectory "modules"
  CreateDirectory "net"
!endif ;RZX_MODULES

  !insertmacro INSTALL_THEME "none"
  
  ;Install the shortcuts in start menu
  !insertmacro MUI_STARTMENU_WRITE_BEGIN "qRezix"
    CreateDirectory "$SMPROGRAMS\$STARTMENU_FOLDER"
    CreateShortCut "$SMPROGRAMS\$STARTMENU_FOLDER\${MUI_PRODUCT}.lnk" "$INSTDIR\qRezix.exe"
    CreateShortCut "$SMPROGRAMS\$STARTMENU_FOLDER\ReadMe.lnk" "$INSTDIR\ReadMe.txt"
    CreateShortCut "$SMPROGRAMS\$STARTMENU_FOLDER\Désinstaller ${MUI_PRODUCT}.lnk" "$INSTDIR\uninstall.exe"
  !insertmacro MUI_STARTMENU_WRITE_END

  ;Enregistrement du chemin de stockage
  DeleteRegKey HKLM "Software\BR\qRezix\InstDir"
  WriteRegStr HKLM "Software\BR\qRezix" "InstDir" "$INSTDIR"
  
  ;Create uninstaller
  WriteRegExpandStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\qRezix-2" "UninstallString" '"$INSTDIR\Uninstall.exe"'
  WriteRegExpandStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\qRezix-2" "InstallLocation" "$INSTDIR"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\qRezix-2" "DisplayName" "${MUI_NAME}"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\qRezix-2" "DisplayIcon" "$INSTDIR\qRezix.exe,0"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\qRezix-2" "DisplayVersion" "${MUI_VERSION}"
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\qRezix-2" "NoModify" "1"
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\qRezix-2" "NoRepair" "1"
  WriteUninstaller "$INSTDIR\Uninstall.exe"
SectionEnd


!ifdef RZX_MODULES
;Les modules à installer
SubSection "Modules à installer" SecModules

Section "Fenêtre principale" SecMainUI
  SetDetailsPrint textonly
  DetailPrint "Modules | Fenêtre principale"
  SetDetailsPrint listonly

  SectionIn 1

  ;Installe le module en lui même
  SetOutPath "$INSTDIR\modules"
  File "..\..\modules\rzxmainui.dll"

  ;Pour ne pas trop compliqué, on installe les rezals avec...
  SetOutPath "$INSTDIR"
  CreateDirectory "rezals"
  File /r /x .svn "..\..\resources\maps"
  File "..\..\rezals\map\map.ini"
  SetOutPath "$INSTDIR\rezals"
  File "..\..\rezals\rezaldetail.dll"
  File "..\..\rezals\rezalview.dll"
  File "..\..\rezals\rezalindex.dll"
  File "..\..\rezals\rezalmap.dll"
  SetOutPath "$INSTDIR"
SectionEnd

Section "Chat" SecChat
  SetDetailsPrint textonly
  DetailPrint "Modules | Chat"
  SetDetailsPrint listonly

  SectionIn 1 2

  SetOutPath "$INSTDIR\modules"
  File "..\..\modules\rzxchat.dll"
  SetOutPath "$INSTDIR"
  File /r /x .svn "..\..\resources\smileys"
SectionEnd

Section "Intégration au ystray" SecTray
  SetDetailsPrint textonly
  DetailPrint "Modules | Systray"
  SetDetailsPrint listonly

  SectionIn 1 2

  SetOutPath "$INSTDIR\modules"
  File "..\..\modules\rzxtray.dll"
  SetOutPath "$INSTDIR"
SectionEnd

Section "Notifications" SecNotifier
  SetDetailsPrint textonly
  DetailPrint "Modules | Notifier"
  SetDetailsPrint listonly

  SectionIn 1

  SetOutPath "$INSTDIR\modules"
  File "..\..\modules\rzxnotifier.dll"
  SetOutPath "$INSTDIR"
SectionEnd

SubSectionEnd ;Modules



SubSection "Protocoles réseau" SecProtocoles

Section "xNet" SecxNet
  SetDetailsPrint textonly
  DetailPrint "Réseau | xNet"
  SetDetailsPrint listonly

  SectionIn 1 2 3 RO ; Tant que le module jabber n'est pas fait pour windows, xNet est le seul client
  SetOutPath "$INSTDIR\net"
  File "..\..\net\rzxnetxnet.dll"
  SetOutPath "$INSTDIR"
SectionEnd  
	
SubSectionEnd ;Protocoles
!endif ;RZX_MODULES



;Les icônes à installer
SubSection "Lancement et raccourcis" SecIcons

Section "Lancer au démarrage de Windows" SecLaunchStartup
  SetDetailsPrint textonly
  DetailPrint "Lancement au démarrage de Windows"
  SetDetailsPrint listonly

!ifdef RZX_MODULES
  SectionIn 1 2 3
!else
  SectionIn 1 2
!endif

  CreateShortCut "$SMSTARTUP\${MUI_PRODUCT}.lnk" "$INSTDIR\qRezix.exe"
SectionEnd

Section "Icône sur le Bureau" SecIconDesktop
  SetDetailsPrint textonly
  DetailPrint "Icône sur le bureau"
  SetDetailsPrint listonly

  SectionIn 1 2

  CreateShortCut "$DESKTOP\${MUI_PRODUCT}.lnk" "$INSTDIR\qRezix.exe"
SectionEnd

SubSectionEnd ; Raccourcis




; Les thèmes d'icônes
SubSection "Thèmes d'icônes" SecTheme

Section "Thème d'icônes 'Classic'" SecThemeClassic
  SetDetailsPrint textonly
  DetailPrint "Thèmes d'icônes | Classic"
  SetDetailsPrint listonly

!ifdef RZX_MODULES
  SectionIn 1 2
!else
  SectionIn 1
!endif

  !insertmacro INSTALL_THEME "Classic"
SectionEnd

Section "Thème d'icônes 'Krystal'" SecThemeKrystal
  SetDetailsPrint textonly
  DetailPrint "Thèmes d'icônes | Krystal"
  SetDetailsPrint listonly

!ifdef RZX_MODULES
  SectionIn 1 2 3 RO
!else
  SectionIn 1 2 RO
!endif

  !insertmacro INSTALL_THEME "Krystal"
SectionEnd

Section "Thème d'icônes 'Noia'" SecThemeNoia
  SetDetailsPrint textonly
  DetailPrint "Thèmes d'icônes | NoiaWarmKDE"
  SetDetailsPrint listonly

!ifdef RZX_MODULES
  SectionIn 1 2
!else
  SectionIn 1
!endif

  !insertmacro INSTALL_THEME "NoiaWarmKDE"
SectionEnd

Section "Thème d'icône 'MacOS X'" SecThemeMacOSX
  SetDetailsPrint textonly
  DetailPrint "Thèmes d'icônes | MacOS X"
  SetDetailsPrint listonly

!ifdef RZX_MODULES
  SectionIn 1 2
!else
  SectionIn 1
!endif
  
  !insertmacro INSTALL_THEME "MacOSX"
SectionEnd

Section "Thème d'icône 'Kids'" SecThemeKids
  SetDetailsPrint textonly
  DetailPrint "Thèmes d'icônes | Kids"
  SetDetailsPrint listonly

!ifdef RZX_MODULES
  SectionIn 1 2
!else
  SectionIn 1
!endif

  !insertmacro INSTALL_THEME "Kids"
SectionEnd

Section "Thème d'icône 'Nuvola'" SecThemeNuvola
  SetDetailsPrint textonly
  DetailPrint "Thèmes d'icônes | Nuvola"
  SetDetailsPrint listonly

!ifdef RZX_MODULES
  SectionIn 1 2
!else
  SectionIn 1
!endif
  
  !insertmacro INSTALL_THEME "Nuvola"
SectionEnd

SubSectionEnd ; Theme




;Traductions
SubSection "Traductions" SecTrans

Section "Traduction française" SecTransFrench
  SetDetailsPrint textonly
  DetailPrint "Traductions | Français"
  SetDetailsPrint listonly

!ifdef RZX_MODULES
  SectionIn 1 2 3
!else
  SectionIn 1 2
!endif

  SetOutPath "$INSTDIR\translations"
  
  File "..\..\resources\translations\*.qm"
  SetOutPath "$INSTDIR"
SectionEnd

SubSectionEnd ; Traductions




; Juste pour s'assurer que le PWD soit bon à l'exécution de qrezix.exe
Section "-Post"
  SetOutPath "$INSTDIR"
SectionEnd


;--------------------------------
;Descriptions

!insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
  !insertmacro MUI_DESCRIPTION_TEXT ${SecBase} $(DESC_SecBase)
  !insertmacro MUI_DESCRIPTION_TEXT ${SecModules} "Modules pour qRezix..."
  !insertmacro MUI_DESCRIPTION_TEXT ${SecMainUI} "Fenêtre principale de qRezix qui permet l'affichage de la liste des connectés"
  !insertmacro MUI_DESCRIPTION_TEXT ${SecChat} "Module qui permet de discuter avec les autres connectés"
  !insertmacro MUI_DESCRIPTION_TEXT ${SecTray} "Module qui permet l'intégration de qRezix au SysTray de Windows (la barre d'icône à côté de l'horloge)"
  !insertmacro MUI_DESCRIPTION_TEXT ${SecNotifier} "Module qui permet d'être averti de la connexion/déconnexion des favoris"
  !insertmacro MUI_DESCRIPTION_TEXT ${SecProtocoles} "Gestion des protocoles réseaux supportés par qRezix..."
  !insertmacro MUI_DESCRIPTION_TEXT ${SecxNet} "Protocole xNet, le protocole d'origine de qRezix"
  !insertmacro MUI_DESCRIPTION_TEXT ${SecIcons} "Raccourcis et lancement"
  !insertmacro MUI_DESCRIPTION_TEXT ${SecLaunchStartup} $(DESC_SecLaunchStartup)
  !insertmacro MUI_DESCRIPTION_TEXT ${SecIconDesktop} $(DESC_SecIconDesktop)
  !insertmacro MUI_DESCRIPTION_TEXT ${SecTheme} "Thèmes d'icônes pour qRezix"
  !insertmacro MUI_DESCRIPTION_TEXT ${SecThemeClassic} $(DESC_SecThemeClassic)
  !insertmacro MUI_DESCRIPTION_TEXT ${SecThemeKrystal} $(DESC_SecThemeKrystal)
  !insertmacro MUI_DESCRIPTION_TEXT ${SecThemeNoia} $(DESC_SecThemeNoia)
  !insertmacro MUI_DESCRIPTION_TEXT ${SecThemeMacOSX} $(DESC_SecThemeMacOSX)
  !insertmacro MUI_DESCRIPTION_TEXT ${SecThemeKids} $(DESC_SecThemeKids)
  !insertmacro MUI_DESCRIPTION_TEXT ${SecThemeNuvola} $(DESC_SecThemeNuvola)
  !insertmacro MUI_DESCRIPTION_TEXT ${SecTrans} "Traductions de qRezix"
  !insertmacro MUI_DESCRIPTION_TEXT ${SecTransFrench} $(DESC_SecTransFrench)
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
    Push "Impossible de supprimer $INSTDIR\qRezix.exe.$\nQuittez ${MUI_PRODUCT} avant de lancer la désinstallation."
    Call un.ShowAbort

  Delete "$INSTDIR\Uninstall.exe"
  Delete "$INSTDIR\ReadMe.txt"
  RMDir /r "$INSTDIR\translations"
  RMDir /r "$INSTDIR\smileys"
  RMDir /r "$INSTDIR\themes"
  RMDir /r "$INSTDIR\maps"
  RMDir /r "$INSTDIR\modules"
  RMDir /r "$INSTDIR\rezals"
  RMDir /r "$INSTDIR\net"
  RMDir "$INSTDIR"

  ;Suppression si nécessaire des DLL partagées
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

  ;Suppression des informations de désinstallation    
  DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\qRezix-2"
  DeleteRegKey HKLM "Software\BR\qRezix\InstDir"
  DeleteRegKey /ifempty HKLM "Software\BR\qRezix"
  DeleteRegKey /ifempty HKLM "Software\BR"
SectionEnd


;Suppression des préférences
Section /o "un.Supprimer les préférences" UninstPref
  SetDetailsPrint textonly
  DetailPrint "Suppression des préférences"
  SetDetailsPrint listonly

  RMDir /r "$INSTDIR"

  ;TODO : virer les préférences pour TOUS les utilisateurs
  ;ça doit se faire avec EnumRegKey...
  DeleteRegKey HKCU "Software\BR\qRezix"
  DeleteRegKey /ifempty HKCU "Software\BR"
SectionEnd


;--------------------------------
;Uninstaller description

!insertmacro MUI_UNFUNCTION_DESCRIPTION_BEGIN
  !insertmacro MUI_DESCRIPTION_TEXT ${Uninstall} "Supprimer qRezix"
  !insertmacro MUI_DESCRIPTION_TEXT ${UninstPref} "Supprime les préférences, historiques des discussions... RAPPELEZ-VOUS DE VOTRE PASS POUR VOUS RECONNECTER AU xNet ULTERIEUREMENT (ne supprime que les préférences de l'utilisateur courant)"
!insertmacro MUI_UNFUNCTION_DESCRIPTION_END
