bin_PROGRAMS = cadiaplayer
#bin_PROGRAMS = cadiaplayer extcadiaplayer uctagent mcmemagent mcagent mastagent tomastagent pastagent fastagent raveagent ravemastagent randomagent ravefastagent mastfastagent ravemastfastagent testplayer testplayer2 analyzer fluxagent ucthagent

GAMECONTROLLER_SOURCES = ../logic/gamecontroller.cpp ../logic/gamecontroller.h 

if HAVE_ECLIPSE
GAMECONTROLLER_SOURCES += ../logic/eclipsecontroller.cpp ../logic/eclipsecontroller.h
endif

if HAVE_YAP
GAMECONTROLLER_SOURCES += ../logic/prologcontroller.cpp ../logic/prologcontroller.h
endif

if HAVE_BREAKTHROUGH
GAMECONTROLLER_SOURCES +=	../logic/breakthroughcontroller.cpp ../logic/breakthroughcontroller.h \
							../logic/games/breakthrough.cpp ../logic/games/breakthrough.h 
endif

SHARED_SOURCES = 	$(GAMECONTROLLER_SOURCES) \
			gmprotocol.cpp gmprotocol.h \
			agent.cpp agent.h\
			../play/shmemutils/shincupdmap.cpp ../play/shmemutils/shincupdmap.h \
			../play/shmemutils/shkeymap.cpp ../play/shmemutils/shkeymap.h \
			../play/gametheory.cpp ../play/gametheory.h \
			../play/move.cpp ../play/move.h \
			../play/heuristic/heuristic.h \
			../play/players/gameplayer.h \
			../play/parsing/compound.cpp ../play/parsing/compound.h \
			../play/parsing/variable.cpp ../play/parsing/variable.h \
			../play/parsing/symtab.cpp ../play/parsing/symtab.h \
			../play/parsing/gameparser.cpp ../play/parsing/gameparser.h \
			../play/parsing/token.h \
			../play/utils/gamestatespace.cpp ../play/utils/gamestatespace.h \
			../play/utils/multistate.cpp ../play/utils/multistate.h \
			../play/utils/multitree.cpp ../play/utils/multitree.h \
			../play/utils/multistack.cpp ../play/utils/multistack.h \
			../logic/gamecontroller.h \
			../logic/prologutils.h \
			../logic/utils/domainanalyzer.cpp ../logic/utils/domainanalyzer.h \
			../play/utils/binaryrelation.cpp ../play/utils/binaryrelation.h \
			../play/utils/relationcounter.cpp ../play/utils/relationcounter.h \
			../logic/utils/sstate.cpp ../logic/utils/sstate.h \
			../logic/utils/sterm.cpp ../logic/utils/sterm.h \
			../logic/utils/smove.h \
			../logic/utils/smovetable.h \
			../utils/tcpserver.cpp ../utils/tcpserver.h \
			../utils/worker.cpp ../utils/worker.h \
			../utils/longlonghash.h \
			../utils/stringhash.h \
			../utils/statistics.cpp ../utils/statistics.h \
			../utils/logger.cpp ../utils/logger.h \
			../utils/timer.h \
			../utils/streamutils.h \
			../utils/strutils.h \
			../utils/settings.cpp ../utils/settings.h \
			../play/utils/movetrails.cpp ../play/utils/movetrails.h
			
MC_SHARED = 		$(SHARED_SOURCES) \
			../play/kits/goalstabilitykit.cpp ../play/kits/goalstabilitykit.h \
			../play/players/selectionplayer.cpp ../play/players/selectionplayer.h \
			../play/players/mcmemoryplayer.cpp ../play/players/mcmemoryplayer.h \
			../play/players/mcplayer.cpp ../play/players/mcplayer.h \
			../play/utils/gt_file.cpp ../play/utils/gt_file.h \
			../play/utils/gt_log.cpp ../play/utils/gt_log.h \
			../play/utils/gtv_data.h

UCT_SHARED = 		$(MC_SHARED) \
			../play/players/pastplayer.cpp ../play/players/pastplayer.h \
			../play/players/tomastplayer.cpp ../play/players/tomastplayer.h \
			../play/players/mastplayer.cpp ../play/players/mastplayer.h \
			../play/players/uctrolloutplayer.cpp ../play/players/uctrolloutplayer.h \
			../play/players/uctplayer.cpp ../play/players/uctplayer.h

MM_SHARED = 		../play/heuristic/heuristic.h \
			../play/heuristic/piececount.cpp ../play/heuristic/piececount.h \
			../play/utils/stateinfo.cpp ../play/utils/stateinfo.h \
			../play/players/minimaxplayer.cpp ../play/players/minimaxplayer.h \
			../play/players/alphabetaplayer.cpp ../play/players/alphabetaplayer.h \
			../play/players/autoheuristicsplayer.cpp ../play/players/autoheuristicsplayer.h
			
TILING_SHARED = 		../play/tiling/tiling.h ../play/tiling/tilings.h \
			../play/tiling/tilecoding.cpp ../play/tiling/tilecoding.h \
			../play/tiling/tilecodinglearner.cpp ../play/tiling/tilecodinglearner.h \
			../play/tiling/valuetiling.cpp ../play/tiling/valuetiling.h \
			../play/tiling/compoundtiling.cpp ../play/tiling/compoundtiling.h \
			../play/tiling/compoundvaluetiling.cpp ../play/tiling/compoundvaluetiling.h \
			../play/tiling/compoundmultivaluetiling.cpp ../play/tiling/compoundmultivaluetiling.h \
			../play/tiling/rolepiecetiling.cpp ../play/tiling/rolepiecetiling.h \
			../play/tiling/piecevaluetiling.cpp ../play/tiling/piecevaluetiling.h \
			../play/kits/tilingplayerkit.cpp ../play/kits/tilingplayerkit.h \
			../play/players/randomplayer.cpp ../play/players/randomplayer.h \
			../play/utils/movepredicatefeature.cpp ../play/utils/movepredicatefeature.h \
			../play/utils/episode.cpp ../play/utils/episode.h
		
#SHMEM_SHARED =	../play/players/uctimproveshplayer.cpp ../play/players/uctimproveshplayer.h

#CUTOFF_SHARED = ../play/players/cutoffplayer.cpp ../play/players/cutoffplayer.h \
#				../play/players/cutoffmastplayer.cpp ../play/players/cutoffmastplayer.h \
#				../play/players/cutoffmastraveplayer.cpp ../play/players/cutoffmastraveplayer.h \
#				../play/kits/cutoffkit.cpp ../play/kits/cutoffkit.h \
#				../play/players/mccutplayer.cpp ../play/players/mccutplayer.h \
#				../play/kits/goalstabilitykit.cpp ../play/kits/goalstabilitykit.h

RAVE_MAST_FAST_SHARED = ../play/players/pastraveplayer.cpp ../play/players/pastraveplayer.h \
			../play/players/tomastraveplayer.cpp ../play/players/tomastraveplayer.h \
			../play/players/mastraveplayer.cpp ../play/players/mastraveplayer.h \
			../play/players/uctraveplayer.cpp ../play/players/uctraveplayer.h \
			../play/players/uctraverolloutplayer.cpp ../play/players/uctraverolloutplayer.h \
			../play/players/fastplayer.cpp ../play/players/fastplayer.h \
			../play/players/fastraveplayer.cpp ../play/players/fastraveplayer.h \
			../play/players/fastmastplayer.cpp ../play/players/fastmastplayer.h \
			../play/players/fastmastraveplayer.cpp ../play/players/fastmastraveplayer.h \
			../play/players/fasttomastplayer.cpp ../play/players/fasttomastplayer.h \
			../play/players/fasttomastraveplayer.cpp ../play/players/fasttomastraveplayer.h \
			../play/players/fastpastplayer.cpp ../play/players/fastpastplayer.h \
			../play/players/fastpastraveplayer.cpp ../play/players/fastpastraveplayer.h

#cadiaplayer_SOURCES = 	cadiaplayer.cpp $(UCT_SHARED) $(RAVE_MAST_FAST_SHARED) $(CUTOFF_SHARED) $(TILING_SHARED)

cadiaplayer_SOURCES = 	cadiaplayer.cpp $(UCT_SHARED) $(RAVE_MAST_FAST_SHARED) $(TILING_SHARED)
			
#extcadiaplayer_SOURCES = extcadiaplayer.cpp $(UCT_SHARED) $(RAVE_MAST_FAST_SHARED) $(CUTOFF_SHARED) $(TILING_SHARED)
			
#			$(SHARED_SOURCES) \
#			../play/players/astarplayer.cpp ../play/players/astarplayer.h \
#			../play/utils/openlist.cpp ../play/utils/openlist.h \
#			../play/heuristic/relaxedheuristic.cpp ../play/heuristic/relaxedheuristic.h \
#			../play/heuristic/heuristic.h

#uctagent_SOURCES = 	uctagent.cpp \
#			$(UCT_SHARED)

#mcmemagent_SOURCES = 	mcmemagent.cpp \
#			$(MC_SHARED)

#mcagent_SOURCES = 	mcagent.cpp \
#			$(MC_SHARED)

#mastagent_SOURCES =		mastagent.cpp \
#			$(UCT_SHARED)

#tomastagent_SOURCES = 	tomastagent.cpp \
#			../play/players/tomastplayer.cpp ../play/players/tomastplayer.h \
#			$(UCT_SHARED)

#pastagent_SOURCES = 	pastagent.cpp \
#			../play/players/pastplayer.cpp ../play/players/pastplayer.h \
#			../play/utils/movepredicatefeature.cpp ../play/utils/movepredicatefeature.h \
#			$(UCT_SHARED)

#fastagent_SOURCES =		fastagent.cpp \
#			../play/players/fastplayer.cpp ../play/players/fastplayer.h \
#			../play/players/ucttilingplayer.cpp ../play/players/ucttilingplayer.h \
#			$(UCT_SHARED) \
#			$(TILING_SHARED)

#raveagent_SOURCES = 	raveagent.cpp \
#			../play/players/uctraveplayer.cpp ../play/players/uctraveplayer.h \
#			$(UCT_SHARED)

#ravemastagent_SOURCES = 	ravemastagent.cpp \
#			../play/players/mastraveplayer.cpp ../play/players/mastraveplayer.h \
#			../play/players/uctraveplayer.cpp ../play/players/uctraveplayer.h \
#			$(UCT_SHARED)

#randomagent_SOURCES = 	randomagent.cpp \
#			$(SHARED_SOURCES) \
#			../play/players/randomplayer.cpp ../play/players/randomplayer.h

EXTRA_UTILS =		../utils/matrix.h \
			../utils/cholesky.hpp \
			../utils/pcg.hpp \
			../utils/precond.hpp \
			../play/kits/relatedactionkit.cpp ../play/kits/relatedactionkit.h \
			../play/kits/goalestimatorkit.cpp ../play/kits/goalestimatorkit.h \
			../play/utils/goalestimator.h ../play/utils/goalestimator.cpp

#			../play/smab/smabutils.h ../play/smab/smabutils.cpp

EXTRA_PLAYERS =	../play/players/mastraveplayer.cpp ../play/players/mastraveplayer.h \
			../play/players/pastraveplayer.cpp ../play/players/pastraveplayer.h \
			../play/players/tomastraveplayer.cpp ../play/players/tomastraveplayer.h \
			../play/players/ucttilingplayer.cpp ../play/players/ucttilingplayer.h \
			../play/players/comboplayer.cpp ../play/players/comboplayer.h \
			../play/players/mastcomboplayer.cpp ../play/players/mastcomboplayer.h \
			../play/players/pastcomboplayer.cpp ../play/players/pastcomboplayer.h \
			../play/players/tomastcomboplayer.cpp ../play/players/tomastcomboplayer.h \
			../play/players/ravecomboplayer.cpp ../play/players/ravecomboplayer.h \
			../play/players/raveplayer.cpp ../play/players/raveplayer.h \
			../play/players/uctraveplayer.cpp ../play/players/uctraveplayer.h \
			../play/players/driftplayer.cpp ../play/players/driftplayer.h \
			../play/players/twoplyplayer.cpp ../play/players/twoplyplayer.h \
			../play/players/backpropplayer.cpp ../play/players/backpropplayer.h \
			../play/players/proximityplayer.cpp ../play/players/proximityplayer.h \
			../play/players/proximitymastplayer.cpp ../play/players/proximitymastplayer.h \
			../play/players/shortcutplayer.cpp ../play/players/shortcutplayer.h \
			../play/players/solverplayer.cpp ../play/players/solverplayer.h \
			../play/players/extuctplayer.cpp ../play/players/extuctplayer.h \
			../play/players/extmastplayer.cpp ../play/players/extmastplayer.h \
			../play/players/extcomboplayer.cpp ../play/players/extcomboplayer.h \
			../play/players/firsturgencyplayer.cpp ../play/players/firsturgencyplayer.h \
			../play/players/goalestimationplayer.cpp ../play/players/goalestimationplayer.h \
			../play/players/relatedactionplayer.cpp ../play/players/relatedactionplayer.h

#			../play/players/smabplayer.cpp ../play/players/smabplayer.h
			
#			../play/players/pastplayer.cpp ../play/players/pastplayer.h \
#			../play/players/domainplayer.cpp ../play/players/domainplayer.h \
#			../play/players/tomastplayer.cpp ../play/players/tomastplayer.h \
#			../play/utils/movereply.cpp ../play/utils/movereply.h \
#			../play/players/uctreplyplayer.cpp ../play/players/uctreplyplayer.h \
#			../play/players/asyncplayer.cpp ../play/players/asyncplayer.h \
#			../play/players/mmuctplayer.cpp ../play/players/mmuctplayer.h \
#			../play/players/skirmishplayer.cpp ../play/players/skirmishplayer.h \
#			../play/players/uctdlambdaplayer.cpp ../play/players/uctdlambdaplayer.h \
#			../play/players/uctdlambdapieceplayer.cpp ../play/players/uctdlambdapieceplayer.h \
#			../play/players/ucttestplayer.cpp ../play/players/ucttestplayer.h \
#			../play/players/goalestimationplayer.cpp ../play/players/goalestimationplayer.h \
#			../play/players/competitionplayer.cpp ../play/players/competitionplayer.h \
#			../play/players/raveplayer.cpp ../play/players/raveplayer.h \
#			../play/players/decayplayer.cpp ../play/players/decayplayer.h \
#			../play/players/uctraveonlyplayer.cpp ../play/players/uctraveonlyplayer.h \
#			../play/players/secondlookplayer.cpp ../play/players/secondlookplayer.h

#ANALYZING = ../play/utils/gamestats.cpp ../play/utils/gamestats.h \
#			../play/utils/traversalelements.cpp ../play/utils/traversalelements.h \
#			../play/utils/dftraverser.cpp ../play/utils/dftraverser.h \
#			../play/utils/traverser.cpp ../play/utils/traverser.h \
#			../play/utils/treelogvisitor.cpp ../play/utils/treelogvisitor.h \
#			../utils/treelog.cpp ../utils/treelog.h \
#			../utils/strutils.h \
#			../play/players/randomplayer.cpp ../play/players/randomplayer.h \
#			../play/utils/visitor.h

#ravefastagent_SOURCES = 	ravefastagent.cpp \
#			../play/players/ravecomboplayer.cpp ../play/players/ravecomboplayer.h \
#			../play/players/uctraveplayer.cpp ../play/players/uctraveplayer.h \
#			$(UCT_SHARED) \
			$(TILING_SHARED)

#mastfastagent_SOURCES = 	mastfastagent.cpp \
#			../play/players/comboplayer.cpp ../play/players/comboplayer.h \
#			$(UCT_SHARED) \
#			$(TILING_SHARED)

#ravemastfastagent_SOURCES = 	ravemastfastagent.cpp \
#			../play/players/mastcomboplayer.cpp ../play/players/mastcomboplayer.h \
#			../play/players/mastraveplayer.cpp ../play/players/mastraveplayer.h \
#			../play/players/uctraveplayer.cpp ../play/players/uctraveplayer.h \
#			$(UCT_SHARED) \
#			$(TILING_SHARED)

#testplayer_SOURCES = 	testplayer.cpp \
#			$(CUTOFF_SHARED) \
#			$(UCT_SHARED) \
#			$(MM_SHARED) \
#			$(TILING_SHARED) \
#			$(EXTRA_PLAYERS) \
#			$(EXTRA_UTILS)
#			 \
#			$(SHMEM_SHARED)

#testplayer2_SOURCES = 	testplayer2.cpp \
#			$(CUTOFF_SHARED) \
#			$(UCT_SHARED) \
#			$(MM_SHARED) \
#			$(TILING_SHARED) \
#			$(EXTRA_PLAYERS) \
#			$(EXTRA_UTILS)
#			 \
#			$(SHMEM_SHARED)

#analyzer_SOURCES = 	analyzer.cpp \
#			$(ANALYZING) \
#			$(SHARED_SOURCES)

#fluxagent_SOURCES = 	fluxagent.cpp \
#			$(SHARED_SOURCES) \
#			../utils/fluxconnector.cpp ../utils/fluxconnector.h \
#			../play/players/fluxplayer.cpp ../play/players/fluxplayer.h

#ucthagent_SOURCES = 	ucthagent.cpp \
#			$(UCT_SHARED) \
#			../utils/fluxconnector.cpp ../utils/fluxconnector.h \
#			../play/players/uctheuristicsplayer.cpp ../play/players/uctheuristicsplayer.h
