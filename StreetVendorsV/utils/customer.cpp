#include <natives.h>
#include "customer.h"

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

void Customer::SetVendorBusy(bool toggle)
{
	if (vendor == NULL)
		return;

	if (toggle)
		busyVendors.emplace(vendor);
	else
		busyVendors.erase(vendor);
	return;
}

//https://alloc8or.re/gta5/doc/enums/eScriptTaskHash.txt
void Customer::PedTaskWalkToAndWait(float x, float y, float z, float heading, int nextState)
{
	if (IsPedPlayer())
		TASK_GO_STRAIGHT_TO_COORD(ped, x, y, z, 1.0f, 10000, heading, 0.1f);
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

void Customer::SequenceFailsafe()
{
	//Stop sequence when timeout timer runs out (default 35 seconds)
	if (sequenceState == STREAM_ASSETS_IN || sequenceState == FINISHED)
		timeoutTimer.Set(0);
	else if (timeoutTimer.Get() > timeout)
	{
		sequenceState = FINISHED;
		DeleteObject(food);
		SetPedReactions();
	}
	return;
}

void Customer::SetPlayerControls()
{
	if (!IsPedPlayer())
		return;

	//Hide Phone and mobile browser
	if (!disabledControlsLastFrame)
	{
		SET_CONTROL_VALUE_NEXT_FRAME(FRONTEND_CONTROL, INPUT_CELLPHONE_CANCEL, 1.0f);
		SET_CONTROL_VALUE_NEXT_FRAME(FRONTEND_CONTROL, INPUT_CURSOR_CANCEL, 1.0f);
		disabledControlsLastFrame = true;
	}
	else
		disabledControlsLastFrame = false;

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

	return;
}

void Customer::SetPedReactions()
{
	if (sequenceState == FINISHED)
	{
		SET_PED_CAN_PLAY_GESTURE_ANIMS(ped, true);

		if (!IsPedPlayer())
		{
			SET_PED_CONFIG_FLAG(ped, PCF_DisableShockingEvents, false);
			SET_BLOCKING_OF_NON_TEMPORARY_EVENTS(ped, false);
		}
	}
	else
	{
		SET_PED_CAN_PLAY_GESTURE_ANIMS(ped, false);

		if (!IsPedPlayer())
		{
			SET_PED_CONFIG_FLAG(ped, PCF_DisableShockingEvents, true);
			SET_BLOCKING_OF_NON_TEMPORARY_EVENTS(ped, true);
		}
	}

	return;
}

void Customer::PlayHotdogEatSequence()
{
	pedLoc = GET_ENTITY_COORDS(ped, false);
	int rightWristID = GET_PED_BONE_INDEX(ped, 28422);

	SequenceFailsafe();
	SetPlayerControls(); //Player control should be disabled here and not during the sequence

	switch (sequenceState)
	{
	case WALK_TO_VENDOR:
		if (IS_PED_WEARING_HELMET(ped))
			REMOVE_PED_HELMET(ped, true);

		//Prevent ped from equipping weapon during sequence
		SET_CURRENT_PED_WEAPON(ped, WEAPON_UNARMED, true);
		PedTaskWalkToAndWait(destination.x, destination.y, destination.z, standHeading, INITIALIZED);
		break;
	case WAITING_FOR_WALK_TASK_TO_END:
		if (GET_SCRIPT_TASK_STATUS(ped, SCRIPT_TASK_GO_STRAIGHT_TO_COORD) == 7)
		{
			SetPlayerCash(ped, GetPlayerCash(ped), -5);
			PlayAmbientSpeech(ped, AmbDialogueBuy);
			sequenceState = nextSequenceState;
		}
		break;
	case INITIALIZED:
		food = CreateObject(hotdogHash);
		SET_ENTITY_AS_MISSION_ENTITY(food, true, true);
		PlayAnimAndWait(chooseAnimDict, chooseAnim, upperAF, TAKE, true);
		break;
	case WAITING_FOR_ANIMATION_TO_END:
		if (!IS_ENTITY_PLAYING_ANIM(ped, lastAnimDict, lastAnim, 3))
			sequenceState = nextSequenceState;
		break;
	case TAKE:
		if (!IS_ENTITY_PLAYING_ANIM(ped, takeAnimDict, takeAnim, 3))
			PlayAnim(takeAnimDict, takeAnim, upperAF);
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
		{
			// Free the vendor sooner for NPCs so the player can eat too
			if (!IsPedPlayer())
			{
				SetVendorBusy(false);
				vendor = NULL;
			}
			PlayAnimAndWait(eatingAnimDict, eatingEndAnim, upperSecondaryAF, FLUSH_ASSETS);
		}
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

	//Disable ped gestures and block non-player peds from reacting to temporary events
	SetPedReactions();
	return;
}

void Customer::PlayBurgerEatSequence()
{
	pedLoc = GET_ENTITY_COORDS(ped, false);
	int leftWristID = GET_PED_BONE_INDEX(ped, 60309);

	SequenceFailsafe();
	SetPlayerControls(); //Player control should be disabled here and not during the sequence

	switch (sequenceState)
	{
	case WALK_TO_VENDOR:
		if (IS_PED_WEARING_HELMET(ped))
			REMOVE_PED_HELMET(ped, true);

		//Prevent ped from equipping weapon during sequence
		SET_CURRENT_PED_WEAPON(ped, WEAPON_UNARMED, true);
		PedTaskWalkToAndWait(destination.x, destination.y, destination.z, standHeading, INITIALIZED);
		break;
	case WAITING_FOR_WALK_TASK_TO_END:
		if (GET_SCRIPT_TASK_STATUS(ped, SCRIPT_TASK_GO_STRAIGHT_TO_COORD) == 7)
		{
			SetPlayerCash(ped, GetPlayerCash(ped), -5);
			PlayAmbientSpeech(ped, AmbDialogueBuy);
			sequenceState = nextSequenceState;
		}
		break;
	case INITIALIZED:
		food = CreateObject(burgerHash);
		SET_ENTITY_AS_MISSION_ENTITY(food, true, true);
		PlayAnimAndWait(chooseAnimDict, chooseAnim, upperAF, TAKE, true);
		break;
	case WAITING_FOR_ANIMATION_TO_END:
		if (!IS_ENTITY_PLAYING_ANIM(ped, lastAnimDict, lastAnim, 3))
			sequenceState = nextSequenceState;
		else if (lastAnimDict == burgerAnimDict && lastAnim == burgerAnim)	//Slow down burger eating anim
			SET_ENTITY_ANIM_SPEED(ped, burgerAnimDict, burgerAnim, 0.65f);
		break;
	case TAKE:
		if (!IS_ENTITY_PLAYING_ANIM(ped, takeBurgerAnimDict, takeBurgerAnim, 3))
			PlayAnim(takeBurgerAnimDict, takeBurgerAnim, upperAF);
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

	//Disable ped gestures and block non-player peds from reacting to temporary events
	SetPedReactions();
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

	if (!PedExists() || IS_ENTITY_DEAD(ped, false) || IS_PED_DEAD_OR_DYING(ped, true) || IS_PED_INJURED(ped)
		|| IS_PED_IN_MELEE_COMBAT(ped) || COUNT_PEDS_IN_COMBAT_WITH_TARGET(ped) > 0 || IS_PED_USING_ANY_SCENARIO(ped))
	{
		ped = NULL;
		sequenceState = FINISHED;
		return;
	}

	if (sequenceState != FINISHED)
	{
		if (!IsPedPlayer())
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
		SetVendorBusy(false);
		vendor = NULL;
		//Return if not enough time has passed since last costumer
		if (intervalTimer.Get() < interval && !IsPedPlayer())
			return;
	}

	DeleteObject(food); //Force delete old food just in case it's still attached
	pedLoc = GET_ENTITY_COORDS(ped, false);
	Object stand = GetVendorStand(pedLoc, 5.0f);
	Ped tmpVendor = NULL;
	//Don't waste resources if stand or vendor aren't found
	if (stand == NULL || !FindVendor(stand, &tmpVendor))
		return;

	//Checks if the vendor isn't busy
	if (busyVendors.find(tmpVendor) != busyVendors.end())
		return;

	//Initialize vars
	Vector3 standLoc = GET_ENTITY_COORDS(stand, false);
	Vector3 standForwardVec = GET_ENTITY_FORWARD_VECTOR(stand);
	standHeading = GET_ENTITY_HEADING(stand);
	destination = GET_OFFSET_FROM_ENTITY_IN_WORLD_COORDS(stand, -0.5f, -0.75f, 0.0f);
	//Get whether the stand is for hotdogs or burgers: NULL, 1 = hotdog stand, 2 = burger stand
	int standType = GetStandType(stand);

	if (IsPedPlayer())
	{
		//Checks if player is near the vendor, prints hotdog message and checks if he's in a correct state to start eat sequence, else returns
		if (VDIST2(pedLoc.x, pedLoc.y, pedLoc.z, destination.x, destination.y, destination.z) > 4.0f)
			return;

		//Checks if hotdog message can be printed (ped isn't wanted & has money) and 
		//if the player isn't falling, jumping etc.	KEEP IN ORDER!
		if (!PrintVendorMessage(GetPlayerCash(ped), standType) || !AdditionalChecks(ped))
			return;

		// Stop vendor from freaking out
		SET_PED_CONFIG_FLAG(tmpVendor, PCF_DisableTalkTo, true);
		REMOVE_PED_DEFENSIVE_AREA(tmpVendor, false);
		REMOVE_PED_DEFENSIVE_AREA(tmpVendor, true);

		if (IS_DISABLED_CONTROL_JUST_PRESSED(FRONTEND_CONTROL, INPUT_CONTEXT))
		{
			CLEAR_ALL_HELP_MESSAGES();
			vendor = tmpVendor;
			SetVendorBusy(true);
			PlayAmbientSpeech(vendor, AmbDialogueHi);
			//Play desired eat sequence based on stand type
			StartSequence(standType);
		}
	}
	else
	{
		//Checks if the ped isn't falling, jumping etc.
		if (AdditionalChecks(ped))
		{
			vendor = tmpVendor;
			SetVendorBusy(true);
			StartSequence(standType);
		}
	}
	return;
}