EXTRA_DIST = run_clients run_ggp_client run_slaves run_ggp_slave run_mastslaves run_ggp_mastslave run_fastmastslaves run_ggp_fastmastslave client.kif slavegame.kif prolog/cadia_connector.ecl prolog/game_description.ecl prolog/gdl_parser.ecl prolog/logger.ecl prolog/main.ecl prolog/match_info.ecl prolog/time_sync.ecl ggp.extensions.pl kif2ggp cadiaplayer.ini

SCRIPTFILES = 
#SCRIPTFILES = run_clients run_ggp_client run_slaves run_ggp_slave run_mastslaves run_ggp_mastslave run_fastmastslaves run_ggp_fastmastslave
KIFFILES = 
#KIFFILES = client.kif slavegame.kif
INIFILES = cadiaplayer.ini

PROLOGFILES =
if HAVE_ECLIPSE
PROLOGFILES += prolog/cadia_connector.ecl prolog/game_description.ecl prolog/gdl_parser.ecl prolog/logger.ecl prolog/main.ecl prolog/match_info.ecl prolog/time_sync.ecl
endif
if HAVE_YAP
PROLOGFILES += ggp.extensions.pl
SCRIPTFILES += kif2ggp
endif

bin_SCRIPTS = $(SCRIPTFILES)
# actually the following should go in data (nobase_data_DATA), but we have to change the code in order to look there
nobase_bin_SCRIPTS = $(PROLOGFILES) $(KIFFILES) $(INIFILES)
#nobase_bin_SCRIPTS = $(PROLOGFILES) $(KIFFILES) $(INIFILES)

install-data-local:
	mkdir -p $(DESTDIR)$(bindir)/games 
	mkdir -p $(DESTDIR)$(bindir)/games/kif 
	mkdir -p $(DESTDIR)$(bindir)/games/pl 
	mkdir -p $(DESTDIR)$(bindir)/games/ggp
#	touch $(DESTDIR)$(bindir)/workers.txt 
	touch $(DESTDIR)$(bindir)/externals.txt
#	mkdir -p $(addprefix $(DESTDIR)$(bindir)/, games games/kif games/pl games/ggp)
#	touch $(addprefix $(DESTDIR)$(bindir)/, workers.txt externals.txt)

