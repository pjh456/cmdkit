#ifndef INCLUDE_CMDKIT
#define INCLUDE_CMDKIT

#include <type_traits>
#include <stdexcept>
#include <variant>
#include <string>
#include <vector>
#include <unordered_set>
#include <unordered_map>
#include <functional>
#include <map>

// result.hpp
namespace cmdkit
{
	template<typename T, typename E>
	class Result;

	template<typename>
	struct is_result : std::false_type {};

	template<typename T, typename E>
	struct is_result<Result<T, E>> : std::true_type {};

	class bad_result_access : public std::logic_error
	{
	public:
		explicit bad_result_access(const std::string& msg) : std::logic_error(msg) {}
		explicit bad_result_access(const char* msg) : std::logic_error(msg) {}
	};

	template<typename T, typename E>
	class Result
	{
		static_assert(!std::is_same_v<T, E>, "Result<T, E>: T and E must not be the same type");
	public:
		using OkType = T;
		using ErrType = E;

	private:
		bool ok_flag;
		std::variant<T, E> data;

		Result(const T& val) : data(val), ok_flag(true) {}
		Result(T&& val) noexcept : data(std::move(val)), ok_flag(true) {}

		Result(const E& val) : data(val), ok_flag(false) {}
		Result(E&& val) noexcept : data(std::move(val)), ok_flag(false) {}

	public:
		static Result<T, E> ok(const T& val) { return Result<T, E>(val); }
		static Result<T, E> ok(T&& val) noexcept { return Result<T, E>(std::move(val)); }

		static Result<T, E> err(const E& val) { return Result<T, E>(val); }
		static Result<T, E> err(E&& val) noexcept { return Result<T, E>(std::move(val)); }

		~Result() = default;

	public:
		Result& operator=(const Result& other) { ok_flag = other.ok_flag, data = other.data; return *this; }

		Result& operator=(Result&& other) noexcept { ok_flag = other.ok_flag, data = std::move(other.data); return *this; }

	public:
		bool is_ok() const noexcept { return ok_flag; }
		bool is_err() const noexcept { return !ok_flag; }
		operator bool() const { return is_ok(); }

	public:
		T& unwrap()& { if (is_ok()) return std::get<T>(data); else throw bad_result_access("unwrap called on an error Result"); }
		const T& unwrap() const& { if (is_ok()) return std::get<T>(data); else throw bad_result_access("unwrap called on an error Result"); }
		T unwrap()&& { if (is_ok()) return std::move(std::get<T>(data)); else throw bad_result_access("unwrap called on an error Result"); }

		E& unwrap_err()& { if (is_err()) return std::get<E>(data); else throw bad_result_access("unwrap_err called on an okay Result"); }
		const E& unwrap_err() const& { if (is_err()) return std::get<E>(data); else throw bad_result_access("unwrap_err called on an okay Result"); }
		E unwrap_err()&& { if (is_err()) return std::move(std::get<E>(data)); else throw bad_result_access("unwrap_err called on an okay Result"); }

	public:
		T& unwrap_or(const T& default_val) & noexcept { return is_ok() ? unwrap() : default_val; }
		const T& unwrap_or(const T& default_val) const& noexcept { return is_ok() ? unwrap() : default_val; }
		T unwrap_or(T default_val) && noexcept { return is_ok() ? std::move(unwrap()) : default_val; }

		E& unwrap_err_or(const E& default_val) & noexcept { return is_err() ? unwrap_err() : default_val; }
		const E& unwrap_err_or(const E& default_val) const& noexcept { return is_err() ? unwrap_err() : default_val; }
		E unwrap_err_or(E default_val) && noexcept { return is_err() ? std::move(unwrap_err()) : default_val; }

	public:
		template<typename F>
		auto map(F&& f) const& ->
			std::enable_if_t<
			std::is_invocable_v<F, const T&>,
			Result<decltype(f(std::declval<const T&>())), E>
			>
		{
			using U = decltype(f(std::declval<const T&&>()));
			if (is_ok()) return Result<U, E>::ok(f(unwrap()));
			else return Result<U, E>::err(std::move(unwrap_err()));
		}

		template<typename F>
		auto map(F&& f) && ->
			std::enable_if_t<
			std::is_invocable_v<F, T&&>,
			Result<decltype(f(std::declval<T&&>())), E>
			>
		{
			using U = decltype(f(std::declval<T&&>()));
			if (is_ok()) return Result<U, E>::ok(f(std::move(unwrap())));
			else return Result<U, E>::err(std::move(unwrap_err()));
		}

	public:
		template<typename F>
		auto map_err(F&& f) const& ->
			std::enable_if_t<
			std::is_invocable_v<F, const E&>,
			Result<T, decltype(f(std::declval<const E&>()))>
			>
		{
			using U = decltype(f(std::declval<const E&>()));
			if (is_err()) return Result<T, U>::err(f(unwrap_err()));
			else return Result<T, U>::ok(std::move(unwrap()));
		}

		template<typename F>
		auto map_err(F&& f) && ->
			std::enable_if_t<
			std::is_invocable_v<F, E&&>,
			Result<T, decltype(f(std::declval<E&&>()))>
			>
		{
			using U = decltype(f(std::declval<E&&>()));
			if (is_err()) return Result<T, U>::err(f(std::move(unwrap_err())));
			else return Result<T, U>::ok(std::move(unwrap()));
		}

	public:
		template<typename F>
		auto and_then(F&& f) const& ->
			std::enable_if_t<
			is_result<std::invoke_result_t<F, const T&>>::value
			&& std::is_same_v<E, typename std::invoke_result_t<F, const T&>::ErrType>,
			std::invoke_result_t<F, const T&>
			>
		{
			using Ret = std::invoke_result_t<F, const T&>;
			if (is_ok()) return std::invoke(std::forward<F>(f), unwrap());
			else return Ret::err(unwrap_err());
		}

		template<typename F>
		auto and_then(F&& f) && ->
			std::enable_if_t<
			is_result<std::invoke_result_t<F, const T&>>::value
			&& std::is_same_v<E, typename std::invoke_result_t<F, const T&>::ErrType>,
			std::invoke_result_t<F, const T&>
			>
		{
			using Ret = std::invoke_result_t<F, const T&>;
			if (is_ok()) return std::invoke(std::forward<F>(f), std::move(unwrap()));
			else return Ret::err(std::move(unwrap_err()));
		}

	public:
		template<
			typename OkFn, typename ErrFn,
			typename OkRet = std::invoke_result_t<OkFn, const T&>,
			typename ErrRet = std::invoke_result_t<ErrFn, const E&>,
			typename std::enable_if_t<std::is_same_v<OkRet, ErrRet>, int> = 0
		>
		decltype(auto) match(OkFn&& ok_func, ErrFn&& err_func) const&
		{
			if (is_ok()) return std::invoke(std::forward<OkFn>(ok_func), unwrap());
			else return std::invoke(std::forward<ErrFn>(err_func), unwrap_err());
		}

		template<
			typename OkFn, typename ErrFn,
			typename OkRet = std::invoke_result_t<OkFn, T&&>,
			typename ErrRet = std::invoke_result_t<ErrFn, E&&>,
			typename std::enable_if_t<std::is_same_v<OkRet, ErrRet>, int> = 0
		>
		decltype(auto) match(OkFn&& ok_func, ErrFn&& err_func)&&
		{
			if (is_ok()) return std::invoke(std::forward<OkFn>(ok_func), std::move(unwrap()));
			else return std::invoke(std::forward<ErrFn>(err_func), std::move(unwrap_err()));
		}
	};
}

// command.hpp
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

// terminal.hpp
namespace cmdkit
{
	class Terminal
	{
	public:
		void register_command(const std::string& name, Command cmd) { command_table[name] = cmd; }
		void register_command(Command cmd) { command_table[cmd.get_name()] = cmd; }


		void invoke(const CommandArgs& command) const
		{
			invoke(command, []() { throw std::runtime_error("Not find command!"); });
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

#endif // INCLUDE_CMDKIT