/* Mission.h
Copyright (c) 2014 by Michael Zahniser

Endless Sky is free software: you can redistribute it and/or modify it under the
terms of the GNU General Public License as published by the Free Software
Foundation, either version 3 of the License, or (at your option) any later version.

Endless Sky is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE.  See the GNU General Public License for more details.
*/

#ifndef MISSION_RANDOM_VALUE_H_
#define MISSION_RANDOM_VALUE_H_

#include <tuple>
#include <pair>
#include <string>

class DataNode;
class DatWriter;

class MissionRandomValue {
public:
	MissionRandomValue() = default;
	MissionRandomValue(const DataNode &node);
	void Load(const DataNode &node, bool allowByPayload);
	void LoadOne(const DataNode &node, bool allowByPayload);
	void Save(DataWriter &out);
	MissionRandomValue Instantiate(int jumps, int payload) const;
	
	void SetRandomUniform(int min, int max);
	bool GetRandomUniform(int &min, int &max) const;
	
	void SetDistanceCalculation(int base, int multiplier, bool usePayload);
	bool GetDistanceCalculation(int &base, int &multiplier, bool &usePayload) const;
	void SetUsePayload(bool use);
	
	void SetNegativeBinomial(int base, int k, double p);
	bool GetNegativeBinomial(int base, int &k, double &p) const;
	
	void SetRounding(int roundTo);
	bool GetRounding(int &roundTo) const;
	
	int GetValue() const;
	
	
private:
	bool haveResult = false;
	int result = 0;
	
	bool round = false;
	int roundTo = 0;
	
	bool uniformRandom = false;
	int uniformMin = 0;
	int uniformMax = 0;
	
	bool distanceCalculation = false;
	int distanceBase = 0;
	int distanceMultiplier = 0;
	bool distanceUsePayload = false;
	
	bool negativeBinomial = false;
	int negativeBinomialBase = 0;
	int polyaK = 0;
	double polyaP = 0;
};

#endif
