#ifndef BATTLESHIP_CONTROLGAME_CONTROLLERGAME_IRENDERERSTRATEGY_H_
#define BATTLESHIP_CONTROLGAME_CONTROLLERGAME_IRENDERERSTRATEGY_H_
#include <string>
#include <functional>
#include "controlGame/Result.h" 

class Game;
class PlayingField;

class IRenderStrategy {
public:
    virtual ~IRenderStrategy() = default;
    virtual void Initialize() = 0;
    virtual void Render(const Game& game, const std::string& controlsLegend) = 0;
    virtual void RenderField(const PlayingField& field, const std::string& title) = 0;
    virtual void OnAttackResult(const AttackResult& result) { OnAttackResult(result, true); }
    virtual void OnAttackResult(const AttackResult& result, bool on_enemy_field) = 0;
    virtual void OnAbilityResult(const AbilityResult& result) = 0;
    virtual void ShowShotBanner(const std::string& text, bool on_enemy_field) = 0;
    virtual void ShowMessage(const std::string& message) = 0;
};

#endif