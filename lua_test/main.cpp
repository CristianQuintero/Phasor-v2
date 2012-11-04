#include <vld.h> // check for memory leaks
#include <iostream>
#include <conio.h>
#include "..\Phasor\Scripting.h"

using namespace std;

const char* script = "i = 0\n"
	"function func(a)\n"
	"   print(a .. '\n')\n"
	"	while 1 do\n"
	"		i = i + 1\n"
	"		print(i)\n"
	"	end\n"
	"end\n";

void main()
{
	/*State* state = State::NewState();

	try
	{
		//state->DoString(script);
		state->DoFile("D:\\Development\\C++\\Phasor\\Release\\lua_test.lua");
		vector<Object*> args;
		args.push_back(state->NewString("Hello"));
		state->Call("funca", args, 5000);
	}
	catch (std::exception e)
	{
		cout << "Exception" << endl;
		cout << e.what() << endl;
	}

	cout << "Closing state" << endl;
	State::Close(state);*/

	try
	{
		Scripting::SetPath("D:\\Development\\C++\\Phasor\\Release");
		Scripting::OpenScript("lua_test");
		Scripting::OpenScript("lua_test1");

		Scripting::PhasorCaller caller;
		caller.AddArg("hello");
		Scripting::Result result = caller.Call("funca");
		cout << result.size() << endl;


	} catch (std::exception e)
	{
		cout << e.what() << endl;
	}

	Scripting::CloseScript("lua_test");
	Scripting::CloseScript("lua_test1");
	
	//Scripting::Call("funca", args);
}