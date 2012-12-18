#include "Hooks.h"
#include "Addresses.h"
#include "../../Common/Common.h"
#include "Server/Server.h"
#include "Game/Game.h"
#include "Server/MapLoader.h"

using namespace halo;

// Server function codecaves
// ------------------------------------------------------------------------


// Codecave for timers, safer than creating threads (hooking console chceking routine)
/*DWORD consoleProc_ret = 0;
__declspec(naked) void OnConsoleProcessing_CC()
{	
	__asm
	{
		pop consoleProc_ret

		pushad
		call server::OnConsoleProcessing
		popad

		PUSH EBX
		XOR EBX,EBX
		push consoleProc_ret
		CMP AL,BL
		ret
	}
}

// Codecave for hooking console events (closing etc)
DWORD conHandler_ret = 0;
__declspec(naked) void ConsoleHandler_CC()
{	
	__asm
	{
		pop conHandler_ret

		pushad

		push esi
		call server::ConsoleHandler

		popad

		mov eax, 1
		push conHandler_ret
		ret
	}
}
*/
// Codecave for intercepting server commands
DWORD cmd_ret = 0;
static const BYTE COMMAND_PROCESSED = e_command_result::kProcessed;
__declspec(naked) void OnServerCommand()
{
	__asm
	{
		// Save return address
		pop cmd_ret

		pushad

		push edi
		call server::ProcessCommand

		cmp al, COMMAND_PROCESSED
		je PROCESSED

		popad

		MOV AL,BYTE PTR DS:[EDI]
		SUB ESP,0x500
		push cmd_ret
		ret

PROCESSED:

		popad
		mov eax, 1
		ret
	}
}

// Codecave for loading maps into the cyc;e
DWORD mapload_ret = 0;
__declspec(naked) void OnMapLoading_CC()
{	
	__asm
	{
		pop mapload_ret

		pushad

		lea eax, dword ptr ds:[eax + esi]
		push eax
		call server::OnMapLoad

		cmp al, 0
		je RELOAD_MAP_DATA

		popad

		MOV ECX,DWORD PTR DS:[ESI+EAX]
		PUSH 0x3F
		push mapload_ret
		ret

RELOAD_MAP_DATA:

		popad

		// reset mapycle index
		mov eax, DWORD PTR ds:[ADDR_MAPCYCLEINDEX]
		MOV [eax], 0
		mov esi, 0 // value of ADDR_MAPCYCLEINDEX, which we just set to 0

		// get data for current map
		mov eax, dword ptr ds:[ADDR_MAPCYCLELIST]
		mov eax, [eax]
		
		//MOV ESI,DWORD PTR DS:[ADDR_MAPCYCLEINDEX]
		//mov ESI, [ESI]

		MOV ECX,DWORD PTR DS:[EAX+ESI]
		push 0x3F
		push mapload_ret
		ret
	}
}
/*
// Game function codecaves
// ------------------------------------------------------------------------
// Codecave for detecting game ending
__declspec(naked) void OnGameEnd_CC()
{	
	__asm
	{
		pop edx // ret addr

		pushad

		push eax
		call game::OnGameEnd

		popad

		SUB ESP,0x0C                      
		PUSH 0
		push edx
		ret
	}
}*/

DWORD newGame_ret = 0;

// Codecave used for detecting a new game
__declspec(naked) void OnNewGame_CC()
{
	__asm
	{
		pop newGame_ret

		MOV EAX,DWORD PTR DS:[ADDR_NEWGAMEMAP]
		mov eax, [eax]

		cmp EAX, 0
		je NOT_NEW_MAP
		pushad

		lea eax, dword ptr ds:[esp + 0x24]
		mov eax, [eax]
		push eax
		call server::OnNewGame

		popad

NOT_NEW_MAP:

		push newGame_ret
		ret
	}
}

/*DWORD playerjoin_ret = 0;

// Codecave for people joining
__declspec(naked) void OnPlayerWelcome_CC()
{	
	__asm
	{
		pop playerjoin_ret
		#ifdef PHASOR_PC
			PUSH EAX
			MOV ESI,EBP
		#elif PHASOR_CE
			PUSH EBP
			MOV ESI,EAX
		#endif

		call dword ptr ds:[FUNC_PLAYERJOINING]
		pushad

		mov eax, [esp - 0x60]
		cmp eax, 0 // 0 when player isn't actually joining (happens if join when game just ends)
		je IGNORE_JOIN
		and eax, 0xff
		push eax
		call game::OnPlayerWelcome
IGNORE_JOIN:

		popad
		push playerjoin_ret
		ret
	}
}

DWORD leaving_ret = 0;

// Codecave used for detecting people leaving
__declspec(naked) void OnPlayerQuit_CC()
{	
	__asm
	{
		pop leaving_ret

		pushad

		and eax, 0xff
		push eax
		call game::OnPlayerQuit

		popad

		PUSH EBX
		PUSH ESI
		MOV ESI,EAX
		MOV EAX,DWORD PTR DS:[ADDR_PLAYERBASE]
		mov eax, [eax]
		push leaving_ret
		ret
	}
}

DWORD teamsel_ret = 0, selection = 0;

// Codecave for team selection
__declspec(naked) void OnTeamSelection_CC()
{
	__asm
	{
		pop teamsel_ret

		// ecx can be modified safely (see FUNC_TEAMSELECT)
		mov ecx, FUNC_TEAMSELECT
		call ecx

		pushad

		mov edi, dword ptr ds:[esp + 0x38]
		push edi // ptr ptr ptr to machine struct
		movsx eax, al
		push eax
		call game::OnTeamSelection
		mov selection, eax

		popad
		mov eax, selection
		push teamsel_ret
		ret
	}
}

// Codecave for handling team changes
DWORD teamchange_ret = 0;
__declspec(naked) void OnTeamChange_CC()
{	
	__asm
	{
		pop teamchange_ret // ret addr cant safely modify

		pushad

		push ebx // team

#ifdef PHASOR_PC
		and eax, 0xff
		push eax // player
#elif	PHASOR_CE
		and edx, 0xff
		push edx // player
#endif
		
		call game::OnTeamChange

		cmp al, 0
		je DO_NOT_CHANGE

		// Allow them to change
		popad

		// overwritten code
		#ifdef PHASOR_PC
		MOVZX ESI,AL
		LEA ECX,DWORD PTR DS:[EDI+8]
		#elif PHASOR_CE
		ADD EAX, 8
		PUSH EBX
		PUSH EAX
		#endif

		push teamchange_ret // return address
		ret

DO_NOT_CHANGE:

		// Don't let them change
		popad

#ifdef PHASOR_PC
		POP EDI
#endif
		POP ESI
		POP EBX
		ADD ESP,0x1C
		ret
	}
}

// Codecave for player spawns (just before server notification)
__declspec(naked) void OnPlayerSpawn_CC()
{	
	__asm
	{
		pop ecx // can safely use ecx (see function call after codecave)

		pushad

		and ebx, 0xff
		push esi // memory object id
		push ebx // player memory id
		call game::OnPlayerSpawn

		popad

		// orig code
		push 0x7ff8
		push ecx
		ret
	}
}

DWORD playerSpawnEnd_ret = 0;

// Codecave for player spawns (after server has been notified).
__declspec(naked) void OnPlayerSpawnEnd_CC()
{	
	__asm
	{
		pop playerSpawnEnd_ret

		mov ecx, FUNC_AFTERSPAWNROUTINE
		call ecx
		add esp, 0x0C

		pushad

		and ebx, 0xff

		push esi // memory object id
		push ebx // player memory id
		call game::OnPlayerSpawnEnd

		popad
		push playerSpawnEnd_ret
		ret
	}
}

DWORD objcreation_ret = 0;

// Codecave for modifying weapons as they're created
__declspec(naked) void OnObjectCreation_CC()
{
	__asm
	{
		pop objcreation_ret

		pushad

		push ebx
		call game::OnObjectCreation

		popad

		MOV ECX,DWORD PTR DS:[EAX+4]
		TEST ECX,ECX
		push objcreation_ret
		ret
	}
}

DWORD wepassignment_ret = 0, wepassign_val = 0;

// Codecave for handling weapon assignment to spawning players
__declspec(naked) void OnWeaponAssignment_CC()
{
	__asm
	{
		pop wepassignment_ret

		pushad

		mov edi, [esp + 0xFC]
		and edi, 0xff

		mov ecx, [esp + 0x2c]
		push ecx
		push eax
		push esi
		push edi
		call game::OnWeaponAssignment
		mov wepassign_val, eax

		popad

		mov eax, wepassign_val // id of weapon to make
		//MOV EAX,DWORD PTR DS:[EAX+0x0C] // original instruction
		CMP EAX,-1
		push wepassignment_ret		
		ret
	}
}

DWORD objInteraction_ret = 0;

// Codecave for handling object pickup interactions
__declspec(naked) void OnObjectInteraction_CC()
{
	__asm
	{
		pop objInteraction_ret

		pushad

		mov eax, dword ptr ds:[esp + 0x38]
		mov edi, dword ptr ds:[esp + 0x3c]

		and eax, 0xff
		push edi
		push eax
		call game::OnObjectInteraction

		cmp al, 0
		je DO_NOT_CONTINUE

		popad

		PUSH EBX
		MOV EBX,DWORD PTR SS:[ESP+0x20]
		push objInteraction_ret
		ret

DO_NOT_CONTINUE:

		popad
		add esp, 0x14
		ret
	}
}

// Codecave for server chat
__declspec(naked) void OnChat_CC()
{
	__asm
	{
		// don't need the return address as we skip this function
		add esp, 4

		pushad

		#ifdef PHASOR_PC
		mov eax, dword ptr ds:[esp + 0x2c]
		and eax, 0xff // other 3 bytes aren't used for player number and can be non-zero
		mov [esp + 0x2c], eax
		lea eax, dword ptr ds:[esp + 0x28]
		#elif PHASOR_CE
		mov eax, dword ptr ds:[esp + 0x40]
		and eax, 0xff // other 3 bytes aren't used for player number and can be non-zero
		mov [esp + 0x40], eax
		lea eax, dword ptr ds:[esp + 0x3C]
		#endif
		
		push eax
		call game::OnChat

		popad

		POP EDI
		#ifdef PHASOR_PC
		ADD ESP,0x224
		#elif PHASOR_CE
		ADD ESP, 0x228
		#endif
		ret
	}
}

// Codecave for player position updates
__declspec(naked) void OnClientUpdate_CC()
{
	__asm
	{
		pop edi // can safely use edi as its poped in return stub

		// Execute the original code
		MOV BYTE PTR DS:[EAX+0x2A6],DL
		JE L008
		MOV DWORD PTR DS:[EAX+0x4BC],EBP
		MOV BYTE PTR DS:[EAX+0x4B8],0x1
		jmp START_PROCESSING
L008:
		MOV BYTE PTR DS:[EAX+0x4B8],0x0
START_PROCESSING:

		pushad

		push eax // player's object
		call game::OnClientUpdate

		popad

		// return out of function
		POP EDI
		POP ESI
		POP EBP
		ret
	}
}

DWORD dmglookup_ret = 0;

// Codecaves for handling weapon damage
__declspec(naked) void OnDamageLookup_CC()
{
	__asm
	{
		pop dmglookup_ret

		pushad

		mov esi, [ebp + 0x0C] // object causing the damage
		lea ecx, dword ptr ds:[edx + eax] // table entry for the damage tag
		mov edi, [esp + 0xcc] // object taking the damage

		push ecx // damage tag		
		push esi // object causing damage
		push edi // object taking damage
		call game::OnDamageLookup

		popad

		ADD EDI,0x1C4
		push dmglookup_ret
		ret
	}
}

DWORD OnVehicleEntry_ret = 0;
__declspec(naked) void OnVehicleEntry_CC()
{
	__asm
	{
		pop OnVehicleEntry_ret

		pushad
		pushfd

		mov eax, [ebp + 0x8]
		and eax, 0xff

		push eax
		call game::OnVehicleEntry

		cmp al, 1

		je ALLOW_VEHICLE_ENTRY

		popfd
		popad

		MOV AL,BYTE PTR SS:[EBP-1]

		POP EDI
		POP ESI
		POP EBX
		MOV ESP,EBP
		POP EBP	

		ret // don't let them change

ALLOW_VEHICLE_ENTRY:

		popfd
		popad

		MOV DWORD PTR SS:[EBP-8],-1
		push OnVehicleEntry_ret
		ret
	}
}

DWORD ondeath_ret = 0;

// Codecave for handling player deaths
__declspec(naked) void OnDeath_CC()
{
	__asm
	{
		pop ondeath_ret

		pushad

		lea eax, dword ptr ds:[esp + 0x38] // victim
		mov eax, [eax]
		and eax, 0xff
		lea ecx, dword ptr ds:[esp + 0x34] // killer
		mov ecx, [ecx]
		and ecx, 0xff

		push esi // mode of death
		push eax // victim
		push ecx // killer
		call game::OnPlayerDeath

		popad

		PUSH EBP
		MOV EBP,DWORD PTR SS:[ESP+0x20]
		push ondeath_ret
		ret
	}
}

DWORD killmultiplier_ret = 0;

// Codecave for detecting double kills, sprees etc
__declspec(naked) void OnKillMultiplier_CC()
{
	__asm
	{
		pop killmultiplier_ret

		pushad

		push esi // muliplier
		and ebp, 0xff
		push ebp // player
		call game::OnKillMultiplier

		popad

		MOV EAX,DWORD PTR SS:[ESP+0x1C]
		PUSH EBX
		push killmultiplier_ret
		ret
	}
}

// Codecave for handling weapon reloading
DWORD weaponReload_ret = 0;
__declspec(naked) void OnWeaponReload_CC()
{
	__asm
	{
		pop weaponReload_ret

		pushad

		mov ecx, [esp + 0x40]
		push ecx
		call game::OnWeaponReload

		cmp al, 1
		je ALLOW_RELOAD

		popad

		AND DWORD PTR DS:[EAX+0x22C],0xFFFFFFF7
		POP EDI
		POP ESI
		POP EBP
		POP EBX
		ADD ESP,0xC
		ret

ALLOW_RELOAD:

		popad

		MOV CL,BYTE PTR SS:[ESP+0x28]
		OR EBP,0xFFFFFFFF
		push weaponReload_ret
		ret
	}
}

// used to control object respawning
DWORD objres_ret = 0;
__declspec(naked) void OnObjectRespawn_CC()
{
	__asm
	{
		pop objres_ret

		pushad
		push ebx // object's memory address
		push esi // object's id
		call objects::ObjectRespawnCheck // returns true if we should respawn, false if not
		cmp al, 2
		je OBJECT_DESTROYED
		cmp al, 1 // return to a JL statement, jump if not respawn
		popad
		push objres_ret
		ret

OBJECT_DESTROYED:
		popad

		mov eax, 1
		pop edi
		pop esi
		pop ebx
		mov esp,ebp
		pop ebp
		ret
	}
}

// used to control itmc destruction
DWORD itmcdes_ret = 0;
__declspec(naked) void OnEquipmentDestroy_CC()
{
	__asm
	{
		pop itmcdes_ret

		pushad
		push ebp // equipment's memory address
		push ebx // equipment's memory id
		push esi // tick count of when the item is due for destruction
		call objects::EquipmentDestroyCheck // returns true if should destroy
		xor al, 1 // 1 -> 0, 0 -> 1
		cmp al, 1 // returns to a JGE so if al < 1 item is destroyed
		popad

		push itmcdes_ret
		ret
	}
}

DWORD vehfeject_ret = 0;
__declspec(naked) void OnVehicleForceEject_CC()
{
	__asm
	{
		pop vehfeject_ret

		pushad
		push 1 // force eject
		push ebx 
		call game::OnVehicleEject // false - don't eject

		cmp al, 1
		je DO_FORCE_EJECT

		// don't let them eject
		popad
		sub vehfeject_ret, 0x2d //56e6d2-56e6a5 = 0x2d
		cmp al, al // to force jump

		push vehfeject_ret
		ret

DO_FORCE_EJECT:
		popad

		CMP WORD PTR DS:[EBX+0x2F0],0x0FFFF
		push vehfeject_ret
		ret
	}
}

DWORD vehueject_ret = 0;
__declspec(naked) void OnVehicleUserEject_CC()
{	
	__asm
	{
		pop vehueject_ret

		TEST BYTE PTR DS:[EBX+0x208],0x40
		je NOT_EJECTING

		pushad
		push 0
		push ebx // not a forceable ejection
		call game::OnVehicleEject // false - don't eject

		cmp al, 1
		je DO_USER_EJECT

		popad 
		cmp al, al // force the jump (stop ejection)

		push vehueject_ret
		ret

DO_USER_EJECT:
		popad
		TEST BYTE PTR DS:[EBX+0x208],0x40
NOT_EJECTING:
		
		push vehueject_ret
		ret
	}
}

void __stdcall OnExitProcess(DWORD retAddr)
{
//	logging::LogData(LOGGING_PHASOR, L"ExitProcess from 0x%08X", retAddr);
}

DWORD exitProc_ret  =0;
__declspec(naked) void ExitProcess_CC()
{
	__asm
	{
		pop exitProc_ret
		pushad
		mov eax, [esp + 0x20]
		push eax
		call OnExitProcess
		popad
		MOV EAX,EAX
		PUSH EBP
		MOV EBP,ESP

		push exitProc_ret
		ret
	}
}

__declspec(naked) void OnHaloPrint_CC()
{
	__asm
	{
		add esp,4 // we don't execute the func so dont need ret addr

		pushad
		push eax
		call server::OnHaloPrint
		popad
		
		ret
	}
}

DWORD halobancheck_ret = 0;
__declspec(naked) void OnHaloBanCheck_CC()
{
	__asm
	{
		pop halobancheck_ret

		pushad
		push edi
		call server::OnHaloBanCheck
		cmp al, 1 // not banned
		JE PLAYER_NOT_BANNED
		popad
		mov al, 1 // banned
		ret
PLAYER_NOT_BANNED:
		popad

		PUSH ECX
		PUSH EBX
		PUSH ESI
		PUSH EDI
		XOR BL,BL
		push halobancheck_ret
		ret
	}
}

DWORD versionBroadcast_ret = 0;
__declspec(naked) void OnVersionBroadcast_CC()
{
	__asm
	{
		pop versionBroadcast_ret

		pushad // -0x20

		mov esi, [esp + 0x28]
		mov edi, [esp + 0x24]

		push esi
		push edi
		call server::OnVersionBroadcast // return value: 1 - do broadcast, 0 - don't
		cmp al, 1
		je do_broadcast

		popad // don't broadcast
		ret

do_broadcast:

		popad

		// overwritten code
		SUB ESP,0x834
		push versionBroadcast_ret
		ret
	}
}

void __stdcall OnJoinCheck(LPBYTE lList, LPBYTE mem)
{
	if (game::GetPlayerList().size() == 16)
	{
		DWORD mask = player->mem->playerJoinCount << 0x10, funccall = 0x494780;
		__asm
		{
			pushad
			mov eax, mask
			call dword ptr ds:[funccall]
			//add esp, 4
			popad
		}
		halo::ExecuteServerCommand(0,"sv_kick 16");
	}
		//halo::ExecuteServerCommand(0,"sv_kick 1");
		/(DWORD*)lList = 0;
		(reserved slots:
		005112AA      90            NOP
			005112AB      90            NOP


			005152FD      83C4 0C       ADD ESP,0C
			00515300      C3            RETN
			00515301      90            NOP
			00515302      90            NOP
			00515303      90            NOP
			00515304      90            NOP

			00512479      90            NOP
			0051247A      90            NOP
			0051247B      90            NOP
			0051247C      90            NOP
			0051247D      90            NOP
			0051247E      90            NOP
}

DWORD joinCheck_ret = 0;
__declspec(naked) void OnJoinCheck_CC()
{
	__asm
	{
		pop joinCheck_ret

		ADD ESP,4
		TEST EAX,EAX

		pushad
		pushfd

		lea edi, [ebp + 0xaa0]
		push eax
		push edi
		call OnJoinCheck

		popfd
		popad

		push joinCheck_ret
		ret
	}
}
*/
namespace halo
{
	using namespace Common;
	// Installs all codecaves and applies all patches
	void InstallHooks()
	{
		// Patches
		// ----------------------------------------------------------------
		// 
		// Ensure we always decide the team
		BYTE nopSkipSelection[2] = {0x90, 0x90};
		WriteBytes(PATCH_TEAMSELECTION, &nopSkipSelection, 2);

		// Stop the server from processing map additions (sv_map, sv_mapcycle_begin)
		BYTE mapPatch[] = {0xB0, 0x01, 0xC3};
		WriteBytes(PATCH_NOMAPPROCESS, &mapPatch, sizeof(mapPatch));

#ifdef PHASOR_PC
		// Make Phasor control the loading of a map
		DWORD addr = (DWORD)UlongToPtr(server::maploader::GetLoadingMapBuffer());
		WriteBytes(PATCH_MAPLOADING, &addr, 4);

		// Set the map table
		DWORD table = server::maploader::GetMapTable();
		WriteBytes(PATCH_MAPTABLE, &table, 4);

		// Remove where the map table is allocated/reallocated by the server
		BYTE nopPatch[0x3E];
		memset(nopPatch, 0x90, sizeof(nopPatch));
		WriteBytes(PATCH_MAPTABLEALLOCATION, &nopPatch, sizeof(nopPatch));	
#endif


		// Server hooks
		// ----------------------------------------------------------------
		// 
		// Codecave for timers, safer than creating threads (hooking console checking routine)
		/*CreateCodeCave(CC_CONSOLEPROC, 5, OnConsoleProcessing_CC);

		// Codecave for hooking console events (closing etc)
		CreateCodeCave(CC_CONSOLEHANDLER, 5, ConsoleHandler_CC);*/

		// Codecave to intercept server commands
		CreateCodeCave(CC_SERVERCMD, 8, OnServerCommand);

		// Codecave used to load non-default maps
		CreateCodeCave(CC_MAPLOADING, 5, OnMapLoading_CC);
		 
		// Game Hooks
		// ----------------------------------------------------------------
		//
		// Codecave for detecting game ending
		//CreateCodeCave(CC_GAMEEND, 5, OnGameEnd_CC);

		// Codecave used to detect a new game
		CreateCodeCave(CC_NEWGAME, 5, OnNewGame_CC);

		// Codecave called when a player joins
		/*CreateCodeCave(CC_PLAYERWELCOME, 8, OnPlayerWelcome_CC);

		// Codecave used to detect people leaving
		CreateCodeCave(CC_PLAYERQUIT, 9, OnPlayerQuit_CC);

		// Codecave used to decide the player's team
		CreateCodeCave(CC_TEAMSELECTION, 5, OnTeamSelection_CC);

		// Codecave for handling team changes
		#ifdef PHASOR_PC
		CreateCodeCave(CC_TEAMCHANGE, 6, OnTeamChange_CC);
		#elif PHASOR_CE
		CreateCodeCave(CC_TEAMCHANGE, 5, OnTeamChange_CC);
		#endif

		// Codecaves for detecting player spawns
		CreateCodeCave(CC_PLAYERSPAWN, 5, OnPlayerSpawn_CC);
		CreateCodeCave(CC_PLAYERSPAWNEND, 8, OnPlayerSpawnEnd_CC);

		// Codecave called when a weapon is created
		CreateCodeCave(CC_OBJECTCREATION, 5, OnObjectCreation_CC);

		// Codecave for handling weapon assignment to spawning players
		CreateCodeCave(CC_WEAPONASSIGN, 6, OnWeaponAssignment_CC);

		// Codecave for interations with pickable objects
		CreateCodeCave(CC_OBJECTINTERACTION, 5, OnObjectInteraction_CC);

		// Codecave for position updates
		CreateCodeCave(CC_CLIENTUPDATE, 6, OnClientUpdate_CC);

		// Codecave for handling damage being done
		CreateCodeCave(CC_DAMAGELOOKUP, 6, OnDamageLookup_CC);

		// Codecave for server chat
		CreateCodeCave(CC_CHAT, 7, OnChat_CC);

		// Codecave for handling vehicle entry
		CreateCodeCave(CC_VEHICLEENTRY, 7, OnVehicleEntry_CC);

		// Codecave for handling weapon reloading
		CreateCodeCave(CC_WEAPONRELOAD, 7, OnWeaponReload_CC);

		// Codecave for detecting player deaths
		CreateCodeCave(CC_DEATH, 5, OnDeath_CC);

		// Codecave for detecting double kills, sprees etc
		CreateCodeCave(CC_KILLMULTIPLIER, 5, OnKillMultiplier_CC);

		// used to control whether or not objects respawn
		CreateCodeCave(CC_OBJECTRESPAWN, 32, OnObjectRespawn_CC);
		CreateCodeCave(CC_EQUIPMENTDESTROY, 6, OnEquipmentDestroy_CC);

		// Codecaves for detecting vehicle ejections
		CreateCodeCave(CC_VEHICLEFORCEEJECT, 8, OnVehicleForceEject_CC);
		CreateCodeCave(CC_VEHICLEUSEREJECT, 7, OnVehicleUserEject_CC);

		// Generic codecaves
		CreateCodeCave(CC_HALOPRINT, 6, OnHaloPrint_CC);
		CreateCodeCave(CC_HALOBANCHECK, 6, OnHaloBanCheck_CC);
		CreateCodeCave(CC_VERSIONBROADCAST, 6, OnVersionBroadcast_CC);

		//CreateCodeCave(0x005112d4, 5, OnJoinCheck_CC);
		//
		//BYTE maxCmp[] = {0x12};
		//WriteBytes(0x5112A9, &maxCmp, sizeof(maxCmp));
		//
		//BYTE maxCmp1[] = {0x83, 0xC4, 0x0C, 0xC3, 0x90, 0x90, 0x90, 0x90};
		//WriteBytes(0x005152FD , &maxCmp1, sizeof(maxCmp1));
		//
		//BYTE curCmp[] = {0x90, 0x90, 0x90, 0x90, 0x90, 0x90};
		//WriteBytes(0x00512479 , &curCmp, sizeof(curCmp));


		// I want to remove haloded's seh chain so that I can get extra exception
		// information (passed to the unhandled exception filter)
		#pragma pack(push, 1)

		struct sehEntry
		{
			sehEntry* next;
			LPBYTE hndFunc;
		};
		#pragma pack(pop)

		// Get the first entry in the chain
		sehEntry* exceptionChain = 0;
		__asm
		{
			pushad
			MOV EAX,DWORD PTR FS:[0]
			mov exceptionChain, eax
			popad
		}

		if (exceptionChain)
		{
			// loop through the exception chain and remove any references to
			// 5B036C (should do a sig search for this value.. meh)
			sehEntry* last = 0, *end = (sehEntry*)0xffffffff;
			while (exceptionChain != end) {
				if (exceptionChain->hndFunc == (LPBYTE)FUNC_HALOEXCEPTIONHANDLER) {
					// remove this entry
					if (last) {
						last->next = exceptionChain->next;
						exceptionChain = last;
					} else if (exceptionChain->next != end) { // only overwrite if there is one to go to
						exceptionChain->hndFunc = exceptionChain->next->hndFunc;
						exceptionChain->next = exceptionChain->next->next;
					}
				}
				last = exceptionChain;			
				exceptionChain = exceptionChain->next;
			}
		}*/	
	}
}