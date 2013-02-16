
#include "Main.h"

#define GenFloorBase				0x6000
#define GenCeilingBase			0x4000
#define GenDoorBase				0x3c00
#define GenLockedBase			0x3800
#define GenLiftBase				0x3400
#define GenStairsBase			0x3000
#define GenCrusherBase			0x2F80

#define TriggerType				0x0007
#define TriggerTypeShift			0

#define FloorCrush				0x1000
#define FloorChange				0x0c00
#define FloorTarget				0x0380
#define FloorDirection			0x0040
#define FloorModel				0x0020
#define FloorSpeed				0x0018

#define FloorCrushShift			12
#define FloorChangeShift			10
#define FloorTargetShift			7
#define FloorDirectionShift		6
#define FloorModelShift			5
#define FloorSpeedShift			3

#define CeilingCrush				0x1000
#define CeilingChange			0x0c00
#define CeilingTarget			0x0380
#define CeilingDirection			0x0040
#define CeilingModel				0x0020
#define CeilingSpeed				0x0018

#define CeilingCrushShift		12
#define CeilingChangeShift		10
#define CeilingTargetShift		7
#define CeilingDirectionShift	6
#define CeilingModelShift		5
#define CeilingSpeedShift		3

#define LiftTarget				0x0300
#define LiftDelay				0x00c0
#define LiftMonster				0x0020
#define LiftSpeed				0x0018

#define LiftTargetShift			8
#define LiftDelayShift			6
#define LiftMonsterShift			5
#define LiftSpeedShift			3

#define StairIgnore				0x0200
#define StairDirection			0x0100
#define StairStep				0x00c0
#define StairMonster				0x0020
#define StairSpeed				0x0018

#define StairIgnoreShift			9
#define StairDirectionShift		8
#define StairStepShift			6
#define StairMonsterShift		5
#define StairSpeedShift			3

#define CrusherSilent			0x0040
#define CrusherMonster			0x0020
#define CrusherSpeed				0x0018

#define CrusherSilentShift		6
#define CrusherMonsterShift		5
#define CrusherSpeedShift		3

#define DoorDelay				0x0300
#define DoorMonster				0x0080
#define DoorKind					0x0060
#define DoorSpeed				0x0018

#define DoorDelayShift			8
#define DoorMonsterShift			7
#define DoorKindShift			5
#define DoorSpeedShift			3

#define LockedNKeys				0x0200
#define LockedKey				0x01c0
#define LockedKind				0x0020
#define LockedSpeed				0x0018

#define LockedNKeysShift			9
#define LockedKeyShift			6
#define LockedKindShift			5
#define LockedSpeedShift			3

namespace BoomGenLineSpecial {
	static const char *Triggers[]=
	{"W1","WR","S1","SR","G1","GR","D1","DR",};

	static const char *FloorTargets[]=
	{"F->HnF","F->LnF","F->NnF","F->LnC","F->C","FbysT","Fby24","Fby32",};

	static const char *Directions[]=
	{"Down","Up",};

	static const char *Speeds[]=
	{"Slow","Normal","Fast","Turbo",};

	static const char *Changers[]=
	{"","c0","cT","cS",};

	static const char *Models[]=
	{"t","n",};

	static const char *Crushers[]=
	{"","Cr",};

	static const char *CeilingTargets[]=
	{"C->HnC","C->LnC","C->NnC","C->LnF","C->F","CbysT","Cby24","Cby32",};

	static const char *Doors1[]=
	{"OpnD","Opn","ClsD","Cls",};

	static const char *Doors2[]=
	{"Cls","","Opn","",};

	static const char *Delays[]=
	{"1","4","9","30",};

	static const char *LockedDelays[]=
	{"4",};

	static const char *Locks[]=
	{"Any Key","Red Card","Blue Card","Yellow Card","Red Skull","Blue Skull","Yellow Skull","All 6 Keys",
	 "Any Key","Red Key","Blue Key" "Yellow Key","Red Key","Blue Key","Yellow Key","All 3 Keys",};

	static const char *LiftTargets[]=
	{"F->LnF","F->NnF","F->LnC","LnF-HnF",};

	static const char *LiftDelays[]=
	{"1","3","5","10",};

	static const char *Steps[]=
	{"s4","s8","s16","s24",};

	string parseLineType(int type) {
		static char linedes[32];

		strcpy(linedes,"");
		if (type>=0x6000)			 // Floor type
		{
			// put in trigger type
			strcat(linedes,Triggers[type&TriggerType]);
			if (!(type&FloorChange) && (type&FloorModel))
				strcat(linedes,"m");
			strcat(linedes," ");

			// put in target description
			strcat(linedes,FloorTargets[(type&FloorTarget)>>FloorTargetShift]);
			strcat(linedes," ");

			// put in direction and speed
			strcat(linedes,Directions[(type&FloorDirection)>>FloorDirectionShift]);
			strcat(linedes,Speeds[(type&FloorSpeed)>>FloorSpeedShift]);

			// if a changer, put in change type and model
			if (type&FloorChange)
			{
				strcat(linedes," ");
				strcat(linedes,Changers[(type&FloorChange)>>FloorChangeShift]);
				strcat(linedes,Models[(type&FloorModel)>>FloorModelShift]);
			}
			// if a crusher, put in crusher indicator
			if (type&FloorCrush)
			{
				strcat(linedes," ");
				strcat(linedes,Crushers[(type&FloorCrush)>>FloorCrushShift]);
			}
		}
	  else if (type>=0x4000) // Ceiling type
		{
			// put in trigger type
			strcat(linedes,Triggers[type&TriggerType]);
			if (!(type&CeilingChange) && (type&CeilingModel))
				strcat(linedes,"m");
			strcat(linedes," ");

			// put in target description
			strcat(linedes,CeilingTargets[(type&CeilingTarget)>>CeilingTargetShift]);
			strcat(linedes," ");

			// put in direction and speed
			strcat(linedes,Directions[(type&CeilingDirection)>>CeilingDirectionShift]);
			strcat(linedes,Speeds[(type&CeilingSpeed)>>CeilingSpeedShift]);

			// if a changer, put in change type and model
			if (type&CeilingChange)
			{
				strcat(linedes," ");
				strcat(linedes,Changers[(type&CeilingChange)>>CeilingChangeShift]);
				strcat(linedes,Models[(type&CeilingModel)>>CeilingModelShift]);
			}
			// if a crusher, put in crusher indicator
			if (type&CeilingCrush)
			{
				strcat(linedes," ");
				strcat(linedes,Crushers[(type&CeilingCrush)>>CeilingCrushShift]);
			}
		}
		else if (type>=0x3c00) // Door type
		{
			string type_string;

			// Trigger
			int trigger = type & TriggerType;
			type_string += Triggers[trigger];
			if (type & DoorMonster)
				type_string += "M";

			type_string += " Door ";

			// Door kind
			int kind = (type & DoorKind) >> DoorKindShift;
			int delay = (type & DoorDelay) >> DoorDelayShift;
			switch (kind) {
				case 0: type_string += S_FMT("Open Wait %s Close", Delays[delay]); break;
				case 1: type_string += "Open Stay"; break;
				case 2: type_string += S_FMT("Close Wait %s Open", Delays[delay]); break;
				case 3: type_string += "Close Stay"; break;
				default: break;
			}

			// Door speed
			int speed = (type & DoorSpeed) >> DoorSpeedShift;
			type_string += S_FMT(" %s", Speeds[speed]);

			return type_string;
		}
		else if (type>=0x3800) // Locked Door type
		{
			string type_string;

			// Trigger
			int trigger = type & TriggerType;
			type_string += Triggers[trigger];

			type_string += " Door ";

			// Lock
			int key = (type & LockedKey) >> LockedKeyShift;
			int num = (type & LockedNKeys) >> LockedNKeysShift;
			type_string += S_FMT("%s ", Locks[num*8+key]);

			// Door kind
			int kind = (type & LockedKind) >> LockedKindShift;
			switch (kind) {
				case 0: type_string += "Open Wait 4 Close"; break;
				case 1: type_string += "Open Stay"; break;
				case 2: type_string += "Close Wait 4 Open"; break;
				case 3: type_string += "Close Stay"; break;
				default: break;
			}

			// Door speed
			int speed = (type & DoorSpeed) >> DoorSpeedShift;
			type_string += S_FMT(" %s", Speeds[speed]);

			return type_string;
		}
		else if (type>=0x3400) // Lift type
		{
			// put in trigger type
			strcat(linedes,Triggers[type&TriggerType]);
			if (type&LiftMonster)
				strcat(linedes,"m");
			strcat(linedes," Lft ");

			// put in lift target and delay
			strcat(linedes,LiftTargets[(type&LiftTarget)>>LiftTargetShift]);
			strcat(linedes," ");
			strcat(linedes,"D");
			strcat(linedes,LiftDelays[(type&LiftDelay)>>LiftDelayShift]);

			// put in lift speed
			strcat(linedes,Speeds[(type&LiftSpeed)>>LiftSpeedShift]);
		}
		else if (type>=0x3000) // Stairs type
		{
			// put in trigger type
			strcat(linedes,Triggers[type&TriggerType]);
			if (type&StairMonster)
				strcat(linedes,"m");
			strcat(linedes," Stair");

			// put in direction
			strcat(linedes,Directions[(type&StairDirection)>>StairDirectionShift]);

			// put in stepsize
			strcat(linedes,Steps[(type&StairStep)>>StairStepShift]);

			// put in speed
			strcat(linedes,Speeds[(type&StairSpeed)>>StairSpeedShift]);

			if (type&StairIgnore)
				strcat(linedes," Ign");
		}
		else if (type>=0x2F80) // Crusher type
		{
			// put in trigger type
			strcat(linedes,Triggers[type&TriggerType]);
			if (type&CrusherMonster)
				strcat(linedes,"m");
			strcat(linedes," Crusher ");

			// put in speed
			strcat(linedes,Speeds[(type&CrusherSpeed)>>CrusherSpeedShift]);

			// put in silent property
			if (type&CrusherSilent)
				strcat(linedes," Silent");
		}
		else return "";

		return linedes;
	}
}
