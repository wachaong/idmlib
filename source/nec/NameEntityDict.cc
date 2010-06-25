/*
 * NameEntityDict.cc
 *
 *  Created on: Mar 31, 2010
 *      Author: eric
 */

#include <iostream>
#include <fstream>
#include <string>
#include <idmlib/nec/NameEntityDict.h>
#include <idmlib/util/Util.hpp>
namespace idmlib
{

hash_set<std::string> NameEntityDict::locSuffix;
hash_set<std::string> NameEntityDict::orgSuffix;
hash_set<std::string> NameEntityDict::peopSuffix;
hash_set<std::string> NameEntityDict::namePrefix;
hash_set<std::string> NameEntityDict::locList_;
hash_set<std::string> NameEntityDict::orgList_;
hash_set<std::string> NameEntityDict::peopList_;
hash_set<std::string> NameEntityDict::noiseList_;
hash_set<std::string> NameEntityDict::otherList_;
hash_set<std::string> NameEntityDict::nounList_;
hash_set<std::string> NameEntityDict::locLeft_;
hash_set<std::string> NameEntityDict::locRight_;
hash_set<std::string> NameEntityDict::orgLeft_;
hash_set<std::string> NameEntityDict::orgRight_;
hash_set<std::string> NameEntityDict::peopLeft_;
hash_set<std::string> NameEntityDict::peopRight_;

void NameEntityDict::loadSuffix(std::string& path, hash_set<std::string>& suffixSet)
{
	if (suffixSet.size() == 0)
		{
            std::istream* inStream = idmlib::util::getResourceStream(path);
// 			std::ifstream inStream(path.c_str());
			if (!inStream)
			{
				return;
			}
			std::string line;
			while(std::getline(*inStream, line))
			{
				if (line.length() > 0)
				{
					std::transform(line.begin(), line.end(), line.begin(), ::tolower);
					suffixSet.insert(line);
				}
			}
            delete inStream;
// 			inStream.close();
		}
}

void NameEntityDict::loadLocSuffix(std::string& path)
{
	loadSuffix(path, locSuffix);
}


void NameEntityDict::loadPeopSuffix(std::string& path)
{
	loadSuffix(path, peopSuffix);
}


void NameEntityDict::loadOrgSuffix(std::string& path)
{
	loadSuffix(path, orgSuffix);
}

void NameEntityDict::loadNamePrefix(std::string& path)
{
	loadSuffix(path, namePrefix);
}

void NameEntityDict::loadLocList(std::string& path)
{
	loadSuffix(path, locList_);
}

void NameEntityDict::loadPeopList( std::string& path)
{
	loadSuffix(path, peopList_);
}

void NameEntityDict::loadOrgList( std::string& path)
{
	loadSuffix(path, orgList_);
}

void NameEntityDict::loadNoiseList( std::string& path)
{
	loadSuffix(path, noiseList_);
}

void NameEntityDict::loadOtherList( std::string& path)
{
	loadSuffix(path, otherList_);
}

void NameEntityDict::loadNounList( std::string& path)
{
	loadSuffix(path, nounList_);
}


void NameEntityDict::loadLocLeft(std::string& path)
{
	loadSuffix(path, locLeft_);
}

void NameEntityDict::loadLocRight( std::string& path)
{
	loadSuffix(path, locRight_);
}

void NameEntityDict::loadOrgLeft( std::string& path)
{
	loadSuffix(path, orgLeft_);
}

void NameEntityDict::loadOrgRight( std::string& path)
{
	loadSuffix(path, orgRight_);
}

void NameEntityDict::loadPeopLeft( std::string& path)
{
	loadSuffix(path, peopLeft_);
}

void NameEntityDict::loadPeopRight( std::string& path)
{
	loadSuffix(path, peopRight_);
}


bool NameEntityDict::isLocSuffix(std::string& str)
{
	std::string tempStr(str);
	std::transform(tempStr.begin(), tempStr.end(), tempStr.begin(), ::tolower);
	return locSuffix.find(tempStr) != locSuffix.end();
}

bool NameEntityDict::isOrgSuffix(std::string& str)
{
	std::string tempStr(str);
	std::transform(tempStr.begin(), tempStr.end(), tempStr.begin(), ::tolower);
	return orgSuffix.find(tempStr) != orgSuffix.end();
}

bool NameEntityDict::isPeopSuffix(std::string& str)
{
	std::string tempStr(str);
	std::transform(tempStr.begin(), tempStr.end(), tempStr.begin(), ::tolower);
	return peopSuffix.find(tempStr) != peopSuffix.end();
}

bool NameEntityDict::isNamePrefix(std::string& str)
{
	std::string tempStr(str);
	std::transform(tempStr.begin(), tempStr.end(), tempStr.begin(), ::tolower);
	return namePrefix.find(tempStr) != namePrefix.end();
}

bool NameEntityDict::isKownLoc(std::string& str)
{
	std::string tempStr(str);
	std::transform(tempStr.begin(), tempStr.end(), tempStr.begin(), ::tolower);
	return locList_.find(tempStr) != locList_.end();
}

bool NameEntityDict::isKownPeop(std::string& str)
{
	return peopList_.find(str) != peopList_.end();
}

bool NameEntityDict::isKownOrg( std::string& str)
{
	std::string tempStr(str);
	std::transform(tempStr.begin(), tempStr.end(), tempStr.begin(), ::tolower);
	return orgList_.find(tempStr) != orgList_.end();
}

bool NameEntityDict::isKownNoise( std::string& str)
{
	std::string tempStr(str);
	std::transform(tempStr.begin(), tempStr.end(), tempStr.begin(), ::tolower);
	return noiseList_.find(tempStr) != noiseList_.end();
}

bool NameEntityDict::isKownOther( std::string& str)
{
	std::string tempStr(str);
    std::transform(tempStr.begin(), tempStr.end(), tempStr.begin(), ::tolower);
	return otherList_.find(tempStr) != otherList_.end();
}

bool NameEntityDict::isNoun( std::string& str)
{
	std::string tempStr(str);
	std::transform(tempStr.begin(), tempStr.end(), tempStr.begin(), ::tolower);
	return nounList_.find(tempStr) != nounList_.end();
}

bool NameEntityDict::isLocLeft(std::string& str)
{
	std::string tempStr(str);
	std::transform(tempStr.begin(), tempStr.end(), tempStr.begin(), ::tolower);
	return locLeft_.find(tempStr) != locLeft_.end();
}

bool NameEntityDict::isLocRight(std::string& str)
{
	std::string tempStr(str);
	std::transform(tempStr.begin(), tempStr.end(), tempStr.begin(), ::tolower);
	return locRight_.find(tempStr) != locRight_.end();
}

bool NameEntityDict::isOrgLeft(std::string& str)
{
	std::string tempStr(str);
	std::transform(tempStr.begin(), tempStr.end(), tempStr.begin(), ::tolower);
	return orgLeft_.find(tempStr) != orgLeft_.end();
}

bool NameEntityDict::isOrgRight( std::string& str)
{
	std::string tempStr(str);
	std::transform(tempStr.begin(), tempStr.end(), tempStr.begin(), ::tolower);
	return orgRight_.find(tempStr) != orgRight_.end();
}

bool NameEntityDict::isPeopLeft( std::string& str)
{
	std::string tempStr(str);
	std::transform(tempStr.begin(), tempStr.end(), tempStr.begin(), ::tolower);
	return peopLeft_.find(tempStr) != peopLeft_.end();
}

bool NameEntityDict::isPeopRight( std::string& str)
{
	std::string tempStr(str);
	std::transform(tempStr.begin(), tempStr.end(), tempStr.begin(), ::tolower);
	return peopRight_.find(tempStr) != peopRight_.end();
}

bool NameEntityDict::isThe(std::string& str)
{
	std::string tempStr(str);
	std::transform(tempStr.begin(), tempStr.end(), tempStr.begin(), ::tolower);
	if(tempStr=="the")
		return true;
	return false;
}

}
