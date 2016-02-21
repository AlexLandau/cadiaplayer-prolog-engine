/*
 *  domainanalyzer.cpp
 *  src
 *
 *  Created by Hilmar Finnsson on 4/29/08.
 *  Copyright 2008 Reykjavik University. All rights reserved.
 *
 */

#include "domainanalyzer.h"
using namespace cadiaplayer::logic::utils;

FactEnumeration::FactEnumeration(cadiaplayer::play::parsing::SymbolTable* symtab)
{
	symbolTable = symtab;
}

StateFactID FactEnumeration::nextStart()
{
	if(seperators.empty())
		return 0;
	Seperator& sep = seperators[seperators.size()-1];
	if(sep.domains.empty())
		return sep.startRange+1;
	StateFactID range = sep.domains[0].size();
	for(size_t n = 1 ; n < sep.domains.size() ; n++)
	{
		range *= sep.domains[n].size();
	}
	return sep.startRange + range;
}
bool FactEnumeration::addPredicate(SymbolID name, std::vector<unsigned int> dsizes, std::vector<SymbolID> atoms)
{
	StateFactID start = nextStart();
	seperators.resize(seperators.size()+1);
	Seperator& sep = seperators[seperators.size()-1];
	sep.startRange = start;
	sep.name = name;
	size_t atomIndex = 0;
	size_t sizeCheck = 0;
	sep.domains.resize(dsizes.size());
	sep.domainMap.resize(dsizes.size());
	for(size_t n = 0 ; n < dsizes.size() ; n++)
	{
		sizeCheck += dsizes[n];
		if(sizeCheck > atoms.size())
		{
			clear();
			return false;
		}
		sep.domains.reserve(dsizes[n]);
		for(size_t i = 0 ; i < dsizes[n] ; i++)
		{
			sep.domainMap[n][atoms[atomIndex]] = sep.domains[n].size();
			sep.domains[n].push_back(atoms[atomIndex++]);
		}
	}
	return true;
}
void FactEnumeration::getFact(const StateFactID& fid, std::stringstream& istr)
{
	FactEnumerationTableItr itr = bufferTable.find(fid);
	if(itr != bufferTable.end())
	{
		istr << itr->second;
		return;
	}
	std::stringstream factstr;
	StateFactID id = fid;
	size_t n;
	for(n = 0 ; n < seperators.size() ; n++)
	{
		if(seperators[n].startRange > id)
		{
			--n;
			break;
		}
		if(seperators[n].startRange == id)
			break;
	}
	if(n == seperators.size())
		--n;
	Seperator& sep = seperators[n];
	id -= sep.startRange;
	cadiaplayer::play::parsing::SymbolTableEntry* entry = symbolTable->lookup(sep.name);
	if(sep.domains.empty())
	{
		if(entry->getType() == cadiaplayer::play::parsing::st_NUM)
			factstr << "( I" << entry->getLexeme() << " )";
		else
			factstr << "( A" << entry->getLexeme() << " )";
		
		bufferTable[fid] = factstr.str();
		istr << factstr.str();
		return;
	}
	factstr << "( C " << entry->getLexeme() << " " << sep.domains.size();
	StateFactID arg;
	SymbolID sid;
	for(n = 0 ; n < sep.domains.size() ; n++)
	{
		arg = id % sep.domains[n].size();
		sid = sep.domains[n][arg];
		id /= sep.domains[n].size();
		
		entry = symbolTable->lookup(sid);
		if(entry->getType() == cadiaplayer::play::parsing::st_NUM)
			factstr << " ( I " << entry->getLexeme() << " )";
		else
			factstr << " ( A " << entry->getLexeme() << " )";
	}
	factstr << " ) ";
	bufferTable[fid] = factstr.str();
	istr << factstr.str();
}
void FactEnumeration::getState(const StateFacts& facts, std::stringstream& istr)
{
	/*std::cout << "Buffer table:\n";
	for(FactEnumerationTableItr itr = bufferTable.begin() ; itr != bufferTable.end() ; itr++)
	{
		std::std::cerr << itr->first << ":" << itr->second.c_str() << std::endl;
	}*/
	for(size_t n = 0 ; n < facts.size() ; n++)
	{
		istr << "( P ";// << facts.size();
		getFact(facts[n], istr);
	};
	istr << " ( A [] )";
	for(size_t n = 0 ; n < facts.size() ; n++)
	{
		istr << ") ";
	}
}
StateFactID FactEnumeration::getFactID(const cadiaplayer::play::parsing::Compound* compound)
{
	StateFactID id = 0;
	size_t n;
	for(n = 0 ; n < seperators.size() ; n++)
	{
		if(seperators[n].name == compound->getName())
			break;
	}
	Seperator& sep = seperators[n];
	id = sep.startRange;
	StateFactID arg;
	StateFactID multi = 1;
	if(compound->getArguments() == NULL)
		return id;
	for(n = 0 ; n < compound->getArguments()->size() ; n++)
	{
		arg = sep.domainMap[n][compound->getArgument(n)->getName()];
		id += (arg * multi);
		multi *= sep.domains[n].size();
	}
	return id;
}
void FactEnumeration::getFactIDs(const cadiaplayer::play::GameState& state, StateFacts& holder)
{
	holder.reserve(state.size());
	for(size_t n = 0 ; n < state.size() ; n++)
	{
		holder.push_back(getFactID(state[n]));
	}
}

void FactEnumeration::generateMarkers(FactParameterMarkers& markers)
{
	for(size_t n = 0 ; n < seperators.size() ; n++)
	{
		if(seperators[n].domains.size() <= 1)
			continue;
		for(size_t m = 0 ; m < seperators[n].domains.size() ; m++)
		{
			if(seperators[n].domains[m].size() <= 12)
			{
				for(size_t o = 0 ; o < seperators[n].domains[m].size() ; o++)
				{
					markers.resize(markers.size()+1);
					FactParameterMarker& marker = markers.back();
					marker.seperator = &seperators[n];
					marker.parameter = m;
					marker.symbol = seperators[n].domains[m][o];
				}
			}
		}
	}
}	
void FactEnumeration::eliminateMarker(FactParameterMarkers& markers, FactParameterMarker& marker)
{
	for(FactParameterMarkersItr itr = markers.begin() ; itr != markers.end() ; itr++)
	{
		if(itr->seperator->name == marker.seperator->name && itr->parameter == marker.parameter && itr->symbol == marker.symbol)
		{
			markers.erase(itr);
			break;
		}
	}
}

size_t FactEnumeration::getMarkerCardinality(FactParameterMarker& marker, cadiaplayer::play::GameState* state)
{
	std::size_t counter = 0;
	for(size_t n = 0 ; n < state->size() ; n++)
	{
		if((*state)[n]->getName() == marker.seperator->name && (*state)[n]->getArgument(marker.parameter) && (*state)[n]->getArgument(marker.parameter)->getName() == marker.symbol)
			counter++;
	}
	return counter;
}

std::string FactEnumeration::markerToString(FactParameterMarker& marker)
{
	std::stringstream ss;
	cadiaplayer::play::parsing::SymbolTableEntry* entry;
	entry = this->symbolTable->lookup(marker.seperator->name);
	ss << entry->getLexeme() << "[" << marker.parameter << ":";
	entry = this->symbolTable->lookup(marker.symbol);
	ss << entry->getLexeme() << "]";
	return ss.str();
}
std::string FactEnumeration::markersToString(FactParameterMarkers& markers)
{
	std::stringstream ss;
	cadiaplayer::play::parsing::SymbolTableEntry* entry;
	for(FactParameterMarkersItr itr = markers.begin() ; itr != markers.end() ; itr++)
	{
		entry = this->symbolTable->lookup(itr->seperator->name);
		ss << entry->getLexeme() << "[" << itr->parameter << ":";
		entry = this->symbolTable->lookup(itr->symbol);
		ss << entry->getLexeme() << "]" << std::endl;
	}
	return ss.str();
}

PredicateDomains DomainAnalyzer::makeStateDomains(cadiaplayer::play::GameTheory& theory)
{
	PredicateDomains domains;
	processInits(theory, domains);
	processTerminals(theory, domains);
	processGoals(theory, domains);
	processNexts(theory, domains);
	processLegals(theory, domains);
	processFunctions(theory, domains);
	
	return domains;
}
FactEnumeration* DomainAnalyzer::getFactEnumeration(cadiaplayer::play::GameTheory& theory, PredicateDomains& domains)
{
	FactEnumeration* fe = new FactEnumeration(theory.getSymbols());
	std::vector<unsigned int> dsizes;
	std::vector<SymbolID> atoms;
	
	for(PredicateDomainsItr itr = domains.begin() ; itr != domains.end() ; itr++)
	{
		for(size_t n = 0 ; n < itr->second.size() ; n++)
		{
			dsizes.push_back(itr->second[n].size());
			for(DomainItr d = itr->second[n].begin() ; d != itr->second[n].end() ; d++)
			{
				atoms.push_back(*d);
			}
		}
		fe->addPredicate(itr->first, dsizes, atoms);
		dsizes.clear();
		atoms.clear();
	}
	return fe;
}
void DomainAnalyzer::processInits(cadiaplayer::play::GameTheory& theory, PredicateDomains& domains)
{
	cadiaplayer::play::KBaseList& inits = *(theory.getStateRef());
	PredicateDomainsItr itr;
	for(size_t n = 0 ; n < inits.size() ; n++)
	{
		itr = domains.find(inits[n]->getName());
		if(itr == domains.end())
			itr = (domains.insert(PredicateDomainsPair(inits[n]->getName(), Domains()))).first;
		if(itr->second.size() < inits[n]->getArguments()->size())
			itr->second.resize(inits[n]->getArguments()->size());
		for(size_t i = 0 ; i < inits[n]->getArguments()->size() ; i++)
		{
			itr->second[i].insert((*(inits[n]->getArguments()))[i]->getName());
		}
	}
}
void DomainAnalyzer::processTerminals(cadiaplayer::play::GameTheory& theory, PredicateDomains& domains)
{
	cadiaplayer::play::KBaseList& terminals = *(theory.getTerminals());
	PredicateDomainsItr itr;
	cadiaplayer::play::parsing::Compound* terminal = NULL;
	std::vector<size_t> varIndexes;
	for(size_t n = 0 ; n < terminals.size() ; n++)
	{
		if(terminals[n]->getOperator() != cadiaplayer::play::parsing::co_IMPLICATION)
			continue;
		for(size_t i = 1 ; i < terminals[n]->getArguments()->size() ; i++)
		{
			terminal = terminals[n]->getArgument(i);
			processPredicate(terminal, domains);	
		}
	}
}
void DomainAnalyzer::processGoals(cadiaplayer::play::GameTheory& theory, PredicateDomains& domains)
{
	cadiaplayer::play::KBaseList& goals = *(theory.getGoals());
	PredicateDomainsItr itr;
	cadiaplayer::play::parsing::Compound* goal = NULL;
	std::vector<size_t> varIndexes;
	for(size_t n = 0 ; n < goals.size() ; n++)
	{
		if(goals[n]->getOperator() != cadiaplayer::play::parsing::co_IMPLICATION)
			continue;
		for(size_t i = 1 ; i < goals[n]->getArguments()->size() ; i++)
		{
			goal = goals[n]->getArgument(i);
			processPredicate(goal, domains);	
		}
	}
}
void DomainAnalyzer::processNexts(cadiaplayer::play::GameTheory& theory, PredicateDomains& domains)
{
	cadiaplayer::play::KBaseList& nexts = *(theory.getNexts());
	PredicateDomainsItr itr;
	cadiaplayer::play::parsing::Compound* next = NULL;
	cadiaplayer::play::parsing::Compound* symbol;
	std::vector<size_t> varIndexes;
	for(size_t n = 0 ; n < nexts.size() ; n++)
	{
		next = nexts[n];
		if(next->getOperator() == cadiaplayer::play::parsing::co_IMPLICATION)
			next = next->getArgument(0);
		itr = domains.find(next->getName());
		if(itr == domains.end())
			itr = (domains.insert(PredicateDomainsPair(next->getName(), Domains()))).first;
		if(next->getArguments() == NULL)
			continue;
		if(itr->second.size() < next->getArguments()->size())
			itr->second.resize(next->getArguments()->size());
		for(size_t i = 0 ; i < next->getArguments()->size() ; i++)
		{
			symbol = (*(next->getArguments()))[i];
			if(symbol->getType() == cadiaplayer::play::parsing::ct_VARIABLE)
				varIndexes.push_back(i);
			else
				itr->second[i].insert(symbol->getName());
		}
		
		if(nexts[n]->getOperator() != cadiaplayer::play::parsing::co_IMPLICATION)
			continue;
		for(size_t i = 1 ; i < nexts[n]->getArguments()->size() ; i++)
		{
			next = nexts[n]->getArgument(i);
			processPredicate(next, domains);
			for(size_t j = 0 ; j < varIndexes.size() ; j++)
			{
				Occurrance occurred;
				processVariable(theory, itr->second[varIndexes[j] ], next, nexts[n]->getArgument(0)->getArgument(varIndexes[j]), occurred);
				occurred.clear();
			}
		}
		
		varIndexes.clear();
	}
}
void DomainAnalyzer::processLegals(cadiaplayer::play::GameTheory& theory, PredicateDomains& domains)
{
	cadiaplayer::play::KBaseList& legals = *(theory.getLegals());
	PredicateDomainsItr itr;
	cadiaplayer::play::parsing::Compound* legal = NULL;
	std::vector<size_t> varIndexes;
	for(size_t n = 0 ; n < legals.size() ; n++)
	{
		if(legals[n]->getOperator() != cadiaplayer::play::parsing::co_IMPLICATION)
			continue;
		for(size_t i = 1 ; i < legals[n]->getArguments()->size() ; i++)
		{
			legal = legals[n]->getArgument(i);
			processPredicate(legal, domains);	
		}
	}
}
void DomainAnalyzer::processFunctions(cadiaplayer::play::GameTheory& theory, PredicateDomains& domains)
{
	cadiaplayer::play::KBaseList& functions = *(theory.getRules());
	PredicateDomainsItr itr;
	cadiaplayer::play::parsing::Compound* function = NULL;
	std::vector<size_t> varIndexes;
	for(size_t n = 0 ; n < functions.size() ; n++)
	{
		if(functions[n]->getOperator() != cadiaplayer::play::parsing::co_IMPLICATION)
			continue;
		for(size_t i = 1 ; i < functions[n]->getArguments()->size() ; i++)
		{
			function = functions[n]->getArgument(i);
			processPredicate(function, domains);	
		}
	}
}
void DomainAnalyzer::processPredicate(cadiaplayer::play::parsing::Compound* predicate, PredicateDomains& domains)
{
	if(predicate->getArguments() == NULL)
		return;
	if(predicate->getOperator() == cadiaplayer::play::parsing::co_DISTINCT)
		return;
	if(predicate->getOperator() == cadiaplayer::play::parsing::co_OR)
	{
		for(size_t i = 0 ; i < predicate->getArguments()->size() ; i++)
		{
			processPredicate(predicate->getArgument(i), domains);
		}
		return;
	}
	if(predicate->getOperator() == cadiaplayer::play::parsing::co_NOT)
	{
		processPredicate(predicate->getArgument(0), domains);
		return;
	}
	if(predicate->getOperator() == cadiaplayer::play::parsing::co_RELATION_TRUE)
	{
		processTrue(predicate, domains);
		return;
	}
	PredicateDomainsItr itr = domains.find(predicate->getName());
	cadiaplayer::play::parsing::Compound* symbol;
	if(itr != domains.end())
	{
		if(itr->second.size() < predicate->getArguments()->size())
			itr->second.resize(predicate->getArguments()->size());
		for(size_t i = 0 ; i < predicate->getArguments()->size() ; i++)
		{
			symbol = (*(predicate->getArguments()))[i];
			if(symbol->getType() != cadiaplayer::play::parsing::ct_VARIABLE)
				itr->second[i].insert(symbol->getName());
		}
		return;
	}
}
void DomainAnalyzer::processTrue(cadiaplayer::play::parsing::Compound* trueCompound, PredicateDomains& domains)
{
	trueCompound = trueCompound->getArgument(0);
	PredicateDomainsItr itr = domains.find(trueCompound->getName());
	if(itr == domains.end())
		itr = (domains.insert(PredicateDomainsPair(trueCompound->getName(), Domains()))).first;
	if(itr->second.size() < trueCompound->getArguments()->size())
		itr->second.resize(trueCompound->getArguments()->size());
	for(size_t i = 0 ; i < trueCompound->getArguments()->size() ; i++)
	{
		itr->second[i].insert((*(trueCompound->getArguments()))[i]->getName());
	}
}
void DomainAnalyzer::processVariable(cadiaplayer::play::GameTheory& theory, Domain& domain, 
									 cadiaplayer::play::parsing::Compound* predicate, 
									 cadiaplayer::play::parsing::Compound* variable, Occurrance& occurred)
{
	if(predicate->getOperator() == cadiaplayer::play::parsing::co_DISTINCT)
		return;
	if(predicate->getOperator() == cadiaplayer::play::parsing::co_OR)
	{
		for(size_t i = 0 ; i < predicate->getArguments()->size() ; i++)
		{
			processVariable(theory, domain, predicate->getArgument(i), variable, occurred);
		}
		return;
	}
	if(predicate->getOperator() == cadiaplayer::play::parsing::co_NOT)
	{
		processVariable(theory, domain, predicate->getArgument(0), variable, occurred);
		return;
	}
	if(predicate->getOperator() == cadiaplayer::play::parsing::co_RELATION_DOES)
	{
		processVariable(theory, domain, predicate->getArgument(1), variable, occurred);
		return;
	}
	if(occurred.has(predicate->getName(), variable->getName()))
		return;
	occurred.add(predicate->getName(), variable->getName());
	cadiaplayer::play::KBaseList& rules = *(theory.getRules());
	cadiaplayer::play::KBaseList& legals = *(theory.getLegals());
	cadiaplayer::play::KBaseList& inits = *(theory.getStateRef());
	cadiaplayer::play::KBaseList& nexts = *(theory.getNexts());
	if(predicate->getArguments() == NULL)
		return;
	for(size_t n = 0 ; n < predicate->getArguments()->size() ; n++)
	{
		if(predicate->getArgument(n)->getName() != variable->getName())
			continue;
		for(size_t i = 0 ; i < rules.size() ; i++)
		{
			if(rules[i]->getOperator() == cadiaplayer::play::parsing::co_IMPLICATION)
			{
				if(predicate->getName() != rules[i]->getArgument(0)->getName())
					continue;
				if(n >= rules[i]->getArgument(0)->getArguments()->size())
					continue;
				if(rules[i]->getArgument(0)->getArgument(n)->getType() == cadiaplayer::play::parsing::ct_VARIABLE)
				{
					for(size_t j = 1 ; j < rules[i]->getArguments()->size() ; j++)
					{
						processVariable(theory, domain, rules[i]->getArgument(j), rules[i]->getArgument(0)->getArgument(n), occurred);
					}
				}
				else
					domain.insert(rules[i]->getArgument(0)->getArgument(n)->getName());
			}
			else
			{
				if(predicate->getName() != rules[i]->getName())
					continue;
				if(rules[i]->getArguments()->size() > n)
					domain.insert(rules[i]->getArgument(n)->getName());
			}
		}
		for(size_t i = 0 ; i < legals.size() ; i++)
		{
			if(legals[i]->getOperator() == cadiaplayer::play::parsing::co_IMPLICATION)
			{
				if(predicate->getName() != legals[i]->getArgument(0)->getName())
					continue;
				if(legals[i]->getArgument(0)->getArguments() == NULL)
					continue;
				if(n >= legals[i]->getArgument(0)->getArguments()->size())
					continue;
				if(legals[i]->getArgument(0)->getArgument(n)->getType() == cadiaplayer::play::parsing::ct_VARIABLE)
				{
					for(size_t j = 1 ; j < legals[i]->getArguments()->size() ; j++)
					{
						processVariable(theory, domain, legals[i]->getArgument(j), legals[i]->getArgument(0)->getArgument(n), occurred);
					}
				}
				else
					domain.insert(legals[i]->getArgument(0)->getArgument(n)->getName());
			}
			else
			{
				if(predicate->getName() != legals[i]->getName())
					continue;
				if(legals[i]->getArguments()->size() > n)
					domain.insert(legals[i]->getArgument(n)->getName());
			}
		}
		for(size_t i = 0 ; i < inits.size() ; i++)
		{
			if(predicate->getName() != inits[i]->getName())
				continue;
			if(inits[i]->getArguments()->size() > n)
				domain.insert(inits[i]->getArgument(n)->getName());
		}
		for(size_t i = 0 ; i < nexts.size() ; i++)
		{
			if(nexts[i]->getOperator() == cadiaplayer::play::parsing::co_IMPLICATION)
			{
				if(predicate->getName() != nexts[i]->getArgument(0)->getName())
					continue;
				if(nexts[i]->getArgument(0)->getArguments() == NULL)
					continue;
				if(n >= nexts[i]->getArgument(0)->getArguments()->size())
					continue;
				if(nexts[i]->getArgument(0)->getArgument(n)->getType() == cadiaplayer::play::parsing::ct_VARIABLE)
				{
					for(size_t j = 1 ; j < nexts[i]->getArguments()->size() ; j++)
						processVariable(theory, domain, nexts[i]->getArgument(j), nexts[i]->getArgument(0)->getArgument(n), occurred);
				}
				else
					domain.insert(nexts[i]->getArgument(0)->getArgument(n)->getName());
			}
			else
			{
				if(predicate->getName() != nexts[i]->getName())
					continue;
				if(nexts[i]->getArguments()->size() > n)
					domain.insert(nexts[i]->getArgument(n)->getName());
			}
		}
	}
}
