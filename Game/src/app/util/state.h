#pragma once

template <typename T>
class State
{
public:
	State(T* object) : object(object) {}
	virtual ~State() {}

	// �J�ڎ��Ɉ�x�����Ă΂��
	virtual void OnStateBegin(State* prev_state) {}
	// ���̏�Ԃ𔲂���Ƃ��Ɉ�x�����Ă΂��
	virtual void OnStateEnd(State* next_state) {}

	// ���̏�Ԓ����t���[���Ăяo�����
	virtual void Update(const float delta_time) {}

	// ���̏�ԂɑJ�ڂ���
	void ChangeState(State* next_state, const float delta_time)
	{
		this->OnStateEnd(next_state);
		object->state_ = next_state;
		next_state->OnStateBegin(this);
		next_state->Update(delta_time);
	}

protected:
	// ���䂷��I�u�W�F�N�g�̃|�C���^
	T* object;
};