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
	std::shared_ptr<T> Load(const std::wstring& path) // ���\�[�X��ǂݍ���
	{
		std::shared_ptr<T> resource;

		if (resource_map_[typeid(T)].find(path) == resource_map_[typeid(T)].end())	// ������Ȃ������ꍇ
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
		if (resource_map_[typeid(T)].find(path) == resource_map_[typeid(T)].end())	// ������Ȃ������ꍇ
		{
			return nullptr;
		}

		return std::static_pointer_cast<T>(resource_map_[typeid(T)][path]);
	}

	template<class T>
	bool Release(const std::wstring& path)	// ���\�[�X���J������
	{
		if (resource_map_[typeid(T)].find(path) == resource_map_[typeid(T)].end())	// ������Ȃ������ꍇ
		{
			return false;
		}

		resource_map_[typeid(T)][path]->Release();
		resource_map_[typeid(T)].erase(path);

		return true;
	}

	void AddResource(std::shared_ptr<Resource> resource);	// ���\�[�X��ǉ�����

private:
	std::map<std::type_index, std::map<std::wstring, std::shared_ptr<Resource>>> resource_map_;	// ���\�[�X�}�b�v
};