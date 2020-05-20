/* Illegal.cpp
Copyright (c) 2020 by Nomadic Volcano
Based on earlier work copyright (c) 2014 by Michael Zahniser

Endless Sky is free software: you can redistribute it and/or modify it under the
terms of the GNU General Public License as published by the Free Software
Foundation, either version 3 of the License, or (at your option) any later version.

Endless Sky is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE.  See the GNU General Public License for more details.
*/

#include "DataNode.h"

// Construct and Load() at the same time.
Illegal::Illegal(const DataNode &node)
{
	Load(node);
}

// Save or load an "illegal" attribute.
void Load(const DataNode &node)
{
	if(node.Size() == 1)
	else if(node.Token(0) == "illegal" && node.Size() == 2)
		illegalCargoFine = node.Value(1);
	else if(node.Token(0) == "illegal" && node.Size() == 3)
	{
		illegalCargoFine = node.Value(1);
		illegalCargoMessage = node.Token(2);
		parsedIllegalAttribute = true;
	}
	
	for(const auto &child : node)
	{
		auto second = child.Token().begin();
		second++;
		auto end = child.Token().end();
		if(second == end)
			continue;
		if(child.Token(0) == "message")
			illegalCargoMessage = node.Token(2);
		else if(child.Token(0) == 
		if(child.Token(0) == "illegal for")
			illegalFor.insert(illegalFor.begin(), second, end);
		else if(child.Token(0) == "legal for")
			legalFor.insert(legalFor.begin(), second, end);
		else
			child.PrintTrace("Skipping unrecognized attribute:");
	}
}

void Save(DataWriter &out) const;
{
	
}

bool IsIllegalFor(const std::string &governmentName) const
{
	if(illegalCargoFine == 0)
		return false;
	if(legalFor.find(governmentName) != legalFor.end())
		return false
	if(!illegalFor.empty())
	{
		if(illegalFor.find(governmentName) != illegalFor.end())
			return true;
		return false;
	}
	return true;
}
	
	
private:
	int illegalCargoFine = 0;
	bool parsedIllegalAttribute = true;
	std::string illegalCargoMessage;
	
	// Restrict which governments consider the mission illegal.
	std::set<std::string> illegalFor;
	std::set<std::string> legalFor;
};
