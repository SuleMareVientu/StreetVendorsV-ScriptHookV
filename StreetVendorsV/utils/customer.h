#pragma once

#include <types.h>
#include "utils.h"

static constexpr char* chooseAnimDict = "gestures@m@standing@casual";
static constexpr char* chooseAnim = "gesture_you_soft";
static constexpr char* takeAnimDict = "mp_common";
static constexpr char* takeAnim = "givetake1_a";
static constexpr char* takeBurgerAnimDict = "mp_doorbell";
static constexpr char* takeBurgerAnim = "ring_bell_a_left";
static constexpr char* eatingAnimDict = "amb@code_human_wander_eating_donut@male@idle_a";
static constexpr char* eatingStartAnim = "idle_b";
static constexpr char* eatingEndAnim = "idle_a";
static constexpr char* burgerAnimDict = "mp_player_inteat@burger";
static constexpr char* burgerAnim = "mp_player_int_eat_burger";

//Audio
static constexpr char* AmbDialogueHi = "GENERIC_HI";
static constexpr char* AmbDialogueBye = "GENERIC_BYE";
static constexpr char* AmbDialogueThanks = "GENERIC_THANKS";
static constexpr char* AmbDialogueBuy = "GENERIC_BUY";
static constexpr char* AmbDialogueEat = "GENERIC_EAT";

enum eEatSequenceState
{
	STREAM_ASSETS_IN,
	WALK_TO_VENDOR,
	WAITING_FOR_WALK_TASK_TO_END,
	INITIALIZED,
	WAITING_FOR_ANIMATION_TO_END,
	TAKE,
	EATING,
	FLUSH_ASSETS,
	FINISHED
};

class Customer {
	Ped ped = NULL;
	Ped vendor = NULL;
	const int timeout = 27500; // in ms
	Timer timeoutTimer;
	bool shouldPlayerStandStill = false;
	int sequence = -1;
	int sequenceState = FINISHED;
	int nextSequenceState = NULL;
	char* lastAnimDict = NULL;
	char* lastAnim = NULL;
	float standHeading = NULL;
	Vector3 pedLoc = { NULL, NULL, NULL, NULL, NULL, NULL };
	Vector3 destination = { NULL, NULL, NULL, NULL, NULL, NULL };
	Object food = NULL;
	bool PedExists();
	void PedTaskWalkToAndWait(float x, float y, float z, float heading, int nextState);
	void PlayAnim(char* animDict, char* anim, int flag, int duration = -1);
	void PlayAnimAndWait(char* animDict, char* anim, int flag, int nextState, bool standStill = false, int duration = -1);
	void StartSequence(int type);
	void PlayHotdogEatSequence();
	void PlayBurgerEatSequence();

public:
	void SetPed(Ped newPed);
	void UpdateSequence();
};