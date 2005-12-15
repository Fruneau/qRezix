TEMPLATE = subdirs
qm.target = FORCE
qm.commands = lrelease ./jabber_fr.ts; \
	lrelease ./index_fr.ts; \
	lrelease ./view_fr.ts; \
	lrelease ./map_fr.ts; \
	lrelease ./xnet_fr.ts; \
	lrelease ./chat_fr.ts; \
	lrelease ./notifier_fr.ts; \
	lrelease ./tray_fr.ts; \
	lrelease ./detail_fr.ts; \
	lrelease ./mainui_fr.ts; \
	lrelease ./qrezix_fr.ts;
QMAKE_EXTRA_TARGETS += qm
