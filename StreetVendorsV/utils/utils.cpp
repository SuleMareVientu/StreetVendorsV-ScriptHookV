#include <natives.h>
#include "utils.h"

//Custom implementation of TIMERA and TIMERB natives
void Timer::Set(int value)
{
	gameTimer = GET_GAME_TIMER() + value;
	return;
}

int Timer::Get()
{
	return (GET_GAME_TIMER() - gameTimer);
}

void PrintHelp(char* string, bool playSound, int overrideDuration)
{
	BEGIN_TEXT_COMMAND_DISPLAY_HELP("STRING");
	ADD_TEXT_COMPONENT_SUBSTRING_PLAYER_NAME(string);
	END_TEXT_COMMAND_DISPLAY_HELP(NULL, false, playSound, overrideDuration);
	return;
}

Object GetVendorStand(Vector3 pedLoc, float radious)
{
	Object stands[] = {
		GET_CLOSEST_OBJECT_OF_TYPE(pedLoc.x, pedLoc.y, pedLoc.z, radious, hotdogStandHash, false, false, false),
		GET_CLOSEST_OBJECT_OF_TYPE(pedLoc.x, pedLoc.y, pedLoc.z, radious, burgerStandHash, false, false, false),
		GET_CLOSEST_OBJECT_OF_TYPE(pedLoc.x, pedLoc.y, pedLoc.z, radious, LChotdogStandHash1, false, false, false),
		GET_CLOSEST_OBJECT_OF_TYPE(pedLoc.x, pedLoc.y, pedLoc.z, radious, LChotdogStandHash2, false, false, false),
		GET_CLOSEST_OBJECT_OF_TYPE(pedLoc.x, pedLoc.y, pedLoc.z, radious, LChotdogStandHash3, false, false, false),
		GET_CLOSEST_OBJECT_OF_TYPE(pedLoc.x, pedLoc.y, pedLoc.z, radious, LChotdogStandHash4, false, false, false)
	};

	Object closestStand = NULL;
	float closestDistance = 10000;

	for (int i = 0; i < sizeof(stands) / sizeof(stands[0]); i++)
	{
		if (stands[i] != NULL)
		{
			Vector3 standLoc = GET_ENTITY_COORDS(stands[i], false);
			float distance = VDIST2(pedLoc.x, pedLoc.y, pedLoc.z, standLoc.x, standLoc.y, standLoc.z);

			if (distance < closestDistance)
			{
				closestDistance = distance;
				closestStand = stands[i];
			}
		}
	}

	return closestStand;
}

int GetStandType(Object stand)
{
	int standType = -1;	//-1, 0 = hotdog stand, 1 = burger stand
	switch (GET_ENTITY_MODEL(stand))
	{
	case hotdogStandHash:
		standType = HOTDOG;	break;
	case burgerStandHash:
		standType = BURGER;	break;
	case LChotdogStandHash1:
		standType = HOTDOG; break;
	case LChotdogStandHash2:
		standType = HOTDOG; break;
	case LChotdogStandHash3:
		standType = HOTDOG; break;
	case LChotdogStandHash4:
		standType = HOTDOG; break;
	}
	return standType;
}

bool IsPedVendor(Ped ped)
{
	if (!DOES_ENTITY_EXIST(ped))
		return false;

	switch (GET_ENTITY_MODEL(ped))
	{
	case S_M_Y_StrVend_01:
	case S_M_M_StrVend_01:
	case A_M_Y_GenStreet_01:
	case A_M_Y_GenStreet_02:
	case A_M_Y_Downtown_01:
	case A_M_Y_Latino_01:
		return true;	break;
	}
	return false;
}

bool FindVendor(Object vendorStand, Ped *vendor)
{
	Ped tempVendor = NULL;
	Vector3 findVendorLoc = GET_OFFSET_FROM_ENTITY_IN_WORLD_COORDS(vendorStand, 0.5f, 0.75f, 0.0f);
	SET_SCENARIO_PEDS_TO_BE_RETURNED_BY_NEXT_COMMAND(true);
	if (GET_CLOSEST_PED(findVendorLoc.x, findVendorLoc.y, findVendorLoc.z, 1.5f, true, true, &tempVendor, false, true, -1))
	{
		if (!DOES_ENTITY_EXIST(tempVendor))
			return false;

		switch (GET_ENTITY_MODEL(tempVendor))
		{
		case S_M_Y_StrVend_01:
		case S_M_M_StrVend_01:
		case A_M_Y_GenStreet_01:
		case A_M_Y_GenStreet_02:
		case A_M_Y_Downtown_01:
		case A_M_Y_Latino_01:
			*vendor = tempVendor;
			return true;	break;
		}
	}
	*vendor = NULL;
	return false;
}

bool RequestAnimDict(char* animDict)
{
	if (!DOES_ANIM_DICT_EXIST(animDict))
		return false;

	if (!HAS_ANIM_DICT_LOADED(animDict))
	{
		REQUEST_ANIM_DICT(animDict);
		return false;
	}
	return true;
}

bool RequestModel(Hash model)
{
	if (!IS_MODEL_VALID(model))
		return false;

	if (!HAS_MODEL_LOADED(model))
	{
		REQUEST_MODEL(model);
		return false;
	}
	return true;
}

Object CreateObject(Hash model, Vector3 loc, float rotX, float rotY, float rotZ)
{
	RequestModel(model);
	Object obj = CREATE_OBJECT_NO_OFFSET(model, loc.x, loc.y, loc.z, false, false, true);
	if (rotX != NULL || rotY != NULL || rotZ != NULL)
		SET_ENTITY_ROTATION(obj, rotX, rotY, rotZ, 2, false);
	return obj;
}

void DeleteObject(Object obj)
{
	if (DOES_ENTITY_EXIST(obj))
	{
		SET_ENTITY_AS_MISSION_ENTITY(obj, false, true);
		DELETE_OBJECT(&obj);
	}
	return;
}

int GetPlayerCash(Ped playerPed)
{
	int playerCash = 10;	//Always return a value > 5 when playerped is not a main character

	switch (GET_ENTITY_MODEL(playerPed))
	{
	case MichaelPed:
		STATS::STAT_GET_INT(SP0_TOTAL_CASH, &playerCash, -1); break;
	case FranklinPed:
		STATS::STAT_GET_INT(SP1_TOTAL_CASH, &playerCash, -1); break;
	case TrevorPed:
		STATS::STAT_GET_INT(SP2_TOTAL_CASH, &playerCash, -1); break;
	}
	return playerCash;
}

Timer showCashTimer;
void SetPlayerCash(Ped playerPed, int currentCash, int change)
{
	if (playerPed != PLAYER_PED_ID())
		return;

	Hash stat = NULL;
	switch (GET_ENTITY_MODEL(playerPed))
	{
	case MichaelPed:
		stat = SP0_TOTAL_CASH;	break;
	case FranklinPed:
		stat = SP1_TOTAL_CASH;	break;
	case TrevorPed:
		stat = SP2_TOTAL_CASH;	break;
	default:
		return;	break;
	}

	//Show cash change with HUD
	DISPLAY_CASH(true);
	showCashTimer.Set(0);
	STATS::STAT_SET_INT(stat, currentCash + change, true); //Set player cash stat
	return;
}

void UpdateCashHUD()
{
	if (showCashTimer.Get() <= 5300)
		SHOW_HUD_COMPONENT_THIS_FRAME(NEW_HUD_CASH);
	return;
}

bool PrintVendorMessage(int playerCash, int standType)
{
	if (GET_PLAYER_WANTED_LEVEL(PLAYER_ID()) != 0)
		return false;
	else if (playerCash < 5)
	{	
		//Print desired help based on stand type
		switch (standType)
		{
		case HOTDOG:
			PrintHelp("You need at least $5 to buy a hotdog.", false, 500);	break;
		case BURGER:
			PrintHelp("You need at least $5 to buy a burger.", false, 500);	break;
		}
		return false;
	}
	else
	{	
		//Print desired help based on stand type
		switch (standType)
		{
		case HOTDOG:
			PrintHelp("Press ~INPUT_CONTEXT~ to buy a hotdog.", true, 500);	break;
		case BURGER:
			PrintHelp("Press ~INPUT_CONTEXT~ to buy a burger.", true, 500);	break;
		}
		return true;
	}
	return true;
}

bool AdditionalChecks(Ped ped)
{
	if (IS_PED_RAGDOLL(ped)	||
		IS_PED_GETTING_UP(ped) ||
		IS_PED_FALLING(ped) ||
		IS_PED_JUMPING(ped) ||
		IS_PED_DIVING(ped) ||
		IS_PED_SWIMMING(ped) ||
		IS_PED_GOING_INTO_COVER(ped) ||
		IS_PED_CLIMBING(ped) ||
		IS_PED_VAULTING(ped) ||
		IS_PED_HANGING_ON_TO_VEHICLE(ped) ||
		IS_PED_IN_ANY_VEHICLE(ped, true) ||
		IS_PED_IN_COVER(ped, false) ||
		!IS_PED_ON_FOOT(ped) ||
		IS_PED_TAKING_OFF_HELMET(ped) ||
		GET_ENTITY_SUBMERGED_LEVEL(ped) >= 0.7f ||
		IS_PED_IN_MELEE_COMBAT(ped) ||
		COUNT_PEDS_IN_COMBAT_WITH_TARGET(ped) > 0)
		return false;

	return true;
}

void DisablePlayerActionsThisFrame()
{
	Ped ped = PLAYER_PED_ID();
	SET_PED_RESET_FLAG(ped, PRF_DisablePlayerJumping, true);
	SET_PED_RESET_FLAG(ped, PRF_DisablePlayerVaulting, true);
	SET_PED_MAX_MOVE_BLEND_RATIO(ped, PEDMOVEBLENDRATIO_WALK);	//Prevents the player from running/sprinting

	if (IS_SCRIPTED_CONVERSATION_ONGOING() || IS_MOBILE_PHONE_CALL_ONGOING())
		STOP_SCRIPTED_CONVERSATION(false);

	if (IS_PED_RINGTONE_PLAYING(ped))
		STOP_PED_RINGTONE(ped);

	DISABLE_CONTROL_ACTION(FRONTEND_CONTROL, INPUT_CELLPHONE_UP, false);
	DISABLE_CONTROL_ACTION(FRONTEND_CONTROL, INPUT_CELLPHONE_DOWN, false);
	DISABLE_CONTROL_ACTION(FRONTEND_CONTROL, INPUT_CELLPHONE_LEFT, false);
	DISABLE_CONTROL_ACTION(FRONTEND_CONTROL, INPUT_CELLPHONE_RIGHT, false);
	DISABLE_CONTROL_ACTION(FRONTEND_CONTROL, INPUT_CELLPHONE_SELECT, false);
	DISABLE_CONTROL_ACTION(FRONTEND_CONTROL, INPUT_CELLPHONE_CANCEL, false);
	DISABLE_CONTROL_ACTION(FRONTEND_CONTROL, INPUT_CELLPHONE_OPTION, false);
	DISABLE_CONTROL_ACTION(FRONTEND_CONTROL, INPUT_CELLPHONE_EXTRA_OPTION, false);
	DISABLE_CONTROL_ACTION(FRONTEND_CONTROL, INPUT_CELLPHONE_SCROLL_FORWARD, false);
	DISABLE_CONTROL_ACTION(FRONTEND_CONTROL, INPUT_CELLPHONE_SCROLL_BACKWARD, false);
	DISABLE_CONTROL_ACTION(FRONTEND_CONTROL, INPUT_CELLPHONE_CAMERA_FOCUS_LOCK, false);
	DISABLE_CONTROL_ACTION(FRONTEND_CONTROL, INPUT_CELLPHONE_CAMERA_GRID, false);
	DISABLE_CONTROL_ACTION(FRONTEND_CONTROL, INPUT_CELLPHONE_CAMERA_SELFIE, false);
	DISABLE_CONTROL_ACTION(FRONTEND_CONTROL, INPUT_CELLPHONE_CAMERA_DOF, false);
	DISABLE_CONTROL_ACTION(FRONTEND_CONTROL, INPUT_CELLPHONE_CAMERA_EXPRESSION, false);

	DISABLE_CONTROL_ACTION(PLAYER_CONTROL, INPUT_TALK, true);	//Disables talking
	DISABLE_CONTROL_ACTION(PLAYER_CONTROL, INPUT_WEAPON_WHEEL_UD, false);
	DISABLE_CONTROL_ACTION(PLAYER_CONTROL, INPUT_WEAPON_WHEEL_LR, false);
	DISABLE_CONTROL_ACTION(PLAYER_CONTROL, INPUT_WEAPON_WHEEL_NEXT, false);
	DISABLE_CONTROL_ACTION(PLAYER_CONTROL, INPUT_WEAPON_WHEEL_PREV, false);
	DISABLE_CONTROL_ACTION(PLAYER_CONTROL, INPUT_SELECT_NEXT_WEAPON, false);
	DISABLE_CONTROL_ACTION(PLAYER_CONTROL, INPUT_SELECT_PREV_WEAPON, false);
	DISABLE_CONTROL_ACTION(PLAYER_CONTROL, INPUT_CHARACTER_WHEEL, false);
	DISABLE_CONTROL_ACTION(PLAYER_CONTROL, INPUT_MULTIPLAYER_INFO, false);
	DISABLE_CONTROL_ACTION(PLAYER_CONTROL, INPUT_JUMP, false);
	DISABLE_CONTROL_ACTION(PLAYER_CONTROL, INPUT_ENTER, false);
	DISABLE_CONTROL_ACTION(PLAYER_CONTROL, INPUT_ATTACK, false);
	DISABLE_CONTROL_ACTION(PLAYER_CONTROL, INPUT_AIM, false);
	DISABLE_CONTROL_ACTION(PLAYER_CONTROL, INPUT_PHONE, false);
	DISABLE_CONTROL_ACTION(PLAYER_CONTROL, INPUT_SPECIAL_ABILITY, false);
	DISABLE_CONTROL_ACTION(PLAYER_CONTROL, INPUT_SPECIAL_ABILITY_SECONDARY, false);
	DISABLE_CONTROL_ACTION(PLAYER_CONTROL, INPUT_DUCK, false);
	DISABLE_CONTROL_ACTION(PLAYER_CONTROL, INPUT_SELECT_WEAPON, false);
	DISABLE_CONTROL_ACTION(PLAYER_CONTROL, INPUT_PICKUP, false);
	DISABLE_CONTROL_ACTION(PLAYER_CONTROL, INPUT_SNIPER_ZOOM, false);
	DISABLE_CONTROL_ACTION(PLAYER_CONTROL, INPUT_SNIPER_ZOOM_IN_ONLY, false);
	DISABLE_CONTROL_ACTION(PLAYER_CONTROL, INPUT_SNIPER_ZOOM_OUT_ONLY, false);
	DISABLE_CONTROL_ACTION(PLAYER_CONTROL, INPUT_SNIPER_ZOOM_IN_SECONDARY, false);
	DISABLE_CONTROL_ACTION(PLAYER_CONTROL, INPUT_SNIPER_ZOOM_OUT_SECONDARY, false);
	DISABLE_CONTROL_ACTION(PLAYER_CONTROL, INPUT_COVER, false);
	DISABLE_CONTROL_ACTION(PLAYER_CONTROL, INPUT_RELOAD, false);
	DISABLE_CONTROL_ACTION(PLAYER_CONTROL, INPUT_DETONATE, false);
	DISABLE_CONTROL_ACTION(PLAYER_CONTROL, INPUT_HUD_SPECIAL, false);
	DISABLE_CONTROL_ACTION(PLAYER_CONTROL, INPUT_ARREST, false);
	DISABLE_CONTROL_ACTION(PLAYER_CONTROL, INPUT_ACCURATE_AIM, false);
	DISABLE_CONTROL_ACTION(PLAYER_CONTROL, INPUT_CONTEXT, false);
	DISABLE_CONTROL_ACTION(PLAYER_CONTROL, INPUT_CONTEXT_SECONDARY, false);
	DISABLE_CONTROL_ACTION(PLAYER_CONTROL, INPUT_WEAPON_SPECIAL, false);
	DISABLE_CONTROL_ACTION(PLAYER_CONTROL, INPUT_WEAPON_SPECIAL_TWO, false);
	DISABLE_CONTROL_ACTION(PLAYER_CONTROL, INPUT_DIVE, false);
	DISABLE_CONTROL_ACTION(PLAYER_CONTROL, INPUT_DROP_WEAPON, false);
	DISABLE_CONTROL_ACTION(PLAYER_CONTROL, INPUT_DROP_AMMO, false);
	DISABLE_CONTROL_ACTION(PLAYER_CONTROL, INPUT_THROW_GRENADE, false);
	DISABLE_CONTROL_ACTION(PLAYER_CONTROL, INPUT_VEH_EXIT, false);
	DISABLE_CONTROL_ACTION(PLAYER_CONTROL, INPUT_MELEE_ATTACK_LIGHT, false);
	DISABLE_CONTROL_ACTION(PLAYER_CONTROL, INPUT_MELEE_ATTACK_HEAVY, false);
	DISABLE_CONTROL_ACTION(PLAYER_CONTROL, INPUT_MELEE_ATTACK_ALTERNATE, false);
	DISABLE_CONTROL_ACTION(PLAYER_CONTROL, INPUT_MELEE_BLOCK, false);
	DISABLE_CONTROL_ACTION(PLAYER_CONTROL, INPUT_SELECT_WEAPON_UNARMED, false);
	DISABLE_CONTROL_ACTION(PLAYER_CONTROL, INPUT_SELECT_WEAPON_MELEE, false);
	DISABLE_CONTROL_ACTION(PLAYER_CONTROL, INPUT_SELECT_WEAPON_HANDGUN, false);
	DISABLE_CONTROL_ACTION(PLAYER_CONTROL, INPUT_SELECT_WEAPON_SHOTGUN, false);
	DISABLE_CONTROL_ACTION(PLAYER_CONTROL, INPUT_SELECT_WEAPON_SMG, false);
	DISABLE_CONTROL_ACTION(PLAYER_CONTROL, INPUT_SELECT_WEAPON_AUTO_RIFLE, false);
	DISABLE_CONTROL_ACTION(PLAYER_CONTROL, INPUT_SELECT_WEAPON_SNIPER, false);
	DISABLE_CONTROL_ACTION(PLAYER_CONTROL, INPUT_SELECT_WEAPON_HEAVY, false);
	DISABLE_CONTROL_ACTION(PLAYER_CONTROL, INPUT_SELECT_WEAPON_SPECIAL, false);
	DISABLE_CONTROL_ACTION(PLAYER_CONTROL, INPUT_SELECT_CHARACTER_MICHAEL, false);
	DISABLE_CONTROL_ACTION(PLAYER_CONTROL, INPUT_SELECT_CHARACTER_FRANKLIN, false);
	DISABLE_CONTROL_ACTION(PLAYER_CONTROL, INPUT_SELECT_CHARACTER_TREVOR, false);
	DISABLE_CONTROL_ACTION(PLAYER_CONTROL, INPUT_SELECT_CHARACTER_MULTIPLAYER, false);
	DISABLE_CONTROL_ACTION(PLAYER_CONTROL, INPUT_SPECIAL_ABILITY_PC, false);
	DISABLE_CONTROL_ACTION(PLAYER_CONTROL, INPUT_INTERACTION_MENU, false);
	DISABLE_CONTROL_ACTION(PLAYER_CONTROL, INPUT_ATTACK2, false);
	DISABLE_CONTROL_ACTION(PLAYER_CONTROL, INPUT_PREV_WEAPON, false);
	DISABLE_CONTROL_ACTION(PLAYER_CONTROL, INPUT_NEXT_WEAPON, false);
	DISABLE_CONTROL_ACTION(PLAYER_CONTROL, INPUT_MELEE_ATTACK1, false);
	DISABLE_CONTROL_ACTION(PLAYER_CONTROL, INPUT_MELEE_ATTACK2, false);
	DISABLE_CONTROL_ACTION(PLAYER_CONTROL, INPUT_SNIPER_ZOOM_IN, false);
	DISABLE_CONTROL_ACTION(PLAYER_CONTROL, INPUT_SNIPER_ZOOM_OUT, false);
	DISABLE_CONTROL_ACTION(PLAYER_CONTROL, INPUT_SNIPER_ZOOM_IN_ALTERNATE, false);
	DISABLE_CONTROL_ACTION(PLAYER_CONTROL, INPUT_SNIPER_ZOOM_OUT_ALTERNATE, false);
	return;
}

void DisablePlayerControlThisFrame()
{
	//DisablePlayerActionsThisFrame(ped);
	DISABLE_CONTROL_ACTION(PLAYER_CONTROL, INPUT_MOVE_LR, false);
	DISABLE_CONTROL_ACTION(PLAYER_CONTROL, INPUT_MOVE_UD, false);
	DISABLE_CONTROL_ACTION(PLAYER_CONTROL, INPUT_MOVE_UP_ONLY, false);
	DISABLE_CONTROL_ACTION(PLAYER_CONTROL, INPUT_MOVE_DOWN_ONLY, false);
	DISABLE_CONTROL_ACTION(PLAYER_CONTROL, INPUT_MOVE_LEFT_ONLY, false);
	DISABLE_CONTROL_ACTION(PLAYER_CONTROL, INPUT_MOVE_RIGHT_ONLY, false);
	DISABLE_CONTROL_ACTION(PLAYER_CONTROL, INPUT_TALK, false);
	DISABLE_CONTROL_ACTION(PLAYER_CONTROL, INPUT_MOVE_LEFT, false);
	DISABLE_CONTROL_ACTION(PLAYER_CONTROL, INPUT_MOVE_RIGHT, false);
	DISABLE_CONTROL_ACTION(PLAYER_CONTROL, INPUT_MOVE_UP, false);
	DISABLE_CONTROL_ACTION(PLAYER_CONTROL, INPUT_MOVE_DOWN, false);
	return;
}

void PlayAmbientSpeech(Ped ped, char* speechName)
{
	AUDIO::SET_AUDIO_FLAG("IsDirectorModeActive", true);
	AUDIO::PLAY_PED_AMBIENT_SPEECH_NATIVE(ped, speechName, "SPEECH_PARAMS_FORCE_NORMAL", false);
	AUDIO::SET_AUDIO_FLAG("IsDirectorModeActive", false);
	return;
}