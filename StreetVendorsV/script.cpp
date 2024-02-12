#include <natives.h>
#include "script.h"
#include "utils\utils.h"
#include "utils\customer.h"

Customer playerCustomer;
Customer randomCustomer;

//This script is now ASYNCHRONOUS
static void update()
{
	Player player = PLAYER_ID();
	Ped playerPed = PLAYER_PED_ID();

	// Check if player ped exists and control is on (e.g. not in a cutscene)
	if (!DOES_ENTITY_EXIST(playerPed) || !IS_PLAYER_CONTROL_ON(player))
		return;
	
	playerCustomer.SetPed(playerPed);
	playerCustomer.UpdateSequence();

	//Random peds run the task every 30 seconds (by default)
	randomCustomer.UpdateSequence();

	Object stand = GetVendorStand(GET_ENTITY_COORDS(playerPed, false), 30.0f);
	if (stand != NULL)
	{
		Vector3 standLoc = GET_ENTITY_COORDS(stand, false);
		SET_SCENARIO_PEDS_TO_BE_RETURNED_BY_NEXT_COMMAND(true);
		Ped ped = GET_RANDOM_PED_AT_COORD(standLoc.x, standLoc.y, standLoc.z, 10.0f, 10.0f, 10.0f, -1);
		if (ped != NULL && ped != playerPed && !IsPedVendor(ped))
			randomCustomer.SetPed(ped);
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