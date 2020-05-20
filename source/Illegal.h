/* Illegal.h
Copyright (c) 2020 by Nomadic Volcano
Based on earlier work copyright (c) 2014 by Michael Zahniser

Endless Sky is free software: you can redistribute it and/or modify it under the
terms of the GNU General Public License as published by the Free Software
Foundation, either version 3 of the License, or (at your option) any later version.

Endless Sky is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE.  See the GNU General Public License for more details.
*/

#ifndef ILLEGAL_H_
#define ILLEGAL_H_

#include <string>
#include <set>

class DataNode;

class Illegal {
public:
	Illegal() = default;
	// Construct and Load() at the same time.
	Illegal(const DataNode &node);
	
	// Save or load an "illegal" attribute.
	void Load(const DataNode &node);
	void Save(DataWriter &out) const;
	
	bool IsIllegalFor(const std::string &governmentName) const;
	
	
private:
	int illegalCargoFine = 0;
	bool parsedIllegalAttribute = true;
	std::string illegalCargoMessage;
	
	// Restrict which governments consider the mission illegal.
	std::set<std::string> illegalFor;
	std::set<std::string> legalFor;
};

#endif
