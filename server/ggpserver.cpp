/****************************************************************************
**
** GGP Server
**
**  ggpserver ggp_program_name [r]
**
****************************************************************************/

#include <iostream>
#include "server.h"

//#define RUN_WITH_NO_ARGS

#define GGPSERVER_DEFAULT_EXECUTABLE "./cadiaplayer"
const int GGPSERVER_DEFAULT_PORT = 59834;
const bool GGPSERVER_DEFAULT_RESTART = true;

const int GGPSERVER_OK						= 0;
const int GGPSERVER_ERROR_BAD_EXECUTABLE	= 1;
const int GGPSERVER_ERROR_BAD_PORT			= 2;

using namespace std;
using namespace cadiaplayer::server;

void printUsage()
{
	cerr << "Usage: <ggp executeable> [port]" << endl;
	cerr << "Default port is " << GGPSERVER_DEFAULT_PORT << endl;
}

int main(int argc, char *argv[])
{
#ifndef RUN_WITH_NO_ARGS
    cout << "Starting program ..." << endl;
    if ( argc < 2  ) {
        cerr << "GGP player executable name is missing!" << endl;
		printUsage();
		return GGPSERVER_ERROR_BAD_EXECUTABLE;
    }
    int  port = GGPSERVER_DEFAULT_PORT;

	if ( argc >= 3 ) 
	{
		try {
			stringstream ssport;
			ssport << argv[2];
			ssport >> port;
		}
		catch (...) {
			std::cout << "Illegal port number (" << argv[2] << ")" << std::endl;
			printUsage();
			return GGPSERVER_ERROR_BAD_PORT;
		}
		
	}
	cout << "Using port: " << port << endl;
    cout << endl;
    
    Server server( argv[1], port, GGPSERVER_DEFAULT_RESTART);
#else
	Server server( GGPSERVER_DEFAULT_EXECUTABLE, GGPSERVER_DEFAULT_PORT, GGPSERVER_DEFAULT_RESTART );
#endif
	server.start();
    return GGPSERVER_OK;
}
