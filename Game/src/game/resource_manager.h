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
	std::shared_ptr<T> Load(const std::wstring& path) // ���\�[�X��ǂݍ���
	{
		std::shared_ptr<T> resource;

		if (!resource_map_[typeid(T)].contains(path))	// ������Ȃ������ꍇ
		{
			resource = T::Load(path);

			// �ǂݍ��ݎ��s
			if (!resource)
			{
				printf("Resource [%ls] �̓ǂݍ��݂Ɏ��s\n", path.c_str());
				return nullptr;
			}

			resource_map_[typeid(T)][path] = resource;
		}
		else // ���������ꍇ
		{
			resource = std::static_pointer_cast<T>(resource_map_[typeid(T)][path]);
		}

		return resource;
	}

	template<class T>
	std::shared_ptr<T> Get(const std::wstring& path) // ���\�[�X���擾����
	{
		if (!resource_map_[typeid(T)].contains(path))	// ������Ȃ������ꍇ
		{
			return nullptr;
		}

		return std::static_pointer_cast<T>(resource_map_[typeid(T)][path]);
	}

	template<class T>
	bool Release(const std::wstring& path)	// ���\�[�X���J������
	{
		if (!resource_map_[typeid(T)].contains(path))	// ������Ȃ������ꍇ
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

	// ���\�[�X�̖��O��ݒ肷��
	void RegisterResourceName(const Resource* resource, const std::wstring& name);

private:
	std::unordered_map<std::type_index, std::unordered_map<std::wstring, std::shared_ptr<Resource>>> resource_map_;	// ���\�[�X�}�b�v
	std::unordered_map<std::wstring, const Resource*> resource_name_map_;
};