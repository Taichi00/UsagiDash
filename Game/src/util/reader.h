#pragma once

#include <vector>
#include <string>

class Reader
{
public:
	Reader() {}
	
	// ƒf[ƒ^‚ğ“Ç‚İ‚Ş
	std::vector<char> ReadData(const std::wstring& filename);
};