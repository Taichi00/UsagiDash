#include "custom_button.h"
#include "game/component/all_components.h"

CustomButton::CustomButton(
	const std::string& name,
	const std::string& text,
	const std::function<void()>& function
) : Entity(name)
{
	auto game = Game::Get();
	
	// テキストプロパティ
	TextProperty text_prop = {};
	text_prop.font = L"Koruri";
	text_prop.font_size = 22;
	text_prop.color = Color(0.35f, 0.3f, 0.3f);
	text_prop.font_weight = TextProperty::WEIGHT_BOLD;
	text_prop.horizontal_alignment = TextProperty::HORIZONTAL_ALIGNMENT_CENTER;
	text_prop.vertical_alignment = TextProperty::VERTICAL_ALIGNMENT_CENTER;

	// パネルプロパティ
	PanelProperty panel_prop = {};
	panel_prop.color = Color(1, 1, 1, 1);
	panel_prop.radius = 24;
	panel_prop.padding = { 24, 6, 24, 6 };

	// アニメーションの定義
	std::vector<std::shared_ptr<Animation>> animations;
	{
		// 押下時のアニメーション
		auto animation = std::make_shared<Animation>("press");
		{
			Animation::Channel channel = {};
			channel.type = Animation::TYPE_GUI;
			channel.gui.name = "control";
			channel.gui.scale_keys = {
				{ 0, Easing::LINEAR, Vec2(0.8f, 0.8f) },
				{ 0.2f, Easing::OUT_BACK, Vec2(1.1f, 1.1f) }
			};
			animation->AddChannel(channel);
		}
		animation->SetDuration(1);
		animation->SetTicksPerSecond(1);

		animations.push_back(animation);
	}
	{
		// ホバー時のアニメーション
		auto animation = std::make_shared<Animation>("hover");
		{
			Animation::Channel channel = {};
			channel.type = Animation::TYPE_GUI;
			channel.gui.name = "panel";
			channel.gui.color_keys = {
				{ 0, Easing::LINEAR, Color(0.1f, 0.1f, 0.1f) }
			};
			animation->AddChannel(channel);
		}
		{
			Animation::Channel channel = {};
			channel.type = Animation::TYPE_GUI;
			channel.gui.name = "label";
			channel.gui.color_keys = {
				{ 0, Easing::LINEAR, Color(1, 1, 1) }
			};
			animation->AddChannel(channel);
		}
		{
			Animation::Channel channel = {};
			channel.type = Animation::TYPE_GUI;
			channel.gui.name = "control";
			channel.gui.scale_keys = {
				{ 0, Easing::LINEAR, Vec2(1, 1) },
				{ 0.1f, Easing::OUT_CUBIC, Vec2(1.1f, 1.1f) }
			};
			animation->AddChannel(channel);
		}
		animation->SetDuration(1);
		animation->SetTicksPerSecond(1);

		animations.push_back(animation);
	}
	{
		// ホバーが外れた時のアニメーション
		auto animation = std::make_shared<Animation>("unhover");
		{
			Animation::Channel channel = {};
			channel.type = Animation::TYPE_GUI;
			channel.gui.name = "panel";
			channel.gui.color_keys = {
				{ 0, Easing::LINEAR, Color(0.1f, 0.1f, 0.1f) },
				{ 0.1f, Easing::LINEAR, Color(1, 1, 1) }
			};
			animation->AddChannel(channel);
		}
		{
			Animation::Channel channel = {};
			channel.type = Animation::TYPE_GUI;
			channel.gui.name = "label";
			channel.gui.color_keys = {
				{ 0, Easing::LINEAR, Color(1, 1, 1) },
				{ 0.1f, Easing::LINEAR, Color(0.35f, 0.3f, 0.3f) }
			};
			animation->AddChannel(channel);
		}
		{
			Animation::Channel channel = {};
			channel.type = Animation::TYPE_GUI;
			channel.gui.name = "control";
			channel.gui.scale_keys = {
				{ 0, Easing::LINEAR, Vec2(1.1f, 1.1f) },
				{ 0.1f, Easing::OUT_CUBIC, Vec2(1, 1) }
			};
			animation->AddChannel(channel);
		}
		animation->SetDuration(1);
		animation->SetTicksPerSecond(1);

		animations.push_back(animation);
	}

	// サウンド
	auto audio_press = game->LoadResource<Audio>(L"assets/se/abs-popup-3.wav");
	auto audio_hover = game->LoadResource<Audio>(L"assets/se/MI_SFX 25.wav");

	// コンポーネントをアタッチする
	auto as_press = AddComponent<AudioSource>(audio_press, 0.5f);
	auto as_hover = AddComponent<AudioSource>(audio_hover, 0.5f);
	auto button = AddComponent<AnimatedButton>(new AnimatedButton(
		text,
		text_prop,
		panel_prop,
		function,
		as_press,
		as_hover,
		false
	));
	AddComponent<Animator>(new Animator(animations));
}
