#ifndef INCLUDE_CMDKIT_COMMAND
#define INCLUDE_CMDKIT_COMMAND

#include <string>
#include <vector>
#include <unordered_set>
#include <unordered_map>
#include <functional>

#include "result.hpp"

namespace cmdkit
{
	class CommandArgs
	{
	private:
		std::unordered_map<std::string, std::string> options;
		std::unordered_set<std::string> flags;
		std::vector<std::string> positional;

	public:
		static CommandArgs parse(const std::vector<std::string>& args)
		{
			CommandArgs result;

			for (size_t idx = 0; idx < args.size(); ++idx)
			{
				const auto& arg = args[idx];
				if (args.size() > 2 && arg[0] == '-' && arg[1] == '-')
				{
					std::string key = arg.substr(2);

					if (idx == args.size() - 1 || (args[idx + 1].size() > 2 && args[idx + 1][0] == '-' && args[idx + 1][1] == '-'))
						result.flags.insert(key);
					else result.options[key] = args[++idx];
				}
				else result.positional.push_back(arg);
			}

			return result;
		}

		static CommandArgs parse(const std::string& args_str)
		{
			std::vector<std::string> vec;
			size_t pos = 0;
			while (pos < args_str.size())
			{
				while (pos < args_str.size() && std::isspace(args_str[pos])) pos++;
				if (pos >= args_str.size()) break;

				size_t start = pos;
				while (pos < args_str.size() && !std::isspace(args_str[pos])) pos++;
				vec.emplace_back(args_str.substr(start, pos - start));
			}

			return parse(vec);
		}

		std::string get_option(const std::string& key, const std::string& default_val = "") const 
		{
			auto it = options.find(key);
			return it != options.end() ? it->second : default_val;
		}

		bool has_flag(const std::string& name) const { return flags.count(name); }

		const std::vector<std::string>& get_positional() const { return positional; }

	public:
		std::string& operator[](size_t idx) { return positional[idx]; }
		const std::string& operator[](size_t idx) const { return positional[idx]; }
	};

	class Command
	{
	public:
		using Handler = std::function<Result<void*, std::string>(const CommandArgs&)>;

		Command() = default;
		Command(const std::string& name, Handler handler) : name(name), description(""), handler(handler) {}
		Command(const std::string& name, const std::string& description, Handler handler) : name(name), description(description), handler(handler) {}

	public:
		Result<void*, std::string> invoke(const CommandArgs& args) const { return handler(args); }
		Result<void*, std::string> invoke(const std::string& args_str) const { return handler(CommandArgs::parse(args_str)); }

	public:
		const std::string& get_name() const { return name; }
		void get_name(const std::string& val) { name = val; }

		const std::string& get_description() const { return description; }
		void get_description(const std::string& val) { description = val; }

	private:
		std::string name;
		std::string description;
		Handler handler;
	};
}

#endif // INCLUDE_CMDKIT_COMMAND