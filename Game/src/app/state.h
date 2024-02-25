#pragma once

template <typename T>
class State
{
public:
	State(T* object) : object_(object) {}
	virtual ~State() {}

	// �J�ڎ��Ɉ�x�����Ă΂��
	virtual void Enter() {}

	// ���̏�Ԓ����t���[���Ăяo�����
	virtual void Execute() {}

	// ���̏�Ԃ𔲂���Ƃ��Ɉ�x�����Ă΂��
	virtual void Exit() {}

	// ���̏�ԂɑJ�ڂ���
	void ChangeState(State* state)
	{
		object_->state_ = state;
		state->Enter();
	}

protected:
	// ���䂷��I�u�W�F�N�g�̃|�C���^
	T* object_;
};