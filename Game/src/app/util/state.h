#pragma once

template <typename T>
class State
{
public:
	State(T* object) : object_(object) {}
	virtual ~State() {}

	// 遷移時に一度だけ呼ばれる
	virtual void Enter() {}

	// この状態中毎フレーム呼び出される
	virtual void Execute() {}

	// この状態を抜けるときに一度だけ呼ばれる
	virtual void Exit() {}

	// 次の状態に遷移する
	void ChangeState(State* state)
	{
		object_->state_ = state;
		state->Enter();
	}

protected:
	// 制御するオブジェクトのポインタ
	T* object_;
};