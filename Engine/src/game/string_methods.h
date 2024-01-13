#pragma once

#include <string>
#include <windows.h>

// std::string(�}���`�o�C�g������)����std::wstring(���C�h������)�𓾂�
class StringMethods
{
public:
	static std::wstring GetWideString(const std::string& str)
	{
		// SJIS �� wstring
		int iBufferSize = MultiByteToWideChar(CP_ACP, 0, str.c_str()
			, -1, (wchar_t*)NULL, 0);

		// �o�b�t�@�̎擾
		wchar_t* cpUCS2 = new wchar_t[iBufferSize];

		// SJIS �� wstring
		MultiByteToWideChar(CP_ACP, 0, str.c_str(), -1, cpUCS2
			, iBufferSize);

		// string�̐���
		std::wstring oRet(cpUCS2, cpUCS2 + iBufferSize - 1);

		// �o�b�t�@�̔j��
		delete[] cpUCS2;

		// �ϊ����ʂ�Ԃ�
		return oRet;
	}

	// �g���q��Ԃ�
	static std::wstring GetFileExtension(const std::wstring& path)
	{
		auto idx = path.rfind(L'.');
		return path.substr(idx + 1, path.length() - idx - 1);
	}
};