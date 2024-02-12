#include <natives.h>
#include "script.h"
#include "utils\utils.h"
#include "utils\customer.h"

Customer playerCustomer;

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