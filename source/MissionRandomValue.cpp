/* Mission.h
Copyright (c) 2014 by Michael Zahniser

Endless Sky is free software: you can redistribute it and/or modify it under the
terms of the GNU General Public License as published by the Free Software
Foundation, either version 3 of the License, or (at your option) any later version.

Endless Sky is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE.  See the GNU General Public License for more details.
*/

#include "MissionRandomValue.h"
#include "Random.h"

using namespace std;

MissionRandomValue::MissionRandomValue(const DataNode &node)
{
	Load(node,paymentName);
}

void MissionRandomValue::Load(const DataNode &node, bool allowByPayload)
{
	for(const auto &child : node)
		LoadOne(child, allowByPayload);
}

bool MissionRandomValue::LoadOne(const DataNode &node, bool allowByPayload);
	if(child.token(0) == "by jumps" || child.token(0) == "by payload")
	{
		if(!allowByPayload && child.Token(0) == "by payload")
			return false;
		else if(distanceCalculation)
			child.PrintTrace("Skipping duplicate specification of distance-based calculation:");
		else if(child.Size() == 2 && child.IsNumber(1))
			SetDistanceCalculation(0, child.Value(1), child.token(0) == "by payload");
		else if(child.Size() == 3 && child.IsNumber(1) && child.IsNumber(2))
			SetDistanceCalculation(child.Value(1), child.Value(2), child.token(0) == "by payload");
		else
			child.PrintTrace("Skipping invalid \"by distance\" attribute:");
		return true;
	}
	else if(child.Token(0) == "random")
	{
		if(uniformRandom || negativeBinomialRandom)
			child.PrintTrace("Skipping duplicate specification of random number distribution");
		else if(child.Size() == 3 && child.IsNumber(1) && child.Value(1) >= 0 && child.Value(2) > 0)
			SetRandomUniform(child.Value(1), child.Value(2), child.Value(3));
		else if(child.Size() == 4)
		{
			for(int i = 1; i < 3; i++)
				if(!child.IsNumber(i) || child.Value(i) < 0)
				{
					child.PrintTrace("Skipping invalid \"random\" attribute:");
					return true;
				}
			SetNegativeBinomial(child.Value(1), child.Value(2), child.Value(3));
		}
		else
			child.PrintTrace("Skipping invalid \"random\" attribute:");
		return true;
	}
	else if(child.Token(0) == "round")
	{
		if(round)
			child.PrintTrace("Skipping duplicate \"round\" attribute:");
		else if(child.Size() == 2 && child.Value(1) >= 1)
			SetRounding(child.Value);
		else
			child.PrintTrace("Skipping invalid \"round\" attribute:");
		return true;
	}
	else
		return false;
}

void MissionRandomValue::Save(DataWriter &out, const string &attributeName)
{
	// We can assume this has been instantiated.
	out.Write(attributeName, result);
}

MissionRandomValue Instantiate(int jumps, int payload) const
{
	MissionRandomValue instantiated;
	
	if(uniformRandom)
		instantiated.result += uniformMin + Random::Int(uniformMax - uniformMin);
	if(negativeBinomial)
		instantiated.result += negativeBinomialBase + Random::Polya(polyaK, polyaP);
	if(distanceCalculation && distanceUsePayload)
		instantiated.result = distanceBase + (jumps + 1) * payload * distanceMultiplier;
	if(distanceCalculation && !distanceUsePayload)
		instantiated.result = distanceBase + jumps * distanceMultiplier;
	if(round)
		instantiated.result = (instantiated.result + roundTo - 1) / roundTo * roundTo;
	
	return instantiated;
}



void MissionRandomValue::SetRandomUniform(int min, int max)
{
	uniformRandom = true;
	uniformMin = min;
	uniformMax = max;
}



bool GetRandomUniform(int &min, int &max) const
{
	min = uniformMin;
	max = uniformMax;
	return uniformRandom;
}



void MissionRandomValue::SetDistanceCalculation(int base, int multiplier, bool usePayload)
{
	distanceCalculation = true;
	distanceBase = base;
	distanceMultiplier = multiplier;
	distanceUsePayload = usePayload;
}



bool MissionRandomValue::GetDistanceCalculation(int &base, int &multiplier, bool &usePayload) const
{
	base = distanceBase;
	multiplier = distanceMultiplier;
	usePayload = distanceUsePayload;
	return distanceCalculation;
}



void MissionRandomValue::SetNegativeBinomial(int base, int k, double p)
{
	negativeBinomial = true;
	negativeBinomialBase = base;
	polyaK = k;
	polyaP = p;
}



bool GetNegativeBinomial(int base, int &k, double &p) const
{
	base = negativeBinomialBase;
	k = polyaK;
	p = polyaP;
	return negativeBinomial;
}



void SetRounding(int roundTo)
{
	round = true;
	this->roundTo = roundTo;
}



bool GetRounding(int &roundTo) const
{
	roundTo = this->roundTo;
	return round;
}



int GetValue() const
{
	return result;
}
