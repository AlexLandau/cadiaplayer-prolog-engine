/*
 *  binaryrelation.h
 *  cadiaplayer
 *
 *  Created by Hilmar Finnsson on 9/24/09.
 *  Copyright 2009 Reykjavik University. All rights reserved.
 *
 */

#ifndef BINARY_RELATION_H
#define BINARY_RELATION_H

#include <ext/hash_map>
#include <string>
#include <sstream>
#include <iostream>
#include "../../utils/longlonghash.h"
#include "../parsing/symtab.h"

namespace cadiaplayer{
	namespace play{
		namespace utils{

			class BinaryRelationNode
				{
				private:
					cadiaplayer::play::parsing::SymbolID m_name;
					BinaryRelationNode* m_left;
					BinaryRelationNode* m_right;
				public:
					BinaryRelationNode(cadiaplayer::play::parsing::SymbolID name);
					~BinaryRelationNode();
					cadiaplayer::play::parsing::SymbolID getName();
					BinaryRelationNode* getLeft();
					BinaryRelationNode* getRight();
					void setLeft(BinaryRelationNode* node);
					void setRight(BinaryRelationNode* node);
					
					std::string toString();
					std::string toString(cadiaplayer::play::parsing::SymbolTable* symbols);
				};
			
			typedef __gnu_cxx::hash_map<cadiaplayer::play::parsing::SymbolID, BinaryRelationNode*, cadiaplayer::utils::LongLongHash>	BinaryRelationNodeMap;
			typedef BinaryRelationNodeMap::iterator BinaryRelationNodeMapItr;
			
			class BinaryRelation
				{
				private:
					cadiaplayer::play::parsing::SymbolID m_name;
					BinaryRelationNodeMap m_map;
					BinaryRelationNode* m_first;
					BinaryRelationNode* m_last;
				public:
					BinaryRelation(cadiaplayer::play::parsing::SymbolID name);
					~BinaryRelation();
					BinaryRelationNode* getFirst();
					BinaryRelationNode* getLast();
					BinaryRelationNode* get(cadiaplayer::play::parsing::SymbolID name);
					BinaryRelationNode* getAny();
					
					bool isEmpty();
					bool isLooped();
					bool isBroken();
					bool isChain();
					
					void add(cadiaplayer::play::parsing::SymbolID left, cadiaplayer::play::parsing::SymbolID right);
					void remove(cadiaplayer::play::parsing::SymbolID name);
					void clear();
					
					std::string toString(cadiaplayer::play::parsing::SymbolTable* symbols);
				private:
					void findEndpoints();
				};
			
			typedef __gnu_cxx::hash_map<cadiaplayer::play::parsing::SymbolID, BinaryRelation*, cadiaplayer::utils::LongLongHash>	BinaryRelationMap;
			typedef BinaryRelationMap::iterator BinaryRelationMapItr;
			class BinaryRelations
				{
				private:
					BinaryRelationMap m_map;
				public:
					BinaryRelations();
					~BinaryRelations();
					
					BinaryRelation* getBinaryRelation(cadiaplayer::play::parsing::SymbolID name);
					BinaryRelation* addBinaryRelation(cadiaplayer::play::parsing::SymbolID name);
					
					void clear();
					
					std::string toString(cadiaplayer::play::parsing::SymbolTable* symbols);
				};
			
		}
	}
}
#endif // BINARY_RELATION_H
