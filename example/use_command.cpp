#include "command.hpp"
#include "terminal.hpp"

#include <iostream>
#include <string>
#include <cassert>

using namespace cmdkit;
using C = Command;
using R = Result<void*, std::string>;

int main()
{
	// Simplest usage of command
	C string_logger(
		"log_str", 
		[](const CommandArgs& args)
		{
			std::cout << "Log string: " << args[1] << std::endl;
			return R::ok(nullptr);
		}
	);
	string_logger.invoke("log_str Hello_world!");

	// Muiltiple arguments usage of command
	C string_linker(
		"link_str",
		[](const CommandArgs& args)
		{
			std::cout << "Link strings: ";
			const auto& vec = args.get_positional();
			for (size_t idx = 1; idx < vec.size(); ++idx) std::cout << vec[idx];
			std::cout << std::endl;
			return R::ok(nullptr);
		}
	);
	string_linker.invoke("link_str Hello world");

	// Using flag and options in command, handling errors
	C string_stringer(
		"string_str",
		[](const CommandArgs& args)
		{

			if (args.has_flag("able"))
			{
				std::cout << "String strings: ";
				const auto& vec = args.get_positional();
				const std::string ch = args.get_option("divide", "-");

				for (size_t idx = 1; idx < vec.size(); ++idx)
					std::cout << (idx == 1 ? "" : ch) << vec[idx];
				std::cout << std::endl;
				return R::ok(nullptr);
			}
			else return R::err("Not able to string strs!");

		}
	);
	std::string cmd1 = "string_str Hello World";
	std::string cmd2 = "string_str Hello World --able";
	std::string cmd3 = "string_str Hello World --able --divide ->";

	auto res = string_stringer.invoke(cmd1);
	assert(res.is_err());
	string_stringer.invoke(cmd2);
	string_stringer.invoke(cmd3);

	// Tips: use command with closure
	int var1 = 1;
	C variable_changer(
		"change_var",
		[&var1](const CommandArgs& args)
		{
			std::cout << "Change var : ";
			std::cout << var1 << " to ";
			var1 = std::stoi(args[1]);
			std::cout << var1 << std::endl;
			return R::ok(nullptr);
		}
	);
	variable_changer.invoke("change_var 2");
	std::cout << "Outside the closure, var1 = " << var1 << std::endl;

	std::cout << "Command examples all passed!" << std::endl;
	getchar();
}