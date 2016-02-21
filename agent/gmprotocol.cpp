#include "gmprotocol.h"

using namespace std;
using namespace cadiaplayer::agent;

bool GMProtocol::hasMessage()
{
	int flags = fcntl(STDIN_FILENO, F_GETFL);
	fcntl(STDIN_FILENO, F_SETFL, flags | O_NONBLOCK);
	char c;
	do
	{
		
		//std::cerr << "starting read...";
		c=fgetc(stdin);
		//std::cerr << "read : " << c << std::endl;
		if(!delimeter(c))
			break;
	}
	while(true);
	fcntl(STDIN_FILENO, F_SETFL, flags);
	if(c != EOF)
	{
		cin.putback(c);
		return true;
	}
	return false;
}
GMMessage GMProtocol::getMessage()
{
	GMMessage msg;
	
	char c;
	// clear leading delimeters
	do{c=getNextChar();}while(delimeter(c));
	// Check for a legal start
	if(c != '(')
	{
		if(c != GM_RESERVED_CHAR)
		{
			cerr << "Message start error, got #" << c << "#(" << ((int)c) << ")\n";
			msg.type = gm_ERROR;
		}
		else
		{
			std::stringstream ss;
			c = getNextChar();
			while(c != GM_RESERVED_CHAR)
			{
				ss << c;
				c = getNextChar();
			}
			ss << c;
			msg.readFromStream(ss);
		}
		return msg;
	}
	// Determine the message type
	msg.type = getMessageType();
	//return msg;
	if(msg.type == gm_ERROR)
	{
		cerr << "Message type error\n";
		return msg;
	}
	else if(msg.type == gm_KILL)
		return msg;
	else if(msg.type == gm_CONFIDENCE)
		return msg;
	else if(msg.type == gm_MCPARALLEL)
		return msg;
	else if(msg.type == gm_STATE || msg.type == gm_THINK || msg.type == gm_AVAIL || msg.type == gm_TERM || msg.type == gm_GOAL || msg.type == gm_HALT)
		return msg;
	else if(msg.type == gm_UNDO)
		return msg;
	else if(msg.type == gm_PING)
	{	
		c=getNextChar();
		if(delimeter(c))
		{
			do{c=getNextChar();/*cerr << "delimiter\n";*/}while(delimeter(c));
		}
		if(c != ')')
		{
			cerr << "PING Message with no closing parenthesis error\n";
			//msg.type = gm_ERROR;
		}
		fflush(stdin);
		//cerr << msg.toString().c_str() << endl;
		return msg;
	}
	msg.matchId = getMessageString();
	// Abort messages has nothing more
	if(msg.type == gm_ABORT)
	{
		c=getNextChar();
		if(delimeter(c))
		{
			do{c=getNextChar();/*cerr << "delimiter\n";*/}while(delimeter(c));
		}
		if(c != ')')
		{
			cerr << "ABORT Message with no closing parenthesis error\n";
			//msg.type = gm_ERROR;
		}
		fflush(stdin);
		//cerr << msg.toString().c_str() << endl;
		return msg;
	}
	// START messages also include role
	if(msg.type == gm_START || msg.type == gm_LOAD || msg.type == gm_FILE )
		msg.role = getMessageString();
	bool terminated = false;
	msg.kif = collectKifBlob(terminated, c);
	if(delimeter(c))
	{
		do{c=getNextChar();}while(delimeter(c));
		if(c == ')')
			terminated = true;
	}
	if(msg.kif.empty())
		return msg;
	//std::cerr << "KIF collected: " << msg.kif << std::endl;
	//std::cerr << "Terminated   : " << terminated << std::endl;
	//std::cerr << "last         : " << last << std::endl;
	// Handle NIL or #turn (GDL 2) sent as PLAY message
	if(isnumber(msg.kif))
	{
		if(terminated)
			return msg;
		// Turn number for GDL 2
		msg.turn = atoi(msg.kif.c_str());
		// Get last move
		msg.kif += " ";
		string tmp = collectKifBlob(terminated, c);
		if(terminated)
			return msg;
		if(tmp.compare("nil") == 0)
		{
			msg.type = gm_INIT;
			// Get percept NIL
			
			msg.kif += tmp;
			msg.kif += " ";
			msg.kif += collectKifBlob(terminated, c);
			//std::cerr << "Get percept NIL: " << msg.kif << std::endl;
			if(delimeter(c))
			{
				do{c=getNextChar();}while(delimeter(c));
			}
			if(c != ')')
			{
				cerr << "Message with no closing parenthesis error\n";
				msg.type = gm_ERROR;
			}
			return msg;
		}
		// Get percepts
		msg.kif += "(";
		msg.kif += tmp;
		msg.kif += ") ";
		msg.kif += collectKifBlob(terminated, c);
		if(terminated)
			return msg;
	}
	else if(msg.kif.compare("nil") == 0)
	{
		msg.type = gm_INIT;
		if(terminated)
			return msg;
	}
	c=getNextChar();
	// Parse in clocks also if this is a START message
	if(msg.type == gm_START || msg.type == gm_LOAD || msg.type == gm_FILE)
	{
		msg.startclock = atoi(getMessageString().c_str());
		//cerr << "startclock = " << msg.startclock << endl;
		std::string pc = getMessageInteger().c_str();
		if(pc[pc.size()-1] == ')')
		{
			c = ')';
			pc[pc.size()-1] = '\0';
		}
		else
			c = ' ';
		msg.playclock = atoi(pc.c_str());
		//cerr << "playclock = " << msg.playclock << endl;
	}
	// End of message
	if(delimeter(c))
	{
		do{c=getNextChar();}while(delimeter(c));
	}
	if(c != ')')
	{
		cerr << "Message with no closing parenthesis error\n";
		msg.type = gm_ERROR;
	}
	fflush(stdin);
	//cerr << msg.toString().c_str() << endl;
	return msg;
}
// No GDL2 support if uncommented
/*GMMessage GMProtocol::getMessage(std::string str)
 {
 GMMessage msg;
 char c;
 int strIndex = 0;
 std::string buffer;
 do
 {
 c = str[strIndex++];
 }
 while(c != '(');
 buffer = getMessageString(str, strIndex);
 // Determine the message type
 msg.type = getMessageType(buffer);
 if(msg.type == gm_ERROR)
 {
 //printf("Type error\n");
 return msg;
 }
 else if(msg.type == gm_KILL)
 return msg;
 msg.matchId = getMessageString(str, strIndex);
 if(msg.type == gm_ABORT)
 return msg;
 // START messages also include role
 if(msg.type == gm_START || msg.type == gm_LOAD)
 msg.role = getMessageString(str, strIndex);
 do{c = str[strIndex++];}while(delimeter(c));
 // Handle NIL sent as PLAY message
 if(c != '(')
 {
 msg.type = gm_INIT;
 msg.kif = "NIL";
 return msg;
 }
 int parenCount = 1;
 // Collect the kif sent as a single blob
 msg.kif = "";
 while(parenCount > 0)
 {
 c = str[strIndex++];
 if(c == GM_RESERVED_CHAR)
 continue;
 if(c == '(')
 ++parenCount;
 else if(c == ')')
 --parenCount;
 if(parenCount > 0)
 msg.kif += c;
 }
 // Parse in clocks also if this is a START message
 if(msg.type == gm_START || msg.type == gm_LOAD)
 {
 msg.startclock = atoi(getMessageString(str, strIndex).c_str());
 msg.playclock = atoi(getMessageString(str, strIndex).c_str());
 }
 // End of message
 return msg;
 }*/

std::string GMProtocol::collectKifBlob(bool& terminated, char& last)
{
	terminated = false;
	std::stringstream kif;
	// Collect the kif sent as a single blob
	std::string tmp = getMessageString();
	if(tmp[0] != '(')
	{
		last = tmp[tmp.size()-1];
		if(last == ')')
		{
			tmp.resize(tmp.size()-1);
			terminated = true;
		}
		kif << tmp;
	}
	else
	{
		int parenCount = 1;
		for(int n = 1 ; n < tmp.size() ; n++)
		{
			last=tmp[n];
			if(last == GM_RESERVED_CHAR)
				continue;
			if(last == '(')
				++parenCount;
			else if(last == ')')
				--parenCount;
			kif << last;
		}
		kif << ' ';
		while(parenCount > 0)
		{
			last=getNextChar();
			if(last == GM_RESERVED_CHAR)
				continue;
			if(last == '(')
				++parenCount;
			else if(last == ')')
				--parenCount;
			if(parenCount > 0)
				kif << last;
		}
	}
	return kif.str();
}

void GMProtocol::sendMessage(GMMessageType replyTo, std::string kif)
{
	if(replyTo == gm_START)
		puts("READY");
	else if(replyTo == gm_PLAY)
		puts(kif.c_str());
	else if(replyTo == gm_STOP)
		puts("DONE");
	else if(replyTo == gm_NIL)
		puts("NIL");
	else
		puts(kif.c_str());
	fflush(stdout);
}

std::string GMProtocol::createSendMessage(GMMessageType replyTo, std::string kif)
{
	if(replyTo == gm_START)
		return "READY";
	else if(replyTo == gm_PLAY)
		return kif.c_str();
	else if(replyTo == gm_STOP)
		return "DONE";
	return kif.c_str();
}

GMMessageType GMProtocol::getMessageType()
{
	char c;
	std::string buffer = "";
	do{c=getNextChar();}while(delimeter(c));
	
	buffer += c;
	do
	{
		c=getNextChar();
		if(c == ')')
			break;
		if(!delimeter(c))
			buffer += c;
	}while(!delimeter(c));
	std::transform(buffer.begin(), buffer.end(), buffer.begin(), (int(*)(int))tolower);
	if(buffer == "start")
		return gm_START;
	if(buffer == "play")
		return gm_PLAY;
	if(buffer == "stop")
		return gm_STOP;
	if(buffer == "mcpar")
		return gm_MCPARALLEL;
	if(buffer == "conf")
		return gm_CONFIDENCE;
	if(buffer == "ping")
		return gm_PING;
	if(buffer == "abort")
		return gm_ABORT;
	if(buffer == "quit")
		return gm_KILL;
	if(buffer == "load")
		return gm_LOAD;
	if(buffer == "state")
		return gm_STATE;
	if(buffer == "move")
		return gm_MOVE;
	if(buffer == "undo")
		return gm_UNDO;
	if(buffer == "file")
		return gm_FILE;
	if(buffer == "think")
		return gm_THINK;
	if(buffer == "avail")
		return gm_AVAIL;
	if(buffer == "term")
		return gm_TERM;
	if(buffer == "goal")
		return gm_GOAL;
	if(buffer == "halt")
		return gm_HALT;
	if(buffer == "info")
		return gm_INFO;
	return gm_ERROR;
}
GMMessageType GMProtocol::getMessageType(std::string str)
{
	std::transform(str.begin(), str.end(), str.begin(), (int(*)(int))tolower);
	if(str == "start")
		return gm_START;
	if(str == "play")
		return gm_PLAY;
	if(str == "stop")
		return gm_STOP;
	if(str == "mcpar")
		return gm_MCPARALLEL;
	if(str == "conf")
		return gm_CONFIDENCE;
	if(str == "ping")
		return gm_PING;
	if(str == "abort")
		return gm_ABORT;
	if(str == "quit")
		return gm_KILL;
	if(str == "load")
		return gm_LOAD;
	if(str == "state")
		return gm_STATE;
	if(str == "move")
		return gm_MOVE;
	if(str == "undo")
		return gm_UNDO;
	if(str == "file")
		return gm_FILE;
	if(str == "think")
		return gm_THINK;
	if(str == "avail")
		return gm_AVAIL;
	if(str == "term")
		return gm_TERM;
	if(str == "goal")
		return gm_GOAL;
	if(str == "halt")
		return gm_HALT;
	if(str == "info")
		return gm_INFO;
	return gm_ERROR;
}
char GMProtocol::getNextChar()
{
	/*char c;
	cerr << "Getting next char ...";
	c = getchar();
	cerr << "[" << c << "]" << endl;
	return c;*/
	
	return getchar();
}
bool GMProtocol::delimeter(char c)
{
	return (c == ' ' || c == '\t' || c == '\n' || c == '\r' || ((int)c) == -1);
}

std::string GMProtocol::getMessageString()
{
	char c;
	std::string buffer = "";
	do{c=getNextChar();}while(delimeter(c));
	buffer += c;
	do{c=getNextChar();if(!delimeter(c))buffer += c;}while(!delimeter(c));
	return buffer;
}
std::string GMProtocol::getMessageInteger()
{
	char c;
	std::string buffer = "";
	do{c=getNextChar();}while(delimeter(c));
	buffer += c;
	do{c=getNextChar();if(!delimeter(c))buffer += c;}while(!delimeter(c)&&c!=')');
	return buffer;
}
bool GMProtocol::isnumber(std::string str)
{
	for(int n = 0 ; n < str.size() ; n++)
	{
		if(!isdigit(str[n]))
			return false;
	}
	return true;
}
/*std::string GMProtocol::getMessageString(std::string& str, int& index)
 {
 char c;
 std::string buffer = "";
 do{c = str[index++];}while(delimeter(c));
 buffer += c;
 do{c = str[index++];if(!delimeter(c))buffer += c;}while(!delimeter(c));
 return buffer;
 }*/
