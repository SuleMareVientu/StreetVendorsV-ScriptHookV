#include <natives.h>
#include "utils.h"

const Hash hotdogStandHash = -1581502570;	//prop_hotdogstand_01
const Hash burgerStandHash = 1129053052;	//prop_burgerstand_01
const Hash S_M_M_StrVend_01 = -829353047;	//S_M_M_StrVend_01
const Hash S_M_Y_StrVend_01 = -1837161693;	//S_M_Y_StrVend_01
const Hash hotdogHash = -1729226035;		//prop_cs_hotdog_01
const Hash eatenHotdogHash = -1490012335;	//prop_cs_hotdog_02
const Hash burgerHash = -2054442544;		//prop_cs_burger_01
const int defaultAF = AF_NOT_INTERRUPTABLE | AF_TAG_SYNC_IN | AF_TAG_SYNC_OUT | AF_HIDE_WEAPON | AF_ABORT_ON_WEAPON_DAMAGE;
const int upperSecondaryAF = AF_NOT_INTERRUPTABLE | AF_UPPERBODY | AF_SECONDARY | AF_HIDE_WEAPON | AF_ABORT_ON_WEAPON_DAMAGE;
char* chooseAnimDict = "gestures@m@standing@casual";
char* chooseAnim = "gesture_you_soft";
char* takeAnimDict = "mp_common";
char* takeAnim = "givetake1_a";
char* takeBurgerAnimDict = "mp_doorbell";
char* takeBurgerAnim = "ring_bell_a_left";
char* eatingAnimDict = "amb@code_human_wander_eating_donut@male@idle_a";
char* eatingStartAnim = "idle_b";
char* eatingEndAnim = "idle_a";
char* burgerAnimDict = "mp_player_inteat@burger";
char* burgerAnim = "mp_player_int_eat_burger";
Ped vendor = NULL;

void PrintHelp(char* string)
{
	HUD::BEGIN_TEXT_COMMAND_DISPLAY_HELP("STRING");
	HUD::ADD_TEXT_COMPONENT_SUBSTRING_PLAYER_NAME(string);
	HUD::END_TEXT_COMMAND_DISPLAY_HELP(0, false, false, -1);
	return;
}

Object GetVendorStand(Vector3 pedLoc, float radious)
{
	Object hotdogStand = OBJECT::GET_CLOSEST_OBJECT_OF_TYPE(pedLoc.x, pedLoc.y, pedLoc.z, radious, hotdogStandHash, false, false, false);
	Object burgerStand = OBJECT::GET_CLOSEST_OBJECT_OF_TYPE(pedLoc.x, pedLoc.y, pedLoc.z, radious, burgerStandHash, false, false, false);

	if (hotdogStand == NULL && burgerStand == NULL)
		return NULL;
	else if (hotdogStand != NULL && burgerStand != NULL)
	{
		Vector3 hotdogStandLoc = ENTITY::GET_ENTITY_COORDS(hotdogStand, false);
		Vector3 burgerStandLoc = ENTITY::GET_ENTITY_COORDS(burgerStand, false);
		float distHotdog = SYSTEM::VDIST2(pedLoc.x, pedLoc.y, pedLoc.z, hotdogStandLoc.x, hotdogStandLoc.y, hotdogStandLoc.z);
		float distBurger = SYSTEM::VDIST2(pedLoc.x, pedLoc.y, pedLoc.z, burgerStandLoc.x, burgerStandLoc.y, burgerStandLoc.z);
		if (distHotdog < distBurger)
			return hotdogStand;
		else
			return burgerStand;
	}
	else
	{
		switch (hotdogStand)
		{
		case NULL:
			return burgerStand;	break;
		default:
			return hotdogStand;	break;
		}
	}
	return NULL;
}

int GetStandType(Object stand)
{
	int standType = NULL;	//NULL, 1 = hotdog stand, 2 = burger stand
	switch (ENTITY::GET_ENTITY_MODEL(stand))
	{
	case hotdogStandHash:
		standType = 1;	break;
	case burgerStandHash:
		standType = 2;	break;
	}
	return standType;
}

bool FindVendor(Object vendorStand)
{
	vendor = NULL;
	Vector3 findVendorLoc = ENTITY::GET_OFFSET_FROM_ENTITY_IN_WORLD_COORDS(vendorStand, 0.5f, 0.75f, 0.0f);
	PED::SET_SCENARIO_PEDS_TO_BE_RETURNED_BY_NEXT_COMMAND(true);
	if (PED::GET_CLOSEST_PED(findVendorLoc.x, findVendorLoc.y, findVendorLoc.z, 1.5f, true, true, &vendor, false, true, -1))
	{
		if (ENTITY::DOES_ENTITY_EXIST(vendor))
		{
			switch (ENTITY::GET_ENTITY_MODEL(vendor))
			{
			case S_M_Y_StrVend_01:
				return true;	break;
			case S_M_M_StrVend_01:
				return true;	break;
			}
		}
	}
	return false;
}

void RequestAnimDict(char* animDict)
{
	if (STREAMING::DOES_ANIM_DICT_EXIST(animDict) && !STREAMING::HAS_ANIM_DICT_LOADED(animDict))
	{
		STREAMING::REQUEST_ANIM_DICT(animDict);
		while (!STREAMING::HAS_ANIM_DICT_LOADED(animDict))
		{
			WAIT(0);
		}
	}
	return;
}

void RequestModel(Hash model)
{
	if (STREAMING::IS_MODEL_VALID(model) && !STREAMING::HAS_MODEL_LOADED(model))
	{
		STREAMING::REQUEST_MODEL(model);
		while (!STREAMING::HAS_MODEL_LOADED(model))
		{
			WAIT(0);
		}
	}
	return;
}

Object CreateObject(Hash model, Vector3 loc, bool setRotation = false, float rotX = NULL, float rotY = NULL, float rotZ = NULL)
{
	RequestModel(model);
	Object obj = OBJECT::CREATE_OBJECT_NO_OFFSET(model, loc.x, loc.y, loc.z, false, false, true);
	if (setRotation)
		ENTITY::SET_ENTITY_ROTATION(obj, rotX, rotY, rotZ, 2, false);
	return obj;
}

void DeleteObject(Object obj)
{
	if (ENTITY::DOES_ENTITY_EXIST(obj))
	{
		ENTITY::SET_ENTITY_AS_MISSION_ENTITY(obj, true, true);
		OBJECT::DELETE_OBJECT(&obj);
	}
	return;
}

int GetPlayerCash(Ped playerPed)
{
	Hash stat = NULL;
	int playerCash = -1;

	switch (ENTITY::GET_ENTITY_MODEL(playerPed))
	{
	case 225514697:					//Michael
		stat = 52740893;	break;	//SP0_TOTAL_CASH
	case -1692214353:				//Franklin
		stat = 1153264002;	break;	//SP1_TOTAL_CASH
	case -1686040670:				//Trevor
		stat = -1921710979;	break;	//SP2_TOTAL_CASH
	default:
		return playerCash;	break;
	}
	STATS::STAT_GET_INT(stat, &playerCash, -1);
	return playerCash;
}

void SetPlayerCash(Ped playerPed, int currentCash, int change)
{
	Hash stat = NULL;
	switch (ENTITY::GET_ENTITY_MODEL(playerPed))
	{
	case 225514697:					//Michael
		stat = 52740893;	break;	//SP0_TOTAL_CASH
	case -1692214353:				//Franklin
		stat = 1153264002;	break;	//SP1_TOTAL_CASH
	case -1686040670:				//Trevor
		stat = -1921710979;	break;	//SP2_TOTAL_CASH
	default:
		return;	break;
	}
	HUD::REMOVE_MULTIPLAYER_HUD_CASH();
	HUD::CHANGE_FAKE_MP_CASH(change, NULL);
	STATS::STAT_SET_INT(stat, currentCash + change, true);
	return;
}

bool PrintVendorMessage(int playerCash, int standType)
{
	if (PLAYER::GET_PLAYER_WANTED_LEVEL(PLAYER::PLAYER_ID()) != 0)
		return false;
	else if (playerCash < 5)
	{	
		//Print desired help based on stand type
		switch (standType)
		{
		case 1:
			PrintHelp("You need at least $5 to buy a hotdog.");	break;
		case 2:
			PrintHelp("You need at least $5 to buy a burger.");	break;
		}
		return false;
	}
	else
	{	
		//Print desired help based on stand type
		switch (standType)
		{
		case 1:
			PrintHelp("Press ~INPUT_CONTEXT~ to buy a hotdog.");	break;
		case 2:
			PrintHelp("Press ~INPUT_CONTEXT~ to buy a burger.");	break;
		}
		return true;
	}
	return true;
}

bool AdditionalChecks(Ped ped)
{
	if (PED::IS_PED_RAGDOLL(ped) ||
		TASK::IS_PED_GETTING_UP(ped) ||
		PED::IS_PED_FALLING(ped) ||
		PED::IS_PED_JUMPING(ped) ||
		PED::IS_PED_IN_MELEE_COMBAT(ped) ||
		PED::IS_PED_IN_COVER(ped, false) ||
		PED::IS_PED_SHOOTING(ped) ||
		!PED::IS_PED_ON_FOOT(ped) ||
		PED::IS_PED_TAKING_OFF_HELMET(ped))
		return false;
	else
		return true;
}

void SetPlayerControl(Ped playerPed, bool toggle)
{
	PED::SET_PED_CAN_PLAY_GESTURE_ANIMS(playerPed, toggle);
	PLAYER::SET_PLAYER_SPRINT(PLAYER::PLAYER_ID(), toggle);
	PAD::DISABLE_CONTROL_ACTION(0, INPUT_TALK, true);
	return;
}

void DisablePlayerActionsThisFrame(Ped playerPed)
{
	PAD::DISABLE_CONTROL_ACTION(0, INPUT_TALK, true);	//Disables talking
	PED::SET_PED_RESET_FLAG(playerPed, 46, true);		//PRF_DisablePlayerJumping
	PED::SET_PED_RESET_FLAG(playerPed, 47, true);		//PRF_DisablePlayerVaulting
	PED::SET_PED_MAX_MOVE_BLEND_RATIO(playerPed, 1.0f);	//Prevents the player from running/sprinting
	return;
}

void PlayAmbientSpeech(Ped ped, char* speechName)
{
	AUDIO::SET_AUDIO_FLAG("IsDirectorModeActive", true);
	AUDIO::PLAY_PED_AMBIENT_SPEECH_NATIVE(ped, speechName, "SPEECH_PARAMS_FORCE_NORMAL", false);
	AUDIO::SET_AUDIO_FLAG("IsDirectorModeActive", false);
	return;
}

//https://alloc8or.re/gta5/doc/enums/eScriptTaskHash.txt
void PedTaskWalkToAndWait(Ped ped, float x, float y, float z, float heading)
{
	TASK::TASK_GO_STRAIGHT_TO_COORD(ped, x, y, z, 1.0f, 5000, heading, 0.1f);
	while (TASK::GET_SCRIPT_TASK_STATUS(ped, 2106541073) != 7)
	{
		DisablePlayerActionsThisFrame(ped);
		WAIT(0);
	}
	return;
}

void PlayAnim(Ped ped, char* animDict, char* anim, int flag, int duration = -1)
{
	RequestAnimDict(animDict);
	DisablePlayerActionsThisFrame(ped);
	TASK::TASK_PLAY_ANIM(ped, animDict, anim, 1.5f, -1.5f, duration, flag, 0.0f, false, false, false);
	return;
}

void PlayAnimAndWait(Ped ped, char* animDict, char* anim, int flag, int duration = -1)
{
	RequestAnimDict(animDict);
	TASK::TASK_PLAY_ANIM(ped, animDict, anim, 1.5f, -1.5f, duration, flag, 0.0f, false, false, false);
	switch (flag)
	{
	case defaultAF:
		while (TASK::GET_SCRIPT_TASK_STATUS(ped, 0x87B9A382) != 7)
		{
			DisablePlayerActionsThisFrame(ped);
			WAIT(0);
		}
		break;
	default:
		DisablePlayerActionsThisFrame(ped);
		WAIT(50);
		while (ENTITY::IS_ENTITY_PLAYING_ANIM(ped, animDict, anim, 3))
		{
			DisablePlayerActionsThisFrame(ped);
			WAIT(0);
		}
		return;
		break;
	}
	STREAMING::REMOVE_ANIM_DICT(animDict);
	return;
}

void PlayAnimWithSpeedAndWait(Ped ped, char* animDict, char* anim, int flag, float speed, int duration = -1)
{
	RequestAnimDict(animDict);
	TASK::TASK_PLAY_ANIM(ped, animDict, anim, 1.5f, -1.5f, duration, flag, 0.0f, false, false, false);
	switch (flag)
	{
	case defaultAF:
		while (TASK::GET_SCRIPT_TASK_STATUS(ped, 0x87B9A382) != 7)
		{
			DisablePlayerActionsThisFrame(ped);
			ENTITY::SET_ENTITY_ANIM_SPEED(ped, animDict, anim, speed);
			WAIT(0);
		}
		break;
	default:
		DisablePlayerActionsThisFrame(ped);
		WAIT(50);
		while (ENTITY::IS_ENTITY_PLAYING_ANIM(ped, animDict, anim, 3))
		{
			DisablePlayerActionsThisFrame(ped);
			ENTITY::SET_ENTITY_ANIM_SPEED(ped, animDict, anim, speed);
			WAIT(0);
		}
		return;
		break;
	}
	STREAMING::REMOVE_ANIM_DICT(animDict);
	return;
}

void PlayHotdogEatSequence(Ped ped)
{
	RequestModel(hotdogHash);
	RequestModel(eatenHotdogHash);
	RequestAnimDict(chooseAnimDict);
	RequestAnimDict(takeAnimDict);
	RequestAnimDict(eatingAnimDict);
	Vector3 pedLoc = ENTITY::GET_ENTITY_COORDS(ped, false);
	Object hotdog = CreateObject(hotdogHash, pedLoc);
	ENTITY::SET_ENTITY_VISIBLE(hotdog, false, false);
	int rightWristID = PED::GET_PED_BONE_INDEX(ped, 28422);

	PlayAnimAndWait(ped, chooseAnimDict, chooseAnim, defaultAF);
	PlayAnim(ped, takeAnimDict, takeAnim, defaultAF);
	while (TASK::GET_SCRIPT_TASK_STATUS(ped, 0x87B9A382) != 7)
	{
		DisablePlayerActionsThisFrame(ped);
		ENTITY::SET_ENTITY_ANIM_SPEED(ped, takeAnimDict, takeAnim, 0.65f);
		if (!ENTITY::IS_ENTITY_ATTACHED(hotdog) && ENTITY::GET_ENTITY_ANIM_CURRENT_TIME(ped, takeAnimDict, takeAnim) > 0.225f)
		{
			ENTITY::SET_ENTITY_VISIBLE(hotdog, true, false);
			ENTITY::ATTACH_ENTITY_TO_ENTITY(hotdog, ped, rightWristID, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, true, true, false, false, 2, true);
			TASK::TASK_LOOK_AT_ENTITY(ped, hotdog, -1, SLF_SLOW_TURN_RATE | SLF_NARROW_YAW_LIMIT, 2);
		}
		if (ENTITY::GET_ENTITY_ANIM_CURRENT_TIME(ped, takeAnimDict, takeAnim) > 0.5f)
			TASK::STOP_ANIM_TASK(ped, takeAnimDict, takeAnim, -2.0f);
		WAIT(0);
	}
	PlayAmbientSpeech(vendor, "GENERIC_BYE");

	PlayAnim(ped, eatingAnimDict, eatingStartAnim, upperSecondaryAF);
	DisablePlayerActionsThisFrame(ped);
	WAIT(50);
	while (ENTITY::IS_ENTITY_PLAYING_ANIM(ped, eatingAnimDict, eatingStartAnim, 3))
	{
		DisablePlayerActionsThisFrame(ped);
		if (ENTITY::GET_ENTITY_ANIM_CURRENT_TIME(ped, eatingAnimDict, eatingStartAnim) > 0.5f && ENTITY::GET_ENTITY_MODEL(hotdog) != eatenHotdogHash)
		{
			ENTITY::CREATE_MODEL_SWAP(pedLoc.x, pedLoc.y, pedLoc.z, 10.0f, hotdogHash, eatenHotdogHash, true);
			ENTITY::ATTACH_ENTITY_TO_ENTITY(hotdog, ped, rightWristID, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, true, true, false, false, 2, true);
		}
		WAIT(0);
	}

	PlayAnimAndWait(ped, eatingAnimDict, eatingEndAnim, upperSecondaryAF);

	TASK::TASK_CLEAR_LOOK_AT(ped);
	DeleteObject(hotdog);
	STREAMING::REMOVE_ANIM_DICT(chooseAnimDict);
	STREAMING::REMOVE_ANIM_DICT(takeAnimDict);
	STREAMING::REMOVE_ANIM_DICT(eatingAnimDict);
	STREAMING::SET_MODEL_AS_NO_LONGER_NEEDED(hotdogHash);
	STREAMING::SET_MODEL_AS_NO_LONGER_NEEDED(eatenHotdogHash);
	return;
}

void PlayBurgerEatSequence(Ped ped)
{
	RequestModel(burgerHash);
	RequestAnimDict(chooseAnimDict);
	RequestAnimDict(takeBurgerAnimDict);
	RequestAnimDict(burgerAnimDict);
	Vector3 pedLoc = ENTITY::GET_ENTITY_COORDS(ped, false);
	Object burger = CreateObject(burgerHash, pedLoc);
	ENTITY::SET_ENTITY_VISIBLE(burger, false, false);
	int leftWristID = PED::GET_PED_BONE_INDEX(ped, 60309);

	PlayAnimAndWait(ped, chooseAnimDict, chooseAnim, defaultAF);
	PlayAnim(ped, takeBurgerAnimDict, takeBurgerAnim, defaultAF);
	while (TASK::GET_SCRIPT_TASK_STATUS(ped, 0x87B9A382) != 7)
	{
		DisablePlayerActionsThisFrame(ped);
		ENTITY::SET_ENTITY_ANIM_SPEED(ped, takeBurgerAnimDict, takeBurgerAnim, 0.65f);
		if (!ENTITY::IS_ENTITY_ATTACHED(burger) && ENTITY::GET_ENTITY_ANIM_CURRENT_TIME(ped, takeBurgerAnimDict, takeBurgerAnim) > 0.175f)
		{
			ENTITY::SET_ENTITY_VISIBLE(burger, true, false);
			ENTITY::ATTACH_ENTITY_TO_ENTITY(burger, ped, leftWristID, 0.08f, 0.028f, 0.037f, -36.0f, 159.0f, 0.0f, true, true, false, false, 2, true);
			TASK::TASK_LOOK_AT_ENTITY(ped, burger, -1, SLF_SLOW_TURN_RATE | SLF_NARROW_YAW_LIMIT, 2);
		}
		if (ENTITY::GET_ENTITY_ANIM_CURRENT_TIME(ped, takeBurgerAnimDict, takeBurgerAnim) > 0.375f)
			TASK::STOP_ANIM_TASK(ped, takeBurgerAnimDict, takeBurgerAnim, -2.0f);
		WAIT(0);
	}
	PlayAmbientSpeech(vendor, "GENERIC_BYE");

	ENTITY::ATTACH_ENTITY_TO_ENTITY(burger, ped, leftWristID, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, true, true, false, false, 2, true);
	PlayAnimWithSpeedAndWait(ped, burgerAnimDict, burgerAnim, upperSecondaryAF, 0.65f);
	PlayAnimWithSpeedAndWait(ped, burgerAnimDict, burgerAnim, upperSecondaryAF, 0.65f);

	TASK::TASK_CLEAR_LOOK_AT(ped);
	DeleteObject(burger);
	STREAMING::REMOVE_ANIM_DICT(chooseAnimDict);
	STREAMING::REMOVE_ANIM_DICT(takeBurgerAnimDict);
	STREAMING::REMOVE_ANIM_DICT(burgerAnimDict);
	STREAMING::SET_MODEL_AS_NO_LONGER_NEEDED(burgerHash);
	return;
}