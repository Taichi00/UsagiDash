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

	template<class T> std::shared_ptr<T> Load(const std::string& path) // ���\�[�X��ǂݍ���
	{
		std::shared_ptr<T> resource;

		// �L�[�̐擪�Ƀ��\�[�X��typename��t�^
		// �����̃t�@�C���ł�type���قȂ��Ă���ΈႤ���\�[�X�Ƃ��ĔF�������邽��
		auto key = std::string(typeid(T).name()) + ":" + path;

		if (resource_map_.find(key) == resource_map_.end())	// ������Ȃ������ꍇ
		{
			resource = T::Load(path);

			// �ǂݍ��ݎ��s
			if (!resource)
			{
				printf("Resource [%s] �̓ǂݍ��݂Ɏ��s\n", key.c_str());
				return nullptr;
			}

			resource_map_[key] = resource;
		}
		else // ���������ꍇ
		{
			resource = std::static_pointer_cast<T>(resource_map_[key]);
		}

		return resource;
	}

	template<class T> std::shared_ptr<T> Get(const std::string& path) // ���\�[�X���擾����
	{
		auto key = std::string(typeid(T).name()) + ":" + path;

		if (resource_map_.find(key) == resource_map_.end())	// ������Ȃ������ꍇ
		{
			return nullptr;
		}

		return std::static_pointer_cast<T>(resource_map_[key]);
	}

	bool Release(const std::string& key);	// ���\�[�X���J������

	void AddResource(std::shared_ptr<Resource> resource);	// ���\�[�X��ǉ�����

private:
	std::string GetExtension(const std::string& path);	// �g���q���擾����

private:
	std::map<std::string, std::shared_ptr<Resource>> resource_map_;	// ���\�[�X�}�b�v
};