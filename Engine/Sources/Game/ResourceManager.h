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

	template<class T> std::shared_ptr<T> Load(const std::string& key) // ���\�[�X��ǂݍ���
	{
		std::shared_ptr<T> resource;

		if (m_resourceMap.find(key) == m_resourceMap.end())	// ������Ȃ������ꍇ
		{
			resource = T::Load(key);

			// �ǂݍ��ݎ��s
			if (!resource)
			{
				printf("Resource [%s] �̓ǂݍ��݂Ɏ��s\n", key.c_str());
				return nullptr;
			}

			m_resourceMap[key] = resource;
		}
		else // ���������ꍇ
		{
			resource = std::static_pointer_cast<T>(m_resourceMap[key]);
		}

		return resource;
	}

	template<class T> std::shared_ptr<T> Get(const std::string& key) // ���\�[�X���擾����
	{
		if (m_resourceMap.find(key) == m_resourceMap.end())	// ������Ȃ������ꍇ
		{
			return nullptr;
		}

		return std::static_pointer_cast<T>(m_resourceMap[key]);
	}

	bool Release(const std::string& key);	// ���\�[�X���J������

	void AddResource(std::shared_ptr<Resource> resource);	// ���\�[�X��ǉ�����

private:
	std::string GetExtension(const std::string& path);	// �g���q���擾����

private:
	std::map<std::string, std::shared_ptr<Resource>> m_resourceMap;	// ���\�[�X�}�b�v
};