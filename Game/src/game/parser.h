#pragma once
#include <iostream>
#include <sstream>
#include <string>
#include <functional>

namespace parser
{
	class Source
	{
		const char* s;
		int line, col;

	public:
		Source(const char* s) : s(s), line(1), col(1) {}
		
		char peek()
		{
			char ch = *s;
			if (!ch) throw ex("too short");
			return ch;
		}

		void next()
		{
			char ch = peek();
			if (ch == '\n')
			{
				++line;
				col = 0;
			}
			++s;
			++col;
		}
		
		std::string ex(const std::string& e)
		{
			std::ostringstream oss;
			oss << "[line " << line << ", col" << col << "] " << e;
			if (*s) oss << ": '" << *s << "'";
			return oss.str();
		}

		bool operator==(const Source& src) const
		{
			return s == src.s;
		}

		bool operator!=(const Source& src) const
		{
			return !(*this == src);
		}
	};

	template <typename T>
	using Parser = std::function<T(Source*)>;

	template <typename T>
	void parseTest(const Parser<T>& p, const Source& src)
	{
		Source s = src;
		try
		{
			std::cout << p(&s) << std::endl;
		}
		catch (const std::string& e)
		{
			std::cout << e << std::endl;
		}
	}

	template <typename T>
	Parser<T> tryp(const Parser<T>& p)
	{
		return [=](Source* s)
			{
				T ret;
				Source bak = *s;
				try
				{
					ret = p(s);
				}
				catch (const std::string&)
				{
					*s = bak;
					throw;
				}
				return ret;
			};
	}

	template <typename T1, typename T2>
	Parser<std::string> operator+(const Parser<T1>& x, const Parser<T2>& y)
	{
		return [=](Source* s)
			{
				std::string ret;
				ret += x(s);
				ret += y(s);
				return ret;
			};
	}

	template <typename T>
	Parser<std::string> operator*(int n, const Parser<T>& x)
	{
		return [=](Source* s)
			{
				std::string ret;
				for (int i = 0; i < n; ++i)
				{
					ret += x(s);
				}
				return ret;
			};
	}

	template <typename T>
	Parser<std::string> operator*(const Parser<T>& x, int n)
	{
		return n * x;
	}

	template <typename T>
	const Parser<T> operator||(const Parser<T>& p1, const Parser<T>& p2)
	{
		return [=](Source* s)
			{
				T ret;
				Source bak = *s;
				try
				{
					ret = p1(s);
				}
				catch (const std::string&)
				{
					if (*s != bak) throw;
					ret = p2(s);
				}
				return ret;
			};
	}

	template <typename T>
	Parser<T> left(const std::string& e)
	{
		return [=](Source* s) -> T
			{
				throw s->ex(e);
			};
	}

	Parser<char> left(const std::string& e)
	{
		return left<char>(e);
	}

	template <typename T>
	Parser<std::string> many(const Parser<T>& p)
	{
		return [=](Source* s)
			{
				std::string ret;
				try
				{
					for (;;) ret += p(s);
				}
				catch (const std::string&) {}
				return ret;
			};
	}

	template <typename T>
	Parser<std::string> many1(const Parser<T>& p)
	{
		return p + many(p);
	}

	Parser<char> satisfy(const std::function<bool(char)>& f)
	{
		return [=](Source* s)
			{
				char ch = s->peek();
				if (!f(ch)) throw s->ex("not satisfy");
				s->next();
				return ch;
			};
	}

	auto anyChar = satisfy([](char) { return true; });

	Parser<char> char1(char ch) {
		return satisfy([=](char c) { return c == ch; })
			|| left(std::string("not char '") + ch + "'");
	}

	Parser<std::string> string(const std::string& str)
	{
		return [=](Source* s)
			{
				for (auto it = str.begin(); it != str.end(); ++it)
				{
					(char1(*it) || left("not string \"" + str + "\""))(s);
				}
				return str;
			};
	}

	int toInt(const std::string& s)
	{
		int ret;
		std::istringstream(s) >> ret;
		return ret;
	}

	bool isDigit(char ch) { return '0' <= ch && ch <= '9'; }
	bool isUpper(char ch) { return 'A' <= ch && ch <= 'Z'; }
	bool isLower(char ch) { return 'a' <= ch && ch <= 'z'; }
	bool isAlpha(char ch) { return isUpper(ch) || isLower(ch); }
	bool isAlphaNum(char ch) { return isAlpha(ch) || isDigit(ch); }
	bool isLetter(char ch) { return isAlpha(ch) || ch == '_'; }
	bool isQuotation(char ch) { return ch == '\'' || ch == '\"'; }

	auto digit = satisfy(isDigit) || left("not digit");
	auto upper = satisfy(isUpper) || left("not upper");
	auto lower = satisfy(isLower) || left("not lower");
	auto alpha = satisfy(isAlpha) || left("not alpha");
	auto alphaNum = satisfy(isAlphaNum) || left("not alphaNum");
	auto letter = satisfy(isLetter) || left("not letter");
	auto quotation = satisfy(isQuotation) || left("not quatation");

	Parser<int> number = [](Source* s)
		{
			return toInt(many1(digit)(s));
		};

	auto test1 = anyChar + anyChar;
	auto test2 = test1 + anyChar;
	auto test3 = letter + 2 * digit;
	auto test4 = many(alpha);
	auto test5 = letter || digit;
	auto test6 = many(letter || digit);

	auto a = char1('a');
	auto b = char1('b');
	auto c = char1('c');

	auto test7 = a + b || b + c;
	auto test8 = a + b || a + c;
	auto test9 = a + (b || c);
	auto test10 = tryp(a + b) || a + c;
	auto test11 = string("ab") || string("ac");
	auto test12 = tryp(string("ab")) || string("ac");

	auto test = quotation + many(letter || digit) + quotation;

	int main() {
		parseTest(test, "\"123\"");

		return 0;
	}
};

