/****************************************************************************
 **
 ** GGP Server
 **
 ****************************************************************************/

#ifndef SERVER_H
#define SERVER_H

#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <string>
#include <fstream>
#include <algorithm>
#include "../utils/childprocess.h"
#include "../utils/timer.h"

namespace cadiaplayer {
	namespace server {
		
		class SocketWrapper
			{
			private: 
				boost::asio::ip::tcp::socket s;
			public:
				SocketWrapper(boost::asio::io_service& service) : s(service){};
				boost::asio::ip::tcp::socket& socket(){return s;};
			};
		typedef boost::shared_ptr<SocketWrapper> ClientSocket;
		
		class Server
			{
			private:
				boost::asio::io_service				m_io_service;
				boost::asio::strand					m_strand;
				boost::asio::ip::tcp::acceptor		m_tcpServer;
				std::ofstream						m_log;
				std::string							m_programName;
				cadiaplayer::utils::ChildProcess*	m_program;
				int									m_port;
				int									m_cntProcessedRequests;
				bool								m_doRestart;
				
			public:
				Server( std::string programName, int port, bool doRestart = false );
				~Server();
				
				void start();
				
			private :
				void accept();
				void handleConnection		(ClientSocket& socket, const boost::system::error_code& error);
				std::string getHTTPRequest	(ClientSocket& socket, bool& emptyok);
				void readStart				(ClientSocket& socket, std::stringstream& ss);
				void readRest				(ClientSocket& socket, int totalLength, std::stringstream& ss);
				std::string processRequest	(ClientSocket& socket, std::string request);
				void sendHTTPResponse		(ClientSocket& socket, std::string response);
				void disconnect				(ClientSocket& socket);
				bool startProgram			(ClientSocket* socket);
				void stopProgram(bool immediately=false);
				void log(std::string str);
				void log(std::string pre, std::string str, std::string post);
			};
	}
}
#endif
