#include "macro_parser.hpp"

#include "macro_language_definitions.hpp"

#include <string>
#include <vector>
#include <iostream>
#include <stack>

#include <assert.h>

// Typedef complexity
// https://en.cppreference.com/w/cpp/language/typedef

enum class State
{
	global_scope,
	comment_line,
	comment_block,
	macro_expression,
	include_directive,

	eof
};

std::string stateNames[] = {
	"global_scope",
	"comment_line",
	"comment_block",
	"macro_expression",
	"include_directive",

	"eof"
};

void parse_cpp(const std::vector<Token>& tokens, Cpp_Parse_Result& result)
{
	std::stack<State>	states;
	Token				nameToken;
	string_ref			stringLiteral;
	size_t				previousLine = 0;
	string_ref          currentFilePath;
	bool				inStringLiteral = false;
	bool				start_new_line = true;

	// Those stacks should have the same size
	// As stack to handle members and methods

	// @Warning used for debug purpose
	size_t  printStart = 0;
	size_t  printEnd = 0;

	states.push(State::global_scope);

	for (const Token& token : tokens)
	{
		State& state = states.top();

		// Handle here states that have to be poped on new line detection
		if (token.line > previousLine
			&& (state == State::macro_expression	// Actually we don't manage every macro directive (we stay on this state)
				|| state == State::comment_line))
		{
			states.pop();
			state = states.top();
			start_new_line = true;
		}

		//if (token.line >= printStart && token.line < printEnd)
		//	std::cout << std::string(states.size() - 1, ' ') << stateNames[(size_t)state]
		//	<< " " << token.line << " " << token.column << " " << token.text.to_string() << std::endl;

		if (state == State::comment_block)
		{
			if (token.punctuation == Punctuation::CloseBlockComment)
				states.pop();
		}
		else if (state == State::macro_expression)
		{
			if (token.keyword == Keyword::_include)
			{
				states.push(State::include_directive);
			}
		}
		else if (state == State::include_directive)
		{
			if (token.punctuation == Punctuation::DoubleQuote
				|| token.punctuation == Punctuation::Less
				|| token.punctuation == Punctuation::Greater)
			{
				if (inStringLiteral)
				{
					Include	include;

					include.type = (token.punctuation == Punctuation::Greater) ? Include_Type::external : Include_Type::local;
					include.path = currentFilePath;
					result.includes.push_back(include);

					inStringLiteral = false;
					states.pop();
				}
				else
				{
					inStringLiteral = true;
					currentFilePath = string_ref();
				}
			}
			else if (token.keyword == Keyword::_unknown
				&& token.punctuation == Punctuation::Unknown)
			{
				// Building the include path (can be splitted into multiple tokens)
				if (inStringLiteral)
				{
					if (currentFilePath.length() == 0)
						currentFilePath = token.text;
					else
						currentFilePath = string_ref(
							token.text.string(),
							currentFilePath.position(),
							(token.text.position() + token.text.length()) - currentFilePath.position());    // Can't simply add length of currentFileName and token.text because white space tokens are skipped
				}
			}
		}
		else
		{
			if (start_new_line	// @Warning to be sure that we are on the beginning of the line
				&& token.punctuation == Punctuation::Hash)    // Macro
				states.push(State::macro_expression);
		}
		start_new_line = false;
		previousLine = token.line;
	}

	assert(states.size() >= 1 && states.size() <= 2);
	assert(states.top() == State::global_scope
		|| states.top() == State::macro_expression);
}
