#include "tutorial_label_controller.h"
#include "game/entity.h"
#include "game/component/animator.h"
#include "game/component/gui/label.h"
#include "app/component/tutorial_text_controller.h"
#include "game/component/audio/audio_source.h"

TutorialLabelController::TutorialLabelController(AudioSource* audio_show, AudioSource* audio_hide)
{
    audio_show_ = audio_show;
    audio_hide_ = audio_hide;
}

bool TutorialLabelController::Init()
{
    label_ = GetEntity()->GetComponent<Label>();
    animator_ = GetEntity()->GetComponent<Animator>();

    // “§–¾‚É‚µ‚Ä‚¨‚­
    label_->SetColor(Color(1, 1, 1, 0));

    return true;
}

void TutorialLabelController::Show(const std::string& text, TutorialTextController* tutorial)
{
    if (tutorial_)
    {
        // •\Ž¦’†‚È‚çÁ‚·
        tutorial_->GetEntity()->Destroy();
    }

    label_->SetText(text);
    animator_->Play("show", 1, false);
    audio_show_->Play(0.5f);

    tutorial_ = tutorial;
}

void TutorialLabelController::Hide()
{
    animator_->Play("hide", 1, false);
    audio_hide_->Play(0.5f);

    tutorial_ = nullptr;
}
