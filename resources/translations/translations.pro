TEMPLATE = app

lts.output  = ${QMAKE_FILE_BASE}.qm
lts.commands = lrelease ${QMAKE_FILE_NAME}
lts.depend_command = 
lts.input = TRANS
TRANS = jabber_fr.ts \
        index_fr.ts \
	view_fr.ts \
	map_fr.ts \
	bob_fr.ts \
	xnet_fr.ts \
	chat_fr.ts \
	notifier_fr.ts \
	tray_fr.ts \
	detail_fr.ts \
	mainui_fr.ts \
	qrezix_fr.ts
QMAKE_EXTRA_COMPILERS += lts

QMAKE_LINK = echo
