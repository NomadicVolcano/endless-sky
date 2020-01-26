/* PlanetPanel.cpp
Copyright (c) 2014 by Michael Zahniser

Endless Sky is free software: you can redistribute it and/or modify it under the
terms of the GNU General Public License as published by the Free Software
Foundation, either version 3 of the License, or (at your option) any later version.

Endless Sky is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE.  See the GNU General Public License for more details.
*/

#include "PlanetPanel.h"

#include "Information.h"

#include "BankPanel.h"
#include "Command.h"
#include "ConversationPanel.h"
#include "Dialog.h"
#include "FontSet.h"
#include "GameData.h"
#include "HiringPanel.h"
#include "Interface.h"
#include "MapDetailPanel.h"
#include "MissionPanel.h"
#include "OutfitterPanel.h"
#include "Planet.h"
#include "PlayerInfo.h"
#include "PlayerInfoPanel.h"
#include "Ship.h"
#include "ShipyardPanel.h"
#include "SpaceportPanel.h"
#include "System.h"
#include "TradingPanel.h"
#include "UI.h"

#include <sstream>
#include <iostream>

using namespace std;



PlanetPanel::PlanetPanel(PlayerInfo &player, function<void()> callback)
	: player(player), callback(callback),
	planet(*player.GetPlanet()), system(*player.GetSystem()),
	ui(*GameData::Interfaces().Get("planet"))
{
	trading.reset(new TradingPanel(player));
	bank.reset(new BankPanel(player));
	spaceport.reset(new SpaceportPanel(player));
	hiring.reset(new HiringPanel(player));
	
	text.SetFont(FontSet::Get(14));
	text.SetAlignment(WrappedText::JUSTIFIED);
	text.SetWrapWidth(480);
	text.Wrap(planet.Description());
	
	// Since the loading of landscape images is deferred, make sure that the
	// landscapes for this system are loaded before showing the planet panel.
	GameData::Preload(planet.Landscape());
	GameData::FinishLoading();
}



void PlanetPanel::Step()
{
	// If the previous mission callback resulted in a "launch", take off now.
	const Ship *flagship = player.Flagship();
	if(flagship && flagship->CanBeFlagship() && (player.ShouldLaunch() || requestedLaunch))
	{
		TakeOffIfReady();
		return;
	}
	
	// If the player starts a new game, exits the shipyard without buying
	// anything, clicks to the bank, then returns to the shipyard and buys a
	// ship, make sure they are shown an intro mission.
	if(GetUI()->IsTop(this) || GetUI()->IsTop(bank.get()))
	{
		Mission *mission = player.MissionToOffer(Mission::LANDING);
		if(mission)
		{
			cout<<"Do mission "<<mission->Name()<<" in PlanetPanel::Step"<<endl;
			mission->Do(Mission::OFFER, player, GetUI());
		}
		else if(!player.HandleRefreshMissions(GetUI()))
			player.HandleBlockedMissions(Mission::LANDING, GetUI());
	}
}



void PlanetPanel::Draw()
{
	if(player.IsDead())
		return;
	
	Information info;
	info.SetSprite("land", planet.Landscape());
	
	const Ship *flagship = player.Flagship();
	if(flagship && flagship->CanBeFlagship())
		info.SetCondition("has ship");
	
	if(planet.CanUseServices())
	{
		if(planet.IsInhabited())
		{
			info.SetCondition("has bank");
			if(flagship)
			{
				info.SetCondition("is inhabited");
				if(system.HasTrade())
					info.SetCondition("has trade");
			}
		}
		
		if(flagship && planet.HasSpaceport())
			info.SetCondition("has spaceport");
		
		if(planet.HasShipyard())
			info.SetCondition("has shipyard");
		
		if(planet.HasOutfitter())
			for(const auto &it : player.Ships())
				if(it->GetSystem() == &system && !it->IsDisabled())
				{
					info.SetCondition("has outfitter");
					break;
				}
	}
	
	ui.Draw(info, this);
	
	if(!selectedPanel)
		text.Draw(Point(-300., 80.), *GameData::Colors().Get("bright"));
}



// Only override the ones you need; the default action is to return false.
bool PlanetPanel::KeyDown(SDL_Keycode key, Uint16 mod, const Command &command, bool isNewPress)
{
	Panel *oldPanel = selectedPanel;
	const Ship *flagship = player.Flagship();
	
	bool hasAccess = planet.CanUseServices();
	if(key == 'd' && flagship && flagship->CanBeFlagship())
		requestedLaunch = true;
	else if(key == 'l')
	{
		selectedPanel = nullptr;
	}
	else if(key == 't' && hasAccess && flagship && planet.IsInhabited() && system.HasTrade())
	{
		selectedPanel = trading.get();
		GetUI()->Push(trading);
	}
	else if(key == 'b' && hasAccess && planet.IsInhabited())
	{
		selectedPanel = bank.get();
		GetUI()->Push(bank);
	}
	else if(key == 'p' && hasAccess && flagship && planet.HasSpaceport())
	{
		selectedPanel = spaceport.get();
		if(isNewPress)
			spaceport->UpdateNews();
		GetUI()->Push(spaceport);
	}
	else if(key == 's' && hasAccess && planet.HasShipyard())
	{
		GetUI()->Push(new ShipyardPanel(player));
		return true;
	}
	else if(key == 'o' && hasAccess && planet.HasOutfitter())
	{
		for(const auto &it : player.Ships())
			if(it->GetSystem() == &system && !it->IsDisabled())
			{
				GetUI()->Push(new OutfitterPanel(player));
				return true;
			}
	}
	else if(key == 'j' && hasAccess && flagship && planet.IsInhabited())
	{
		GetUI()->Push(new MissionPanel(player));
		return true;
	}
	else if(key == 'h' && hasAccess && flagship && planet.IsInhabited())
	{
		selectedPanel = hiring.get();
		GetUI()->Push(hiring);
	}
	else if(command.Has(Command::MAP))
	{
		GetUI()->Push(new MapDetailPanel(player));
		return true;
	}
	else if(command.Has(Command::INFO))
	{
		GetUI()->Push(new PlayerInfoPanel(player));
		return true;
	}
	else
		return false;
	
	// If we are here, it is because something happened to change the selected
	// planet UI panel. So, we need to pop the old selected panel:
	if(oldPanel)
		GetUI()->Pop(oldPanel);
	
	return true;
}



void PlanetPanel::TakeOffIfReady()
{
	// If we're currently showing a conversation or dialog, wait for it to close.
	if(!GetUI()->IsTop(this) && !GetUI()->IsTop(trading.get()) && !GetUI()->IsTop(bank.get())
			&& !GetUI()->IsTop(spaceport.get()) && !GetUI()->IsTop(hiring.get()))
		return;
	
	// If something happens here that cancels the order to take off, don't try
	// to take off until the button is clicked again.
	requestedLaunch = false;
	
	// Check for any landing missions that have not been offered.
	Mission *mission = player.MissionToOffer(Mission::LANDING);
	if(mission)
	{
		cout<<"Do mission "<<mission->Name()<<" in PlanetPanel::TakeOffIfReady";
		mission->Do(Mission::OFFER, player, GetUI());
		return;
	}
	
	// Update the completion and failure status of missions with the refresh flag set
	if(player.HandleRefreshMissions(GetUI()))
		return;

	// Check whether the player should be warned before taking off.
	if(player.ShouldLaunch())
	{
		TakeOff();
		return;
	}
	
	// Check if any of the player's ships are configured in such a way that they
	// will be impossible to fly.
	for(const shared_ptr<Ship> &ship : player.Ships())
	{
		if(ship->GetSystem() != &system || ship->IsDisabled() || ship->IsParked())
			continue;
		
		string check = ship->FlightCheck();
		if(!check.empty() && check.back() == '!')
		{
			GetUI()->Push(new ConversationPanel(player,
				*GameData::Conversations().Get("flight check: " + check), nullptr, ship));
			return;
		}
	}
	
	// The checks that follow are typically caused by parking or selling
	// ships or changing outfits.
	const Ship *flagship = player.Flagship();
	
	// Are you overbooked? Don't count fireable flagship crew. If your
	// ship can't hold the required crew, count it as having no fireable
	// crew rather than a negative number.
	const CargoHold &cargo = player.Cargo();
	int overbooked = -cargo.BunksFree() - max(0, flagship->Crew() - flagship->RequiredCrew());
	int missionCargoToSell = cargo.MissionCargoSize() - cargo.Size();
	// Will you have to sell something other than regular cargo?
	int cargoToSell = -(cargo.Free() + cargo.CommoditiesSize());
	int droneCount = 0;
	int fighterCount = 0;
	for(const auto &it : player.Ships())
		if(!it->IsParked() && !it->IsDisabled() && it->GetSystem() == &system)
		{
			const string &category = it->Attributes().Category();
			droneCount += (category == "Drone") - it->BaysFree(false);
			fighterCount += (category == "Fighter") - it->BaysFree(true);
		}
	
	if(fighterCount > 0 || droneCount > 0 || cargoToSell > 0 || overbooked > 0)
	{
		ostringstream out;
		if(missionCargoToSell > 0 || overbooked > 0)
		{
			bool both = ((cargoToSell > 0 && cargo.MissionCargoSize()) && overbooked > 0);
			out << "If you take off now you will fail a mission due to not having enough ";

			if(overbooked > 0)
			{
				out << "bunks available for " << overbooked;
				out << (overbooked > 1 ? " of the passengers" : " passenger");
				out << (both ? " and not having enough " : ".");
			}

			if(missionCargoToSell > 0)
			{
				out << "cargo space to hold " << missionCargoToSell;
				out << (missionCargoToSell > 1 ? " tons" : " ton");
				out << " of your mission cargo.";
			}
		}
		else
		{
			out << "If you take off now you will have to sell ";
			bool triple = (fighterCount > 0 && droneCount > 0 && cargoToSell > 0);

			if(fighterCount == 1)
				out << "a fighter";
			else if(fighterCount > 0)
				out << fighterCount << " fighters";
			if(fighterCount > 0 && (droneCount > 0 || cargoToSell > 0))
				out << (triple ? ", " : " and ");
		
			if(droneCount == 1)
				out << "a drone";
			else if(droneCount > 0)
				out << droneCount << " drones";
			if(droneCount > 0 && cargoToSell > 0)
				out << (triple ? ", and " : " and ");

			if(cargoToSell == 1)
				out << "a ton of cargo";
			else if(cargoToSell > 0)
				out << cargoToSell << " tons of cargo";
			out << " that you do not have space for.";
		}
		out << " Are you sure you want to continue?";
		GetUI()->Push(new Dialog(this, &PlanetPanel::TakeOff, out.str()));
		return;
	}
	
	// There was no need to ask the player whether we can get rid of anything,
	// so go ahead and take off.
	TakeOff();
}



void PlanetPanel::TakeOff()
{
	player.Save();
	if(player.TakeOff(GetUI()))
	{
		if(callback)
			callback();
		if(selectedPanel)
			GetUI()->Pop(selectedPanel);
		GetUI()->Pop(this);
	}
}
