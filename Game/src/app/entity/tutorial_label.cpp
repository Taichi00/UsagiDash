#include "tutorial_label.h"
#include "game/component/gui/label.h"
#include "game/component/animator.h"
#include "app/component/tutorial_label_controller.h"
#include "app/component/pause_behavior.h"
#include <vector>
#include <memory>

TutorialLabel::TutorialLabel() : Entity("tutorial_label")
{
	Vec2 position = Vec2(0, 130);

	TextProperty text_prop{};
	text_prop.font = L"Koruri";
	text_prop.font_size = 22;
	text_prop.color = Color(0.35f, 0.3f, 0.3f);
	text_prop.font_weight = TextProperty::WEIGHT_BOLD;
	text_prop.horizontal_alignment = TextProperty::HORIZONTAL_ALIGNMENT_CENTER;
	text_prop.vertical_alignment = TextProperty::VERTICAL_ALIGNMENT_CENTER;

	PanelProperty panel_prop{};
	panel_prop.color = Color(1, 1, 1, 1);
	panel_prop.radius = 22;
	panel_prop.padding = { 24, 6, 24, 6 };

	auto label = AddComponent<Label>(new Label("", text_prop, panel_prop, true));
	label->SetAnchorPoint(Vec2(0.5f, 0));
	label->SetPivot(Vec2(0.5f, 0.5f));
	label->SetPosition(position);
	label->Transform();

	std::vector<std::shared_ptr<Animation>> animations;
	{
		auto animation = std::make_shared<Animation>("show");
		{
			Animation::Channel channel = {};
			channel.type = Animation::TYPE_GUI;
			channel.gui.name = "control";
			channel.gui.color_keys = {
				{ 0, Easing::LINEAR, Color(1, 1, 1, 0) },
				{ 0.1f, Easing::LINEAR, Color(1, 1, 1, 1) },
			};
			animation->AddChannel(channel);
		}
		{
			Animation::Channel channel = {};
			channel.type = Animation::TYPE_GUI;
			channel.gui.name = "control";
			channel.gui.scale_keys = {
				{ 0, Easing::LINEAR, Vec2(0.8f, 0.8f) },
				{ 0.2f, Easing::OUT_BACK, Vec2(1, 1) }
			};
			animation->AddChannel(channel);
		}
		{
			Animation::Channel channel = {};
			channel.type = Animation::TYPE_GUI;
			channel.gui.name = "control";
			channel.gui.position_keys = {
				{ 0, Easing::LINEAR, position },
			};
			animation->AddChannel(channel);
		}
		animation->SetDuration(0.4f);
		animation->SetTicksPerSecond(1);

		animations.push_back(animation);
	}
	{
		auto animation = std::make_shared<Animation>("hide");
		{
			Animation::Channel channel = {};
			channel.type = Animation::TYPE_GUI;
			channel.gui.name = "control";
			channel.gui.color_keys = {
				{ 0, Easing::LINEAR, Color(1, 1, 1, 1) },
				{ 0.05f, Easing::LINEAR, Color(0.35f, 1, 0.4f, 1) },
				{ 0.9f, Easing::LINEAR, Color(0.35f, 1, 0.4f, 1) },
				{ 1, Easing::LINEAR, Color(0.35f, 1, 0.4f, 0) },
			};
			animation->AddChannel(channel);
		}
		{
			Animation::Channel channel = {};
			channel.type = Animation::TYPE_GUI;
			channel.gui.name = "control";
			channel.gui.scale_keys = {
				{ 0, Easing::LINEAR, Vec2(0.8f, 0.8f) },
				{ 0.15f, Easing::OUT_BACK, Vec2(1.0f, 1.0f) },
			};
			animation->AddChannel(channel);
		}
		{
			Animation::Channel channel = {};
			channel.type = Animation::TYPE_GUI;
			channel.gui.name = "control";
			channel.gui.position_keys = {
				{ 0, Easing::LINEAR, position },
				{ 0.8f, Easing::LINEAR, position },
				{ 1, Easing::IN_QUAD, position - Vec2(0, 30) },
			};
			animation->AddChannel(channel);
		}
		animation->SetDuration(1.0f);
		animation->SetTicksPerSecond(1);

		animations.push_back(animation);
	}

	AddComponent(new Animator(animations));
	AddComponent(new TutorialLabelController());
	AddComponent(new PauseBehavior());
}
