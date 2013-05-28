#ifdef LUA_EQEMU

#include "lua.hpp"
#include <luabind/luabind.hpp>
#include <luabind/object.hpp>

#include <sstream>
#include <list>
#include <map>

#include "lua_parser.h"
#include "lua_item.h"
#include "lua_iteminst.h"
#include "lua_mob.h"
#include "QuestParserCollection.h"
#include "questmgr.h"

struct Events { };

struct lua_registered_event {
	std::string encounter_name;
	luabind::object lua_reference;
	QuestEventID event_id;
};

extern std::map<std::string, std::list<lua_registered_event>> lua_encounter_events_registered;

void load_encounter(std::string name) {
	parse->EventEncounter(EVENT_ENCOUNTER_LOAD, name, 0);
}

void unload_encounter(std::string name) {
	parse->EventEncounter(EVENT_ENCOUNTER_UNLOAD, name, 0);
}

void register_event(std::string package_name, std::string name, int evt, luabind::object func) {
	lua_registered_event e;
	e.encounter_name = name;
	e.lua_reference = func;
	e.event_id = static_cast<QuestEventID>(evt);
	
	auto liter = lua_encounter_events_registered.find(package_name);
	if(liter == lua_encounter_events_registered.end()) {
		std::list<lua_registered_event> elist;
		elist.push_back(e);
		lua_encounter_events_registered[package_name] = elist;
	} else {
		std::list<lua_registered_event> elist = liter->second;
		auto iter = elist.begin();
		while(iter != elist.end()) {
			if(iter->event_id == evt && iter->encounter_name.compare(name) == 0) {
				//already registered this event for this encounter
				return;
			}
			++iter;
		}

		elist.push_back(e);
		lua_encounter_events_registered[package_name] = elist;
	}
}

void unregister_event(std::string package_name, std::string name, int evt) {
	auto liter = lua_encounter_events_registered.find(package_name);
	if(liter != lua_encounter_events_registered.end()) {
		std::list<lua_registered_event> elist = liter->second;
		auto iter = elist.begin();
		while(iter != elist.end()) {
			if(iter->event_id == evt && iter->encounter_name.compare(name) == 0) {
				iter = elist.erase(iter);
			}
		}
		lua_encounter_events_registered[package_name] = elist;
	}
}

void register_npc_event(std::string name, int evt, int npc_id, luabind::object func) {
	if(luabind::type(func) == LUA_TFUNCTION) {
		std::stringstream package_name;
		package_name << "npc_" << npc_id;

		register_event(package_name.str(), name, evt, func);
	}
}

void unregister_npc_event(std::string name, int evt, int npc_id) {
	std::stringstream package_name;
	package_name << "npc_" << npc_id;

	unregister_event(package_name.str(), name, evt);
}

void register_player_event(std::string name, int evt, luabind::object func) {
	if(luabind::type(func) == LUA_TFUNCTION) {
		register_event("player", name, evt, func);
	}
}

void unregister_player_event(std::string name, int evt) {
	unregister_event("player", name, evt);
}

void register_item_event(std::string name, int evt, Lua_Item item, luabind::object func) {
	const Item_Struct *itm = item;
	if(!itm) {
		return;
	}

	std::stringstream package_name;
	package_name << "item_";
	
	std::stringstream item_name;
	if(EVENT_SCALE_CALC == evt || EVENT_ITEM_ENTER_ZONE == evt)
	{
		item_name << itm->CharmFile;
	}
	else if(EVENT_ITEM_CLICK == evt || EVENT_ITEM_CLICK_CAST == evt)
	{
		item_name << "script_";
		item_name << itm->ScriptFileID;
	}
	else
	{
		item_name << "item_";
		item_name << itm->ID;
	}
	
	package_name << item_name;

	if(luabind::type(func) == LUA_TFUNCTION) {
		register_event(package_name.str(), name, evt, func);
	}
}

void unregister_item_event(std::string name, int evt, Lua_Item item) {
	const Item_Struct *itm = item;
	if(!itm) {
		return;
	}

	std::stringstream package_name;
	package_name << "item_";
	
	std::stringstream item_name;
	if(EVENT_SCALE_CALC == evt || EVENT_ITEM_ENTER_ZONE == evt)
	{
		item_name << itm->CharmFile;
	}
	else if(EVENT_ITEM_CLICK == evt || EVENT_ITEM_CLICK_CAST == evt)
	{
		item_name << "script_";
		item_name << itm->ScriptFileID;
	}
	else
	{
		item_name << "item_";
		item_name << itm->ID;
	}
	
	package_name << item_name;
	unregister_event(package_name.str(), name, evt);
}

void register_spell_event(std::string name, int evt, int spell_id, luabind::object func) {
	if(luabind::type(func) == LUA_TFUNCTION) {
		std::stringstream package_name;
		package_name << "spell_" << spell_id;

		register_event(package_name.str(), name, evt, func);
	}
}

void unregister_spell_event(std::string name, int evt, int spell_id) {
	std::stringstream package_name;
	package_name << "spell_" << spell_id;

	unregister_event(package_name.str(), name, evt);
}

void lua_say(const char *str) {
	quest_manager.say(str);
}

void lua_say(const char *str, int language) {
	quest_manager.say(str, language);
}

void lua_me(const char *str) {
	quest_manager.me(str);
}

void lua_summon_item(uint32 itemid, int charges = 0) {
	quest_manager.summonitem(itemid, charges);
}

Lua_Mob lua_spawn2(int npc_type, int grid, int unused, double x, double y, double z, double heading) {
	return Lua_Mob(quest_manager.spawn2(npc_type, grid, unused,
		static_cast<float>(x), static_cast<float>(y), static_cast<float>(z), static_cast<float>(heading)));
}

Lua_Mob lua_unique_spawn(int npc_type, int grid, int unused, double x, double y, double z, double heading = 0.0) {
	return Lua_Mob(quest_manager.unique_spawn(npc_type, grid, unused,
		static_cast<float>(x), static_cast<float>(y), static_cast<float>(z), static_cast<float>(heading)));
}

Lua_Mob lua_spawn_from_spawn2(uint32 spawn2_id) {
	return Lua_Mob(quest_manager.spawn_from_spawn2(spawn2_id));
}

void lua_enable_spawn2() {

}

void lua_disable_spawn2() {

}

void lua_cast_spell() {

}

void lua_self_cast() {

}

void lua_set_timer() {

}

void lua_stop_timer() {

}

void lua_stop_all_timers() {

}

void lua_emote() {

}

void lua_shout() {

}

void lua_gmsay() {

}

void lua_depop() {

}

void lua_depop_with_timer() {

}

void lua_follow() {

}

void lua_stop_follow() {

}

void lua_change_deity() {

}

void lua_is_disc_tome() {

}

void lua_safe_move() {

}

void lua_rain() {

}

void lua_snow() {

}

void lua_surname() {

}

void lua_perma_class() {

}

void lua_perma_race() {

}

void lua_perma_gender() {

}

void lua_scribe_spells() {

}

void lua_train_discs() {

}

void lua_give_cash() {

}

void lua_move_group() {

}

void lua_faction() {

}

void lua_set_sky() {

}

void lua_set_guild() {

}

void lua_create_guild() {

}

void lua_set_time() {

}

void lua_signal() {

}

void lua_set_global() {

}

void lua_target_global() {

}

void lua_delete_global() {

}

void lua_ding() {

}

void lua_bind() {

}

void lua_start() {

}

void lua_stop() {

}

void lua_pause() {

}

void lua_move_to() {

}

void lua_path_resume() {

}

void lua_set_next_hp_event() {

}

void lua_set_next_inc_hp_event() {

}

void lua_respawn() {

}

void lua_choose_random() {

}

void lua_set_proximity() {

}

void lua_clear_proximity() {

}

void lua_enable_proximity_say() {

}

void lua_disable_proximity_say() {

}

void lua_set_anim() {

}

void lua_spawn_condition() {

}

void lua_get_spawn_condition() {

}

void lua_toggle_spawn_event() {

}

void lua_has_zone_flag() {

}

void lua_set_zone_flag() {

}

void lua_clear_zone_flag() {

}

void lua_summon_burried_player_corpse() {

}

void lua_summon_all_player_corpses() {

}

void lua_get_player_burried_corpse_count() {

}

void lua_bury_player_corpse() {

}

void lua_depop_all() {

}

void lua_depop_zone() {

}

void lua_repop_zone() {

}

void lua_task_selector() {

}

void lua_task_set_selector() {

}

void lua_enable_task() {

}

void lua_disable_task() {

}

void lua_is_task_enabled() {

}

void lua_is_task_active() {

}

void lua_is_task_activity_active() {

}

void lua_get_task_activity_done_count() {

}

void lua_update_task_activity() {

}

void lua_reset_task_activity() {

}

void lua_task_explored_area() {

}

void lua_assign_task() {

}

void lua_fail_task() {

}

void lua_task_time_left() {

}

void lua_is_task_completed() {

}

void lua_enabled_task_count() {

}

void lua_first_task_in_set() {

}

void lua_last_task_in_set() {

}

void lua_next_task_in_set() {

}

void lua_active_speak_task() {

}

void lua_active_speak_activity() {

}

void lua_active_tasks_in_set() {

}

void lua_completed_tasks_in_set() {

}

void lua_is_task_appropriate() {

}

void lua_popup() {

}

void lua_clear_spawn_timers() {

}

void lua_zone_emote() {

}

void lua_world_emote() {

}

void lua_get_level() {

}

void lua_create_ground_object() {

}

void lua_create_ground_object_from_model() {

}

void lua_create_door() {

}

void lua_modify_npc_stat() {

}

void lua_count_items() {

}

void lua_update_spawn_timer() {

}

void lua_merchant_set_item() {

}

void lua_merchant_count_item() {

}

void lua_item_link() {

}

void lua_say_link() {

}

void lua_get_guild_name_by_id() {

}

void lua_create_instance() {

}

void lua_destroy_instance() {

}

void lua_get_instance_id() {

}

void lua_assign_to_instance() {

}

void lua_assign_group_to_instance() {

}

void lua_assign_raid_to_instance() {

}

void lua_flag_instance_by_group_leader() {

}

void lua_flag_instance_by_raid_leader() {

}

void lua_fly_mode() {

}

void lua_faction_value() {

}

void lua_check_title() {

}

void lua_enable_title() {

}

void lua_remove_title() {

}

void lua_wear_change() {

}

void lua_voice_tell() {

}

void lua_send_mail() {

}

void lua_cross_zone_signal_client_by_char_id() {

}

void lua_cross_zone_signal_client_by_name() {

}

void lua_cross_zone_message_player_by_name() {

}


luabind::scope lua_register_general() {
	return luabind::namespace_("eq")
	[
		luabind::def("load_encounter", &load_encounter),
		luabind::def("unload_encounter", &unload_encounter),
		luabind::def("register_npc_event", &register_npc_event),
		luabind::def("unregister_npc_event", &unregister_npc_event),
		luabind::def("register_player_event", &register_player_event),
		luabind::def("unregister_player_event", &unregister_player_event),
		luabind::def("register_item_event", &register_item_event),
		luabind::def("unregister_item_event", &unregister_item_event),
		luabind::def("register_spell_event", &register_spell_event),
		luabind::def("unregister_spell_event", &unregister_spell_event),
		luabind::def("say", (void(*)(const char*))&lua_say),
		luabind::def("say", (void(*)(const char*, int))&lua_say),
		luabind::def("me", (void(*)(const char*))&lua_me),
		luabind::def("summon_item", (void(*)(uint32))&lua_summon_item),
		luabind::def("summon_item", (void(*)(uint32,int))&lua_summon_item),
		luabind::def("spawn2", (Lua_Mob(*)(int,int,int,double,double,double,double))&lua_spawn2),
		luabind::def("unique_spawn", (Lua_Mob(*)(int,int,int,double,double,double))&lua_unique_spawn),
		luabind::def("unique_spawn", (Lua_Mob(*)(int,int,int,double,double,double,double))&lua_unique_spawn),
		luabind::def("spawn_from_spawn2", (Lua_Mob(*)(uint32))&lua_spawn_from_spawn2),
		luabind::def("enable_spawn2", &lua_enable_spawn2),
		luabind::def("disable_spawn2", &lua_disable_spawn2),
		luabind::def("cast_spell", &lua_cast_spell),
		luabind::def("self_cast", &lua_self_cast),
		luabind::def("set_timer", &lua_set_timer),
		luabind::def("stop_timer", &lua_stop_timer),
		luabind::def("stop_all_timers", &lua_stop_all_timers),
		luabind::def("emote", &lua_emote),
		luabind::def("shout", &lua_shout),
		luabind::def("gmsay", &lua_gmsay),
		luabind::def("depop", &lua_depop),
		luabind::def("depop_with_timer", &lua_depop_with_timer),
		luabind::def("follow", &lua_follow),
		luabind::def("stop_follow", &lua_stop_follow),
		luabind::def("change_deity", &lua_change_deity),
		luabind::def("is_disc_tome", &lua_is_disc_tome),
		luabind::def("safe_move", &lua_safe_move),
		luabind::def("rain", &lua_rain),
		luabind::def("snow", &lua_snow),
		luabind::def("surname", &lua_surname),
		luabind::def("perma_class", &lua_perma_class),
		luabind::def("perma_race", &lua_perma_race),
		luabind::def("perma_gender", &lua_perma_gender),
		luabind::def("scribe_spells", &lua_scribe_spells),
		luabind::def("train_discs", &lua_train_discs),
		luabind::def("give_cash", &lua_give_cash),
		luabind::def("move_group", &lua_move_group),
		luabind::def("faction", &lua_faction),
		luabind::def("set_sky", &lua_set_sky),
		luabind::def("set_guild", &lua_set_guild),
		luabind::def("create_guild", &lua_create_guild),
		luabind::def("set_time", &lua_set_time),
		luabind::def("signal", &lua_signal),
		luabind::def("set_global", &lua_set_global),
		luabind::def("target_global", &lua_target_global),
		luabind::def("delete_global", &lua_delete_global),
		luabind::def("ding", &lua_ding),
		luabind::def("bind", &lua_bind),
		luabind::def("start", &lua_start),
		luabind::def("stop", &lua_stop),
		luabind::def("pause", &lua_pause),
		luabind::def("move_to", &lua_move_to),
		luabind::def("resume", &lua_path_resume),
		luabind::def("set_next_hp_event", &lua_set_next_hp_event),
		luabind::def("set_next_inc_hp_event", &lua_set_next_inc_hp_event),
		luabind::def("respawn", &lua_respawn),
		luabind::def("choose_random", &lua_choose_random),
		luabind::def("set_proximity", &lua_set_proximity),
		luabind::def("clear_proximity", &lua_clear_proximity),
		luabind::def("enable_proximity_say", &lua_enable_proximity_say),
		luabind::def("disable_proximity_say", &lua_disable_proximity_say),
		luabind::def("set_anim", &lua_set_anim),
		luabind::def("spawn_condition", &lua_spawn_condition),
		luabind::def("get_spawn_condition", &lua_get_spawn_condition),
		luabind::def("toggle_spawn_event", &lua_toggle_spawn_event),
		luabind::def("has_zone_flag", &lua_has_zone_flag),
		luabind::def("set_zone_flag", &lua_set_zone_flag),
		luabind::def("clear_zone_flag", &lua_clear_zone_flag),
		luabind::def("summon_burried_player_corpse", &lua_summon_burried_player_corpse),
		luabind::def("summon_all_player_corpses", &lua_summon_all_player_corpses),
		luabind::def("get_player_burried_corpse_count", &lua_get_player_burried_corpse_count),
		luabind::def("bury_player_corpse", &lua_bury_player_corpse),
		luabind::def("depop_all", &lua_depop_all),
		luabind::def("depop_zone", &lua_depop_zone),
		luabind::def("repop_zone", &lua_repop_zone),
		luabind::def("task_selector", &lua_task_selector),
		luabind::def("task_set_selector", &lua_task_set_selector),
		luabind::def("enable_task", &lua_enable_task),
		luabind::def("disable_task", &lua_disable_task),
		luabind::def("is_task_enabled", &lua_is_task_enabled),
		luabind::def("is_task_active", &lua_is_task_active),
		luabind::def("is_task_activity_active", &lua_is_task_activity_active),
		luabind::def("get_task_activity_done_count", &lua_get_task_activity_done_count),
		luabind::def("update_task_activity", &lua_update_task_activity),
		luabind::def("reset_task_activity", &lua_reset_task_activity),
		luabind::def("task_explored_area", &lua_task_explored_area),
		luabind::def("assign_task", &lua_assign_task),
		luabind::def("fail_task", &lua_fail_task),
		luabind::def("task_time_left", &lua_task_time_left),
		luabind::def("is_task_completed", &lua_is_task_completed),
		luabind::def("enabled_task_count", &lua_enabled_task_count),
		luabind::def("first_task_in_set", &lua_first_task_in_set),
		luabind::def("last_task_in_set", &lua_last_task_in_set),
		luabind::def("next_task_in_set", &lua_next_task_in_set),
		luabind::def("active_speak_task", &lua_active_speak_task),
		luabind::def("active_speak_activity", &lua_active_speak_activity),
		luabind::def("active_tasks_in_set", &lua_active_tasks_in_set),
		luabind::def("completed_tasks_in_set", &lua_completed_tasks_in_set),
		luabind::def("is_task_appropriate", &lua_is_task_appropriate),
		luabind::def("popup", &lua_popup),
		luabind::def("clear_spawn_timers", &lua_clear_spawn_timers),
		luabind::def("zone_emote", &lua_zone_emote),
		luabind::def("world_emote", &lua_world_emote),
		luabind::def("get_level", &lua_get_level),
		luabind::def("create_ground_object", &lua_create_ground_object),
		luabind::def("create_ground_object_from_model", &lua_create_ground_object_from_model),
		luabind::def("create_door", &lua_create_door),
		luabind::def("modify_npc_stat", &lua_modify_npc_stat),
		luabind::def("count_items", &lua_count_items),
		luabind::def("update_spawn_timer", &lua_update_spawn_timer),
		luabind::def("merchant_set_item", &lua_merchant_set_item),
		luabind::def("merchant_count_item", &lua_merchant_count_item),
		luabind::def("item_link", &lua_item_link),
		luabind::def("say_link", &lua_say_link),
		luabind::def("get_guild_name_by_id", &lua_get_guild_name_by_id),
		luabind::def("create_instance", &lua_create_instance),
		luabind::def("destroy_instance", &lua_destroy_instance),
		luabind::def("get_instance_id", &lua_get_instance_id),
		luabind::def("assign_to_instance", &lua_assign_to_instance),
		luabind::def("assign_group_to_instance", &lua_assign_group_to_instance),
		luabind::def("assign_raid_to_instance", &lua_assign_raid_to_instance),
		luabind::def("flag_instance_by_group_leader", &lua_flag_instance_by_group_leader),
		luabind::def("flag_instance_by_raid_leader", &lua_flag_instance_by_raid_leader),
		luabind::def("fly_mode", &lua_fly_mode),
		luabind::def("faction_value", &lua_faction_value),
		luabind::def("check_title", &lua_check_title),
		luabind::def("enable_title", &lua_enable_title),
		luabind::def("remove_title", &lua_remove_title),
		luabind::def("wear_change", &lua_wear_change),
		luabind::def("voice_tell", &lua_voice_tell),
		luabind::def("send_mail", &lua_send_mail),
		luabind::def("cross_zone_signal_client_by_char_id", &lua_cross_zone_signal_client_by_char_id),
		luabind::def("cross_zone_signal_client_by_name", &lua_cross_zone_signal_client_by_name),
		luabind::def("cross_zone_message_player_by_name", &lua_cross_zone_message_player_by_name)
	];
}

luabind::scope lua_register_events() {
	return luabind::class_<Events>("Event")
		.enum_("constants")
		[
			luabind::value("say", static_cast<int>(EVENT_SAY)),
			luabind::value("trade", static_cast<int>(EVENT_TRADE)),
			luabind::value("death", static_cast<int>(EVENT_DEATH)),
			luabind::value("spawn", static_cast<int>(EVENT_SPAWN)),
			luabind::value("attack", static_cast<int>(EVENT_ATTACK)),
			luabind::value("combat", static_cast<int>(EVENT_COMBAT)),
			luabind::value("aggro", static_cast<int>(EVENT_AGGRO)),
			luabind::value("slay", static_cast<int>(EVENT_SLAY)),
			luabind::value("npc_slay", static_cast<int>(EVENT_NPC_SLAY)),
			luabind::value("waypoint_arrive", static_cast<int>(EVENT_WAYPOINT_ARRIVE)),
			luabind::value("waypoint_depart", static_cast<int>(EVENT_WAYPOINT_DEPART)),
			luabind::value("timer", static_cast<int>(EVENT_TIMER)),
			luabind::value("signal", static_cast<int>(EVENT_SIGNAL)),
			luabind::value("hp", static_cast<int>(EVENT_HP)),
			luabind::value("enter", static_cast<int>(EVENT_ENTER)),
			luabind::value("exit", static_cast<int>(EVENT_EXIT)),
			luabind::value("enter_zone", static_cast<int>(EVENT_ENTER_ZONE)),
			luabind::value("click_door", static_cast<int>(EVENT_CLICK_DOOR)),
			luabind::value("loot", static_cast<int>(EVENT_LOOT)),
			luabind::value("zone", static_cast<int>(EVENT_ZONE)),
			luabind::value("level_up", static_cast<int>(EVENT_LEVEL_UP)),
			luabind::value("killed_merit ", static_cast<int>(EVENT_KILLED_MERIT )),
			luabind::value("cast_on", static_cast<int>(EVENT_CAST_ON)),
			luabind::value("task_accepted", static_cast<int>(EVENT_TASK_ACCEPTED)),
			luabind::value("task_stage_complete", static_cast<int>(EVENT_TASK_STAGE_COMPLETE)),
			luabind::value("task_update", static_cast<int>(EVENT_TASK_UPDATE)),
			luabind::value("task_complete", static_cast<int>(EVENT_TASK_COMPLETE)),
			luabind::value("task_fail", static_cast<int>(EVENT_TASK_FAIL)),
			luabind::value("aggro_say", static_cast<int>(EVENT_AGGRO_SAY)),
			luabind::value("player_pickup", static_cast<int>(EVENT_PLAYER_PICKUP)),
			luabind::value("popup_response", static_cast<int>(EVENT_POPUP_RESPONSE)),
			luabind::value("proximity_say", static_cast<int>(EVENT_PROXIMITY_SAY)),
			luabind::value("cast", static_cast<int>(EVENT_CAST)),
			luabind::value("scale_calc", static_cast<int>(EVENT_SCALE_CALC)),
			luabind::value("item_enter_zone", static_cast<int>(EVENT_ITEM_ENTER_ZONE)),
			luabind::value("target_change", static_cast<int>(EVENT_TARGET_CHANGE)),
			luabind::value("hate_list", static_cast<int>(EVENT_HATE_LIST)),
			luabind::value("spell_effect_client", static_cast<int>(EVENT_SPELL_EFFECT_CLIENT)),
			luabind::value("spell_effect_npc", static_cast<int>(EVENT_SPELL_EFFECT_NPC)),
			luabind::value("spell_effect_buff_tic_client", static_cast<int>(EVENT_SPELL_EFFECT_BUFF_TIC_CLIENT)),
			luabind::value("spell_effect_buff_tic_npc", static_cast<int>(EVENT_SPELL_EFFECT_BUFF_TIC_NPC)),
			luabind::value("spell_effect_translocate_complete", static_cast<int>(EVENT_SPELL_EFFECT_TRANSLOCATE_COMPLETE)),
			luabind::value("combine_success ", static_cast<int>(EVENT_COMBINE_SUCCESS )),
			luabind::value("combine_failure ", static_cast<int>(EVENT_COMBINE_FAILURE )),
			luabind::value("item_click", static_cast<int>(EVENT_ITEM_CLICK)),
			luabind::value("item_click_cast", static_cast<int>(EVENT_ITEM_CLICK_CAST)),
			luabind::value("group_change", static_cast<int>(EVENT_GROUP_CHANGE)),
			luabind::value("forage_success", static_cast<int>(EVENT_FORAGE_SUCCESS)),
			luabind::value("forage_failure", static_cast<int>(EVENT_FORAGE_FAILURE)),
			luabind::value("fish_start", static_cast<int>(EVENT_FISH_START)),
			luabind::value("fish_success", static_cast<int>(EVENT_FISH_SUCCESS)),
			luabind::value("fish_failure", static_cast<int>(EVENT_FISH_FAILURE)),
			luabind::value("click_object", static_cast<int>(EVENT_CLICK_OBJECT)),
			luabind::value("discover_item", static_cast<int>(EVENT_DISCOVER_ITEM)),
			luabind::value("disconnect", static_cast<int>(EVENT_DISCONNECT)),
			luabind::value("connect", static_cast<int>(EVENT_CONNECT)),
			luabind::value("item_tick", static_cast<int>(EVENT_ITEM_TICK)),
			luabind::value("duel_win", static_cast<int>(EVENT_DUEL_WIN)),
			luabind::value("duel_lose", static_cast<int>(EVENT_DUEL_LOSE)),
			luabind::value("encounter_load", static_cast<int>(EVENT_ENCOUNTER_LOAD)),
			luabind::value("encounter_unload", static_cast<int>(EVENT_ENCOUNTER_UNLOAD))
		];
}

luabind::scope lua_register_faction() {
	return luabind::class_<Events>("Faction")
		.enum_("constants")
		[
			luabind::value("Ally", static_cast<int>(FACTION_ALLY)),
			luabind::value("Warmly", static_cast<int>(FACTION_WARMLY)),
			luabind::value("Kindly", static_cast<int>(FACTION_KINDLY)),
			luabind::value("Amiable", static_cast<int>(FACTION_AMIABLE)),
			luabind::value("Indifferent", static_cast<int>(FACTION_INDIFFERENT)),
			luabind::value("Apprehensive", static_cast<int>(FACTION_APPREHENSIVE)),
			luabind::value("Dubious", static_cast<int>(FACTION_DUBIOUS)),
			luabind::value("Threatenly", static_cast<int>(FACTION_THREATENLY)),
			luabind::value("Scowls", static_cast<int>(FACTION_SCOWLS))
		];
}

luabind::scope lua_register_slot() {
	return luabind::class_<Events>("Slot")
		.enum_("constants")
		[
			luabind::value("Charm", static_cast<int>(SLOT_CHARM)),
			luabind::value("Ear1", static_cast<int>(SLOT_EAR01)),
			luabind::value("Head", static_cast<int>(SLOT_HEAD)),
			luabind::value("Face", static_cast<int>(SLOT_FACE)),
			luabind::value("Ear2", static_cast<int>(SLOT_EAR02)),
			luabind::value("Neck", static_cast<int>(SLOT_NECK)),
			luabind::value("Shoulder", static_cast<int>(SLOT_SHOULDER)),
			luabind::value("Arms", static_cast<int>(SLOT_ARMS)),
			luabind::value("Back", static_cast<int>(SLOT_BACK)),
			luabind::value("Bracer1", static_cast<int>(SLOT_BRACER01)),
			luabind::value("Bracer2", static_cast<int>(SLOT_BRACER02)),
			luabind::value("Range", static_cast<int>(SLOT_RANGE)),
			luabind::value("Hands", static_cast<int>(SLOT_HANDS)),
			luabind::value("Primary", static_cast<int>(SLOT_PRIMARY)),
			luabind::value("Secondary", static_cast<int>(SLOT_SECONDARY)),
			luabind::value("Ring1", static_cast<int>(SLOT_RING01)),
			luabind::value("Ring2", static_cast<int>(SLOT_RING02)),
			luabind::value("Chest", static_cast<int>(SLOT_CHEST)),
			luabind::value("Legs", static_cast<int>(SLOT_LEGS)),
			luabind::value("Feet", static_cast<int>(SLOT_FEET)),
			luabind::value("Waist", static_cast<int>(SLOT_WAIST)),
			luabind::value("Ammo", static_cast<int>(SLOT_AMMO)),
			luabind::value("PersonalBegin", static_cast<int>(SLOT_PERSONAL_BEGIN)),
			luabind::value("PersonalEnd", static_cast<int>(SLOT_PERSONAL_END)),
			luabind::value("Cursor", static_cast<int>(SLOT_CURSOR)),
			luabind::value("CursorEnd", 0xFFFE),
			luabind::value("Tradeskill", static_cast<int>(SLOT_TRADESKILL)),
			luabind::value("Augment", static_cast<int>(SLOT_AUGMENT)),
			luabind::value("PowerSource", static_cast<int>(SLOT_POWER_SOURCE)),
			luabind::value("Invalid", 0xFFFF)
		];
}

luabind::scope lua_register_material() {
	return luabind::class_<Events>("Material")
		.enum_("constants")
		[
			luabind::value("Head", MATERIAL_HEAD),
			luabind::value("Chest", MATERIAL_CHEST),
			luabind::value("Arms", MATERIAL_ARMS),
			luabind::value("Bracer", MATERIAL_BRACER),
			luabind::value("Hands", MATERIAL_HANDS),
			luabind::value("Legs", MATERIAL_LEGS),
			luabind::value("Feet", MATERIAL_FEET),
			luabind::value("Primary", MATERIAL_PRIMARY),
			luabind::value("Secondary", MATERIAL_SECONDARY),
			luabind::value("Max", MAX_MATERIALS)
		];
}

luabind::scope lua_register_client_version() {
	return luabind::class_<Events>("ClientVersion")
		.enum_("constants")
		[
			luabind::value("Unknown", static_cast<int>(EQClientUnknown)),
			luabind::value("62", static_cast<int>(EQClient62)),
			luabind::value("Titanium", static_cast<int>(EQClientTitanium)),
			luabind::value("SoF", static_cast<int>(EQClientSoF)),
			luabind::value("SoD", static_cast<int>(EQClientSoD)),
			luabind::value("Underfoot", static_cast<int>(EQClientUnderfoot)),
			luabind::value("RoF", static_cast<int>(EQClientRoF))
		];
}

#endif
