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
	
	// リストから離脱
	bool Remove()
	{
		// すでに離脱している場合
		if (!cell)
			return false;

		// 登録空間に通知する
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

	// 空間を登録
	void RegistCell(OctreeCell<T>* cell)
	{
		this->cell = cell;
	}

	// 次のオブジェクトへのポインタを取得
	OctreeObject<T>* Next()
	{
		return next;
	}

public:
	OctreeCell<T>* cell; // 登録空間
	T* object; // 判定対象オブジェクト
	OctreeObject<T>* previous; // 前のObject
	OctreeObject<T>* next; // 次のObject
	AABB aabb;
	void* group;
	int layer; // レイヤーのインデックス番号
};

template <class T>
class OctreeCell
{
public:
	OctreeCell() {}

	virtual ~OctreeCell() {}

	// オブジェクトをプッシュ
	bool Push(OctreeObject<T>* object)
	{
		// objectがnullptrの場合
		if (!object)
			return false;
		
		// 2重登録させない
		if (object->cell == this)
			return false;

		if (!latest_)
		{
			// 空間に新規登録
			latest_ = object;
		}
		else
		{
			// 最新Objectを更新
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

	// 削除されるオブジェクトをチェック
	bool OnRemove(OctreeObject<T>* object)
	{
		// 離脱するObjectがlatest出会った場合、次のObjectに入れ替える
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

	// 8分木配列を構築する
	bool Init(const unsigned int level, const AABB& aabb)
	{
		// 最高レベル以上の空間は作れない
		if (level >= OCTREE_MAX_LEVEL)
			return false;

		// 各レベルでの空間数を算出
		pow_[0] = 1;
		for (int i = 1; i < OCTREE_MAX_LEVEL + 1; i++)
		{
			pow_[i] = pow_[i - 1] * 8;
		}

		// 配列作成
		cell_num_ = (pow_[level + 1] - 1) / 7;
		cell_array_ = new OctreeCell<T>*[cell_num_];
		memset(cell_array_, 0, sizeof(OctreeCell<T>*) * cell_num_);

		// 有効領域を登録
		aabb_ = aabb;
		width_ = aabb.Size();
		unit_ = width_ / ((float)(1 << level));

		level_ = level;

		return true;
	}

	// オブジェクトを登録する
	bool Regist(OctreeObject<T>* object)
	{
		// オブジェクトの境界範囲から登録モートン番号を算出
		unsigned long elem = GetMortonNumber(object->aabb);
		if (elem < cell_num_)
		{
			// 空間がない場合は新規作成
			if (!cell_array_[elem])
			{
				CreateNewCell(elem);
			}
			return cell_array_[elem]->Push(object);
		}

		return false; // 登録失敗
	}

	unsigned long GetAllCollisionList(std::vector<T*>& col_vect)
	{
		col_vect.clear();

		// ルート空間の存在をチェック
		if (!cell_array_[0])
			return 0; // 空間が存在していない

		// ルート空間を処理
		std::vector<OctreeObject<T>*> col_stac;
		GetCollisionList(0, col_vect, col_stac);

		return (unsigned long)col_stac.size();
	}

protected:
	// 空間内で衝突リストを作成する
	bool GetCollisionList(const unsigned long& elem, std::vector<T*>& col_vect, std::vector<OctreeObject<T>*>& col_stac)
	{
		// 空間内のオブジェクト同士の衝突リスト作成
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
					// 衝突リスト作成
					col_vect.push_back(obj1->object);
					col_vect.push_back(obj2->object);
				}

				obj2 = obj2->next;
			}

			// 衝突スタックとの衝突リスト作成
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

		// 子空間に移動
		unsigned long obj_num = 0;
		unsigned long next_elem;
		for (unsigned long i = 0; i < 8; i++)
		{
			next_elem = elem * 8 + 1 + i;
			if (next_elem < cell_num_ && cell_array_[elem * 8 + 1 + i])
			{
				if (!child_flag)
				{
					// 登録オブジェクトをスタックに追加
					obj1 = cell_array_[elem]->GetFirstObject();
					while (obj1)
					{
						col_stac.push_back(obj1);
						obj_num++;
						obj1 = obj1->next;
					}
				}
				child_flag = true;
				GetCollisionList(elem * 8 + 1 + i, col_vect, col_stac); // 子空間へ
			}
		}
		
		// スタックからオブジェクトを外す
		if (child_flag)
		{
			for (unsigned long i = 0; i < obj_num; i++)
			{
				col_stac.pop_back();
			}
		}

		return true;
	}

	// 空間を生成
	bool CreateNewCell(unsigned long elem)
	{
		while (!cell_array_[elem])
		{
			// 指定の要素番号に空間を新規作成
			cell_array_[elem] = new OctreeCell<T>();

			// 親空間にジャンプ（親空間が無ければ親空間も生成する）
			elem = (elem - 1) >> 3;
			if (elem >= cell_num_) break;
		}
		return true;
	}

	// 座標から空間番号を算出
	unsigned long GetMortonNumber(const AABB& aabb)
	{
		// 最小レベルにおける各軸位置を算出
		unsigned long lt = GetPointElem(aabb.min);
		unsigned long rb = GetPointElem(aabb.max);

		// 空間番号を引き算して最上位区切りから所属レベルを算出
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

	// ビット分割関数
	unsigned long BitSeparatorFor3D(unsigned char n)
	{
		unsigned long s = n;
		s = (s | s << 8) & 0x0000f00f;
		s = (s | s << 4) & 0x000c30c3;
		s = (s | s << 2) & 0x00249249;
		return s;
	}

	// 3Dモートン空間番号算出関数
	unsigned long Get3DMortonNumber(unsigned char x, unsigned char y, unsigned char z)
	{
		return BitSeparatorFor3D(x) | BitSeparatorFor3D(y) << 1 | BitSeparatorFor3D(z) << 2;
	}

	// 座標→線形8分木要素番号変換関数
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
	OctreeCell<T>** cell_array_; // 線形空間ポインタ配列
	unsigned int pow_[OCTREE_MAX_LEVEL + 1]; // 各レベルでの空間数
	Vec3 width_;
	Vec3 unit_; // 最小レベル空間の辺の長さ
	AABB aabb_;
	unsigned long cell_num_; // 空間の数
	unsigned int level_; // 最下位レベル

	LayerManager* layer_manager_ = nullptr;
};