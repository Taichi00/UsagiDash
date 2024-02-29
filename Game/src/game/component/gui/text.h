#pragma once

#include "game/component/gui/text_property.h"
#include "math/range.h"
#include <string>
#include <vector>

class Text
{
public:
	struct FormatTagInfo
	{
		std::wstring key = L"";
		std::wstring value = L"";
		Range range = {};
	};

	Text();
	~Text() {}

	void Parse();

	std::vector<FormatTagInfo> FormatTags() const { return format_tags_; }

public:
	std::wstring string = L"";
	TextProperty prop;

private:
	std::vector<FormatTagInfo> format_tags_;
};