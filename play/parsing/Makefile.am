LFLAGS = -+ -f

EXTRA_DIST = tplex.l

all: lex.yy.cc

lex.yy.cc: tplex.l
	flex $(LFLAGS) tplex.l
