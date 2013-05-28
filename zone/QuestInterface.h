/*  EQEMu:  Everquest Server Emulator
	Copyright (C) 2001-2006  EQEMu Development Team (http://eqemulator.net)

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; version 2 of the License.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY except by those people which sell it, which
	are required to give you total support for your newly bought product;
	without even the implied warranty of MERCHANTABILITY or FITNESS FOR
	A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#ifndef _EQE_QUESTINTERFACE_H
#define _EQE_QUESTINTERFACE_H

#include "../common/types.h"
#include "event_codes.h"

class ItemInst;
class Client;
class NPC;

class QuestInterface {
public:
	virtual int EventNPC(QuestEventID evt, NPC* npc, Mob *init, std::string data, uint32 extra_data) { return 0; }
	virtual int EventGlobalNPC(QuestEventID evt, NPC* npc, Mob *init, std::string data, uint32 extra_data) { return 0; }
	virtual int EventPlayer(QuestEventID evt, Client *client, std::string data, uint32 extra_data) { return 0; }
	virtual int EventGlobalPlayer(QuestEventID evt, Client *client, std::string data, uint32 extra_data) { return 0; }
	virtual int EventItem(QuestEventID evt, Client *client, ItemInst *item, uint32 objid, uint32 extra_data) { return 0; }
	virtual int EventSpell(QuestEventID evt, NPC* npc, Client *client, uint32 spell_id, uint32 extra_data) { return 0; }
	virtual int EventEncounter(QuestEventID evt, std::string encounter_name, uint32 extra_data) { return 0; }
	
	virtual bool HasQuestSub(uint32 npcid, const char *subname) { return false; }
	virtual bool HasGlobalQuestSub(const char *subname) { return false; }
	virtual bool PlayerHasQuestSub(const char *subname) { return false; }
	virtual bool GlobalPlayerHasQuestSub(const char *subname) { return false; }
	virtual bool SpellHasQuestSub(uint32 spell_id, const char *subname) { return false; }
	virtual bool ItemHasQuestSub(ItemInst *itm, const char *subname) { return false; }

	virtual void LoadNPCScript(std::string filename, int npc_id) { }
	virtual void LoadGlobalNPCScript(std::string filename) { }
	virtual void LoadPlayerScript(std::string filename) { }
	virtual void LoadGlobalPlayerScript(std::string filename) { }
	virtual void LoadItemScript(std::string filename, std::string item_script) { }
	virtual void LoadSpellScript(std::string filename, uint32 spell_id) { }
	virtual void LoadEncounterScript(std::string filename, std::string encounter_name) { }

	virtual void DispatchEventNPC(QuestEventID evt, NPC* npc, Mob *init, std::string data, uint32 extra_data) { }
	virtual void DispatchEventPlayer(QuestEventID evt, Client *client, std::string data, uint32 extra_data) { }
	virtual void DispatchEventItem(QuestEventID evt, Client *client, ItemInst *item, uint32 objid, uint32 extra_data) { }
	virtual void DispatchEventSpell(QuestEventID evt, NPC* npc, Client *client, uint32 spell_id, uint32 extra_data) { }
	
	virtual void AddVar(std::string name, std::string val) { }
	virtual std::string GetVar(std::string name) { return std::string(); }
	virtual void Init() { }
	virtual void ReloadQuests() { }
	virtual uint32 GetIdentifier() = 0;
	
	//TODO: Set maximum quest errors instead of hard coding it
	virtual void GetErrors(std::list<std::string> &err) {
		err.insert(err.end(), errors_.begin(), errors_.end());
	}

	virtual void AddError(std::string error) { 
		errors_.push_back(error);
		if(errors_.size() > 30) {
			errors_.pop_front();
		}
	}
	
protected:
	std::list<std::string> errors_;
};

#endif

