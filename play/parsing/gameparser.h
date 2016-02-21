#ifndef GAMEPARSER_H
#define GAMEPARSER_H

#include "../gametheory.h"
                                                                                                                                                                                                                                                     
#include <sstream>
#include <string>
#include <ext/hash_map>	
#include <sys/stat.h> 

#include "symtab.h"
#include "token.h"
#include "compound.h"
#include "variable.h"	

#include "../utils/binaryrelation.h"
#include "../utils/relationcounter.h"


namespace cadiaplayer {
	namespace play{
		namespace parsing {
			
			class GameParser
				{
				private:
					cadiaplayer::play::GameTheory* theory;
					SymbolTable* table;
					cadiaplayer::play::utils::BinaryRelations* binaryRelations;
					cadiaplayer::play::utils::RelationCounters* relationCounters;
					OpType relation;
					unsigned int variableName;
					__gnu_cxx::hash_map<unsigned int, unsigned int> variableTable;
					bool quietmode;
					int depth;
					bool error;
					CompoundList stored;
				public:
					GameParser(bool quiet=false);
					~GameParser();
					
					void setQuiet(bool quiet){quietmode = quiet;};
					bool isQuiet(void){return quietmode;};
					SymbolTable* getSymbols(){return table;};
					
					bool parseFile(cadiaplayer::play::GameTheory* t, const char* filename);
					bool parseString(cadiaplayer::play::GameTheory* t, const std::string gdl);
					bool parseString(cadiaplayer::play::GameTheory* t, const std::string gdl, CompoundList* result);
					
				private:
					Token* getNextToken();
					void store(Compound* c);
					void storeRelation(Compound* c, CompoundOperator relation);
					bool isBinaryRelation(Compound* c);
					void storeBinaryRelation(Compound* c);
					void storeRelationCounter(Compound* c);
					
					bool parse(CompoundList* result = NULL);
					Compound* parseSentence(Token* token);
					Compound* mapOperator(Token* token);
					CompoundOperator mapRelation(Token* token);
					
					void standardizeApart(Compound* c);
					void standardizeApartTraverse(Compound* c);
				};
			
		}}} // namespaces 

#endif // GAMEPARSER_H
