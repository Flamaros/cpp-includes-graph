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

void parse_macros(const std::vector<Token>& tokens, Macro_Parsing_Result& result)
{
	std::stack<State>	states;
	Token				name_token;
	size_t				previous_line = 0;
	string_ref          string_litteral;
	bool				in_string_literal = false;
	bool				start_new_line = true;

	// Those stacks should have the same size
	// As stack to handle members and methods

	// @Warning used for debug purpose
	size_t  print_start = 0;
	size_t  print_end = 0;

	states.push(State::global_scope);

	for (const Token& token : tokens)
	{
		State& state = states.top();

		// Handle here states that have to be poped on new line detection
		if (token.line > previous_line
			&& (state == State::macro_expression	// Actually we don't manage every macro directive (we stay on this state)
				|| state == State::comment_line))
		{
			states.pop();
			state = states.top();
			start_new_line = true;
		}

		//if (token.line >= print_start && token.line < print_end)
		//	std::cout << std::string(states.size() - 1, ' ') << stateNames[(size_t)state]
		//	<< " " << token.line << " " << token.column << " " << token.text.to_string() << std::endl;

		if (state == State::comment_block)
		{
			if (token.punctuation == Punctuation::close_block_comment)
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
			if (token.punctuation == Punctuation::double_quote
				|| token.punctuation == Punctuation::less
				|| token.punctuation == Punctuation::greater)
			{
				if (in_string_literal)
				{
					Include	include;

					include.type = (token.punctuation == Punctuation::greater) ? Include_Type::external : Include_Type::local;
					include.path = string_litteral;
					result.includes.push_back(include);

					in_string_literal = false;
					states.pop();
				}
				else
				{
					in_string_literal = true;
					string_litteral = string_ref();
				}
			}
			else if (token.keyword == Keyword::_unknown
				&& token.punctuation == Punctuation::unknown)
			{
				// Building the include path (can be splitted into multiple tokens)
				if (in_string_literal)
				{
					if (string_litteral.length() == 0)
						string_litteral = token.text;
					else
						string_litteral = string_ref(
							token.text.string(),
							string_litteral.position(),
							(token.text.position() + token.text.length()) - string_litteral.position());    // Can't simply add length of currentFileName and token.text because white space tokens are skipped
				}
			}
		}
		else
		{
			if (start_new_line	// @Warning to be sure that we are on the beginning of the line
				&& token.punctuation == Punctuation::hash)    // Macro
				states.push(State::macro_expression);
		}
		start_new_line = false;
		previous_line = token.line;
	}

	assert(states.size() >= 1 && states.size() <= 2);
	assert(states.top() == State::global_scope
		|| states.top() == State::macro_expression);
}
