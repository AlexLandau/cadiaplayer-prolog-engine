#ifndef SYMTAB_H_
#define SYMTAB_H_

#include <ext/hash_map>
#include <string>
#include <sstream>
#include "../../utils/stringhash.h"

namespace cadiaplayer{ 
	namespace play{ 
		namespace parsing {
			
			const unsigned int MIN_SYMBOL_CONSTANT = 0;
			const unsigned int MIN_SYMBOL_VARIABLE = ((unsigned int)(-1))/2;
			
			typedef unsigned int SymbolID;
			
			enum SymbolType {st_NUM, st_STRING};
			
			class SymbolTableEntry
				{
				private:
					std::string lexeme;
					SymbolID id;
					SymbolType type;
					bool altered;
					std::string originalLexeme;
				public:
					SymbolTableEntry(std::string l, SymbolID i);
					std::string getLexeme(void);
					SymbolID getId(void);
					void overwriteId(SymbolID i);
					SymbolType getType(){return type;};
					std::string getOriginalLexeme(void);
					void alter(std::string original){altered = true;originalLexeme = original;}
					bool isAltered(){return altered;};
				};
			
			typedef __gnu_cxx::hash_map<std::string, SymbolTableEntry*, cadiaplayer::utils::StringHash> SymbolMap;
			typedef __gnu_cxx::hash_map<SymbolID, SymbolTableEntry*> SymbolIdMap;
			class SymbolTable
				{
				private:
					SymbolMap table;
					SymbolIdMap idTable;
					unsigned int constantCount;
					unsigned int variableCount;
					unsigned int moveClassCount;
					unsigned int parameterMax;
					std::vector<int> numberMap;
				public:
					SymbolTable(void);
					~SymbolTable(void);
					unsigned int getConstantCount(void);
					unsigned int getVariableCount(void);
					unsigned int getParameterMax(void);
					SymbolID insertConstant(std::string lexeme);
					SymbolID insertVariable(std::string lexeme);
					SymbolID insertMoveClass(std::string lexeme);
					SymbolID insertMoveClass(SymbolID id);
					void updateParameterMax(unsigned int count);
					SymbolTableEntry* lookup(std::string lexeme);
					SymbolTableEntry* lookup(SymbolID id);
					std::string getName(SymbolID id);
					std::string getOriginalName(SymbolID id);
					int getNum(SymbolID id);
					void swap(std::string lexeme, SymbolID newid);
					void print(void);
					
					static bool isVariable(SymbolID id)
					{
						return id >= MIN_SYMBOL_VARIABLE; 
					};
					static bool isConstant(SymbolID id)
					{
						return id < MIN_SYMBOL_VARIABLE; 
					};
					static bool isIllegal(SymbolTable* symtab, SymbolID id)
					{
						return id > symtab->getConstantCount() && id < MIN_SYMBOL_VARIABLE; 
					};
				};
		}}} // namespaces
#endif /*SYMTAB_H_*/
