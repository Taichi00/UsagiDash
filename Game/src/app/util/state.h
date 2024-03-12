#pragma once

template <typename T>
class State
{
public:
	State(T* object) : object(object) {}
	virtual ~State() {}

	// 遷移時に一度だけ呼ばれる
	virtual void OnStateBegin(State* prev_state) {}
	// この状態を抜けるときに一度だけ呼ばれる
	virtual void OnStateEnd(State* next_state) {}

	// この状態中毎フレーム呼び出される
	virtual void Update(const float delta_time) {}

	// 次の状態に遷移する
	void ChangeState(State* next_state, const float delta_time)
	{
		this->OnStateEnd(next_state);
		object->state_ = next_state;
		next_state->OnStateBegin(this);
		next_state->Update(delta_time);
	}

protected:
	// 制御するオブジェクトのポインタ
	T* object;
};