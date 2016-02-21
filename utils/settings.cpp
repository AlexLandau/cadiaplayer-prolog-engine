/*
 *  settings.cpp
 *  cadiaplayer
 *
 *  Created by Hilmar Finnsson on 9/21/12.
 *  Copyright 2012 Reykjavik University. All rights reserved.
 *
 */

#include "settings.h"

using namespace cadiaplayer::utils;

bool Settings::save()
{
	std::fstream file;
	file.open(m_filename.c_str(), std::ios::trunc|std::ios::out);
	if(file.fail())
	{
		m_ok = false;
		return false;
	}

	if(m_lines.empty())
	{
		file.close();
		return true;
	}

	for (int n = 0 ; n < (m_lines.size()-1) ; n++) 
	{
		file << m_lines[n] << std::endl;
	}
	file << m_lines[m_lines.size()-1];
	file.close();
	
	return true;
}
bool Settings::load()
{
	std::ifstream file;
	file.open(m_filename.c_str());
	if(file.fail())
	{
		save();
		file.open(m_filename.c_str());
		if(file.fail())
		{
			m_ok = false;
			return false;
		}
	}
	
	// All is ok
	std::string line;
	std::string temp;
	std::vector<std::string> tokens;
	SettingsEntry* entry;
	SettingsTable::iterator itr;
	int pos = -1;
	while (getline(file, line))
	{
		++pos;
		m_lines.push_back(line);
		temp = StringUtils::trim(line);
		if(temp.empty() || temp[0] == SETTINGS_COMMENT)
			continue;
		tokens.clear();
		StringUtils::tokenize(temp, tokens, SETTINGS_EQUALS);
		if(tokens.size() != 2)
			continue;
		
		entry = new SettingsEntry();
		entry->name = StringUtils::trim(tokens[0]);
		entry->filepos = pos;
		entry->value = StringUtils::trim(tokens[1]);
		m_settings[entry->name] = entry;
	}
	file.close();
	
	return true;
}
SettingsEntry* Settings::lookup(std::string settingname)
{
	SettingsTable::iterator itr;
	itr = m_settings.find(settingname);
	if(itr == m_settings.end())
		return NULL;
	return itr->second;
}
SettingsEntry* Settings::insert(std::string settingname)
{
	SettingsEntry* entry = lookup(settingname);
	if(entry)
		return entry;
	entry = new SettingsEntry();
	entry->filepos = m_lines.size();
	entry->name = settingname;
	m_settings[entry->name] = entry;
	m_lines.push_back("");
	
	return entry;
}
std::string Settings::createLine(SettingsEntry* entry)
{
	return entry->name + SETTINGS_EQUALS + entry->value + '\n';
}

Settings::Settings(std::string filename):
m_ok(true),
m_filename(filename)
{
	load();
}
Settings::~Settings()
{
	SettingsTable::iterator itr;
	for(itr = m_settings.begin() ; itr != m_settings.end() ; itr++)
	{
		delete itr->second;
	}
	m_settings.clear();
}

bool Settings::isOk()
{
	return m_ok;
}

bool Settings::getIntValue(std::string settingname, int& settingvalue)
{
	if(!isOk())
		return false;
	
	SettingsEntry* entry = lookup(settingname);
	if(!entry)
		return false;
	
	std::stringstream ss;
	ss << entry->value;
	ss >> settingvalue;
	return true;
}
bool Settings::getDoubleValue(std::string settingname, double& settingvalue)
{
	if(!isOk())
		return false;
	
	SettingsEntry* entry = lookup(settingname);
	if(!entry)
		return false;
	
	std::stringstream ss;
	ss << entry->value;
	ss >> settingvalue;
	return true;
}
bool Settings::getStringValue(std::string settingname, std::string& settingvalue)
{
	if(!isOk())
		return false;
	
	SettingsEntry* entry = lookup(settingname);
	if(!entry)
		return false;
	
	settingvalue = entry->value;
	return true;
}

bool Settings::set(std::string settingname, int settingvalue)
{
	if(!isOk())
		return false;
	
	settingname = StringUtils::trim(settingname);
	SettingsEntry* entry = insert(settingname);
	std::stringstream ss;
	ss << settingvalue;
	entry->value = ss.str();
	m_lines[entry->filepos] = createLine(entry);
	save();
	
	return true;
}
bool Settings::set(std::string settingname, double settingvalue)
{
	if(!isOk())
		return false;
	
	settingname = StringUtils::trim(settingname);
	SettingsEntry* entry = insert(settingname);
	std::stringstream ss;
	ss << settingvalue;
	entry->value = ss.str();
	m_lines[entry->filepos] = createLine(entry);
	save();
	
	return true;
}
bool Settings::set(std::string settingname, std::string settingvalue)
{
	if(!isOk())
		return false;
	settingname = StringUtils::trim(settingname);
	settingvalue = StringUtils::trim(settingvalue);
	SettingsEntry* entry = insert(settingname);
	entry->value = settingvalue;
	m_lines[entry->filepos] = createLine(entry);
	save();
	
	return true;
}

void Settings::printSettings(std::ostream& os)
{
	if(!isOk())
	{
		os << SETTINGS_FILE_ERROR << std::endl;
		return;
	}

	SettingsTable::iterator itr;
	SettingsEntry* entry;
	for(itr = m_settings.begin() ; itr != m_settings.end() ; itr++)
	{
		entry = itr->second;
		os << entry->filepos << ") " << entry->name << SETTINGS_EQUALS << entry->value << std::endl;
	}
}
void Settings::printFile(std::ostream& os)
{
	if(!isOk())
	{
		os << SETTINGS_FILE_ERROR << std::endl;
		return;
	}

	for(int n = 0 ; n < m_lines.size() ; n++)
	{
		os << m_lines[n] << std::endl;
	}
}

