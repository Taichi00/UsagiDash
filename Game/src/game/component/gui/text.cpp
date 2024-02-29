#include "text.h"

Text::Text()
{
	format_tags_.clear();
}

void Text::Parse()
{
	auto text_p = string.data();
	auto start = text_p;

	format_tags_.clear();

	while (*text_p)
	{
		if (*text_p == '<')
		{
			Range range = {};
			range.start = (unsigned int)(text_p - start);

			++text_p;

			// �L�[��ǂݎ��
			std::wstring key = L"";
			while (*text_p > 32)
			{
				key += *text_p;
				++text_p;
			}

			++text_p;

			// �l��ǂݎ��
			std::wstring value = L"";
			while (*text_p != '>')
			{
				value += *text_p;
				++text_p;
			}

			range.length = (unsigned int)(text_p - start - range.start) + 1;

			// �^�O��ǉ�
			FormatTagInfo tag = {};
			tag.key = key;
			tag.value = value;
			tag.range = range;
			
			format_tags_.push_back(tag);
		}

		++text_p;
	}
}