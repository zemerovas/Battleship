#ifndef BATTLESHIP_CONTROLGAME_CONTROLLERGAME_CONSOLE_CONSOLERENDERER_H_
#define BATTLESHIP_CONTROLGAME_CONTROLLERGAME_CONSOLE_CONSOLERENDERER_H_

#include "../Renderer.h"
#include "Game.h"
#include "core/PlayingField.h"
#include "Result.h"
#include "IRendererStrategy.h"
#include "controlGame/GameState.h"

#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include <regex>
#include <deque>
#include <sstream>
#include <algorithm>

class ConsoleRenderer : public IRenderStrategy {
public:
    ConsoleRenderer();
    ~ConsoleRenderer() override;

    void Initialize() override;
    void Render(const Game& game, const std::string& controlsLegend) override;
    void RenderField(const PlayingField& board, const std::string& title) override; 

    void OnAttackResult(const AttackResult& result) override;
    void OnAttackResult(const AttackResult& result, bool on_enemy_field) override;

    void OnAbilityResult(const AbilityResult& result) override;
    void AddMessage(const std::string& message);

    void ShowShotBanner(const std::string& text, bool on_enemy_field) override;
    void ShowMessage(const std::string& message) override;

private:
    std::vector<std::string> prev_frame_;
    std::vector<std::string> curr_frame_;
    bool first_frame_ = true;
    bool ansi_ready_  = true;

    std::deque<std::string> player_log_;
    std::deque<std::string> enemy_log_;
    std::deque<std::string> ability_log_;
    int last_round_ = -1;
    std::string error_message_;
    std::string save_load_message_;
    int show_save_load_message_ = 0;
    const size_t max_log_lines_ = 10;
    const size_t max_log_error_lines_ = 4;
    int new_error_index_ = -1;

    std::deque<std::string> log_;
    size_t log_max_ = 8;
    std::vector<std::string> current_ships_info_;

    std::vector<std::string> BuildFrame(const Game& game, const std::string& controls_legend);

    std::vector<std::string> BuildFieldBlock(const PlayingField& field, const std::string& title,
                                             int cursor_x_, int cursor_y_, bool highlight_cursor, bool revealships_);

    char CellGlyph(const PlayingField& field, int x, int y, bool revealships_) const;

    void PrintDiff(const std::vector<std::string>& next);
    void ClearScreenFull();
    void HideCursor(bool hide);
    void MoveCursor(int row, int col);

    std::string StatusToString(GameStatus status_, PlacementMode mode) const;

    void PushLog(const std::string& msg);

    std::vector<std::string> RenderBaseFields(const Game& game, int playercursor_x_, int playercursor_y_,  int enemycursor_x_, 
                                             int enemycursor_y_, bool showPlayerCursor, bool showEnemyCursor);

    std::vector<std::string> AddRightPanel(const std::vector<std::string>& leftBlock, 
                                                       const std::vector<std::string>& rightInfo);

    void ClearLog();
    void RenderSaveLoadMessage();
    void RenderShipsInfo(const Game& game);
    void RenderHelp(const Game& game);
    void RenderFieldSizeSelection(const Game& game, const std::string& controlLegend);
    void RenderShipSizeSelection(const Game& game, const std::string& controlLegend);
    std::vector<std::string> RenderStats(const Game& game);
    void RenderQuestionDialog(const std::string& question, const std::string& controlsLegend);
    void RenderDialogFiles(const Game& game, const std::string& controlLegend);

    std::vector<std::pair<std::string, std::string>> ExtractAllButtons(const std::string& str); 
    std::string choice_lines(const std::string& text, const std::string& substring);
};

#endif