#pragma once

#include <vector>
#include <string>

class Reader
{
public:
	Reader() {}
	
	// �f�[�^��ǂݍ���
	std::vector<char> ReadData(const std::wstring& filename);
};