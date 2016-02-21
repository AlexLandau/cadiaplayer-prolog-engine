
/****************************************************************************
 **
 ** TCP Communication software.
 **
 ****************************************************************************/

#include <iostream>
#include <string>
#include <sstream>
#include "tcpserver.h"
using namespace cadiaplayer::utils;
using namespace std;
using namespace boost::asio::ip;

#ifdef USE_READWRITE_BOOST_MUTEX
#include <boost/thread.hpp>
boost::mutex tcpserver_read_mutex;
boost::mutex tcpserver_write_mutex;
#endif 

#ifdef USE_READWRITE_PTHREAD_MUTEX
#include <pthread.h>
pthread_mutex_t tcpserver_read_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t tcpserver_write_mutex = PTHREAD_MUTEX_INITIALIZER;
#endif

//////////////////////////////////  STATIC FUNCTIONS  ///////////////////////////////////////

#ifdef USE_READWRITE_INSTANCIATED
bool TCPClient::readMsg( tcp::socket *clientConnection, Message& msg, vector<char> buffer, int timeOut )
#else
static bool readMsg( tcp::socket *clientConnection, Message& msg, vector<char> buffer, int timeOut = -1 )
#endif
{
	#ifdef USE_READWRITE_BOOST_MUTEX
	boost::mutex::scoped_lock lock_it( tcpserver_read_mutex );
	#endif
	#ifdef USE_READWRITE_PTHREAD_MUTEX
	pthread_mutex_lock( &tcpserver_read_mutex );
	#endif
	
	boost::array<char, 128> buf;
	size_t len = 0;
	boost::system::error_code error;
	if(timeOut != -1)
	{
		if(!clientConnection->available() && !buffer.size())
		{
#ifdef USE_READWRITE_PTHREAD_MUTEX	
			pthread_mutex_unlock( &tcpserver_read_mutex );
#endif
			return false;
		}
	}
	//cerr << "Blocking read" << endl;
	while(buffer.size() < sizeof(unsigned int))
	{
		len = clientConnection->read_some(boost::asio::buffer(buf), error);
		for(int n = 0 ; n < len ; n++)
		{
			buffer.push_back(buf.data()[n]);
		}
		if (error == boost::asio::error::eof)
		{
			// break; // Connection closed cleanly by peer.
			clientConnection->close();
#ifdef USE_READWRITE_PTHREAD_MUTEX	
			pthread_mutex_unlock( &tcpserver_read_mutex );
#endif
			return false; // Some other error.
		}	
		else if (error)
		{
			clientConnection->close();
#ifdef USE_READWRITE_PTHREAD_MUTEX	
			pthread_mutex_unlock( &tcpserver_read_mutex );
#endif
			return false; // Some other error.
		}
	}
	/*if(!buffer.size())
	{
		std::cerr << "Message resulting in an empty buffer received!\n";
		return true;
	}*/
	//cerr << "Data available" << endl;
	msg.setID( 0 );
	msg.setType( Message::msgUnknown );
	msg.clearContent( );
	msg.setReturnCode( -1 );
	unsigned int contentSize = 0;
	memcpy(&contentSize, &buffer[0], sizeof(unsigned int));
	buffer.erase(buffer.begin(), buffer.begin()+4);
	//std::cerr << "Receive Contentsize : " << contentSize << std::endl;
	try
	{
		while (buffer.size() < contentSize)
		{
			len = clientConnection->read_some(boost::asio::buffer(buf), error);
			//std::cout << "[Raw data(" << len << "):" << buf.data() << "]\n"; 
			for(int n = 0 ; n < len ; n++)
			{
				buffer.push_back(buf.data()[n]);
			}
			if (error == boost::asio::error::eof)
				break; // Connection closed cleanly by peer.
			else if (error)
				throw boost::system::system_error(error); // Some other error.
		}
	}
	catch (std::exception& e)
	{
		clientConnection->close();
		std::cerr << "readMsg error caugth : " << e.what() << std::endl;
#ifdef USE_READWRITE_PTHREAD_MUTEX	
		pthread_mutex_unlock( &tcpserver_read_mutex );
#endif
		return false;
	}
	if(!buffer.size())
	{	
#ifdef USE_READWRITE_PTHREAD_MUTEX	
		pthread_mutex_unlock( &tcpserver_read_mutex );
#endif
		return false;
	}
	
	stringstream ss;
	ss.write(static_cast<char*>(&*buffer.begin()), buffer.size());
	streampos begin = ss.tellg();
				 
	Message::MsgType msgType;
	int  msgID;
	char msgCommand;
	int  msgLen;
	
	ss >> msgID >> msgCommand >> msgLen; 
	
	msg.setID( msgID );
	switch ( msgCommand ) {
		case 'G': msgType = Message::msgGame; break;
		case 'D': msgType = Message::msgGameDesc; break;
		case 'S': msgType = Message::msgSearch; break;
		case 'P': msgType = Message::msgPing; break;
		case 'F': msgType = Message::msgFinish; break;
		case 'R': msgType = Message::msgReset; break;
		case 'Q': msgType = Message::msgQuit; break;
		case 'U': msgType = Message::msgUpdateHH; break;
		case 'L': msgType = Message::msgParallel; break;
		default : msgType = Message::msgUnknown; break;
	}
	msg.setType( msgType );
	if ( msgLen < 0 )
	{
		buffer.erase(buffer.begin(), buffer.begin() + (ss.tellg()-begin)); 
#ifdef USE_READWRITE_PTHREAD_MUTEX	
		pthread_mutex_unlock( &tcpserver_read_mutex );
#endif
		return false;
	}
	std::stringstream content;
	if(msgLen > 0)
		ss.get();
	for(int n = 0 ; n < msgLen ; n++)
	{
		content.put(ss.get());
	}
	msg.setContent( content.str() );
	msg.setReturnCode( 0 );
	
	buffer.erase(buffer.begin(), buffer.begin() + (ss.tellg()-begin));
	
	//std::cerr << "Read : <" << msg.toStr() << ">" << std::endl;
#ifdef USE_READWRITE_PTHREAD_MUTEX	
	pthread_mutex_unlock( &tcpserver_read_mutex );
#endif
	return true;
}

#ifdef USE_READWRITE_INSTANCIATED
bool TCPClient::writeMsg( tcp::socket *clientConnection, const Message& msg )
#else
static bool writeMsg( tcp::socket *clientConnection, const Message& msg )
#endif
{
#ifdef USE_READWRITE_BOOST_MUTEX
	boost::mutex::scoped_lock lock_it( tcpserver_write_mutex );
#endif
#ifdef USE_READWRITE_PTHREAD_MUTEX
	pthread_mutex_lock( &tcpserver_write_mutex );
#endif
	
	char msgCommand;
    switch ( msg.getType() ) {
        case Message::msgGame:     msgCommand = 'G'; break;
        case Message::msgGameDesc: msgCommand = 'D'; break;
        case Message::msgSearch:   msgCommand = 'S'; break;
        case Message::msgPing:     msgCommand = 'P'; break;
        case Message::msgFinish:   msgCommand = 'F'; break;
        case Message::msgReset:    msgCommand = 'R'; break;
        case Message::msgQuit:     msgCommand = 'Q'; break;
        case Message::msgUpdateHH: msgCommand = 'U'; break;
        case Message::msgParallel: msgCommand = 'L'; break;
        default:                   msgCommand = '?'; break;
    }
    
	std::stringstream ss;
	ss << msg.getID() << ' ' << msgCommand << ' ' << msg.getContent().size() << '\n'  << msg.getContent();
	unsigned int contentSize = ss.str().size();
	//std::cerr << "Sent Contentsize : " << contentSize << std::endl;
	char* sizep= reinterpret_cast<char*>(&contentSize);
	bool ok = true;
	//std::cerr << "Attempting to write : <" << msg.toStr() << ">" << std::endl;
	try
	{
		boost::system::error_code ignored_error;
		boost::asio::write(*clientConnection, boost::asio::buffer(sizep, sizeof(unsigned int)),
						   boost::asio::transfer_all(), ignored_error);
		boost::asio::write(*clientConnection, boost::asio::buffer(ss.str()),
						   boost::asio::transfer_all(), ignored_error);
		//clientConnection->close();
	}
	catch (std::exception& e)
	{
		std::cerr << "writeMsg failed: " << e.what() << std::endl;
		ok = false;
	}
	
	//std::cerr << "Wrote : <" << msg.toStr() << ">" << std::endl;
#ifdef USE_READWRITE_PTHREAD_MUTEX	
	pthread_mutex_unlock( &tcpserver_write_mutex );
#endif
    return ok;
}

/////////////////////////////////////  TCP CLIENT //////////////////////////////////////////


TCPClient::TCPClient( std::string host, short port )
: m_host( host ), m_port( port ), m_connected ( false )
{
	m_tcpSocket = new tcp::socket(m_io_service);
}


TCPClient::~TCPClient()
{
    delete m_tcpSocket;
}


bool TCPClient::connect( int msecMaxWait )
{
	if ( m_tcpSocket == 0 ) return false;    
	if ( this->isConnected() ) return true;
	
	boost::system::error_code error = boost::asio::error::host_not_found;
	
	boost::system::error_code ec;
	boost::asio::ip::address addr = boost::asio::ip::address::from_string(m_host, ec);
	boost::asio::detail::throw_error(ec);
	boost::asio::ip::tcp::endpoint host(addr, m_port);
	
	//std::cerr << "Trying to connect: '" << m_host << "' " << m_port << std::endl;
	m_tcpSocket->close();
	m_tcpSocket->connect(host, error);
	
	if(!error)
	{
		std::cerr << "Connected to : '" << m_host << "' " << m_port << std::endl;
		m_connected = true;
		return true;
	}
	else
	{
		//std::cerr << "Failed to connect to : '" << m_host << "' " << m_port << std::endl;
		return false;
	}
	//return m_tcpSocket->waitForConnected( msecMaxWait );
}
void TCPClient::disconnect() 
{
    if ( m_tcpSocket != 0 )
       m_tcpSocket->close();
	m_connected = false;
}

bool TCPClient::isConnected( ) const
{
	//std::cout << '{' << m_tcpSocket->state() << '}' << std::endl;
    bool connected =  m_connected && m_tcpSocket != 0 && m_tcpSocket->is_open();
	return connected;
}


bool TCPClient::send( const Message& msg )
{
    if ( !this->isConnected() ) return false;
    
    writeMsg( m_tcpSocket, msg ); 
	
	if(msg.getType() == Message::msgReset)
	{
		disconnect();
	}
    return true;
}


bool TCPClient::read( Message& msg, bool blocking )
{
	if ( !this->isConnected() ) 
		return false;
	int timeOut = (blocking) ? -1 : 0;
	//if ( !blocking && m_tcpSocket->bytesAvailable() == 0 ) return false;
	
	bool result = readMsg( m_tcpSocket, msg, buffer, timeOut );
	//if(!result)
	//	disconnect();
	//if(msg.getType() == Message::msgReset)
	//	disconnect();
	return result;
}

/////////////////////////////////////  TCP SERVER //////////////////////////////////////////

TCPServer::TCPServer( int port )
: m_tcpServer( 0 ), m_clientConnection( 0 ), m_port(port)
{
	m_tcpServer = new tcp::acceptor(m_io_service);
	boost::asio::ip::tcp::endpoint endpoint(boost::asio::ip::tcp::v4(), port);
	m_tcpServer->open(endpoint.protocol());
	m_tcpServer->set_option(boost::asio::ip::tcp::acceptor::reuse_address(true));
	m_tcpServer->bind(endpoint);
	m_tcpServer->listen();
	
	accept();
}

void TCPServer::accept()
{
	if(m_clientConnection != 0)
	{
		if(m_clientConnection->is_open())
			m_clientConnection->close();
		delete m_clientConnection;
	}
	m_clientConnection = new tcp::socket(m_io_service);
	
	std::cerr << "Starting accept...\n";
	m_tcpServer->accept(*m_clientConnection);
	std::cerr << "...accept started\n";
}

TCPServer::~TCPServer( )
{
	if ( m_clientConnection != 0 ) 
	{
		m_clientConnection->close();
		delete m_clientConnection;
		m_clientConnection = 0;
	}
	delete m_tcpServer;
}


bool TCPServer::wait( Message& msg )
{
    // NOTE: delete clientConnection?
	if ( m_clientConnection == 0 || !m_clientConnection->is_open() ) 
	{
		//std::cout << "Waiting for new connection ..." << std::endl;
		m_clientConnection = new tcp::socket(m_io_service);
		m_tcpServer->accept(*m_clientConnection);
		//std::cerr << " ... new connection." << std::endl;
	}
	bool ok = readMsg( m_clientConnection, msg, buffer );
	if(!ok)
		disconnect();
	return ok;
}


bool TCPServer::send( const Message& msg )
{
    return writeMsg( m_clientConnection, msg );
}

bool TCPServer::isConnected( ) const
{
	bool connected =  (m_clientConnection != 0) && (m_clientConnection->is_open());
	if(connected)
		std::cerr << "Connection check ok" << std::endl;
	else
		std::cerr << "Connection check failed" << std::endl;
	return connected;
}

bool TCPServer::disconnect() 
{
	if ( m_clientConnection != 0 ) 
	{
		//m_clientConnection->disconnectFromHost();
		m_clientConnection->close();
		// NOTE: delete m_clientConnection.
	}
	return 0;
}

#ifdef USE_READWRITE_INSTANCIATED
bool TCPServer::readMsg( tcp::socket *clientConnection, Message& msg, vector<char> buffer, int timeOut )
{
	boost::array<char, 128> buf;
	size_t len = 0;
	boost::system::error_code error;
	if(timeOut != -1)
	{
		if(!clientConnection->available() && !buffer.size())
			return false;
	}
	//cerr << "Blocking read" << endl;
	while(buffer.size() < sizeof(unsigned int))
	{
		len = clientConnection->read_some(boost::asio::buffer(buf), error);
		for(int n = 0 ; n < len ; n++)
		{
			buffer.push_back(buf.data()[n]);
		}
		if (error == boost::asio::error::eof)
		{
			// break; // Connection closed cleanly by peer.
			clientConnection->close();
			return false; // Some other error.
		}	
		else if (error)
		{
			clientConnection->close();
			return false; // Some other error.
		}
	}
	/*if(!buffer.size())
	 {
	 std::cerr << "Message resulting in an empty buffer received!\n";
	 return true;
	 }*/
	//cerr << "Data available" << endl;
	msg.setID( 0 );
	msg.setType( Message::msgUnknown );
	msg.clearContent( );
	msg.setReturnCode( -1 );
	unsigned int contentSize = 0;
	memcpy(&contentSize, &buffer[0], sizeof(unsigned int));
	buffer.erase(buffer.begin(), buffer.begin()+4);
	//std::cerr << "Receive Contentsize : " << contentSize << std::endl;
	try
	{
		while (buffer.size() < contentSize)
		{
			len = clientConnection->read_some(boost::asio::buffer(buf), error);
			//std::cout << "[Raw data(" << len << "):" << buf.data() << "]\n"; 
			for(int n = 0 ; n < len ; n++)
			{
				buffer.push_back(buf.data()[n]);
			}
			if (error == boost::asio::error::eof)
				break; // Connection closed cleanly by peer.
			else if (error)
				throw boost::system::system_error(error); // Some other error.
		}
	}
	catch (std::exception& e)
	{
		clientConnection->close();
		std::cerr << "readMsg error caugth : " << e.what() << std::endl;
		return false;
	}
	if(!buffer.size())
		return false;
	
	stringstream ss;
	ss.write(static_cast<char*>(&*buffer.begin()), buffer.size());
	streampos begin = ss.tellg();
	
	Message::MsgType msgType;
	int  msgID;
	char msgCommand;
	int  msgLen;
	
	ss >> msgID >> msgCommand >> msgLen; 
	
	msg.setID( msgID );
	switch ( msgCommand ) {
		case 'G': msgType = Message::msgGame; break;
		case 'D': msgType = Message::msgGameDesc; break;
		case 'S': msgType = Message::msgSearch; break;
		case 'P': msgType = Message::msgPing; break;
		case 'F': msgType = Message::msgFinish; break;
		case 'R': msgType = Message::msgReset; break;
		case 'Q': msgType = Message::msgQuit; break;
		case 'U': msgType = Message::msgUpdateHH; break;
		case 'L': msgType = Message::msgParallel; break;
		default : msgType = Message::msgUnknown; break;
	}
	msg.setType( msgType );
	if ( msgLen < 0 )
	{
		buffer.erase(buffer.begin(), buffer.begin() + (ss.tellg()-begin)); 
		return false;
	}
	std::stringstream content;
	if(msgLen > 0)
		ss.get();
	for(int n = 0 ; n < msgLen ; n++)
	{
		content.put(ss.get());
	}
	msg.setContent( content.str() );
	msg.setReturnCode( 0 );
	
	buffer.erase(buffer.begin(), buffer.begin() + (ss.tellg()-begin));
	
	//std::cerr << "Read : <" << msg.toStr() << ">" << std::endl;
	
	return true;
}


bool TCPServer::writeMsg( tcp::socket *clientConnection, const Message& msg )
{
	char msgCommand;
    switch ( msg.getType() ) {
        case Message::msgGame:     msgCommand = 'G'; break;
        case Message::msgGameDesc: msgCommand = 'D'; break;
        case Message::msgSearch:   msgCommand = 'S'; break;
        case Message::msgPing:     msgCommand = 'P'; break;
        case Message::msgFinish:   msgCommand = 'F'; break;
        case Message::msgReset:    msgCommand = 'R'; break;
        case Message::msgQuit:     msgCommand = 'Q'; break;
        case Message::msgUpdateHH: msgCommand = 'U'; break;
        case Message::msgParallel: msgCommand = 'L'; break;
        default:                   msgCommand = '?'; break;
    }
    
	std::stringstream ss;
	ss << msg.getID() << ' ' << msgCommand << ' ' << msg.getContent().size() << '\n'  << msg.getContent();
	unsigned int contentSize = ss.str().size();
	//std::cerr << "Sent Contentsize : " << contentSize << std::endl;
	char* sizep= reinterpret_cast<char*>(&contentSize);
	bool ok = true;
	//std::cerr << "Attempting to write : <" << msg.toStr() << ">" << std::endl;
	try
	{
		boost::system::error_code ignored_error;
		boost::asio::write(*clientConnection, boost::asio::buffer(sizep, sizeof(unsigned int)),
						   boost::asio::transfer_all(), ignored_error);
		boost::asio::write(*clientConnection, boost::asio::buffer(ss.str()),
						   boost::asio::transfer_all(), ignored_error);
		//clientConnection->close();
	}
	catch (std::exception& e)
	{
		std::cerr << "writeMsg failed: " << e.what() << std::endl;
		ok = false;
	}
	
	//std::cerr << "Wrote : <" << msg.toStr() << ">" << std::endl;
	
    return ok;
}
#endif
