#pragma once

#include <string>
#include <windows.h>

// std::string(マルチバイト文字列)からstd::wstring(ワイド文字列)を得る
class StringMethods
{
public:
	static std::wstring GetWideString(const std::string& str)
	{
		// SJIS → wstring
		int iBufferSize = MultiByteToWideChar(CP_ACP, 0, str.c_str()
			, -1, (wchar_t*)NULL, 0);

		// バッファの取得
		wchar_t* cpUCS2 = new wchar_t[iBufferSize];

		// SJIS → wstring
		MultiByteToWideChar(CP_ACP, 0, str.c_str(), -1, cpUCS2
			, iBufferSize);

		// stringの生成
		std::wstring oRet(cpUCS2, cpUCS2 + iBufferSize - 1);

		// バッファの破棄
		delete[] cpUCS2;

		// 変換結果を返す
		return oRet;
	}

	// 拡張子を返す
	static std::wstring GetFileExtension(const std::wstring& path)
	{
		auto idx = path.rfind(L'.');
		return path.substr(idx + 1, path.length() - idx - 1);
	}
};