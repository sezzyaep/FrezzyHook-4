#include "js_functions.hpp"
#include "../duktate api/duk_config.h"
#include <fstream>
#include "../duktate api/duktape.h"
#include "../java loader/js_loader.h"
c_js_funcs js_funcs;
#define non_stat duk_ret_t

non_stat c_js_funcs::add_log(duk_ctx)
{
	Msg(duk_to_string(ctx, 0), color_t(255, 255, 255));
	return 1;
}
non_stat c_js_funcs::add_rectfilled(duk_ctx)
{
	g_Render->FilledRect(duk_to_int(ctx, 0), duk_to_int(ctx, 1), duk_to_int(ctx, 2), duk_to_int(ctx, 3), color_t(duk_to_int(ctx, 4), duk_to_int(ctx, 5), duk_to_int(ctx, 6), duk_to_int(ctx, 7)));
	return 1;
}
non_stat c_js_funcs::add_rect(duk_ctx)
{
	g_Render->Rect(duk_to_int(ctx, 0), duk_to_int(ctx, 1), duk_to_int(ctx, 2), duk_to_int(ctx, 3), color_t(duk_to_int(ctx, 4), duk_to_int(ctx, 5), duk_to_int(ctx, 6), duk_to_int(ctx, 7)));
	return 1;
}
non_stat c_js_funcs::add_to_hook(duk_ctx)
{
	for (auto f : c_js_loader::scripts)
	{
		f.execution_location = duk_to_int(ctx, 0);
	}
	return 0;
}
non_stat c_js_funcs::set_AA_Desync_Type(duk_ctx)
{
	vars.antiaim.desync_type = duk_to_int(ctx, 1);
	return 1;
}
non_stat c_js_funcs::set_AA_Pitch(duk_ctx)
{
	vars.antiaim.pitch = duk_to_int(ctx, 1);
	return 1;
}
void c_js_funcs::setup_js()
{
	const auto add_function = [&](const duk_c_function fn, const int args, const std::string& fn_name) -> void
	{
		duk_push_c_function(csgo->ctx, fn, args);
		duk_put_global_string(csgo->ctx, fn_name.c_str());
	};
	//logging
	add_function(add_log, 1, "Log"); 
	//drawing
	add_function(add_rectfilled, 8, "Rectangle_Filled");
	add_function(add_rect, 8, "Rectangle");
	add_function(set_AA_Desync_Type, 1, "Set_Desync_Type");
	add_function(set_AA_Pitch, 1, "Set_Pitch");
}

std::string c_js_funcs::read_script_from_file(const std::string path)
{
	std::fstream script_file;
	script_file.open(path, std::ios::in); //open a file to perform read operation using file object
	std::string script;
	if (script_file.is_open()) {   //checking whether the file is open
		std::string tp;
		while (std::getline(script_file, tp)) {  //read data from file object and put it into string.
			script += tp;  //print the data of the string
		}
		script_file.close();   //close the file object.
	}
	return script;
}

std::vector<std::string> c_js_funcs::get_js_files()
{
	std::vector<std::string> scripts;
	WIN32_FIND_DATA ffd;
	static TCHAR path[MAX_PATH];
	LPCSTR directory = "c:\\";
	LPCSTR Spath = "\\CatChair\\scripts\\*";
	if (SUCCEEDED(SHGetFolderPath(NULL, CSIDL_APPDATA, NULL, 0, path)))
	{
		directory = (string(path) + string(Spath)).c_str();
	}
	auto hFind = FindFirstFile(directory, &ffd);
	while (FindNextFile(hFind, &ffd))
	{
		if (!(ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
		{
			std::string file_name = ffd.cFileName;
			scripts.push_back(file_name);
		}
	}
	return scripts;
}
