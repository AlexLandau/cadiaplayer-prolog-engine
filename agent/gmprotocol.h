#ifndef GMPROTOCOL_H
#define GMPROTOCOL_H

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <cctype>
#include <algorithm>
#include <fcntl.h>

namespace cadiaplayer{
	namespace agent {
		
		const char GM_RESERVED_CHAR = '$';
		const int GM_GDL1_TURN = -1;
		
		enum GMMessageType
		{
			gm_START = 0,
			gm_INIT,
			gm_PLAY,
			gm_STOP,
			gm_ERROR,
			gm_PING,
			gm_ABORT,
			gm_KILL,
			gm_CONFIDENCE,
			gm_LOAD,
			gm_STATE,
			gm_MOVE,
			gm_UNDO,
			gm_FILE,
			gm_THINK,
			gm_AVAIL,
			gm_TERM,
			gm_GOAL,
			gm_HALT,
			gm_INFO,
			gm_MCPARALLEL,
			gm_NIL
		};
		
		struct GMMessage
		{
			GMMessageType type;
			std::string matchId;
			std::string role;
			std::string kif;
			int startclock;
			int playclock;
			int turn;
			
			GMMessage()
			{
				type = gm_START;
				matchId = "";
				role = "";
				kif = "";
				startclock = 0;
				playclock = 0;
				turn = GM_GDL1_TURN;
			};
			
			std::string toString()
			{
				std::stringstream str;
				if(type == gm_START)
					str << "start " << matchId << " " << role << " " << startclock << " " << playclock;
				else if(type == gm_INIT)
					str << "init " << matchId << " ";
				else if(type == gm_PLAY && turn == GM_GDL1_TURN)
					str << "play " << matchId << " ";
				else if(type == gm_PLAY)
					str << "play " << matchId << " " << turn << " ";
				else if(type == gm_STOP)
					str << "stop " << matchId << " ";
				else if(type == gm_ERROR)
					str << "error " << matchId << " ";
				else if(type == gm_KILL)
					str << "quit " << matchId << " ";
				else if(type == gm_MCPARALLEL)
					str << "mcpar" << matchId << " ";
				else if(type == gm_CONFIDENCE)
					str << "conf " << matchId << " ";
				else if(type == gm_PING)
					str << "ping" << matchId << " ";
				else if(type == gm_ABORT)
					str << "abort" << matchId << " ";
				else if(type == gm_LOAD)
					str << "load " << matchId << " ";
				else if(type == gm_STATE)
					str << "state " << matchId << " ";
				else if(type == gm_MOVE)
					str << "move " << matchId << " ";
				else if(type == gm_UNDO)
					str << "undo " << matchId << " ";
				else if(type == gm_FILE)
					str << "file " << matchId << " " << role << " " << startclock << " " << playclock;
				else if(type == gm_THINK)
					str << "think " << matchId << " ";
				else if(type == gm_AVAIL)
					str << "avail " << matchId << " ";
				else if(type == gm_TERM)
					str << "term " << matchId << " ";
				else if(type == gm_GOAL)
					str << "goal " << matchId << " ";
				else if(type == gm_HALT)
					str << "halt " << matchId << " ";
				else if(type == gm_INFO)
					str << "info" << matchId << " ";
				else if(type == gm_NIL)
					str << "nil";
				str << "\n";
				str << kif << "\n";
				return str.str();
			};
			void writeToStream(std::ostream& os)
			{
				os << std::endl << type << std::endl;
				os << matchId << std::endl;
				if(role.empty())
					os << "n/a" << std::endl;
				else
					os << role << std::endl;
				os << startclock << std::endl;
				os << playclock << std::endl;
				if(kif.empty())
					os << "nil";
				else
					os << kif;
				os << std::endl << GM_RESERVED_CHAR;
			};
			void encodeToStream(std::ostream& os)
			{
				os << GM_RESERVED_CHAR;
				writeToStream(os);
			};
			bool readFromStream(std::istream& is)
			{
				int t;
				is >> t;
				if(is.eof())
					return false;
				type = static_cast<GMMessageType>(t);
				is >> matchId;
				is >> role;
				is >> startclock;
				is >> playclock;
				char buffer;
				kif = "";
				is.get(buffer);
				while(buffer == '\n') 
					is.get(buffer);
				while(buffer != GM_RESERVED_CHAR)
				{
					kif += buffer;
					is.get(buffer);
					//std::cerr << buffer;
				}
				//std::cerr << std::endl;
				return true;
			};
			void log(std::string str)
			{
				std::ofstream file;
				file.open("gmprotocol.log");
				file << str << std::endl;
				file.close();
			};
		};
		
		class GMProtocol
			{
			public:
				static bool hasMessage();
				static GMMessage getMessage();
				//GMMessage getMessage(std::string str);
				static void sendMessage(GMMessageType replyTo, std::string kif);
				std::string createSendMessage(GMMessageType replyTo, std::string kif);
			private:
				static char getNextChar();
				static std::string collectKifBlob(bool& terminated, char& last);
				static bool delimeter(char c);
				static GMMessageType getMessageType();
				GMMessageType getMessageType(std::string str);
				static std::string getMessageString();
				static std::string getMessageInteger();
				static bool isnumber(std::string str);
				//std::string getMessageString(std::string& str, int& index);
			};
	}} // namespaces
#endif //GMPROTOCOL_H
