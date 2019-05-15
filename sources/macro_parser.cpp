#include "macro_parser.hpp"

#include "macro_language_definitions.hpp"

#include <string>
#include <vector>
#include <iostream>
#include <stack>

#include <assert.h>

// Typedef complexity
// https://en.cppreference.com/w/cpp/language/typedef

namespace macro
{
	enum class State
	{
		global_scope,
		comment_line,
		comment_block,
		macro_expression,
		include_directive,

		eof
	};

	static std::string state_names[] = {
		"global_scope",
		"comment_line",
		"comment_block",
		"macro_expression",
		"include_directive",

		"eof"
	};

	static bool	is_one_line_state(State state)
	{
		return state == State::macro_expression	// Actually we don't manage every macro directive (we stay on this state)
			|| state == State::comment_line;
	}

	void macro::parse_macros(const std::vector<Token>& tokens, Macro_Parsing_Result& result)
	{
		std::stack<State>	states;
		Token				name_token;
		size_t				previous_line = 0;
		std::string_view    string_litteral;
		bool				in_string_literal = false;
		bool				start_new_line = true;
		const char*			string_views_buffer = nullptr;	// @Warning all string views are about this string_views_buffer

		// Those stacks should have the same size
		// As stack to handle members and methods

		// @Warning used for debug purpose
		size_t  print_start = 0;
		size_t  print_end = 0;

		states.push(State::global_scope);

		if (tokens.size()) {
			string_views_buffer = tokens[0].text.data();	// @Warning all string views are about this string_views_buffer
		}

		for (const Token& token : tokens)
		{
			State& state = states.top();

			if (token.line > previous_line) {
				start_new_line = true;
			}

			// Handle here states that have to be poped on new line detection
			if (start_new_line
				&& is_one_line_state(state))
			{
				states.pop();
				state = states.top();
			}

			if (token.line >= print_start && token.line < print_end) {
				std::cout
					<< std::string(states.size() - 1, ' ') << state_names[(size_t)state]
					<< " " << token.line << " " << token.column << " " << token.text << std::endl;
			}

			if (state == State::comment_block)
			{
				if (token.punctuation == Punctuation::close_block_comment)
					states.pop();
			}
			else if (state == State::macro_expression)
			{
				if (token.keyword == Keyword::_include)
				{
					states.pop();
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
						string_litteral = std::string_view();
					}
				}
				else if (token.keyword == Keyword::_unknown
					&& token.punctuation == Punctuation::unknown)
				{
					// Building the string litteral (can be splitted into multiple tokens)
					if (in_string_literal)
					{
						if (string_litteral.length() == 0)
							string_litteral = token.text;
						else
							string_litteral = std::string_view(
								string_litteral.data(),
								(token.text.data() + token.text.length()) - string_litteral.data());    // Can't simply add length of string_litteral and token.text because white space tokens are skipped
					}
				}
			}
			else if (state == State::global_scope)
			{
				if (start_new_line	// @Warning to be sure that we are on the beginning of the line
					&& token.punctuation == Punctuation::hash) {   // Macro
					states.push(State::macro_expression);
				}
				else if (token.punctuation == Punctuation::open_block_comment) {
					states.push(State::comment_block);
				}
				else if (token.punctuation == Punctuation::line_comment) {
					states.push(State::comment_line);
				}
			}
			start_new_line = false;
			previous_line = token.line;
		}

		// @Warning we should finish on the global_scope state or one that can stay active only on one line
		assert(states.size() >= 1 && states.size() <= 2);
		assert(states.top() == State::global_scope
			|| is_one_line_state(states.top()));
	}
}
