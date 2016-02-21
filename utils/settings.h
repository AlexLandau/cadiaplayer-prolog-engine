/*
 *  settings.h
 *  cadiaplayer
 *
 *  Created by Hilmar Finnsson on 9/21/12.
 *  Copyright 2012 Reykjavik University. All rights reserved.
 *
 */

#ifndef SETTINGS_H
#define SETTINGS_H

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include <ext/hash_map>
#include "stringhash.h"
#include "strutils.h"

#define SETTINGS_FILE_ERROR "Could not open settings file."

namespace cadiaplayer {
	namespace utils {

		const char SETTINGS_COMMENT = '#';
		const char SETTINGS_EQUALS = '=';
		
		typedef std::vector<std::string> Filelines;
		struct SettingsEntry
		{
			int filepos;
			
			std::string name;
			std::string value;
		};
		typedef __gnu_cxx::hash_map<std::string, SettingsEntry*, StringHash> SettingsTable;
		
		class Settings
		{
		private:
			std::string m_filename;
			bool m_ok;
			bool m_create;
			Filelines m_lines;
			SettingsTable m_settings;
			
			bool save();
			bool load();
			SettingsEntry* lookup(std::string settingname);
			SettingsEntry* insert(std::string settingname);
			std::string createLine(SettingsEntry* entry);
		public:
			Settings(std::string m_filename);
			~Settings();

			bool isOk();
			
			bool getIntValue(std::string settingname, int& settingvalue);
			bool getDoubleValue(std::string settingname, double& settingvalue);
			bool getStringValue(std::string settingname, std::string& settingvalue);

			bool set(std::string settingname, int settingvalue);
			bool set(std::string settingname, double settingvalue);
			bool set(std::string settingname, std::string settingvalue);
			
			void printSettings(std::ostream& os);
			void printFile(std::ostream& os);
		};
	}} // namespaces
#endif // SETTINGS_H
