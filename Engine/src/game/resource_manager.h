#pragma once
#include <string>
#include <map>
#include <memory>
#include <typeindex>

class Resource;
class Texture2D;
class Model;

class ResourceManager {
public:
	ResourceManager();
	~ResourceManager();

	template<class T>
	std::shared_ptr<T> Load(const std::wstring& path) // リソースを読み込む
	{
		std::shared_ptr<T> resource;

		if (resource_map_[typeid(T)].find(path) == resource_map_[typeid(T)].end())	// 見つからなかった場合
		{
			resource = T::Load(path);

			// 読み込み失敗
			if (!resource)
			{
				printf("Resource [%ls] の読み込みに失敗\n", path.c_str());
				return nullptr;
			}

			resource_map_[typeid(T)][path] = resource;
		}
		else // 見つかった場合
		{
			resource = std::static_pointer_cast<T>(resource_map_[typeid(T)][path]);
		}

		return resource;
	}

	template<class T>
	std::shared_ptr<T> Get(const std::wstring& path) // リソースを取得する
	{
		if (resource_map_[typeid(T)].find(path) == resource_map_[typeid(T)].end())	// 見つからなかった場合
		{
			return nullptr;
		}

		return std::static_pointer_cast<T>(resource_map_[typeid(T)][path]);
	}

	template<class T>
	bool Release(const std::wstring& path)	// リソースを開放する
	{
		if (resource_map_[typeid(T)].find(path) == resource_map_[typeid(T)].end())	// 見つからなかった場合
		{
			return false;
		}

		resource_map_[typeid(T)][path]->Release();
		resource_map_[typeid(T)].erase(path);

		return true;
	}

	void AddResource(std::shared_ptr<Resource> resource);	// リソースを追加する

private:
	std::map<std::type_index, std::map<std::wstring, std::shared_ptr<Resource>>> resource_map_;	// リソースマップ
};