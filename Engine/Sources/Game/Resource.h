#pragma once
#include <string>

class Resource
{
public:
	Resource();
	~Resource();

	virtual bool Release();

private:
	std::string m_key;	// ���\�[�X�����ʂ��邽�߂̃L�[�i�ʏ�̓t�@�C���ւ̃p�X�j
};