#include "PhasorAPI.h"
#include "../Common/Common.h"
#include <list>
#include <assert.h>
#include "memory.h"
#include "output.h"
#include "deprecated.h"
#include "playerinfo.h"

using namespace Common;
using namespace Manager;

namespace PhasorAPI
{
	void testf(CallHandler& handler, 
		Object::unique_deque& args, Object::unique_list& results)
	{
		ObjBool& b = (ObjBool&)*args[1];
		printf("Received %i value %i\n", args[1]->GetType(), b.GetValue());
		results.push_back(std::unique_ptr<Object>(
			new ObjString("Hello, register test.")));
	}

	// Functions to register for scripts use.
	// When any function is called all parameters have been type checked
	// and so can be treated as valid.
	const ScriptCallback PhasorExportTable[] =
	{
		// {&cfunc, "funcname", min_args, {arg1_t, arg2_t, .. argn_t}}
		// remember: n > min_args if there are overloads.
		{&testf, "test_func", 3, {TYPE_NUMBER, TYPE_BOOL, TYPE_STRING}},
		// Memory related functions: see memory.h
		{&l_readbit, "readbit", 2, {TYPE_NUMBER, TYPE_NUMBER, TYPE_NUMBER}},
		{&l_readbyte, "readbyte", 1, {TYPE_NUMBER, TYPE_NUMBER}},
		{&l_readchar, "readchar", 1, {TYPE_NUMBER}},
		{&l_readword, "readword", 1, {TYPE_NUMBER, TYPE_NUMBER}},
		{&l_readshort, "readshort", 1, {TYPE_NUMBER}},
		{&l_readdword, "readdword", 1, {TYPE_NUMBER, TYPE_NUMBER}},
		{&l_readint, "readint", 1, {TYPE_NUMBER}},
		{&l_readfloat, "readfloat", 1, {TYPE_NUMBER, TYPE_NUMBER}},
		{&l_readdouble, "readdouble", 1, {TYPE_NUMBER, TYPE_NUMBER}},
		{&l_writebit, "writebit", 3, {TYPE_NUMBER, TYPE_NUMBER, TYPE_NUMBER, TYPE_NUMBER}},
		{&l_writebyte, "writebyte", 2, {TYPE_NUMBER, TYPE_NUMBER, TYPE_NUMBER}},
		{&l_writechar, "writechar", 2, {TYPE_NUMBER, TYPE_NUMBER, TYPE_NUMBER}},
		{&l_writeword, "writeword", 2, {TYPE_NUMBER, TYPE_NUMBER, TYPE_NUMBER}},
		{&l_writeshort, "writeshort", 2, {TYPE_NUMBER, TYPE_NUMBER, TYPE_NUMBER}},
		{&l_writedword, "writedword", 2, {TYPE_NUMBER, TYPE_NUMBER, TYPE_NUMBER}},
		{&l_writeint, "writeint", 2, {TYPE_NUMBER, TYPE_NUMBER, TYPE_NUMBER}},
		{&l_writefloat, "writefloat", 2, {TYPE_NUMBER, TYPE_NUMBER, TYPE_NUMBER}},
		{&l_writedouble, "writedouble", 2, {TYPE_NUMBER, TYPE_NUMBER, TYPE_NUMBER}},
		// Output related functions: see output.h
		{&l_hprintf, "hprintf", 1, {TYPE_STRING}},
		{&l_say, "say", 1, {TYPE_STRING}},
		{&l_privatesay, "privatesay", 2, {TYPE_NUMBER, TYPE_STRING}},
		{&l_sendconsoletext, "sendconsoletext", 2, {TYPE_NUMBER, TYPE_STRING}},
		{&l_respond, "respond", 1, {TYPE_STRING}},
		{&l_log_msg, "log_msg", 2, {TYPE_NUMBER, TYPE_STRING}},
		// Player info related functions: see playerinfo.h
		{&l_resolveplayer, "resolveplayer", 1, {TYPE_NUMBER}},
		{&l_rresolveplayer, "rresolveplayer", 1, {TYPE_NUMBER}},
		{&l_getplayer, "getplayer", 1, {TYPE_NUMBER}},
		{&l_getip, "getip", 1, {TYPE_NUMBER}},
		{&l_getport, "getport", 1, {TYPE_NUMBER}},
		{&l_getteam, "getteam", 1, {TYPE_NUMBER}},
		{&l_getname, "getname", 1, {TYPE_NUMBER}},
		{&l_gethash, "gethash", 1, {TYPE_NUMBER}},
		{&l_getteamsize, "getteamsize", 1, {TYPE_NUMBER}},
		{&l_getplayerobjectid, "getplayerobjectid", 1, {TYPE_NUMBER}},
		{&l_isadmin, "isadmin", 1, {TYPE_NUMBER}},
		{&l_setadmin, "setadmin", 1, {TYPE_NUMBER}}
	};
	static const size_t export_table_size = sizeof(PhasorExportTable)/sizeof(PhasorExportTable[0]);

	void Register(Manager::ScriptState& state)
	{
		RegisterFunctions(state, PhasorExportTable, export_table_size);
	}
}