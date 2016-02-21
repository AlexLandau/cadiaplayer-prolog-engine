#include "symtab.h"
#include <string>
#include <stdio.h>

using namespace cadiaplayer::play::parsing;

SymbolTableEntry::SymbolTableEntry(std::string l, SymbolID i):
lexeme(l),
id(i),
altered(false)
{
	for (size_t i = 0; i < l.length(); i++) 
	{
		if (!std::isdigit(l[i]))
		{
			type = st_STRING;
			return;
		}
	}
	type = st_NUM;
}
std::string SymbolTableEntry::getLexeme(void)
{
	return this->lexeme;
}
std::string SymbolTableEntry::getOriginalLexeme(void)
{
	if(originalLexeme.length())
		return this->originalLexeme;
	return getLexeme();
}
SymbolID SymbolTableEntry::getId(void)
{
	return this->id;
}
void SymbolTableEntry::overwriteId(SymbolID i)
{
	this->id = i;
}

SymbolTable::SymbolTable(void):
constantCount(MIN_SYMBOL_CONSTANT),
variableCount(MIN_SYMBOL_VARIABLE),
moveClassCount(0),
parameterMax(1)
{}
SymbolTable::~SymbolTable(void)
{
	SymbolMap::iterator it = table.begin();
	while(it != table.end())
	{
		if((*it).second != NULL)
			delete (*it).second;
		it++;
	}
	table.clear();
	idTable.clear();
}
unsigned int SymbolTable::getConstantCount(void)
{
	return constantCount;
}
unsigned int SymbolTable::getVariableCount(void)
{
	return variableCount;
}
unsigned int SymbolTable::getParameterMax(void)
{
	return parameterMax;
}
SymbolID SymbolTable::insertConstant(std::string lexeme)
{
	SymbolTableEntry* entry = lookup(lexeme);
	if( entry != NULL)
		return entry->getId();
	bool subst = false;
	std::string original = lexeme;
	for(size_t n = 0 ; n < lexeme.length() ; n++)
	{
		if(lexeme[n] == '+')
		{
			subst = true;
			lexeme[n] = 'p';
		}
		else if(lexeme[n] == '-')
		{
			subst = true;
			lexeme[n] = 'm';
		}
	}
	entry = new SymbolTableEntry(lexeme, constantCount);
	if(subst)
		entry->alter(original);
	table[lexeme] = entry;
	idTable[constantCount] = entry;
	constantCount++;
	
	// Add to number map is a number
	if(entry->getType() == st_NUM)
	{
		int num = atoi(lexeme.c_str());
		while(constantCount >= numberMap.size())
		{
			numberMap.resize(constantCount+1, -1);
		}
		numberMap[entry->getId()] = num;
	}
	
	return constantCount-1;
}
SymbolID SymbolTable::insertVariable(std::string lexeme)
{
	SymbolTableEntry* entry = lookup(lexeme);
	if( entry != NULL)
		return entry->getId();
	for(size_t n = 0 ; n < lexeme.length() ; n++)
	{
		if(lexeme[n] == '-')
			lexeme[n] = '_';
	}
	bool subst = false;
	std::string original = lexeme;
	for(size_t n = 0 ; n < lexeme.length() ; n++)
	{
		if(lexeme[n] == '+')
		{
			subst = true;
			lexeme[n] = 'p';
		}
		else if(lexeme[n] == '-')
		{
			subst = true;
			lexeme[n] = 'm';
		}
	}
	entry = new SymbolTableEntry(lexeme, variableCount);
	if(subst)
		entry->alter(original);
	table[lexeme] = entry;
	idTable[variableCount] = entry;
	variableCount++;
	return variableCount-1;
}
SymbolID SymbolTable::insertMoveClass(std::string lexeme)
{
	std::stringstream ss;
	ss << lexeme << "_" << (++moveClassCount);
	SymbolID id = insertConstant(ss.str());
	SymbolTableEntry* entry = lookup(id);
	entry->alter(lexeme);
	return id;
}
SymbolID SymbolTable::insertMoveClass(SymbolID id)
{
	SymbolTableEntry* entry = lookup(id);
	return insertMoveClass(entry->getLexeme());
}
void SymbolTable::updateParameterMax(unsigned int count)
{
	if(parameterMax < count)
	{
		//printf("Parameter max set to %d\n", count);
		parameterMax = count;
	}
}
SymbolTableEntry* SymbolTable::lookup(std::string lexeme)
{
	SymbolMap::iterator it = table.find(lexeme);
	if(it == table.end())
		return NULL;
	return it->second;
}
SymbolTableEntry* SymbolTable::lookup(SymbolID id)
{
	SymbolIdMap::iterator it = idTable.find(id);
	if(it == idTable.end())
		return NULL;
	return it->second;
}
std::string SymbolTable::getName(SymbolID id)
{
	SymbolIdMap::iterator it = idTable.find(id);
	
	if(it == idTable.end())
		return "";
	
	return it->second->getLexeme();
}

std::string SymbolTable::getOriginalName(SymbolID id)
{
	SymbolIdMap::iterator it = idTable.find(id);
	
	if(it == idTable.end())
		return "";
	if(it->second->isAltered())
		return it->second->getOriginalLexeme();
	return it->second->getLexeme();
}

int SymbolTable::getNum(SymbolID id)
{
	/*SymbolIdMap::iterator it = idTable.find(id);
	
	if(it == idTable.end())
		return -1;
	SymbolTableEntry* e = it->second;
	return e->getType() == st_NUM ? atoi(e->getLexeme().c_str()) : -1;*/
	
	return numberMap[id];
}

void SymbolTable::swap(std::string lexeme, SymbolID newid)
{
	SymbolTableEntry* swapentry = table[lexeme];
	SymbolTableEntry* swapfor = idTable[newid];
	
	SymbolTableEntry* newswapentry = new SymbolTableEntry(swapentry->getLexeme(), swapfor->getId());
	SymbolTableEntry* newswapfor = new SymbolTableEntry(swapfor->getLexeme(), swapentry->getId());
	
	delete swapentry;
	delete swapfor;
	
	table.erase(newswapentry->getLexeme());
	table.erase(newswapfor->getLexeme());
	idTable.erase(newswapentry->getId());
	idTable.erase(newswapfor->getId());
	
	table[newswapentry->getLexeme()] = newswapentry;
	idTable[newswapentry->getId()] = newswapentry;
	
	table[newswapfor->getLexeme()] = newswapfor;
	idTable[newswapfor->getId()] = newswapfor;
}

void SymbolTable::print(void)
{
	SymbolMap::iterator it = table.begin();
	while(it != table.end())
	{
		printf("\t%s -> %u\n", it->first.c_str(), it->second->getId());
		it++;
	}
	printf("\n");
}
