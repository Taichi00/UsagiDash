#pragma once
#include <string>
#include <map>
#include <memory>

class Resource;
class Texture2D;
class Model;

class ResourceManager {
public:
	ResourceManager();
	~ResourceManager();

	template<class T> std::shared_ptr<T> Load(const std::string& key) // リソースを読み込む
	{
		std::shared_ptr<T> resource;

		if (m_resourceMap.find(key) == m_resourceMap.end())	// 見つからなかった場合
		{
			resource = T::Load(key);

			// 読み込み失敗
			if (!resource)
			{
				printf("Resource [%s] の読み込みに失敗\n", key.c_str());
				return nullptr;
			}

			m_resourceMap[key] = resource;
		}
		else // 見つかった場合
		{
			resource = std::static_pointer_cast<T>(m_resourceMap[key]);
		}

		return resource;
	}

	template<class T> std::shared_ptr<T> Get(const std::string& key) // リソースを取得する
	{
		if (m_resourceMap.find(key) == m_resourceMap.end())	// 見つからなかった場合
		{
			return nullptr;
		}

		return std::static_pointer_cast<T>(m_resourceMap[key]);
	}

	bool Release(const std::string& key);	// リソースを開放する

	void AddResource(std::shared_ptr<Resource> resource);	// リソースを追加する

private:
	std::string GetExtension(const std::string& path);	// 拡張子を取得する

private:
	std::map<std::string, std::shared_ptr<Resource>> m_resourceMap;	// リソースマップ
};