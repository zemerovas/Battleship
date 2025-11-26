#ifndef BATTLESHIP_CONTROLGAME_CONTROLLERGAME_RENDERER_H_
#define BATTLESHIP_CONTROLGAME_CONTROLLERGAME_RENDERER_H_
#include "IRendererStrategy.h"
#include <memory>
#include <utility>
#include <functional>
#include <SFML/Graphics.hpp>
#include "GUI/GUIRenderer.h"
#include "console/ConsoleRenderer.h"
#include <type_traits>


template <class T>
class Renderer : public IRenderStrategy {
public:
    template <typename... Args>
    explicit Renderer(Args&&... args) : strategy(std::make_unique<T>(std::forward<Args>(args)...)) {}
    explicit Renderer(std::unique_ptr<T> impl) : strategy(std::move(impl)) {}

    void Initialize() override;
    void Render(const Game& game, const std::string& controls_legend) override;
    void RenderField(const PlayingField& field, const std::string& title) override;
    void OnAttackResult(const AttackResult& result) override;
    void OnAttackResult(const AttackResult& result, bool on_enemy_field) override;
    void OnAbilityResult(const AbilityResult& result) override;
    void ShowShotBanner(const std::string& text, bool on_enemy_field) override;
    void ShowMessage(const std::string& message) override;
   
private:
    std::unique_ptr<T> strategy;
};

#endif
