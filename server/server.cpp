
/****************************************************************************
**
** GGP Server
**
****************************************************************************/

#include <iostream>
#include <sstream>
#include "server.h"

using namespace std;
using namespace cadiaplayer::server;
using namespace boost::asio::ip;

Server::Server( std::string programName, int port, bool doRestart ) :
m_tcpServer(m_io_service, tcp::endpoint(tcp::v4(), port)),
m_strand(m_io_service),
m_port(port),
m_program(NULL),
m_programName(programName), 
m_cntProcessedRequests(0), 
m_doRestart(doRestart)
{
	m_log.open( "server.log" );
	startProgram(NULL);
    cerr << "Listening to port: " << m_port << endl;
}


Server::~Server( )
{
    stopProgram();
    m_log.close();
}

void Server::start()
{
	accept();
	m_io_service.run();
}
void Server::accept()
{
	ClientSocket socket = ClientSocket(new SocketWrapper(m_io_service));
	log("Accepting connections...");
	m_tcpServer.async_accept(socket->socket(),
		m_strand.wrap(boost::bind(&Server::handleConnection, this, socket,boost::asio::placeholders::error)));
}
void Server::handleConnection(ClientSocket& socket, const boost::system::error_code& error)
{
	string request = "";
	string response = ""; 
	log("Connection accepted");
	bool emptyresponse;
	request = getHTTPRequest(socket, emptyresponse );
	log("HTTP request received");
	response = processRequest(socket, request);
	int failThreshold = 10;
	if(response == "" && emptyresponse)
		log("HTTP empty response bounced back to an empty request");
	else
	{
		while(response == "")
		{
			if(!failThreshold)
				break;
			--failThreshold;
			log("Re-Starting program because responce was empty");
			log("Calling stopProgram...");
			stopProgram(true);
			log("Calling startProgram...");
			startProgram(&socket);
			log("Trying to re-generate the responce");
			response = processRequest(socket, request);
		}
	}
	if(failThreshold)
	{
		log("HTTP response generated");
		sendHTTPResponse(socket, response);
	}
	else
	{
		log("ERROR processing HTTP request");
		sendHTTPResponse(socket, "ERROR processing HTTP request");
	}
	log("HTTP response sent");
	disconnect(socket);
	log("Connection closed");
	
	accept();
/*	string request;
	string response = ""; 
	log("Connection accepted");
	request = getHTTPRequest(socket);
	log("HTTP request received");*/
	/*if(!request.empty())
	{	
		response = processRequest(socket, request);
	
		int failThreshold = 10;
		while(response == "")
		{
			if(!failThreshold)
				break;
			--failThreshold;
			log("Re-Starting program because responce was empty");
			log("Calling stopProgram...");
			stopProgram(true);
			log("Calling startProgram...");
			startProgram(&socket);
			log("Trying to re-generate the responce");
			response = processRequest(socket, request);
		}
		if(failThreshold)
		{
			log("HTTP response generated");
			sendHTTPResponse(socket, response);
		}
		else
		{
			log("ERROR processing HTTP request");
			sendHTTPResponse(socket, "ERROR processing HTTP request");
		}
		
	}*/	
/*	
	if(!request.empty())
	{	
		response = processRequest(socket, request);
		log("HTTP response generated");
	}
	else
		log("HTTP empty response bounced back to an empty request");
	
	sendHTTPResponse(socket, response);
	log("HTTP response sent");
		
	disconnect(socket);
	log("Connection closed");
		
	accept();*/
}
void Server::disconnect(ClientSocket& socket)
{
	socket->socket().close();
}
std::string Server::getHTTPRequest(ClientSocket& socket, bool& emptyok)
{
	emptyok = false;
	while(!socket->socket().available())
	{
		sleep(0);
	}
	stringstream request;
	try
	{
		readStart(socket, request);
	}
	catch (std::exception& e)
	{
		std::cerr << "HTTP request error caugth : " << e.what() << std::endl;
		return "";
	}
	//std::cerr << request.str();
	int contentLength = 0;
	string lower = request.str();
	std::transform(lower.begin(), lower.end(), lower.begin(), (int(*)(int))tolower);
	if(lower.size() >= 7 && lower.substr(0,7) == "options")
	{
		// OPTIONS request, no data
		log("OPTIONS request, no data");
		emptyok = true;
		return "";
	}
	int clPos = lower.find("content-length:");
	if(clPos != lower.npos)
	{
		clPos += 15;
		request.seekg(clPos);
		request >> contentLength;
	}
	int offset = 4;
	int headerEnds = request.str().find("\r\n\r\n");
	if(headerEnds == request.str().npos)
	{
		offset = 2;
		headerEnds = request.str().find("\n\n");
	}
	if(headerEnds == request.str().npos)
	{
		std::cerr << "HTTP Header end not found.  Trying returning the content as the whole HTTP request." << std::endl;
		log("H:[NOT DETECTED]");
		log("C:[", request.str(), "]");
		return request.str();
	}
	int totalLength = headerEnds+offset+contentLength;
	if(totalLength > request.str().size())
	{
		try
		{
			readRest(socket, totalLength, request);
		}
		catch (std::exception& e)
		{
			std::cerr << "HTTP request error caugth : " << e.what() << std::endl;
			return "";
		}
	}
	
	/*std::cerr << endl;
	std::cerr << "Header ends    : " << headerEnds << endl;
	std::cerr << "Offset         : " << offset << endl;
	std::cerr << "Content length : " << contentLength << endl;
	std::cerr << "Total length   : " << totalLength << endl;
	std::cerr << "request length : " << request.str().size() << endl;*/
	
	string header = request.str().substr(0,headerEnds);
	string content = "";
	if(contentLength)
		content = request.str().substr(headerEnds+offset);
	log("H:[", header, "]");
	log("C:[", content, "]");
	return content;
}
void Server::readStart(ClientSocket& socket, std::stringstream& ss)
{
	int len = 0;
	boost::array<char, 128> buf;
	boost::system::error_code error;
	while(ss.str().size() < 1 || socket->socket().available())
	{
		len = socket->socket().read_some(boost::asio::buffer(buf), error);
		ss.write(buf.data(), len);
		if (error == boost::asio::error::eof)
			break; // Connection closed cleanly by peer.
		else if (error)
			throw boost::system::system_error(error); // Some other error.
	}
}
void Server::readRest(ClientSocket& socket, int totalLength, std::stringstream& ss)
{
	int len = 0;
	boost::array<char, 128> buf;
	boost::system::error_code error;
	while(ss.str().size() < totalLength)
	{
		len = socket->socket().read_some(boost::asio::buffer(buf), error);
		ss.write(buf.data(), len);
		if (error == boost::asio::error::eof)
			break; // Connection closed cleanly by peer.
		else if (error)
			throw boost::system::system_error(error); // Some other error.
	}
}

std::string Server::processRequest(ClientSocket& socket, std::string request)
{
	if(!request.size())
	{
		std::cerr << "Not calling program because the request content is empty" << endl;
		return "";
	}
	log("RCVD-FROM-TCP: [", request.c_str(), "]");
    
    string str = request + "\r\n";
	for ( unsigned int i=0; i< str.size(); ++i ) 
	{  
		// convert to lower-case.
        str[i] = tolower(str[i]);
	}
        
	if ( m_doRestart && m_cntProcessedRequests > 0 && strncmp( str.c_str(), " ( start", 8 ) == 0 ) 
	{
		std::cerr << "Need to restart program (cause:1)" << std::endl;
		// need to restart program.
		std::cerr << "Stopping program" << std::endl;
		stopProgram();
		std::cerr << "Starting program" << std::endl;
		startProgram(&socket);
		std::cerr << "Program started" << std::endl;
	}
	else if ( m_doRestart && m_cntProcessedRequests > 0 && strncmp( str.c_str(), "( start", 7 ) == 0 ) 
	{
		std::cerr << "Need to restart program (cause:2)" << std::endl;
		// need to restart program.
		std::cerr << "Stopping program" << std::endl;
		stopProgram();
		std::cerr << "Starting program" << std::endl;
		startProgram(&socket);
		std::cerr << "Program started" << std::endl;
	}
	else if ( m_doRestart && m_cntProcessedRequests > 0 && strncmp( str.c_str(), "(start", 6 ) == 0 ) 
	{
		std::cerr << "Need to restart program (cause:3)" << std::endl;
		// need to restart program.
		std::cerr << "Stopping program" << std::endl;
		stopProgram();
		std::cerr << "Starting program" << std::endl;
		startProgram(&socket);
		std::cerr << "Program started" << std::endl;
	}
	else if ( m_doRestart && m_cntProcessedRequests > 0 && strncmp( str.c_str(), "(load", 5 ) == 0 ) 
	{
		std::cerr << "Need to restart program (cause:4)" << std::endl;
		// need to restart program.
		std::cerr << "Stopping program" << std::endl;
		stopProgram();
		std::cerr << "Starting program" << std::endl;
		startProgram(&socket);
		std::cerr << "Program started" << std::endl;
	}
	/*else // not a restarting request
	{
		std::cerr << "Passing non-restarting message to program " << std::endl;
	}*/
	m_cntProcessedRequests++;
    
    log("SEND-TO-PRG:[", str, "]");
     
	m_program->writeToStdIn( str );
	string msg = m_program->readFromStdOut( );
	
	log("RCVI-FROM-PRG: [", msg, "]");
	return msg;

}

void Server::sendHTTPResponse(ClientSocket& socket, std::string response)
{
	stringstream out;
	
	out << "HTTP/1.0 200 OK\n";

	// For Tilty yard ggp.org kiosk start
	out << "Access-Control-Allow-Origin: *\n";
	out << "Access-Control-Allow-Methods: POST, GET, OPTIONS\n";
	out << "Access-Control-Allow-Headers: Content-Type\n";
	out << "Access-Control-Allow-Age: 86400\n";
	// For Tilty yard ggp.org kiosk end
	
	out << "Content-type: text/acl\n";
	out << "Content-length: "; 
	out << response.size();
	out << "\r\n\r\n";
	out << response;
	//cerr << endl << out.str() << endl;
	log("SEND-TO-TCP:[", out.str(), "]");
	string reply = out.str();
	try
	{
		boost::system::error_code ignored_error;
		boost::asio::write(socket->socket(), boost::asio::buffer(reply),
						   boost::asio::transfer_all(), ignored_error);
	}
	catch (std::exception& e)
	{
		log("Received empty HTTP msg from serer");
	}
}
		  
bool Server::startProgram(ClientSocket* socket)
{
	bool ok = false;
	m_cntProcessedRequests = 0;
    m_program = new cadiaplayer::utils::ChildProcess( m_programName );    
    if ( m_program != NULL ) 
	{
        cerr << "Trying to start program '" << m_programName << "'" << endl;
        if(socket != NULL && m_program->start(m_io_service, (*socket)->socket())) 
		{
            cerr << "GGP Program started sucessfully and closed an additional forked socket." << endl;
            ok = true;
        }
		else if(m_program->start()) 
		{
            cerr << "GGP Program started sucessfully!" << endl;
            ok = true;
        }
    }
	else
		cerr << "Program did not start successfully!" << endl;
    return ok;
}


void Server::stopProgram(bool immediately)
{
    if ( m_program != NULL ) 
	{
		if(!immediately)
		{
			cerr << "Telling program to quit" << endl;
			if(m_program->writeToStdIn( "(quit)\n", false ))
				cerr << "Quit successfully sent to program" << endl;
			else
				cerr << "Quit not sent to program - not able to communicate with it" << endl;
		}
		m_program->stop();
        cerr << "Program stopped successfully!" << endl;
		delete m_program;
        m_program = NULL;
    }
	else
		cerr << "No program to stop" << endl;
}

void Server::log(std::string str)
{
	m_log << cadiaplayer::utils::Timer::getTimeStamp() << " : " << str << endl;
}
void Server::log(std::string pre, std::string str, std::string post)
{
	m_log << cadiaplayer::utils::Timer::getTimeStamp() << " : " << pre << str << post << endl;
}
