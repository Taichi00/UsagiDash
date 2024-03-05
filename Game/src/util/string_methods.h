#pragma once

#include <string>
#include <windows.h>
#include <filesystem>

namespace fs = std::filesystem;

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

	static std::string GetString(const std::wstring& value)
	{
		auto length = WideCharToMultiByte(CP_UTF8, 0U, value.data(), -1, nullptr, 0, nullptr, nullptr);
		auto buffer = new char[length];

		WideCharToMultiByte(CP_UTF8, 0U, value.data(), -1, buffer, length, nullptr, nullptr);

		std::string result(buffer);
		delete[] buffer;
		buffer = nullptr;

		return result;
	}

	// �g���q��Ԃ�
	static std::wstring GetFileExtension(const std::wstring& path)
	{
		auto idx = path.rfind(L'.');
		return path.substr(idx + 1, path.length() - idx - 1);
	}
	
	// �f�B���N�g���̃p�X��Ԃ�
	static std::wstring GetDirectoryPath(const std::wstring& origin)
	{
		fs::path p = origin.c_str();
		return p.remove_filename().c_str();
	}
};