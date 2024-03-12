#include "tutorial_text_controller.h"
#include "game/component/collider/collider.h"
#include "game/entity.h"
#include "game/scene.h"
#include "app/component/player_controller_2.h"
#include "app/component/tutorial_label_controller.h"
#include "game/input/input.h"

std::unordered_map<std::string, TutorialTextController::Task> TutorialTextController::key_task_map_ =
{
    { "move", TutorialTextController::TASK_MOVE },
    { "jump", TutorialTextController::TASK_JUMP },
    { "camera", TutorialTextController::TASK_CAMERA },
    { "dashjump", TutorialTextController::TASK_DASHJUMP },
    { "walljump", TutorialTextController::TASK_WALLJAMP },
};

TutorialTextController::TutorialTextController(const std::string& text, const std::string& task_key)
{
    text_ = text;
    task_ = key_task_map_[task_key];
}

bool TutorialTextController::Init()
{
    label_ = GetScene()->FindEntity("tutorial_label")->GetComponent<TutorialLabelController>();
    player_ = GetScene()->FindEntity("player")->GetComponent<PlayerController2>();
    collider_ = GetEntity()->GetComponent<Collider>();

    return true;
}

void TutorialTextController::Update(const float delta_time)
{
    if (!is_active_)
        return;

    if (CheckTask())
    {
        // タスクが達成されたら削除
        label_->Hide();
        GetEntity()->Destroy();
    }
}

void TutorialTextController::OnCollisionEnter(Collider* collider)
{
    if (collider->GetEntity()->tag == "player")
    {
        // 領域内に入ったらテキストを表示
        label_->Show(text_, this);

        collider_->enabled = false;
        is_active_ = true;
    }
}

bool TutorialTextController::CheckTask()
{
    auto state = player_->GetCurrentState();

    switch (task_)
    {
    case TASK_MOVE:
        return state == PlayerController2::STATE_RUN;
    case TASK_JUMP:
        return state == PlayerController2::STATE_JUMP;
    case TASK_CAMERA:
        return Input::GetAxis("camera_horizontal") || Input::GetAxis("camera_vertical");
    case TASK_DASHJUMP:
        return state == PlayerController2::STATE_DASHJUMP;
    case TASK_WALLJAMP:
        return state == PlayerController2::STATE_WALLJUMP;
    }

    return false;
}
