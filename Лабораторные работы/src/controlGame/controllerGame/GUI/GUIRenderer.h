#ifndef BATTLESHIP_CONTROLGAME_CONTROLLERGAME_GUI_GUIRENDERER_H_
#define BATTLESHIP_CONTROLGAME_CONTROLLERGAME_GUI_GUIRENDERER_H_

#include <iostream>
#include <memory>
#include <vector>
#include <array>
#include <SFML/Graphics.hpp>
#include <string>
#include <functional>
#include <cstring>
#include "core/Ship.h"
#include "GameState.h"
#include "core/PlayingField.h"
#include "IRendererStrategy.h"
#include "Game.h"
#include "SoundManager.h"


template<class T> class Renderer;
class GUIRenderer : public IRenderStrategy {
public:
    friend class Renderer<GUIRenderer>;
    GUIRenderer(sf::RenderWindow& window, int cell);
    ~GUIRenderer() override;
    void Initialize() override;
    void Render(const Game& game, const std::string& controls_legend) override;
    void RenderField(const PlayingField& field, const std::string& title) override;
   
    void OnAttackResult(const AttackResult& result) override;
    void OnAttackResult(const AttackResult& result, bool on_enemy_field) override;
    void OnAbilityResult(const AbilityResult& result) override;
    void ShowShotBanner(const std::string& text, bool on_enemy_field) override;
    void ShowMessage(const std::string& message) override;
    void AddMessage(const std::string& message);
private:
    struct ShipTextureSet {
        std::vector<sf::Texture> intact;
        std::vector<sf::Texture> damaged;
    };
    std::array<std::array<ShipTextureSet, 3>, 5> ship_texture_{};
    sf::RenderWindow& window_;
   
    SoundManager& sound_manager_;
    std::string ai_name_;
    int cell_size_;
    int cell_spacing_;
    sf::Vector2f player_pos_;
    sf::Vector2f enemy_pos_;
    sf::Font font_;
    sf::Texture water_texture_;
   
    std::vector<sf::Texture> wreckage_textures_;
    struct ShotBanner {
        std::string text;
        sf::Clock timer;
        bool active{false};
        bool on_enemy_field{true};
    } banner_;
    
    float banner_duration_sec_ = 0.7f;
    

    const size_t max_log_lines_ = 20;
    int last_round_ = -1;
    sf::Vector2f log_position_;
    std::vector<std::string> player_log_;
    std::vector<std::string> ai_log_;
    std::vector<std::string> ability_log_;
    std::string save_load_message_;
    int show_save_load_message_ = 0; 
    std::string error_message_;
 
    float grid_width_;
    float grid_height_;
    float total_width_;
    float total_height_;
   
    const float offset_x_ = 20.f;
    const float offset_y_ = 100.f;
    const float column_width_ = 315.f;
    const float column_height_ = max_log_lines_ * 22.f + 40.f;
    const float column_spacing_ = 10.f;
    const float ability_height_ = 70.f;
    const float input_height_ = 90.f;
    float panel_height_ = 300.f;
    float ai_column_x_;
    float player_column_x_;
    float frame_y_;
    const float dialog_width_ = 500.f;
    const float dialog_height_ = 150.f;

    const float help_width = 710.f;
    const float help_height = 880.f;
    float help_x;
    float help_y;

    const float dialog_files_width_ = 730.f;
    const float dialog_files_height_ = 250.f;
    float dialog_files_x_;
    float dialog_files_y_;
    const float slot_width = 120.f;
    const float slot_height = 70.f;
    float slot_start_x;
    float slot_start_y;

    float ask_window_x;
    float ask_window_y;
    const float ask_window_inner_offset = 5.f;
    const float ask_window_inner_thickness = 2.f;

    const float field_select_width_ = 440.f;
    const float field_select_height_ = 395.f;
    float field_select_x_;
    float field_select_y_;

    const float ship_sizes_width_ = 480.f;
    const float ship_sizes_height_ = 415.f;
    float ship_sizes_x_;
    float ship_sizes_y_;

    const float title_width = 500.f;
    const float title_height = 60.f;
    float title_x;
    const float title_y = 10.f;

    const float stats_width = column_width_ * 2 + column_spacing_;
    const float stats_height = 270.f;
    float stats_x;
    float stats_y;
   
    void RenderAbilitiesInfo(std::string info);
    void RenderShipsInfo(const Game& game);
    void RenderHelp(const Game& game);
    void RenderStats(const Game& game);
   
    bool LoadResources();
    bool LoadShipTextures();
    bool LoadWreckageTextures();

    void RenderGameTitle(int round);
    void RenderScore(const Game& game);
    void RenderFieldFrame(const PlayingField& field, const sf::Vector2f& position, const std::string& title);
    void RenderCell(const PlayingField& field, CellState state, int ship_size_,
                    const std::tuple<int, SegmentState, Orientation>& segmentInfo,
                    const sf::Vector2f& position, bool is_enemy_field);
    void RenderShipPart(const sf::Vector2f& position, int ship_size_,
                        const std::tuple<int, SegmentState, Orientation>& segmentInfo,
                        bool fullyDestroyed);
   
    void ClearLog();
    void RenderCursor(const Game& game);
    void RenderGameStatus(const Game& game);
    void RenderBanners();
    void RenderLog();
    void RenderInput(const Game& game);
    void RenderControlsLegend(const std::string& legend);
    void RenderFieldSizeSelection(const Game& game);
    void RenderShipSizeSelection(const Game& game);
    void RenderAskRound(const Game& game);
    void RenderSettingships_(const Game& game);
    void RenderAskSave(const Game& game);
    void RenderAskExit(const Game& game);
    void RenderAskWindow(const std::string& info);
    void RenderSaveLoadMessage();
    void RenderDialogFiles(const Game& game);
    void RenderError();
    void RenderErrorShip(const Game& game, int start_x, int start_y);
   
    int CellIdx() const;
    void DrawHitMark(const sf::Vector2f& center);
    void DrawMissMark(const sf::Vector2f& center);
    
    void DrawScanMark(const sf::Vector2f& center, bool positive);
    std::vector<std::string> wrapText(const std::string& text, float maxWidth, unsigned int characterSize);
    static sf::String utf8(const std::string& s) { return sf::String::fromUtf8(s.begin(), s.end()); }
    static sf::String utf8(const char* s) { const char* e = s + std::strlen(s); return sf::String::fromUtf8(s, e); }
};

#endif