#ifndef DOMAINANALYZER_H
#define DOMAINANALYZER_H

/*
 *  domainanalyzer.h
 *  src
 *
 *  Created by Hilmar Finnsson on 4/29/08.
 *  Copyright 2008 Reykjavik University. All rights reserved.
 *
 */

#include <sstream>
#include <set>
#include <list>
#include "../../play/gametheory.h"

namespace cadiaplayer {
	namespace logic {
		namespace utils {
			
			// Move SymbolID and StateFactID classes to this namespace
			typedef cadiaplayer::play::parsing::SymbolID SymbolID;
			typedef cadiaplayer::play::StateFactID StateFactID;
			typedef cadiaplayer::play::StateFacts StateFacts;
			
			typedef std::set<SymbolID> Domain;
			typedef Domain::iterator DomainItr;
			typedef std::vector<Domain> Domains;
			typedef __gnu_cxx::hash_map<SymbolID, Domains> PredicateDomains;
			typedef PredicateDomains::iterator PredicateDomainsItr;
			typedef std::pair<SymbolID, Domains> PredicateDomainsPair;
			typedef __gnu_cxx::hash_map<SymbolID, short> VarOccurrance;
			typedef VarOccurrance::iterator VarOccurranceItr; 
			typedef __gnu_cxx::hash_map< SymbolID, VarOccurrance> PredOccurrance;
			typedef PredOccurrance::iterator PredOccurranceItr; 
			
			class Occurrance
				{
				private:
					PredOccurrance occMap;
				public:
					void add(SymbolID key1, SymbolID key2)
					{
						occMap[key1][key2] = 1;
					};
					void remove(SymbolID key1, SymbolID key2)
					{
						PredOccurranceItr itr1 = occMap.find(key1);
						if(itr1 == occMap.end())
							return;
						VarOccurranceItr itr2 = itr1->second.find(key2);
						if(itr2 == itr1->second.end())
							return;
						
						itr1->second.erase(key2);
						if(itr1->second.size() == 0)
							occMap.erase(key1);
					};
					bool has(SymbolID key1, SymbolID key2)
					{
						PredOccurranceItr itr1 = occMap.find(key1);
						if(itr1 == occMap.end())
							return false;
						VarOccurranceItr itr2 = itr1->second.find(key2);
						if(itr2 == itr1->second.end())
							return false;
						return true;
					};
					void clear()
					{
						for(PredOccurranceItr itr = occMap.begin() ; itr != occMap.end() ; itr++)
						{
							itr->second.clear();
						}
						occMap.clear();
					};
				};		
			
			struct Seperator 
			{
				StateFactID startRange;
				SymbolID name;
				std::vector< __gnu_cxx::hash_map<SymbolID, size_t> > domainMap;
				std::vector< std::vector<SymbolID> > domains;
			};
			typedef std::vector<Seperator> Seperators;
			typedef __gnu_cxx::hash_map<StateFactID, std::string, cadiaplayer::utils::LongLongHash> FactEnumerationTable;
			typedef FactEnumerationTable::iterator FactEnumerationTableItr;
			struct FactParameterMarker
			{
				Seperator* seperator;
				size_t parameter;
				SymbolID symbol;
			};
			typedef std::list<FactParameterMarker> FactParameterMarkers;
			typedef FactParameterMarkers::iterator FactParameterMarkersItr;
			class FactEnumeration
				{
				private:
					Seperators seperators;
					cadiaplayer::play::parsing::SymbolTable* symbolTable;
					FactEnumerationTable bufferTable;
				public:
					FactEnumeration(cadiaplayer::play::parsing::SymbolTable* symtab);
					
					StateFactID nextStart();
					bool addPredicate(SymbolID name, std::vector<unsigned int> dsizes, std::vector<SymbolID> atoms);
					void getFact(const StateFactID& fid, std::stringstream& istr);
					void getState(const StateFacts& facts, std::stringstream& istr);
					StateFactID getFactID(const cadiaplayer::play::parsing::Compound* compound);
					void getFactIDs(const cadiaplayer::play::GameState& state, StateFacts& holder);
					
					void clear(){seperators.clear();};
					
					void generateMarkers(FactParameterMarkers& markers);
					void eliminateMarker(FactParameterMarkers& markers, FactParameterMarker& marker);
					size_t getMarkerCardinality(FactParameterMarker& marker, cadiaplayer::play::GameState* state);
					std::string markerToString(FactParameterMarker& marker);
					std::string markersToString(FactParameterMarkers& markers);
				};		
			class DomainAnalyzer
				{
				private:
					void processInits(cadiaplayer::play::GameTheory& theory, PredicateDomains& domains);
					void processTerminals(cadiaplayer::play::GameTheory& theory, PredicateDomains& domains);
					void processGoals(cadiaplayer::play::GameTheory& theory, PredicateDomains& domains);
					void processNexts(cadiaplayer::play::GameTheory& theory, PredicateDomains& domains);
					void processLegals(cadiaplayer::play::GameTheory& theory, PredicateDomains& domains);
					void processFunctions(cadiaplayer::play::GameTheory& theory, PredicateDomains& domains);
					void processPredicate(cadiaplayer::play::parsing::Compound* predicate, PredicateDomains& domains);
					void processTrue(cadiaplayer::play::parsing::Compound* trueCompound, PredicateDomains& domains);
					void processVariable(cadiaplayer::play::GameTheory& theory, Domain& domain, cadiaplayer::play::parsing::Compound* next, cadiaplayer::play::parsing::Compound* variable, Occurrance& occurred);
				public:
					PredicateDomains makeStateDomains(cadiaplayer::play::GameTheory& theory);
					FactEnumeration* getFactEnumeration(cadiaplayer::play::GameTheory& theory, PredicateDomains& domains);
					
					static std::string predicateDomainsToString(cadiaplayer::play::GameTheory& theory, PredicateDomains& domains)
					{
						std::stringstream ss;
						for(PredicateDomainsItr itr = domains.begin() ; itr != domains.end() ; itr++)
						{
							ss << theory.getSymbols()->getName(itr->first) << std::endl;
							for(size_t n = 0 ; n < itr->second.size() ; n++)
							{
								bool first = true;
								ss << "\t[" << n+1 << "] : {";
								for(DomainItr dom = itr->second[n].begin() ; dom != itr->second[n].end() ; dom++)
								{
									if(!first)
										ss << ", ";
									ss << theory.getSymbols()->getName(*dom);
									first = false;
								}
								ss << "}\n";
							}
						}
						return ss.str();
					}
				};
		}}} // namespaces
#endif // DOMAINANALYZER_H
