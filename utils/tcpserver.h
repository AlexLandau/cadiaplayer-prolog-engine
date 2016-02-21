/****************************************************************************
 **
 ** TCP Communication software.
 **
 ****************************************************************************/

#ifndef TCPSERVER_H
#define TCPSERVER_H

#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <string>
#include <sstream>

#define USE_READWRITE_INSTANCIATED
//#define USE_READWRITE_BOOST_MUTEX
//#define USE_READWRITE_PTHREAD_MUTEX

namespace cadiaplayer {
	namespace utils {
		class Message
			{
			public:
				typedef enum { msgGame, msgGameDesc, msgSearch, msgPing, msgFinish, msgReset, msgQuit, msgUpdateHH, msgParallel, msgUnknown } MsgType;
				
				Message( MsgType type = Message::msgUnknown ):
				m_id(0), m_type(type), m_returnCode(0) {}
				
				int          getID() const { return m_id; }
				MsgType      getType() const { return m_type; }
				std::string  getContent() const { return m_content; }
				int          getReturnCode() const { return m_returnCode; }
				
				void setID( int mid )					{ m_id = mid; }
				void setType( MsgType type )			{ m_type = type; }
				void setContent( std::string msg )		{ m_content = msg; }
				void clearContent( )					{ m_content.clear(); }
				void setReturnCode( int code )			{ m_returnCode = code; }
				
				std::string toStr() const {
					std::stringstream ss;
					ss << '[' << m_id << ':' << m_type << ':' 
					<< m_returnCode << ':' << m_content << ']'; 
					return ss.str();
				}
				
			private:
				int         m_id;
				MsgType     m_type;
				std::string m_content;
				int         m_returnCode;
			};
		
		
		class TCPClient
			{
				bool m_connected;
			public:
				TCPClient( std::string host, short port );
				~TCPClient();
				
				bool isConnected( ) const;
				bool connect( int  msecMaxWait = 200 );
				void disconnect(); 
				bool send( const Message& msg );    
				bool read( Message& msg, bool blocking = false );    
				std::vector<char> buffer;
			private:
				std::string  m_host;
				short        m_port;
				boost::asio::io_service m_io_service;
				boost::asio::ip::tcp::socket *m_tcpSocket;
				
#ifdef USE_READWRITE_INSTANCIATED
				bool readMsg( boost::asio::ip::tcp::socket *clientConnection, Message& msg, std::vector<char> buffer, int timeOut = -1 );
				bool writeMsg( boost::asio::ip::tcp::socket *clientConnection, const Message& msg );
#endif
			};
		
		
		class TCPServer
			{
			public:
				TCPServer( int port );
				~TCPServer();
				
				void accept();
				bool wait( Message& msg );       
				bool send( const Message& msg ); 
				bool isConnected( ) const;
				bool disconnect(); 
				
			private:
				int				m_port;
				boost::asio::io_service		m_io_service;
				boost::asio::ip::tcp::acceptor*	m_tcpServer;
				boost::asio::ip::tcp::socket*	m_clientConnection;    
				std::vector<char>		buffer;
				
#ifdef USE_READWRITE_INSTANCIATED
				bool readMsg( boost::asio::ip::tcp::socket *clientConnection, Message& msg, std::vector<char> buffer, int timeOut = -1 );
				bool writeMsg( boost::asio::ip::tcp::socket *clientConnection, const Message& msg );
#endif
			};
	}} // namespaces
#endif

