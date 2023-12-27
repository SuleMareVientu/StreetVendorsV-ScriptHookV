#include <natives.h>
#include "utils.h"
#include "script.h"

static bool alreadyShownCash = false;

//This script is SYNCHRONOUS, for better or for worse, keep that in mind
static void update()
{
	Player player = PLAYER::PLAYER_ID();
	Ped playerPed = PLAYER::PLAYER_PED_ID();

	// Check if player ped exists and control is on (e.g. not in a cutscene)
	if (!ENTITY::DOES_ENTITY_EXIST(playerPed) || !PLAYER::IS_PLAYER_CONTROL_ON(player))
		return;

	//Fix showing cash change for the first time
	if (!alreadyShownCash)
	{
		HUD::REMOVE_MULTIPLAYER_HUD_CASH();
		HUD::CHANGE_FAKE_MP_CASH(-5, NULL);
		alreadyShownCash = true;
	}

	Vector3 playerLoc = ENTITY::GET_ENTITY_COORDS(playerPed, false);
	Object stand = GetVendorStand(playerLoc, 5.0f);

	//Don't waste resources if stand isn't found...
	if (stand == NULL)
		return;

	//Initialize vars
	Vector3 standLoc = ENTITY::GET_ENTITY_COORDS(stand, false);
	Vector3 standForwardVec = ENTITY::GET_ENTITY_FORWARD_VECTOR(stand);
	float standHeading = ENTITY::GET_ENTITY_HEADING(stand);
	Vector3 destination = ENTITY::GET_OFFSET_FROM_ENTITY_IN_WORLD_COORDS(stand, -0.5f, -0.75f, 0.0f);
	int playerCash = GetPlayerCash(playerPed);
	//Get whether the stand is for hotdogs or burgers: NULL, 1 = hotdog stand, 2 = burger stand
	int standType = GetStandType(stand);

	//Checks if player is near vendor, prints hotdog message and checks if he's in a correct state to start eat sequence, else returns
	if (SYSTEM::VDIST2(playerLoc.x, playerLoc.y, playerLoc.z, destination.x, destination.y, destination.z) > 4.0f)
		return;

	//Checks if the vendor is near the player, if hotdog message can be printed (ped isn't wanted & has money) and 
	//if the player isn't falling, jumping etc.	KEEP IN ORDER!
	if (!FindVendor(stand) || !PrintVendorMessage(playerCash, standType) || !AdditionalChecks(playerPed))
		return;

	PAD::DISABLE_CONTROL_ACTION(0, INPUT_TALK, true);	//Stops player from talking to other random peds (this frame)
	PED::SET_PED_CONFIG_FLAG(vendor, 329, true);		//PCF_DisableTalkTo

	if (PAD::IS_DISABLED_CONTROL_JUST_PRESSED(2, INPUT_CONTEXT))
	{
		HUD::CLEAR_ALL_HELP_MESSAGES();
		if (PED::IS_PED_WEARING_HELMET(playerPed))
			PED::REMOVE_PED_HELMET(playerPed, true);
		//Prevent player from doing unwanted things during the sequence
		SetPlayerControl(playerPed, false);
		WEAPON::SET_CURRENT_PED_WEAPON(playerPed, -1569615261, true);	//WEAPON_UNARMED
		PlayAmbientSpeech(vendor, "GENERIC_HI");
		PedTaskWalkToAndWait(playerPed, destination.x, destination.y, destination.z, standHeading);
		PlayAmbientSpeech(playerPed, "GENERIC_BUY");
		SetPlayerCash(playerPed, playerCash, -5);
		//Play desired eat sequence based on stand type
		switch (standType)
		{
		case 1:
			PlayHotdogEatSequence(playerPed);	break;
		case 2:
			PlayBurgerEatSequence(playerPed);	break;
		}
		SetPlayerControl(playerPed, true);
		ENTITY::SET_ENTITY_HEALTH(playerPed, PED::GET_PED_MAX_HEALTH(playerPed), NULL);
	}
	return;
}

void ScriptMain()
{
	while (true)
	{
		update();
		WAIT(0);
	}
	return;
}