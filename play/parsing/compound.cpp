#include "compound.h"
#include <stdio.h>

using namespace std;
using namespace cadiaplayer::play::parsing;

Compound::Compound()
{
	name = NO_NAME;
	type = ct_COMPOUND;
	op = co_NONE;
	args = NULL;
	parent = NULL;
}
Compound::Compound(CompoundType t, SymbolID n, CompoundOperator o)
{
	args = NULL;
	type = t;
	name = n;
	op = o;
	parent = NULL;
}

Compound::Compound(const Compound &c)
{
	type = c.type;
	name = c.name;
	op = c.op;
	if(c.args != NULL)
	{
		args = new CompoundArgs();
		for(size_t n = 0 ; n < c.args->size() ; n++)
		{
			args->push_back(new Compound(*(*c.args)[n]));
		}
	}
	else
		args = NULL;
	parent = c.parent;
}
#include <iostream>
Compound::~Compound(void)
{
	//std::cerr << "~Compound\n";
	if(args == NULL)
	{
	//	std::cerr << "~Compound ends with no args\n";
		return;
	}
	//if(args->size())
	//	std::cerr << "deleting args\n";
	for(size_t n = 0; n < args->size() ; n++)
	{
		try
		{
			Compound* c = (*args)[n];
			delete c;
			//delete (*args)[n];
		}
		catch(...)
		{
			if((*args)[n]==NULL)
				std::cerr << "Error, the arg (" << n << ") is NULL and cannot be deleted\n";
			else
				std::cerr << "Error, the arg (" << n << ") cannot be deleted\n";
		}
	}
	args->clear();
	//std::cerr << "deleting args vector\n";
	delete args;
	args = NULL;
	//std::cerr << "~Compound ends\n";
}
int Compound::operator ==(const Compound& c)
{
	if(op != c.op)
		return op - c.op;
	if(type != c.type)
		return type - c.type;
	if(name != c.name)
		return name - c.name;
	if(args == NULL)
	{
		if(c.args == NULL)
			return 0;
		return -1;
	}
	if(c.args == NULL)
		return 1;
	
	if(args->size() != c.args->size())
		return args->size() - c.args->size();
	Compound *arg1, *arg2; 
	int argc = 0;
	for(size_t n = 0 ; n < args->size() ; n++)
	{
		arg1 = (*(args))[n];
		arg2 = (*(c.args))[n];
		argc = ((*arg1) == (*arg2));
		if(argc == 0)
			continue;
		return argc;
	}
	return 0;
}
int Compound::operator <(const Compound& c)
{
	return (*this==c) < 0;
}

Compound& Compound::operator =(const Compound& c)
{
	type = c.type;
	name = c.name;
	op = c.op;
	if(c.args != NULL)
	{
		args = new CompoundArgs();
		for(size_t n = 0 ; n < c.args->size() ; n++)
		{
			args->push_back(new Compound(*(*c.args)[n]));
		}
	}
	else
		args = NULL;
	parent = c.parent;
	
	return *this;
}

SymbolID Compound::getName(void) const
{
	return name;
}
CompoundOperator Compound::getOperator(void) const
{
	return op;
}
CompoundArgs* Compound::getArguments(void) const
{
	return args;
}
Compound* Compound::getArgument(size_t i) const
{
	assert(args->size() > i);
	return (*args)[i];
}
CompoundType Compound::getType(void) const
{
	return type;
}
Compound* Compound::getParent(void) const
{
	return parent;
}
void Compound::setName(SymbolID n)
{
	name = n;
}
void Compound::setOperator(CompoundOperator o)
{
	op = o;
}
void Compound::setType(CompoundType t)
{
	type = t;
}
void Compound::addArgument(Compound* c)
{
	if(c == NULL)
		return;
	if(type == ct_VALUE || type == ct_VARIABLE)
	{
		type = ct_COMPOUND;
		op = co_PREDICATE;
	}
	if(args == NULL)
		args = new CompoundArgs();
	args->push_back(c);
	return;
}
void Compound::addArgumentFront(Compound* c)
{
	if(c == NULL)
		return;
	if(type == ct_VALUE || type == ct_VARIABLE)
	{
		type = ct_COMPOUND;
		op = co_PREDICATE;
	}
	if(args == NULL)
		args = new CompoundArgs();
	args->insert(args->begin(), c);
	return;
}
void Compound::removeArgumentFront()
{
	if(args == NULL || args->size() == 0)
		return;
	args->erase(args->begin());
	return;
}
void Compound::setParent(Compound* c)
{
	parent = c;
}
bool Compound::hasVariable(void) const
{
	if(type==ct_VARIABLE)
		return true;
	if(type==ct_VALUE)
		return false;
	if(args == NULL)
		return true;
	CompoundArgs::iterator it = args->begin();
	while(it!=args->end())
	{
		if((*it)->hasVariable())
			return true;
		it++;
	}
	return false;
}
bool Compound::isGround(void) const
{
	return !hasVariable();
}

static const char* OpStrings[] =
{
"",
"~",
"&",
"|",
"/",
"",
"=>",
"",
"role",
"init",
"next",
"does",
"true",
"legal",
"goal",
"terminal"
};
static const int OpStringsLength = 17;

std::string Compound::toString(SymbolTable* table) const
{
	if(table == NULL)
		return "";
	
	if(type == ct_VALUE || type == ct_VARIABLE)
		return table->getName(name);
	
	std::string str = "";
	
	CompoundArgs::iterator it;
	if(op == co_IMPLICATION)
	{
		str = args->front()->toString(table);
		str += " ";
		str += OpStrings[op];
		for(it = ++(args->begin()) ; it != args->end() ; it++)
		{
			str += " ";
			str += (*it)->toString(table);
		}
		return str;
	}
	if(op == co_PREDICATE || op >= co_RELATION)
	{
		str += "(";
		str += table->getName(name);
	}
	else if(op < OpStringsLength)
		str += OpStrings[op];
	else
	{
		str += "!ERR!";
		return str;
	}
	if(args != NULL)
	{
		it = args->begin();
		while(it != args->end())
		{
			if(*it == NULL)
			{
				printf("Null argument in compound detected!!!\n");
				break;
			}
			str += " ";
			str += (*it)->toString(table);
			it++;
		}
	}	
	
	if(op == co_PREDICATE || op >= co_RELATION)
		str += ")";
	return str;
}
std::string Compound::toPlayString(SymbolTable* table)
{
	if(table == NULL)
		return "";
	
	if(type == ct_VALUE || type == ct_VARIABLE || !args || !args->size())
		return table->getOriginalName(name);
	
	std::string str = "(";
	
	str += table->getOriginalName(name);
	CompoundArgs::iterator it = args->begin();
	//it++;
	while(it != args->end())
	{
		str += " ";
		str += table->getOriginalName((*it)->getName());
		it++;
	}
	str += ")";
	return str;
}

long Compound::sizeInBytes(void)
{
	long size = sizeof(Compound);
	if(args == NULL)
		return size;
	for(size_t n = 0 ; n < args->size() ; n++)
	{
		if((*args)[n] == NULL)
			continue;
		size += (*args)[n]->sizeInBytes();
	}
	return size;
}

std::string Compound::toSaveString(Compound* c)
{
	if(c == NULL)
		return "";
	//printf("Generating save std::string for : %u\n", c->getName()); 
	char buffer[1024];
	std::string str = "";
	if(c->getArguments() != NULL && c->getType() == ct_COMPOUND)
	{
		bool first = true;
		str += "(";
		for(size_t n = 0 ; n < c->getArguments()->size() ; ++n)
		{
			if(!first)
				str += ",";
			first = false;
			str += toSaveString(c->getArgument(n));
		}
		str += ")";
	}
	sprintf(buffer, "[%un%ut%uo", c->getName(), c->getType(), c->getOperator());
	str = buffer +str;
	str += "]";
	return str;
}
Compound* Compound::fromSaveString(const std::string& str, size_t& index)
{
	fprintf(stderr, "Generating compound from save std::string : %s\n", str.c_str()); 
	Compound* c = new Compound();
	std::string buffer = "";
	++index;
	while(str[index] != 'n' )
	{
		buffer += str[index];	
		++index;
	}
	++index;
	c->setName(strtoul(buffer.c_str(), NULL, 0));
	buffer = "";
	while(str[index] != 't' )
	{
		buffer += str[index];	
		++index;
	}
	++index;
	c->setType(static_cast<CompoundType>(strtol(buffer.c_str(), NULL, 0)));
	buffer = "";
	while(str[index] != 'o' )
	{
		buffer += str[index];	
		++index;
	}
	++index;
	c->setOperator(static_cast<CompoundOperator>(strtol(buffer.c_str(), NULL, 0)));
	buffer = "";
	
	if(str[index] == ']')
	{
		++index;
		return c;
	}
	
	do
	{		
		++index; // '(' or ','	
		c->addArgument(fromSaveString(str, index));
	}
	while(str[index] != ')');
	++index; //')'
	++index; //']'
	return c;
}
