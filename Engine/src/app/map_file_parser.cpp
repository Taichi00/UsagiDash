#include "map_file_parser.h"
#include <iostream>
#include <fstream>

MapFileParser::MapFileParser()
{
	data_ptr_ = nullptr;
	token_ = "";
}

bool MapFileParser::Load(const std::wstring& path, Map& map)
{
	data_ptr_ = nullptr;
	token_ = "";

	std::string buf;

	LoadFile(path, buf);

	StartTokenParsing(buf.data());

	std::vector<Entity>& entities = map.entities;
	while (true)
	{
		Entity entity;
		if (!ParseEntity(entity)) break;
		entities.push_back(entity);
	}

	return true;
}

float MapFileParser::ToFloat(const std::string& s)
{
	return std::stof(s);
}

Vec3 MapFileParser::ToVec3(const std::string& s)
{
	Vec3 v;

	int i = 0;
	std::string token = "";

	const char* p = s.data();
	while (true)
	{
		// 空白文字
		if (*p <= 32)
		{
			v[i] = std::stof(token);
			if (++i > 2) break;
			token = "";
		}
		token += *p++;
	}

	float y = v.y;
	v.y = v.z;
	v.z = -y;

	return v;
}

bool MapFileParser::LoadFile(const std::wstring& path, std::string& buf)
{
	std::ifstream ifs(path);

	if (!ifs)
	{
		printf("Failed to open file. %ls\n", path.c_str());
		return false;
	}

	std::string str;
	while (std::getline(ifs, str))
	{
		buf += str + '\n';
	}

	ifs.close();
	return true;
}

void MapFileParser::StartTokenParsing(char* data)
{
	data_ptr_ = data;
}

bool MapFileParser::GetToken()
{
	// トークンをリセット
	token_ = "";

	// コメントでない行までスキップ
	while (true)
	{
		// 空白文字（文字コード<=32）をスキップ
		while (*data_ptr_ <= 32)
		{
			// EOF
			if (!*data_ptr_) return false;

			data_ptr_++;
		}

		// コメント行をスキップ
		if (data_ptr_[0] == '/' && data_ptr_[1] == '/')
		{
			while (*data_ptr_ != '\n')
			{
				// EOF
				if (!*data_ptr_) return false;

				data_ptr_++;
			}
			continue;
		}
		else
		{
			break;
		}
	}

	// クォーテーションの場合は中身をトークンとして格納
	if (*data_ptr_ == '"')
	{
		data_ptr_++;
		while (*data_ptr_ != '"')
		{
			// EOF
			if (!*data_ptr_) return false;

			token_ += *data_ptr_++;
		}
		data_ptr_++;
	}
	else while (*data_ptr_ > 32) // 空白文字までをトークンとして格納
	{
		token_ += *data_ptr_++;
	}

	return true;
}

bool MapFileParser::ParsePair(std::string& value)
{
	if (!GetToken()) return false;

	value = token_;

	return true;
}

bool MapFileParser::ParseBrush()
{
	// 使わないのでブロックの終わりまでスキップ
	while (*data_ptr_ != '}')
	{
		// EOF
		if (!*data_ptr_) return false;
		data_ptr_++;
	}

	data_ptr_++;
	return true;
}

bool MapFileParser::ParseEntity(Entity& entity)
{
	// トークンを取得
	if (!GetToken()) return false;

	// トークンが { でない場合終了
	if (token_ != "{")
	{
		printf("ParseEntity: { not found\n");
		return false;
	}

	while (true)
	{
		// トークンを取得
		if (!GetToken()) return false;
		
		// } なら終了
		if (token_ == "}") break;

		if (token_ == "{") // Brush
		{
			if (!ParseBrush()) return false;
		}
		else // Pair
		{
			if (!ParsePair(entity.pairs[token_])) return false;
		}
	}

	return true;
}
