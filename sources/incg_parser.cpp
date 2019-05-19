#include "incg_parser.hpp"

#include "incg_language_definitions.hpp"

#include <string>
#include <vector>
#include <iostream>
#include <stack>

#include <assert.h>

// Typedef complexity
// https://en.cppreference.com/w/cpp/language/typedef

namespace incg
{
	enum class State
	{
		global_scope,
		comment_line,
		string_litteral,
		string_litteral_agregation,
		string_list,
		project_block,
		project_name_property,
		project_output_folder_property,
		project_sources_folders_property,
		project_include_directories_property,

		eof
	};

	static std::string state_names[] = {
		"global_scope",
		"comment_line",
		"current_string_litteral",
		"string_litteral_agregation",
		"current_string_list",
		"project_block",
		"project_name_property",
		"project_output_folder_property",
		"project_sources_folders_property",
		"project_include_directories_property",

		"eof"
	};

	static bool	is_one_line_state(State state)
	{
		return state == State::comment_line;
	}

	bool incg::parse_configuration(const std::vector<Token>& tokens, Configuration& result)
	{
		std::stack<State>				states;
		Token							name_token;
		size_t							previous_line = 0;
		std::string_view* current_string_litteral = nullptr;	// @Warning current because it directly point on the value
		std::vector<std::string_view>* current_string_list = nullptr;		// @Warning current because it directly point on the value
		bool							start_new_line = true;
		const char* __string_views_buffer = nullptr;	// @Warning all string views are about this __string_views_buffer


		// Those stacks should have the same size
		// As stack to handle members and methods

		// @Warning used for debug purpose
		size_t  print_start = 0;
		size_t  print_end = 0;

		states.push(State::global_scope);

		if (tokens.size()) {
			__string_views_buffer = tokens[0].text.data();	// @Warning all string views are about this __string_views_buffer
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

			if (state == State::project_block)
			{
				if (token.keyword == Keyword::name) {
					states.push(State::project_name_property);
				}
				else if (token.keyword == Keyword::output_folder) {
					states.push(State::project_output_folder_property);
				}
				else if (token.keyword == Keyword::sources_folders) {
					states.push(State::project_sources_folders_property);
				}
				else if (token.keyword == Keyword::include_directories) {
					states.push(State::project_include_directories_property);
				}
				else if (token.punctuation == Punctuation::close_brace) {
					states.pop();
				}
				else if (token.punctuation == Punctuation::open_brace) {
					result.projects.push_back(Project());
				}
				// @Warning
				else if (token.punctuation == Punctuation::hash) {
					states.push(State::comment_line);
				}
				else {
					std::cerr << "Syntax error near: " << token.text << " line: " << token.line << " column: " << token.column << std::endl
						<< "\t" "A project property is expected [name, output_folder, sources_folders, include_directories] or '{' and '}' characters to delemit the Project block." << std::endl;
					return false;
				}
			}
			else if (state == State::project_name_property)
			{
				if (token.punctuation == Punctuation::colon) {
					current_string_litteral = &result.projects.back().name;
					states.pop();	// @Warning this state ends at the same time as the string_litteral
					states.push(State::string_litteral);
				}
				else {
					std::cerr << "Syntax error near: " << token.text << " line: " << token.line << " column: " << token.column << std::endl
						<< "\t" "The ':' assignment character to assign the string value." << std::endl;
					return false;
				}
			}
			else if (state == State::project_sources_folders_property)
			{
				if (token.punctuation == Punctuation::colon) {
					current_string_list = &result.projects.back().sources_folders;
					states.pop();	// @Warning this state ends at the same time as the string_list
					states.push(State::string_list);
				}
				else {
					std::cerr << "Syntax error near: " << token.text << " line: " << token.line << " column: " << token.column << std::endl
						<< "\t" "The ':' assignment character to assign the string list value." << std::endl;
					return false;
				}
			}
			else if (state == State::project_include_directories_property)
			{
				if (token.punctuation == Punctuation::colon) {
					current_string_list = &result.projects.back().include_directories;
					states.pop();	// @Warning this state ends at the same time as the string_list
					states.push(State::string_list);
				}
				else {
					std::cerr << "Syntax error near: " << token.text << " line: " << token.line << " column: " << token.column << std::endl
						<< "\t" "The ':' assignment character to assign the string list value." << std::endl;
					return false;
				}
			}
			else if (state == State::project_output_folder_property)
			{
				if (token.punctuation == Punctuation::colon) {
					current_string_litteral = &result.projects.back().output_folder;
					states.pop();	// @Warning this state ends at the same time as the string_litteral
					states.push(State::string_litteral);
				}
				else {
					std::cerr << "Syntax error near: " << token.text << " line: " << token.line << " column: " << token.column << std::endl
						<< "\t" "The ':' assignment character to assign the string value." << std::endl;
					return false;
				}
			}
			else if (state == State::string_list)
			{
				if (token.punctuation == Punctuation::close_brace) {
					current_string_litteral = nullptr;
					states.pop();
				}
				else if (token.punctuation == Punctuation::open_brace) {
					current_string_list->push_back(std::string_view());
					current_string_litteral = &current_string_list->back();

					states.push(State::string_litteral);
				}
				else if (token.punctuation == Punctuation::comma) {
					current_string_list->push_back(std::string_view());
					current_string_litteral = &current_string_list->back();

					states.push(State::string_litteral);
				}
				// @Warning
				else if (token.punctuation == Punctuation::hash) {
					states.push(State::comment_line);
				}
				else {
					std::cerr << "Syntax error near: " << token.text << " line: " << token.line << " column: " << token.column << std::endl
						<< "\t" "Only '{', '}' and ',' characters are allowed to delemit the string list or separate elements." << std::endl;
					return false;
				}
			}
			else if (state == State::string_litteral)
			{
				if (token.punctuation == Punctuation::double_quote) {
					states.pop();	// @Warning string_litteral_agregation will finish on the double_quote too
					states.push(State::string_litteral_agregation);
				}
				// @Warning
				else if (token.punctuation == Punctuation::hash) {
					states.push(State::comment_line);
				}
				else {
					std::cerr << "Syntax error near: " << token.text << " line: " << token.line << " column: " << token.column << std::endl
						<< "\t" "Only '\"' character is allowed to delemit the string." << std::endl;
					return false;
				}
			}
			else if (state == State::string_litteral_agregation)
			{
				if (token.punctuation == Punctuation::double_quote) {
					current_string_litteral = nullptr;
					states.pop();
				}
				else if (current_string_litteral)
				{
					// Building the string litteral (can be splitted into multiple tokens)
					if (current_string_litteral->length() == 0) {
						*current_string_litteral = token.text;
					}
					else {
						*current_string_litteral = std::string_view(
							current_string_litteral->data(),
							(token.text.data() + token.text.length()) - current_string_litteral->data());    // Can't simply add length of current_string_litteral and token.text because white space tokens are skipped
					}
				}
				else {
					assert(false);
				}
			}
			else if (state == State::global_scope)
			{
				if (token.keyword == Keyword::project) {
					states.push(State::project_block);
				}
				// @Warning
				else if (token.punctuation == Punctuation::hash) {
					states.push(State::comment_line);
				}
				else {
					std::cerr << "Syntax error near: " << token.text << " line: " << token.line << " column: " << token.column << std::endl
						<< "\t" "Only comments and the Project are allowed in the global scope." << std::endl;
					return false;
				}
			}
			start_new_line = false;
			previous_line = token.line;
		}

		// @Warning we should finish on the global_scope state or one that can stay active only on one line
		assert(states.size() >= 1 && states.size() <= 2);
		assert(states.top() == State::global_scope
			|| is_one_line_state(states.top()));

		return true;
	}
}
