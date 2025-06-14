#ifndef INCLUDE_CMDKIT_TERMINAL
#define INCLUDE_CMDKIT_TERMINAL

#include <string>
#include <vector>
#include <map>
#include <stdexcept>
#include <type_traits>

#include "command.hpp"

namespace cmdkit
{
	class Terminal
	{
	public:
		void register_command(const std::string& name, Command cmd) { command_table[name] = cmd; }
		void register_command(Command cmd) { command_table[cmd.get_name()] = cmd; }


		void invoke(const CommandArgs& command) const
		{
			invoke(command, []() { throw std::runtime_error("Not find command!"); } );
		}
		
		template<typename Fn, std::enable_if_t<std::is_invocable_v<Fn>, int> = 0>
		void invoke(const CommandArgs& command, Fn&& not_find_callback) const
		{
			auto it = command_table.find(command[0]);
			if (it != command_table.end()) (it->second).invoke(command);
			else std::invoke(std::forward<Fn>(not_find_callback));
		}

		template<typename Fn, std::enable_if_t<std::is_invocable_v<Fn>, int> = 0>
		void invoke(const std::string& command, Fn&& not_find_callback) const
		{
			return invoke(CommandArgs::parse(command), not_find_callback);
		}

		void invoke(const std::string& command) const
		{
			return invoke(CommandArgs::parse(command), []() { throw std::runtime_error("Not find command!"); });
		}

	private:
		std::map<std::string, Command> command_table;
	};
}

#endif // INCLUDE_CMDKIT_TERMINAL