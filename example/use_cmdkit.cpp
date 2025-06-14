#include "cmdkit.hpp"

#include <iostream>

using namespace cmdkit;
using R = Result<void*, std::string>;
using C = Command;
using T = Terminal;

int main()
{
	T terminal;
	// Simplest usage of command
	C string_logger(
		"log_str",
		[](const CommandArgs& args)
		{
			std::cout << "Log string: " << args[1] << std::endl;
			return R::ok(nullptr);
		}
	);
	terminal.register_command(string_logger);

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
	terminal.register_command(string_linker);

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
	terminal.register_command(string_stringer);

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
	terminal.register_command(variable_changer);

	auto func = []() {std::cout << "Can't find command!" << std::endl; };
	std::string input;
	while (true)
	{
		std::getline(std::cin, input);
		terminal.invoke(input, func);
	}
}