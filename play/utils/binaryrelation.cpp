/*
 *  binaryrelation.cpp
 *  cadiaplayer
 *
 *  Created by Hilmar Finnsson on 9/24/09.
 *  Copyright 2009 Reykjavik University. All rights reserved.
 *
 */

#include "binaryrelation.h"

using namespace cadiaplayer::play;
using namespace cadiaplayer::play::utils;

BinaryRelationNode::BinaryRelationNode(cadiaplayer::play::parsing::SymbolID name):
m_name(name),
m_left(NULL),
m_right(NULL)
{
}
BinaryRelationNode::~BinaryRelationNode()
{
}
cadiaplayer::play::parsing::SymbolID BinaryRelationNode::getName()
{
	return this->m_name;
}
BinaryRelationNode* BinaryRelationNode::getLeft()
{
	return this->m_left;
}
BinaryRelationNode* BinaryRelationNode::getRight()
{
	return this->m_right;
}
void BinaryRelationNode::setLeft(BinaryRelationNode* node)
{
	this->m_left = node;
}
void BinaryRelationNode::setRight(BinaryRelationNode* node)
{
	this->m_right = node;
}

std::string BinaryRelationNode::toString()
{
	std::stringstream ss;
	if(m_left)
		ss << m_left->getName();
	else
		ss << "NULL";
		ss << "<-" << this->m_name << "->";
	if(m_right)
		ss << m_right->getName();
	else
		ss << "NULL";
	return ss.str();
}
std::string BinaryRelationNode::toString(cadiaplayer::play::parsing::SymbolTable* symbols)
{
	std::stringstream ss;
	if(m_left)
		ss << symbols->getName(m_left->getName());
	else
		ss << "NULL";
	ss << "<-" << symbols->getName(this->m_name) << "->";
	if(m_right)
		ss << symbols->getName(m_right->getName());
	else
		ss << "NULL";
	return ss.str();
}

BinaryRelation::BinaryRelation(cadiaplayer::play::parsing::SymbolID name) :
m_name(name),
m_first(NULL),
m_last(NULL)
{
}
BinaryRelation::~BinaryRelation()
{
	this->clear();
}
BinaryRelationNode* BinaryRelation::getFirst()
{
	return this->m_first;
}
BinaryRelationNode* BinaryRelation::getLast()
{
	return this->m_last;
}
BinaryRelationNode* BinaryRelation::get(cadiaplayer::play::parsing::SymbolID name)
{
	BinaryRelationNodeMapItr itr = this->m_map.find(name);
	if(itr == this->m_map.end())
		return NULL;
	return itr->second;
}
BinaryRelationNode* BinaryRelation::getAny()
{
	return this->m_map.size() ? this->m_map.begin()->second : NULL;
}

bool BinaryRelation::isEmpty()
{
	return !m_map.size();
}
bool BinaryRelation::isLooped()
{
	return (!m_first && m_map.size());
}
bool BinaryRelation::isBroken()
{
	return (m_first && !m_last);
}
bool BinaryRelation::isChain()
{
	return (m_first && m_last);
}

void BinaryRelation::add(cadiaplayer::play::parsing::SymbolID left, cadiaplayer::play::parsing::SymbolID right)
{
	BinaryRelationNode* l = get(left);
	if(!l)
	{
		l = new BinaryRelationNode(left);
		this->m_map[left] = l;
	}
	BinaryRelationNode* r = get(right);
	if(!r)
	{
		r = new BinaryRelationNode(right);
		this->m_map[right] = r;
	}
	
	l->setRight(r);
	r->setLeft(l);
	
	findEndpoints();
}
void BinaryRelation::remove(cadiaplayer::play::parsing::SymbolID name)
{
	BinaryRelationNodeMapItr itr = this->m_map.find(name);
	if(itr == this->m_map.end())
		return;
	BinaryRelationNode* temp = itr->second;
	if(!temp)
		return;
	if(temp->getLeft())
		temp->getLeft()->setRight(NULL);
	if(temp->getRight())
		temp->getRight()->setLeft(NULL);
	this->m_map.erase(itr);
	delete temp;
}
void BinaryRelation::clear()
{
	BinaryRelationNodeMapItr itr;
	for(itr = this->m_map.begin() ; itr != this->m_map.end() ; itr++)
	{
		delete itr->second;
	}
	this->m_map.clear();
}
void BinaryRelation::findEndpoints()
{
	this->m_first = NULL;
	this->m_last = NULL;
	BinaryRelationNodeMapItr itr;
	for(itr = this->m_map.begin() ; itr != this->m_map.end() ; itr++)
	{
		if(!itr->second->getLeft())
		{
			this->m_first = itr->second;
			break;
		}
	}
	if(!m_first)
		return;
	
	size_t n = 0;
	BinaryRelationNode* temp = m_first;
	while(++n < m_map.size() && temp->getRight())
	{
		temp = temp->getRight(); 
	}
	if(n == m_map.size() && !(temp->getRight()))
		m_last = temp;
}

std::string BinaryRelation::toString(cadiaplayer::play::parsing::SymbolTable* symbols)
{
	std::stringstream ss;
	ss << symbols->getName(this->m_name) << " : ";
	if(isEmpty())
	{
		ss << "(empty)";
		return ss.str();
	}
	size_t n = 0;
	if(m_first)
	{
		BinaryRelationNode* temp = m_first;
		ss << "<" << symbols->getName(temp->getName());
		while(temp->getRight())
		{
			if(++n >= this->m_map.size())
				break;
			ss << ">-<" << symbols->getName(temp->getRight()->getName());
			temp = temp->getRight();
		}
		ss << ">";
	}
	if(isLooped())
		ss << " (looped)";
	if(isBroken())
		ss << " (broken)";
	if(isChain())
		ss << " (chain)";
	return ss.str();
}

BinaryRelations::BinaryRelations()
{
}
BinaryRelations::~BinaryRelations()
{
	this->clear();
}

BinaryRelation* BinaryRelations::getBinaryRelation(cadiaplayer::play::parsing::SymbolID name)
{
	BinaryRelationMapItr itr = this->m_map.find(name);
	if(itr == this->m_map.end())
		return NULL;
	return itr->second;
}
BinaryRelation* BinaryRelations::addBinaryRelation(cadiaplayer::play::parsing::SymbolID name)
{
	BinaryRelation* temp = getBinaryRelation(name);
	if(!temp)
	{
		temp = new BinaryRelation(name);
		this->m_map[name] = temp;
	}
	return temp;
}

void BinaryRelations::clear()
{
	BinaryRelationMapItr itr;
	for(itr = this->m_map.begin() ; itr != this->m_map.end() ; itr++)
	{
		delete itr->second;
	}
	this->m_map.clear();
}

std::string BinaryRelations::toString(cadiaplayer::play::parsing::SymbolTable* symbols)
{
	std::stringstream ss;
	ss << "Binary Relations:" << std::endl; 
	BinaryRelationMapItr itr;
	for(itr = this->m_map.begin() ; itr != this->m_map.end() ; itr++)
	{
		ss << itr->second->toString(symbols) << std::endl;
	}
	return ss.str();
}

