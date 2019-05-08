#include "../macro_tokenizer.hpp"

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

		}
	};
}
