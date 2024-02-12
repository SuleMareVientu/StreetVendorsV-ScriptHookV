#include <natives.h>
#include "customer.h"

static constexpr int busyVendorsMaxSize = 20;
static std::set<Ped> busyVendors = {};

bool Customer::PedExists()
{
	if (DOES_ENTITY_EXIST(ped) && IS_ENTITY_A_PED(ped))
		return true;

	return false;
}

bool Customer::IsPedPlayer()
{
	if (ped == PLAYER_PED_ID())
		return true;

	return false;
}

//https://alloc8or.re/gta5/doc/enums/eScriptTaskHash.txt
void Customer::PedTaskWalkToAndWait(float x, float y, float z, float heading, int nextState)
{
	//Different teleport time for player/peds
	if (IsPedPlayer())
		TASK_GO_STRAIGHT_TO_COORD(ped, x, y, z, 1.0f, 5000, heading, 0.1f);
	else
		TASK_GO_STRAIGHT_TO_COORD(ped, x, y, z, 1.0f, 30000, heading, 0.1f);

	sequenceState = WAITING_FOR_WALK_TASK_TO_END;
	nextSequenceState = nextState;
	return;
}

void Customer::PlayAnim(char* animDict, char* anim, int flag, int duration)
{
	TASK_PLAY_ANIM(ped, animDict, anim, 1.5f, -1.5f, duration, flag, 0.0f, false, false, false);
	return;
}

void Customer::PlayAnimAndWait(char* animDict, char* anim, int flag, int nextState, bool standStill, int duration)
{
	TASK_PLAY_ANIM(ped, animDict, anim, 1.5f, -1.5f, duration, flag, 0.0f, false, false, false);
	sequenceState = WAITING_FOR_ANIMATION_TO_END;
	nextSequenceState = nextState;
	shouldPlayerStandStill = standStill;
	lastAnimDict = animDict;
	lastAnim = anim;
	return;
}

void Customer::SetVendorBusy()
{
	if (sequenceState == FINISHED)
		busyVendors.erase(vendor);
	else
		busyVendors.emplace(vendor);
	return;
}

void Customer::StartSequence(int type)
{
	switch (type)
	{
	case HOTDOG:
		sequence = HOTDOG;
		sequenceState = STREAM_ASSETS_IN;
		PlayHotdogEatSequence();
		break;
	case BURGER:
		sequence = BURGER;
		sequenceState = STREAM_ASSETS_IN;
		PlayBurgerEatSequence();
		break;
	}
	return;
}

void Customer::PlayHotdogEatSequence()
{
	pedLoc = GET_ENTITY_COORDS(ped, false);
	int rightWristID = GET_PED_BONE_INDEX(ped, 28422);

	//Player control should be disabled here and not during the sequence
	if (IsPedPlayer())
	{
		if (sequenceState != STREAM_ASSETS_IN && sequenceState != FLUSH_ASSETS && sequenceState != FINISHED)
			DisablePlayerActionsThisFrame();

		if (sequenceState == WAITING_FOR_WALK_TASK_TO_END || sequenceState == INITIALIZED || sequenceState == TAKE)
			DisablePlayerControlThisFrame();

		if (sequenceState == WAITING_FOR_ANIMATION_TO_END)
		{
			if (shouldPlayerStandStill)
				DisablePlayerControlThisFrame();
		}
		else
			shouldPlayerStandStill = false;
	}
	else
	{
		SET_PED_SHOULD_PLAY_NORMAL_SCENARIO_EXIT(ped);
		if (sequenceState != FINISHED)
			SET_PED_CONFIG_FLAG(ped, 294, true);	//PCF_DisableShockingEvents
		else
			SET_PED_CONFIG_FLAG(ped, 294, false);	//PCF_DisableShockingEvents
	}

	switch (sequenceState)
	{
	case WALK_TO_VENDOR:
		if (IS_PED_WEARING_HELMET(ped))
			REMOVE_PED_HELMET(ped, true);

		//Prevent ped from equipping weapon during sequence
		SET_CURRENT_PED_WEAPON(ped, -1569615261, true);	//WEAPON_UNARMED
		PedTaskWalkToAndWait(destination.x, destination.y, destination.z, standHeading, INITIALIZED);
		break;
	case WAITING_FOR_WALK_TASK_TO_END:
		if (GET_SCRIPT_TASK_STATUS(ped, 2106541073) == 7)
		{
			if (IsPedPlayer())
				SetPlayerCash(ped, GetPlayerCash(ped), -5);

			PlayAmbientSpeech(ped, AmbDialogueBuy);
			sequenceState = nextSequenceState;
		}
		break;
	case INITIALIZED:
		food = CreateObject(hotdogHash);
		SET_ENTITY_AS_MISSION_ENTITY(food, true, true);
		PlayAnimAndWait(chooseAnimDict, chooseAnim, defaultAF, TAKE, true);
		break;
	case WAITING_FOR_ANIMATION_TO_END:
		if (!IS_ENTITY_PLAYING_ANIM(ped, lastAnimDict, lastAnim, 3))
			sequenceState = nextSequenceState;
		break;
	case TAKE:
		if (!IS_ENTITY_PLAYING_ANIM(ped, takeAnimDict, takeAnim, 3))
			PlayAnim(takeAnimDict, takeAnim, defaultAF);
		else
		{
			SET_ENTITY_ANIM_SPEED(ped, takeAnimDict, takeAnim, 0.65f);

			if (!IS_ENTITY_ATTACHED(food) && GET_ENTITY_ANIM_CURRENT_TIME(ped, takeAnimDict, takeAnim) > 0.225f)
			{
				ATTACH_ENTITY_TO_ENTITY(food, ped, rightWristID, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, true, true, false, false, 2, true);
				TASK_LOOK_AT_ENTITY(ped, food, -1, SLF_SLOW_TURN_RATE | SLF_NARROW_YAW_LIMIT, 2);
			}

			if (GET_ENTITY_ANIM_CURRENT_TIME(ped, takeAnimDict, takeAnim) > 0.5f)
			{
				STOP_ANIM_TASK(ped, takeAnimDict, takeAnim, -2.0f);
				PlayAmbientSpeech(vendor, AmbDialogueThanks);
				sequenceState = EATING;
				PlayAnim(eatingAnimDict, eatingStartAnim, upperSecondaryAF);
			}
		}
		break;
	case EATING:
		if (IS_ENTITY_PLAYING_ANIM(ped, eatingAnimDict, eatingStartAnim, 3))
		{
			if (GET_ENTITY_ANIM_CURRENT_TIME(ped, eatingAnimDict, eatingStartAnim) > 0.5f && GET_ENTITY_MODEL(food) != eatenHotdogHash)
			{
				CREATE_MODEL_SWAP(pedLoc.x, pedLoc.y, pedLoc.z, 10.0f, hotdogHash, eatenHotdogHash, true);
				ATTACH_ENTITY_TO_ENTITY(food, ped, rightWristID, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, true, true, false, false, 2, true);
			}
		}
		else
			PlayAnimAndWait(eatingAnimDict, eatingEndAnim, upperSecondaryAF, FLUSH_ASSETS);
		break;
	case FLUSH_ASSETS:
		DeleteObject(food);
		REMOVE_ANIM_DICT(chooseAnimDict);
		REMOVE_ANIM_DICT(takeAnimDict);
		REMOVE_ANIM_DICT(eatingAnimDict);
		SET_MODEL_AS_NO_LONGER_NEEDED(hotdogHash);
		SET_MODEL_AS_NO_LONGER_NEEDED(eatenHotdogHash);
		sequenceState = FINISHED;
		TASK_CLEAR_LOOK_AT(ped);
		SET_ENTITY_HEALTH(ped, GET_PED_MAX_HEALTH(ped), NULL);
		PlayAmbientSpeech(ped, AmbDialogueEat);
		ped = NULL;
		break;
	case STREAM_ASSETS_IN:
		if (RequestModel(hotdogHash) && RequestModel(eatenHotdogHash) && RequestAnimDict(chooseAnimDict) &&
			RequestAnimDict(takeAnimDict) && RequestAnimDict(eatingAnimDict))
		{
			sequenceState = WALK_TO_VENDOR;
			nextSequenceState = NULL;
			shouldPlayerStandStill = false;
			lastAnimDict = NULL;
			lastAnim = NULL;
			food = NULL;
		}
		break;
	}

	//Needs to be added at the end of the sequence
	SetVendorBusy();

	//Set player gestures
	if (sequenceState == FINISHED)
		SET_PED_CAN_PLAY_GESTURE_ANIMS(ped, true);
	else
		SET_PED_CAN_PLAY_GESTURE_ANIMS(ped, false);
	return;
}

void Customer::PlayBurgerEatSequence()
{
	pedLoc = GET_ENTITY_COORDS(ped, false);
	int leftWristID = GET_PED_BONE_INDEX(ped, 60309);

	//Stop sequence when timeout timer runs out (default 35 seconds)
	if (sequenceState == STREAM_ASSETS_IN || sequenceState == FINISHED)
		timeoutTimer.Set(0);
	else if (timeoutTimer.Get() > timeout)
		sequenceState = FINISHED;

	//Player control should be disabled here and not during the sequence
	if (IsPedPlayer())
	{
		if (sequenceState != STREAM_ASSETS_IN && sequenceState != FLUSH_ASSETS && sequenceState != FINISHED)
			DisablePlayerActionsThisFrame();

		if (sequenceState == WAITING_FOR_WALK_TASK_TO_END || sequenceState == INITIALIZED || sequenceState == TAKE)
			DisablePlayerControlThisFrame();

		if (sequenceState == WAITING_FOR_ANIMATION_TO_END)
		{
			if (shouldPlayerStandStill)
				DisablePlayerControlThisFrame();
		}
		else
			shouldPlayerStandStill = false;
	}
	else
	{
		SET_PED_SHOULD_PLAY_NORMAL_SCENARIO_EXIT(ped);
		if (sequenceState != FINISHED)
			SET_PED_CONFIG_FLAG(ped, 294, true);	//PCF_DisableShockingEvents
		else
			SET_PED_CONFIG_FLAG(ped, 294, false);	//PCF_DisableShockingEvents
	}

	switch (sequenceState)
	{
	case WALK_TO_VENDOR:
		if (IS_PED_WEARING_HELMET(ped))
			REMOVE_PED_HELMET(ped, true);

		//Prevent ped from equipping weapon during sequence
		SET_CURRENT_PED_WEAPON(ped, -1569615261, true);	//WEAPON_UNARMED
		PedTaskWalkToAndWait(destination.x, destination.y, destination.z, standHeading, INITIALIZED);
		break;
	case WAITING_FOR_WALK_TASK_TO_END:
		if (GET_SCRIPT_TASK_STATUS(ped, 2106541073) == 7)
		{
			if (IsPedPlayer())
				SetPlayerCash(ped, GetPlayerCash(ped), -5);

			PlayAmbientSpeech(ped, AmbDialogueBuy);
			sequenceState = nextSequenceState;
		}
		break;
	case INITIALIZED:
		food = CreateObject(burgerHash);
		SET_ENTITY_AS_MISSION_ENTITY(food, true, true);
		PlayAnimAndWait(chooseAnimDict, chooseAnim, defaultAF, TAKE, true);
		break;
	case WAITING_FOR_ANIMATION_TO_END:
		if (!IS_ENTITY_PLAYING_ANIM(ped, lastAnimDict, lastAnim, 3))
			sequenceState = nextSequenceState;
		else if (lastAnimDict == burgerAnimDict && lastAnim == burgerAnim)	//Special case for burger eating anim
			SET_ENTITY_ANIM_SPEED(ped, burgerAnimDict, burgerAnim, 0.65f);
		break;
	case TAKE:
		if (!IS_ENTITY_PLAYING_ANIM(ped, takeBurgerAnimDict, takeBurgerAnim, 3))
			PlayAnim(takeBurgerAnimDict, takeBurgerAnim, defaultAF);
		else
		{
			SET_ENTITY_ANIM_SPEED(ped, takeBurgerAnimDict, takeBurgerAnim, 0.65f);

			if (!IS_ENTITY_ATTACHED(food) && GET_ENTITY_ANIM_CURRENT_TIME(ped, takeBurgerAnimDict, takeBurgerAnim) > 0.225f)
			{
				ATTACH_ENTITY_TO_ENTITY(food, ped, leftWristID, 0.08f, 0.028f, 0.037f, -36.0f, 159.0f, 0.0f, true, true, false, false, 2, true);
				TASK_LOOK_AT_ENTITY(ped, food, -1, SLF_SLOW_TURN_RATE | SLF_NARROW_YAW_LIMIT, 2);
			}

			if (GET_ENTITY_ANIM_CURRENT_TIME(ped, takeBurgerAnimDict, takeBurgerAnim) > 0.375f)
			{
				STOP_ANIM_TASK(ped, takeBurgerAnimDict, takeBurgerAnim, -2.0f);
				PlayAmbientSpeech(vendor, AmbDialogueThanks);
				sequenceState = EATING;
				ATTACH_ENTITY_TO_ENTITY(food, ped, leftWristID, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, true, true, false, false, 2, true);	//Reset burger relative rotation
				PlayAnim(burgerAnimDict, burgerAnim, upperSecondaryAF);
			}
		}
		break;
	case EATING:
		if (!IS_ENTITY_PLAYING_ANIM(ped, burgerAnimDict, burgerAnim, 3))
			PlayAnimAndWait(burgerAnimDict, burgerAnim, upperSecondaryAF, FLUSH_ASSETS);
		else
			SET_ENTITY_ANIM_SPEED(ped, burgerAnimDict, burgerAnim, 0.65f);
		break;
	case FLUSH_ASSETS:
		DeleteObject(food);
		REMOVE_ANIM_DICT(chooseAnimDict);
		REMOVE_ANIM_DICT(takeBurgerAnimDict);
		REMOVE_ANIM_DICT(burgerAnimDict);
		SET_MODEL_AS_NO_LONGER_NEEDED(burgerHash);
		sequenceState = FINISHED;
		TASK_CLEAR_LOOK_AT(ped);
		SET_ENTITY_HEALTH(ped, GET_PED_MAX_HEALTH(ped), NULL);
		PlayAmbientSpeech(ped, AmbDialogueEat);
		ped = NULL;
		break;
	case STREAM_ASSETS_IN:
		if (RequestModel(burgerHash) && RequestAnimDict(chooseAnimDict) &&
			RequestAnimDict(takeBurgerAnimDict) && RequestAnimDict(burgerAnimDict))
		{
			sequenceState = WALK_TO_VENDOR;
			nextSequenceState = NULL;
			shouldPlayerStandStill = false;
			lastAnimDict = NULL;
			lastAnim = NULL;
			food = NULL;
		}
		break;
	}

	//Needs to be added at the end of the sequence
	SetVendorBusy();

	//Set player gestures
	if (sequenceState == FINISHED)
		SET_PED_CAN_PLAY_GESTURE_ANIMS(ped, true);
	else
		SET_PED_CAN_PLAY_GESTURE_ANIMS(ped, false);
	return;
}

//////////////////////////////PUBLIC//////////////////////////////

void Customer::SetPed(Ped newPed)
{
	if (DOES_ENTITY_EXIST(newPed) && IS_ENTITY_A_PED(newPed) && sequenceState == FINISHED)
	{
		if (newPed == PLAYER_PED_ID())
			ped = newPed;
		else if (!IS_ENTITY_A_MISSION_ENTITY(newPed))
			ped = newPed;
	}
	return;
}

void Customer::UpdateSequence()
{
	//Clear set when it exceeds busyVendorsMaxSize
	if (busyVendors.size() > busyVendorsMaxSize)
		busyVendors.clear();

	if (!PedExists() || IS_ENTITY_DEAD(ped, false) || IS_PED_DEAD_OR_DYING(ped, true) || IS_PED_INJURED(ped))
	{
		ped = NULL;
		sequenceState = FINISHED;
		SetVendorBusy();
		return;
	}

	//Prevent peds from blocking vendor and other customers	DO NOT RETURN
	if (IS_PED_USING_ANY_SCENARIO(ped))
		sequenceState = FINISHED;

	if (sequenceState != FINISHED)
	{
		intervalTimer.Set(0); //Reset ped timer
		switch (sequence)
		{
		case HOTDOG:
			PlayHotdogEatSequence();	break;
		case BURGER:
			PlayBurgerEatSequence();	break;
		}
		return;
	}
	else
	{
		//Return if not enough time has passed since last costumer
		if (intervalTimer.Get() < interval && !IsPedPlayer())
			return;
	}

	if (IsPedPlayer())
	{
		//Checks if the vendor isn't busy
		if (busyVendors.find(vendor) != busyVendors.end())
			return;

		pedLoc = GET_ENTITY_COORDS(ped, false);
		Object stand = GetVendorStand(pedLoc, 5.0f);

		//Don't waste resources if stand isn't found...
		if (stand == NULL)
			return;

		//Initialize vars
		Vector3 standLoc = GET_ENTITY_COORDS(stand, false);
		Vector3 standForwardVec = GET_ENTITY_FORWARD_VECTOR(stand);
		standHeading = GET_ENTITY_HEADING(stand);
		destination = GET_OFFSET_FROM_ENTITY_IN_WORLD_COORDS(stand, -0.5f, -0.75f, 0.0f);
		int playerCash = GetPlayerCash(ped);
		//Get whether the stand is for hotdogs or burgers: NULL, 1 = hotdog stand, 2 = burger stand
		int standType = GetStandType(stand);

		//Checks if player is nwDADear vendor, prints hotdog message and checks if he's in a correct state to start eat sequence, else returns
		if (VDIST2(pedLoc.x, pedLoc.y, pedLoc.z, destination.x, destination.y, destination.z) > 4.0f)
			return;

		//Checks if the vendor is near the player, if hotdog message can be printed (ped isn't wanted & has money) and 
		//if the player isn't falling, jumping etc.	KEEP IN ORDER!
		if (!FindVendor(stand, &vendor) || busyVendors.count(vendor) != 0 || !PrintVendorMessage(playerCash, standType) || !AdditionalChecks(ped))
			return;

		SET_PED_CONFIG_FLAG(vendor, 329, true);	//PCF_DisableTalkTo

		if (IS_DISABLED_CONTROL_JUST_PRESSED(2, INPUT_CONTEXT))
		{
			CLEAR_ALL_HELP_MESSAGES();
			PlayAmbientSpeech(vendor, AmbDialogueHi);
			//Play desired eat sequence based on stand type
			StartSequence(standType);
		}
		return;
	}
	else
	{
		pedLoc = GET_ENTITY_COORDS(ped, false);
		Object stand = GetVendorStand(pedLoc, 20.0f);

		//Don't waste resources if stand isn't found...
		if (stand == NULL)
			return;

		//Initialize vars
		Vector3 standLoc = GET_ENTITY_COORDS(stand, false);
		Vector3 standForwardVec = GET_ENTITY_FORWARD_VECTOR(stand);
		standHeading = GET_ENTITY_HEADING(stand);
		destination = GET_OFFSET_FROM_ENTITY_IN_WORLD_COORDS(stand, -0.5f, -0.75f, 0.0f);
		//Get whether the stand is for hotdogs or burgers: NULL, 1 = hotdog stand, 2 = burger stand
		int standType = GetStandType(stand);

		//Checks if the vendor is near the ped and he's not busy
		//if the ped isn't falling, jumping etc.	KEEP IN ORDER!
		if (!FindVendor(stand, &vendor) || busyVendors.count(vendor) != 0 || !AdditionalChecks(ped))
			return;

		StartSequence(standType);
	}
	return;
}