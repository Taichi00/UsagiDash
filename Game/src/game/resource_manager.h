#pragma once
#include "resource/all_resources.h"
#include <string>
#include <unordered_map>
#include <memory>
#include <typeindex>

class ResourceManager {
public:
	ResourceManager();
	~ResourceManager();

	template<class T>
	std::shared_ptr<T> Load(const std::wstring& path) // リソースを読み込む
	{
		std::shared_ptr<T> resource;

		if (!resource_map_[typeid(T)].contains(path))	// 見つからなかった場合
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
		if (!resource_map_[typeid(T)].contains(path))	// 見つからなかった場合
		{
			return nullptr;
		}

		return std::static_pointer_cast<T>(resource_map_[typeid(T)][path]);
	}

	template<class T>
	bool Release(const std::wstring& path)	// リソースを開放する
	{
		if (!resource_map_[typeid(T)].contains(path))	// 見つからなかった場合
		{
			return false;
		}

		auto& resource = resource_map_[typeid(T)][path];

		if (!resource->Name().empty)
		{
			resource_name_map_.erase(resource.get());
		}

		resource->Release();
		resource_map_[typeid(T)].erase(path);

		return true;
	}

	template<class T>
	void Add(const std::wstring& key, const std::shared_ptr<T>& resource)
	{
		resource_map_[typeid(T)][key] = resource;
	}

	const Resource* GetResourceFromName(const std::wstring& name);

	// リソースの名前を設定する
	void RegisterResourceName(const Resource* resource, const std::wstring& name);

private:
	std::unordered_map<std::type_index, std::unordered_map<std::wstring, std::shared_ptr<Resource>>> resource_map_;	// リソースマップ
	std::unordered_map<std::wstring, const Resource*> resource_name_map_;
};