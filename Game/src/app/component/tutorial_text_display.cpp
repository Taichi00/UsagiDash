#include "tutorial_text_display.h"
#include "game/component/collider/collider.h"
#include "game/entity.h"
#include "game/component/gui/label.h"
#include "game/scene.h"

TutorialTextDisplay::TutorialTextDisplay(const std::string& text, const int index, Collider* collider)
{
    text_ = text;
    index_ = index;
    collider_ = collider;
}

bool TutorialTextDisplay::Init()
{
    label_ = GetScene()->FindEntity("tutorial_label")->GetComponent<Label>();

    return true;
}

void TutorialTextDisplay::OnCollisionEnter(Collider* collider)
{
    if (collider->GetEntity()->tag == "player")
    {
        label_->SetText(text_);
        GetEntity()->Destroy();
    }
}
