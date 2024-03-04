#pragma once

#include "math/vec.h"
#include "math/aabb.h"
#include "game/layer_manager.h"
#include "game/game.h"
#include <windows.h>
#include <vector>
#include <list>
#include <memory>

template <class T>
class OctreeCell;

template <class T>
class OctreeObject
{
public:
	OctreeObject()
	{
		cell = nullptr;
		object = nullptr;
		next = nullptr;
		previous = nullptr;
		group = nullptr;
		layer = 0;
	}

	virtual ~OctreeObject()
	{
	}
	
	// ���X�g���痣�E
	bool Remove()
	{
		// ���łɗ��E���Ă���ꍇ
		if (!cell)
			return false;

		// �o�^��Ԃɒʒm����
		if (!cell->OnRemove(this))
			return false;

		if (previous)
		{
			previous->next = next;
		}
		if (next)
		{
			next->previous = previous;
		}

		previous = nullptr;
		next = nullptr;
		cell = nullptr;
		return true;
	}

	// ��Ԃ�o�^
	void RegistCell(OctreeCell<T>* cell)
	{
		this->cell = cell;
	}

	// ���̃I�u�W�F�N�g�ւ̃|�C���^���擾
	OctreeObject<T>* Next()
	{
		return next;
	}

public:
	OctreeCell<T>* cell; // �o�^���
	T* object; // ����ΏۃI�u�W�F�N�g
	OctreeObject<T>* previous; // �O��Object
	OctreeObject<T>* next; // ����Object
	AABB aabb;
	void* group;
	int layer; // ���C���[�̃C���f�b�N�X�ԍ�
};

template <class T>
class OctreeCell
{
public:
	OctreeCell() {}

	virtual ~OctreeCell() {}

	// �I�u�W�F�N�g���v�b�V��
	bool Push(OctreeObject<T>* object)
	{
		// object��nullptr�̏ꍇ
		if (!object)
			return false;
		
		// 2�d�o�^�����Ȃ�
		if (object->cell == this)
			return false;

		if (!latest_)
		{
			// ��ԂɐV�K�o�^
			latest_ = object;
		}
		else
		{
			// �ŐVObject���X�V
			object->next = latest_;
			latest_->previous = object;
			latest_ = object;
		}

		object->RegistCell(this);
		return true;
	}

	OctreeObject<T>* GetFirstObject()
	{
		return latest_;
	}

	// �폜�����I�u�W�F�N�g���`�F�b�N
	bool OnRemove(OctreeObject<T>* object)
	{
		// ���E����Object��latest�o������ꍇ�A����Object�ɓ���ւ���
		if (object == latest_)
		{
			if (latest_)
				latest_ = latest_->next;
		}
		return true;
	}

private:
	OctreeObject<T>* latest_ = nullptr;
};

#define OCTREE_MAX_LEVEL 7

template <class T>
class Octree
{
public:
	Octree()
	{
		level_ = 0;
		width_ = Vec3::Zero();
		unit_ = Vec3::Zero();
		aabb_ = { Vec3::Zero(), Vec3::Zero() };
		cell_num_ = 0;
		cell_array_ = nullptr;
		dim_ = 0;

		layer_manager_ = Game::Get()->GetLayerManager();
	}

	virtual ~Octree()
	{
		for (unsigned long i = 0; i < cell_num_; i++)
		{
			if (cell_array_[i] != nullptr)
			{
				delete cell_array_[i];
			}
		}
		delete[] cell_array_;
	}

	// 8���ؔz����\�z����
	bool Init(const unsigned int level, const AABB& aabb)
	{
		// �ō����x���ȏ�̋�Ԃ͍��Ȃ�
		if (level >= OCTREE_MAX_LEVEL)
			return false;

		// �e���x���ł̋�Ԑ����Z�o
		pow_[0] = 1;
		for (int i = 1; i < OCTREE_MAX_LEVEL + 1; i++)
		{
			pow_[i] = pow_[i - 1] * 8;
		}

		// �z��쐬
		cell_num_ = (pow_[level + 1] - 1) / 7;
		cell_array_ = new OctreeCell<T>*[cell_num_];
		memset(cell_array_, 0, sizeof(OctreeCell<T>*) * cell_num_);

		// �L���̈��o�^
		aabb_ = aabb;
		width_ = aabb.Size();
		unit_ = width_ / ((float)(1 << level));

		level_ = level;

		return true;
	}

	// �I�u�W�F�N�g��o�^����
	bool Regist(OctreeObject<T>* object)
	{
		// �I�u�W�F�N�g�̋��E�͈͂���o�^���[�g���ԍ����Z�o
		unsigned long elem = GetMortonNumber(object->aabb);
		if (elem < cell_num_)
		{
			// ��Ԃ��Ȃ��ꍇ�͐V�K�쐬
			if (!cell_array_[elem])
			{
				CreateNewCell(elem);
			}
			return cell_array_[elem]->Push(object);
		}

		return false; // �o�^���s
	}

	unsigned long GetAllCollisionList(std::vector<T*>& col_vect)
	{
		col_vect.clear();

		// ���[�g��Ԃ̑��݂��`�F�b�N
		if (!cell_array_[0])
			return 0; // ��Ԃ����݂��Ă��Ȃ�

		// ���[�g��Ԃ�����
		std::vector<OctreeObject<T>*> col_stac;
		GetCollisionList(0, col_vect, col_stac);

		return (unsigned long)col_stac.size();
	}

protected:
	// ��ԓ��ŏՓ˃��X�g���쐬����
	bool GetCollisionList(const unsigned long& elem, std::vector<T*>& col_vect, std::vector<OctreeObject<T>*>& col_stac)
	{
		// ��ԓ��̃I�u�W�F�N�g���m�̏Փ˃��X�g�쐬
		OctreeObject<T>* obj1 = cell_array_[elem]->GetFirstObject();
		while (obj1 != nullptr)
		{
			OctreeObject<T>* obj2 = obj1->next;
			while (obj2 != nullptr)
			{
				if (obj1->group == obj2->group)
				{
					obj2 = obj2->next;
					continue;
				}

				if (!layer_manager_->IsCollisionEnabled(obj1->layer, obj2->layer))
				{
					obj2 = obj2->next;
					continue;
				}

				if (obj1->aabb.Intersects(obj2->aabb))
				{
					// �Փ˃��X�g�쐬
					col_vect.push_back(obj1->object);
					col_vect.push_back(obj2->object);
				}

				obj2 = obj2->next;
			}

			// �Փ˃X�^�b�N�Ƃ̏Փ˃��X�g�쐬
			for (auto& stac : col_stac)
			{
				if (obj1->group == stac->group) continue;
				if (!layer_manager_->IsCollisionEnabled(obj1->layer, stac->layer)) continue;
				if (obj1->aabb.Intersects(stac->aabb))
				{
					col_vect.push_back(obj1->object);
					col_vect.push_back(stac->object);
				}
			}
			obj1 = obj1->next;
		}

		bool child_flag = false;

		// �q��ԂɈړ�
		unsigned long obj_num = 0;
		unsigned long next_elem;
		for (unsigned long i = 0; i < 8; i++)
		{
			next_elem = elem * 8 + 1 + i;
			if (next_elem < cell_num_ && cell_array_[elem * 8 + 1 + i])
			{
				if (!child_flag)
				{
					// �o�^�I�u�W�F�N�g���X�^�b�N�ɒǉ�
					obj1 = cell_array_[elem]->GetFirstObject();
					while (obj1)
					{
						col_stac.push_back(obj1);
						obj_num++;
						obj1 = obj1->next;
					}
				}
				child_flag = true;
				GetCollisionList(elem * 8 + 1 + i, col_vect, col_stac); // �q��Ԃ�
			}
		}
		
		// �X�^�b�N����I�u�W�F�N�g���O��
		if (child_flag)
		{
			for (unsigned long i = 0; i < obj_num; i++)
			{
				col_stac.pop_back();
			}
		}

		return true;
	}

	// ��Ԃ𐶐�
	bool CreateNewCell(unsigned long elem)
	{
		while (!cell_array_[elem])
		{
			// �w��̗v�f�ԍ��ɋ�Ԃ�V�K�쐬
			cell_array_[elem] = new OctreeCell<T>();

			// �e��ԂɃW�����v�i�e��Ԃ�������ΐe��Ԃ���������j
			elem = (elem - 1) >> 3;
			if (elem >= cell_num_) break;
		}
		return true;
	}

	// ���W�����Ԕԍ����Z�o
	unsigned long GetMortonNumber(const AABB& aabb)
	{
		// �ŏ����x���ɂ�����e���ʒu���Z�o
		unsigned long lt = GetPointElem(aabb.min);
		unsigned long rb = GetPointElem(aabb.max);

		// ��Ԕԍ��������Z���čŏ�ʋ�؂肩�珊�����x�����Z�o
		unsigned long def = rb ^ lt;
		unsigned int hi_level = 1;
		for (unsigned int i = 0; i < level_; i++)
		{
			unsigned long check = (def >> (i * 3)) & 0x7;
			if (check != 0)
				hi_level = i + 1;
		}

		unsigned long space_num = rb >> (hi_level * 3);
		unsigned long add_num = (pow_[level_ - hi_level] - 1) / 7;
		space_num += add_num;

		if (space_num > cell_num_)
		{
			return 0xffffffff;
		}

		return space_num;
	}

	// �r�b�g�����֐�
	unsigned long BitSeparatorFor3D(unsigned char n)
	{
		unsigned long s = n;
		s = (s | s << 8) & 0x0000f00f;
		s = (s | s << 4) & 0x000c30c3;
		s = (s | s << 2) & 0x00249249;
		return s;
	}

	// 3D���[�g����Ԕԍ��Z�o�֐�
	unsigned long Get3DMortonNumber(unsigned char x, unsigned char y, unsigned char z)
	{
		return BitSeparatorFor3D(x) | BitSeparatorFor3D(y) << 1 | BitSeparatorFor3D(z) << 2;
	}

	// ���W�����`8���ؗv�f�ԍ��ϊ��֐�
	unsigned long GetPointElem(const Vec3& p)
	{
		return Get3DMortonNumber(
			(unsigned char)((p.x - aabb_.min.x) / unit_.x),
			(unsigned char)((p.y - aabb_.min.y) / unit_.y),
			(unsigned char)((p.z - aabb_.min.z) / unit_.z)
		);
	}

protected:
	unsigned int dim_;
	OctreeCell<T>** cell_array_; // ���`��ԃ|�C���^�z��
	unsigned int pow_[OCTREE_MAX_LEVEL + 1]; // �e���x���ł̋�Ԑ�
	Vec3 width_;
	Vec3 unit_; // �ŏ����x����Ԃ̕ӂ̒���
	AABB aabb_;
	unsigned long cell_num_; // ��Ԃ̐�
	unsigned int level_; // �ŉ��ʃ��x��

	LayerManager* layer_manager_ = nullptr;
};