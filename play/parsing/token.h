#ifndef TOKEN_H_
#define TOKEN_H_

#include <iostream>
#include <string>

namespace cadiaplayer{ 
	namespace play{ 
		namespace parsing {
			
			enum TokenCode
			{
				tc_NONE = 0,
				tc_LPAREN,
				tc_RPAREN,
				tc_RELATION,
				tc_OP,
				tc_PREDICATE,
				tc_VARIABLE,
				tc_CONSTANT,
				tc_COMMENT,
				tc_EOF
			};
			
			static const char* TokenCodeStrings[] = 
			{
			"none",
			"lparen",
			"rparen",
			"relation",
			"operator",
			"predicate",
			"variable",
			"constant",
			"comment",
			"eof"
			};
			
			static const char* TokenCodeFormatStrings[] = 
			{
			"none     ",
			"lparen   ",
			"rparen   ",
			"relation ",
			"operator ",
			"predicate",
			"variable ",
			"constant ",
			"comment  ",
			"eof      "
			};
			
			enum OpType
			{
				op_NONE = 0,
				op_ROLE,
				op_TRUE,
				op_INIT,
				op_NEXT,
				op_LEGAL,
				op_DOES,
				op_GOAL,
				op_TERMINAL,
				op_IMPLICATION,
				op_OR,
				op_NOT,
				op_DISTINCT,
				op_ATOM,
				op_INT
			};
			static const char* OpTypeStrings[] = 
			{
			"none",
			"role",
			"true",
			"init",
			"next",
			"legal",
			"does",
			"goal",
			"terminal",
			"implication",
			"or",
			"not",
			"distinct",
			"ATOM",
			"INT"
			};
			
			static const char* OpTypeFormatStrings[] = 
			{
			"none       ",
			"role       ",
			"true       ",
			"init       ",
			"next       ",
			"legal      ",
			"does       ",
			"goal       ",
			"terminal   ",
			"implication",
			"or         ",
			"not        ",
			"distinct   ",
			"ATOM       ",
			"INT        "
			};
			
			class Token
				{
				private:
					TokenCode 			m_tokenCode;
					OpType 				m_opType;
					std::string				m_lexeme;
					
				public:
					// Construction
					Token(void){};
					
					// Getters
					TokenCode 			getTokenCode(void){return m_tokenCode;};
					OpType				getOpType(void){return m_opType;};
					std::string		 		getLexeme(void){return m_lexeme;};
					
					// Setters
					void setTokenCode(TokenCode tokenCode){this->m_tokenCode = tokenCode;};
					void setOpType(OpType opType){this->m_opType = opType;};
					void setLexeme(std::string lexeme)
					{
						// check prolog reserved words. (match kif2pl.ccp by Yngvi)
						if(lexeme == "succ")
							lexeme = "succs";
						else if(lexeme == "number")
							lexeme = "x_number_x";
						else if(lexeme == "state")
							lexeme = "x_state_x";
						this->m_lexeme = lexeme;
					};
					
					// Utils
					std::string tokenCodeToString(){return TokenCodeStrings[m_tokenCode];};
					std::string opTypeToString(){return OpTypeStrings[m_opType];};
					static std::string tokenCodeToString(TokenCode tc){return TokenCodeStrings[tc];};
					static std::string opTypeToString(OpType ot){return OpTypeStrings[ot];};
					
					std::string tokenCodeToFormatString(){return TokenCodeFormatStrings[m_tokenCode];};
					std::string opTypeToFormatString(){return OpTypeFormatStrings[m_opType];};
					static std::string tokenCodeToFormatString(TokenCode tc){return TokenCodeFormatStrings[tc];};
					static std::string opTypeToFormatString(OpType ot){return OpTypeFormatStrings[ot];};
					
				};	
		}}} // namespaces
#endif /*TOKEN_H_*/
