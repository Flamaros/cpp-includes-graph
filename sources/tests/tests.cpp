#include "../macro_tokenizer.hpp"
#include "../macro_parser.hpp"

#include <CppUnitTest.h>

#include <vector>

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace tests
{
	TEST_CLASS(macro_tokenizer_tests)
	{
	public:
		
		TEST_METHOD(empty_text)
		{
			std::vector<Token>	tokens;
			std::string			text =
				"";

			tokenize(text, tokens);

			Assert::AreEqual(tokens.size(), size_t(0));
		}

		TEST_METHOD(white_spaces_text)
		{
			std::vector<Token>	tokens;
			std::string			text =
				"  \t  \r\n";

			tokenize(text, tokens);

			Assert::AreEqual(tokens.size(), size_t(0));
		}

		TEST_METHOD(comment_line_text_01)
		{
			std::vector<Token>	tokens;
			std::string			text =
				"// ";

			tokenize(text, tokens);

			Assert::AreEqual(tokens.size(), size_t(1));
			Assert::AreEqual((int)tokens[0].punctuation, (int)Punctuation::line_comment);
		}

		TEST_METHOD(comment_line_text_02)
		{
			std::vector<Token>	tokens;
			std::string			text =
				"/// ";

			tokenize(text, tokens);

			Assert::AreEqual(tokens.size(), size_t(2));
			Assert::AreEqual((int)tokens[0].punctuation, (int)Punctuation::line_comment);
		}

		TEST_METHOD(comment_line_text_03)
		{
			std::vector<Token>	tokens;
			std::string			text =
				"///\r\n"
				"///\r\n";

			tokenize(text, tokens);

			Assert::AreEqual(tokens.size(), size_t(4));
			Assert::AreEqual((int)tokens[0].punctuation, (int)Punctuation::line_comment);
			Assert::AreEqual((int)tokens[0].line, (int)1);
			Assert::AreEqual((int)tokens[1].punctuation, (int)Punctuation::slash);
			Assert::AreEqual((int)tokens[1].line, (int)1);
			Assert::AreEqual((int)tokens[2].punctuation, (int)Punctuation::line_comment);
			Assert::AreEqual((int)tokens[2].line, (int)2);
			Assert::AreEqual((int)tokens[3].punctuation, (int)Punctuation::slash);
			Assert::AreEqual((int)tokens[3].line, (int)2);
		}

		TEST_METHOD(include_text)
		{
			std::vector<Token>	tokens;
			std::string			text =
				"/// comment\r\n"
				"\r\n"
				"#include <string>\r\n";

			tokenize(text, tokens);

			Assert::AreEqual(tokens.size(), size_t(8));
			Assert::AreEqual((int)tokens[0].punctuation, (int)Punctuation::line_comment);
			Assert::AreEqual((int)tokens[1].punctuation, (int)Punctuation::slash);
			Assert::AreEqual(std::string(tokens[2].text), std::string("comment"));
			Assert::AreEqual((int)tokens[3].punctuation, (int)Punctuation::hash);
			Assert::AreEqual((int)tokens[4].keyword, (int)Keyword::_include);
			Assert::AreEqual((int)tokens[5].punctuation, (int)Punctuation::less);
			Assert::AreEqual(std::string(tokens[6].text), std::string("string"));
			Assert::AreEqual((int)tokens[7].punctuation, (int)Punctuation::greater);
		}
	};

	TEST_CLASS(macro_parser)
	{
	public:

		TEST_METHOD(includes)
		{
			Macro_Parsing_Result	parsing_result;
			std::vector<Token>		tokens;
			std::string				text =
				"#include <string>\r\n"
				"#include \"assert.h\"";

			tokenize(text, tokens);

			parse_macros(tokens, parsing_result);

			Assert::AreEqual(parsing_result.includes.size(), size_t(2));
		}

		TEST_METHOD(glm_hpp_bug_01)
		{
			Macro_Parsing_Result	parsing_result;
			std::vector<Token>		tokens;
			std::string				text =
				"/// comment\r\n"
				"\r\n"
				"#include <string>\r\n";

			tokenize(text, tokens);

			parse_macros(tokens, parsing_result);

			Assert::AreEqual(parsing_result.includes.size(), size_t(1));
		}

		TEST_METHOD(glm_hpp_bug_02)
		{
			Macro_Parsing_Result	parsing_result;
			std::vector<Token>		tokens;
			std::string				text =
				"/// <a href=\"http://www.opengl.org/registry/doc/GLSLangSpec.4.20.8.clean.pdf\">version 4.2\n"
				"\n"
				"#include \"detail/_fixes.hpp\"\n";

			tokenize(text, tokens);

			parse_macros(tokens, parsing_result);

			Assert::AreEqual(parsing_result.includes.size(), size_t(1));
		}
	};
}
