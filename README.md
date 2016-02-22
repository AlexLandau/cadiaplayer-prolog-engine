# cadiaplayer-prolog-engine
A fork of CadiaPlayer for use with the gdl-perf testing framework.

CadiaPlayer was written by Hilmar Finnsson, Yngvi Bjornsson, et alia. More information at: http://cadia.ru.is/wiki/public:cadiaplayer:main

Installation instructions are on that wiki page. However, the following notes may also be helpful.

If you are using the aptitude package manager, you can fulfill the Boost library requirement in the 'configure' step by installing the libasio-dev package. You may also need to install libgmp-dev, ncurses-dev, and libreadline-dev to get it to pass. Also consider installing flex and libboost-system-dev, needed for the next step.

You can also choose to install YAP Prolog via the yap package.

For the 'make' step, you may need flex installed. This corresponds to the 'flex' package in aptitude. You may also need to separately install the libboost-system-dev package.

You may also need to add some libraries to the LIBS line in a couple of Makefiles:

* agent/Makefile: Add to the end: -lboost_system -lpthread
* server/Makefile: Add to the end: -lboost_system

(Also the STDIN_FILENO issue)
If agent/gmprotocol.cpp fails to compile due to errors involving STDIN_FILENO, add the following line after the "using namespace" lines:

#define STDIN_FILENO 0


