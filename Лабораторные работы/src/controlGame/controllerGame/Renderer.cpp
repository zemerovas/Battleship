#include "Renderer.h"

template <class T>
void Renderer<T>::Initialize() { 
    strategy->Initialize(); 
}

template <class T>
void Renderer<T>::Render(const Game& game, const std::string& controls_legend) { 
    strategy->Render(game, controls_legend); 
}

template <class T>
void Renderer<T>::RenderField(const PlayingField& field, const std::string& title) {
    strategy->RenderField(field, title);
}

template <class T>
void Renderer<T>::OnAttackResult(const AttackResult& result) { 
    strategy->OnAttackResult(result); 
}

template <class T>
void Renderer<T>::OnAttackResult(const AttackResult& result, bool on_enemy_field) {
    strategy->OnAttackResult(result, on_enemy_field);
}

template <class T>
void Renderer<T>::OnAbilityResult(const AbilityResult& result) { 
    strategy->OnAbilityResult(result); 
}

template <class T>
void Renderer<T>::ShowShotBanner(const std::string& text, bool on_enemy_field) {
    strategy->ShowShotBanner(text, on_enemy_field);
}

template <class T>
void Renderer<T>::ShowMessage(const std::string& message) { 
    strategy->ShowMessage(message); 
}


template class Renderer<ConsoleRenderer>;
template class Renderer<GUIRenderer>;