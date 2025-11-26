#ifndef BATTLESHIP_CONTROLGAME_CONTROLLERGAME_GAMECONTROLLER_H_
#define BATTLESHIP_CONTROLGAME_CONTROLLERGAME_GAMECONTROLLER_H_
#include <memory>
#include <iostream>
#include <thread>
#include <chrono>
#include <string>
#include <algorithm>

#include "Command.h"
#include "Game.h"
#include "AbilityException.h"
#include "ShipCoordinateExceptions.h"


template <class InputHandlerType, class RendererType>
class GameController {
public:
    GameController(
        Game& game,
        std::unique_ptr<InputHandlerType> input_handler,
        std::shared_ptr<RendererType> renderer
    )
        : game_(game),
          input_handler_(std::move(input_handler)),
          renderer_(std::move(renderer)) {}

    bool Initialize() {
        if (!input_handler_ || !renderer_) {
            std::cerr << "Ошибка: обработчик ввода или рендерер не инициализированы!\n";
            return false;
        }
        if (!input_handler_->Initialize()) {
            std::cerr << "Ошибка инициализации обработчика ввода!\n";
            return false;
        }
        renderer_->Initialize();

        return true;
    }

    void RenderOnce() {
        if (renderer_) {
            renderer_->Render(game_, input_handler_->control_legend());
        }
    }

    void Run() {
        using namespace std::chrono_literals;

        RenderOnce();
        int last_cx = game_.current_state().cursor_x();
        int last_cy = game_.current_state().cursor_y();
        auto current_status = game_.game_status();

        while (game_.IsRunning()) {
            bool need_render = false;

            GameStatus st = game_.game_status();
            auto& st_obj = game_.current_state();
            int cx = st_obj.cursor_x();
            int cy = st_obj.cursor_y();


            if (st != current_status) { 
                current_status = st;
                need_render = true; 
            }
            if (cx != last_cx || cy != last_cy) { 
                last_cx = cx; 
                last_cy = cy; 
                need_render = true; 
            }

            if (st == GameStatus::ENEMY_TURN) {
                ProcessAITurn();
                need_render = true;
            } else {
                if (auto command = input_handler_->GetCommand()) {
                    ExecuteCommand(*command);
                    need_render = true;
                }
            }

            if (need_render) renderer_->Render(game_, input_handler_->control_legend());;
            std::this_thread::sleep_for(16ms);
        }
    }

private:
    void ProcessAITurn() {
        using namespace std::chrono_literals;
        renderer_->ShowShotBanner( game_.ai_name() + " стреляет...", false);
        renderer_->Render(game_, input_handler_->control_legend());
        std::this_thread::sleep_for(16ms);
        renderer_->OnAttackResult(game_.MakeAIMove(), false);
        std::this_thread::sleep_for(150ms);
    }

    void AutoPlacementShip(){
        game_.MoveAIShips();
        game_.MoveRandomShips();
        game_.set_player_turn_status();
    }

    bool CanExecuteCommand(const Command& command) const {
        auto status = game_.game_status(); 

        if (!game_.IsHelpClosed()) {
            return command.type() == CommandType::HELP;
        }
    
        switch (command.type()) {
            case CommandType::EXIT:
            case CommandType::HELP:
                return status != GameStatus::SELECT_SAVE_SLOT && status != GameStatus::SELECT_LOAD_SLOT &&
                        status != GameStatus::SET_SIZES && status != GameStatus::SET_FIELD &&
                        status != GameStatus::ASK_EXIT && status != GameStatus::ASK_SAVE; 
            case CommandType::PAUSE:       return status == GameStatus::PLAYER_TURN || status == GameStatus::PAUSED;
            case CommandType::RESTART:     return status == GameStatus::PLAYER_WON || status == GameStatus::ENEMY_WON || status == GameStatus::GAME_OVER || status == GameStatus::PAUSED;
            case CommandType::SAVE:        return game_.CanSaveGame();
            case CommandType::LOAD:        return game_.CanLoadGame();
            case CommandType::REMOVE_SHIP: return status == GameStatus::PLACING_SHIPS && game_.placement_mode() == PlacementMode::MANUAL;
            case CommandType::USE_ABILITY: return status == GameStatus::PLAYER_TURN;
            case CommandType::ATTACK:      return status == GameStatus::PLAYER_TURN;
            case CommandType::SET_NEW_SHIP_SIZES: 
            case CommandType::SET_NEW_FIELD: 
            case CommandType::TOGGLE_PLACEMENT_MODE:
                return status == GameStatus::PLACING_SHIPS;
            case CommandType::SHOW_SHIPS:  return status == GameStatus::PLACING_SHIPS  && game_.placement_mode() == PlacementMode::MANUAL;
            case CommandType::STATS:       return status != GameStatus::GAME_OVER;
            case CommandType::ROTATE_SHIP: return status == GameStatus::PLACING_SHIPS && game_.placement_mode() == PlacementMode::MANUAL;
            case CommandType::PLACE_SHIP:  return status == GameStatus::PLACING_SHIPS;
            case CommandType::MOVE_UP:
            case CommandType::MOVE_DOWN:
            case CommandType::MOVE_LEFT:
            case CommandType::MOVE_RIGHT:
                return status == GameStatus::PLAYER_TURN || status == GameStatus::PLACING_SHIPS;
            case CommandType::SET_1:
            case CommandType::SET_2:
            case CommandType::SET_3:
            case CommandType::SET_4:
            case CommandType::SET_5:
                return status == GameStatus::SET_FIELD || status == GameStatus::SET_SIZES ||
                        status == GameStatus::SELECT_SAVE_SLOT || status == GameStatus::SELECT_LOAD_SLOT;
            case CommandType::YES:
            case CommandType::NO:
                return status == GameStatus::ASK_SAVE || status == GameStatus::SETTING_SHIPS 
                        || status == GameStatus::ASK_EXIT || status == GameStatus::WAITING_NEXT_ROUND ||
                        status == GameStatus::SELECT_LOAD_SLOT || status == GameStatus::SELECT_SAVE_SLOT ||
                        status == GameStatus::SET_FIELD || status == GameStatus::SET_SIZES;
                            
            default:
                return false;
        }
    }

    void ExecuteCommand(const Command& command) {
        using namespace std::chrono_literals;
        try {
            switch (command.type()) {
                case CommandType::RESTART:
                    if (CanExecuteCommand(command)) game_.RestartGame();
                    break;

                case CommandType::EXIT: 
                    if (CanExecuteCommand(command)) {
                        last_status_ = game_.game_status(); 
                        game_.set_game_status(GameStatus::ASK_EXIT);
                    }
                    break;

                // Пауза
                case CommandType::PAUSE:
                    if (CanExecuteCommand(command)) {
                        game_.PauseGame();
                    }
                    break;
                // Сохранение/загрузка
                case CommandType::SAVE:
                    if (CanExecuteCommand(command)){
                        last_status_ = game_.game_status();
                        game_.set_game_status(GameStatus::SELECT_SAVE_SLOT); 
                    }
                    break;
                case CommandType::LOAD:
                    if (CanExecuteCommand(command)) {
                        last_status_ = game_.game_status();
                        game_.set_game_status(GameStatus::SELECT_LOAD_SLOT); 
                    }
                    break;

                // Настройка
                case CommandType::SET_NEW_SHIP_SIZES:
                    if (CanExecuteCommand(command)) {
                        last_status_ = game_.game_status();
                        game_.set_game_status(GameStatus::SET_SIZES);
                        game_.ClearShipSizes();
                    }
                    break;
                case CommandType::SET_NEW_FIELD:
                    if (CanExecuteCommand(command)){
                        last_status_ = game_.game_status();
                        game_.set_game_status(GameStatus::SET_FIELD);
                    } 
                    break;
                case CommandType::TOGGLE_PLACEMENT_MODE:
                    if (CanExecuteCommand(command)) game_.TogglePlacementMode();
                    break;
                // показать информацию
                case CommandType::SHOW_SHIPS:
                    if (CanExecuteCommand(command)) game_.ToggleShipsInfo();
                    break;
                case CommandType::STATS:
                    if (CanExecuteCommand(command)) game_.ToggleStats();
                    break;
                case CommandType::HELP:
                    if (CanExecuteCommand(command)) game_.ToggleHelp();
                    break;
                // Корабли
                case CommandType::PLACE_SHIP:
                    if (CanExecuteCommand(command)) {
                        if (game_.placement_mode() == PlacementMode::AUTO){
                            AutoPlacementShip();
                        }
                        else {
                            int x = game_.current_state().cursor_x();
                            int y = game_.current_state().cursor_y();
                            game_.MoveShip(x, y, game_.ship_orientation());
                        }
                    }
                    break;
                case CommandType::REMOVE_SHIP:
                    if (CanExecuteCommand(command)){
                        int x = game_.current_state().cursor_x();
                        int y = game_.current_state().cursor_y();
                        game_.ClearShip(x,y);
                    }
                    break;
                case CommandType::ROTATE_SHIP:
                    if (CanExecuteCommand(command)) game_.RotateShip();
                    break;
                case CommandType::ATTACK: {
                    if (CanExecuteCommand(command)) {
                        renderer_->ShowShotBanner("Вы стреляете...", true);
                        renderer_->Render(game_, input_handler_->control_legend());
                        std::this_thread::sleep_for(120ms);

                        AttackResult result = game_.AttackShip();
                        renderer_->OnAttackResult(result, true);

                        if (game_.current_state().game_status() == GameStatus::ENEMY_TURN) {
                            renderer_->Render(game_, input_handler_->control_legend());
                            std::this_thread::sleep_for(120ms);
                            ProcessAITurn();
                        }
                    }
                    break;
                }
                case CommandType::USE_ABILITY: {
                    if (CanExecuteCommand(command)) {
                        int x = game_.current_state().cursor_x();
                        int y = game_.current_state().cursor_y();
                        AbilityResult result = game_.UseAbility(x,y);
                        renderer_->OnAbilityResult(result);
                    }
                    break;
                }
                // перемещение курсора
                case CommandType::MOVE_UP:
                    if (CanExecuteCommand(command)) {
                        const int x = game_.current_state().cursor_x();
                        const int y = game_.current_state().cursor_y();
                        game_.current_state().set_cursor(x, std::max(0, y - 1));
                    }
                    break;
                case CommandType::MOVE_DOWN:
                    if (CanExecuteCommand(command)) {
                        const int x = game_.current_state().cursor_x();
                        const int y = game_.current_state().cursor_y();
                        const int max_y = game_.enemy_field().y_size() - 1;
                        game_.current_state().set_cursor(x, std::min(max_y, y + 1));

                        
                    }
                    break;
                case CommandType::MOVE_LEFT:
                    if (CanExecuteCommand(command)) {
                        const int x = game_.current_state().cursor_x();
                        const int y = game_.current_state().cursor_y();
                        game_.current_state().set_cursor(std::max(0, x - 1), y);
                    }
                    break;
                case CommandType::MOVE_RIGHT:
                    if (CanExecuteCommand(command)) {
                        const int x = game_.current_state().cursor_x();
                        const int y = game_.current_state().cursor_y();
                        const int max_x = game_.enemy_field().x_size() - 1;
                        game_.current_state().set_cursor(std::min(max_x, x + 1), y);
                    }
                    break;
                // Выборы
                case CommandType::SET_1:
                case CommandType::SET_2:
                case CommandType::SET_3:
                case CommandType::SET_4:
                case CommandType::SET_5:
                    if (CanExecuteCommand(command)){
                        auto status_ = game_.game_status();
                        int number = static_cast<int>(command.type()) - static_cast<int>(CommandType::SET_1) + 1;

                        if (status_ == GameStatus::SET_FIELD) {
                            game_.set_temp_field_size(9 + number);
                        } else if (status_ == GameStatus::SET_SIZES) {
                            if (number <= 4) {
                                game_.AddShipSize(number);
                            } else {
                                game_.set_auto_ship_sizes();
                                game_.Initialize();
                            }
                        } else if (status_ == GameStatus::SELECT_SAVE_SLOT && number <= 4) {
                            std::string slot_name = common_slot_name_ + std::to_string(number);
                            game_.set_game_status(last_status_);
                            game_.SaveGame(slot_name);
                            renderer_->ShowMessage(std::string(" Сохранено в слот ") + slot_name);
                        }
                        else if (status_ == GameStatus::SELECT_LOAD_SLOT) {
                            std::string slot_name;
                            if (number == 5) {
                                slot_name = file_name_exit_;
                            } else {
                                slot_name = common_slot_name_ + std::to_string(number);
                            }
                            game_.LoadGame(slot_name);
                            renderer_->ShowMessage(std::string(" Загружен слот ") + slot_name);
                        }
                    }
                    break;
                case CommandType::YES:
                    if (CanExecuteCommand(command)){
                        auto status_ = game_.game_status(); 
                        if (status_ == GameStatus::WAITING_NEXT_ROUND){
                            game_.PrepareNextRound();
                        } else if (status_ == GameStatus::SETTING_SHIPS) {
                            game_.ClearFields();
                            game_.set_game_status(GameStatus::PLACING_SHIPS);
                        } else if (status_ == GameStatus::ASK_SAVE) {
                            game_.set_game_status(last_status_);
                            game_.SaveGame(file_name_exit_);
                            game_.ExitGame();
                        } else if (status_ == GameStatus::ASK_EXIT){
                            game_.set_game_status(GameStatus::ASK_SAVE);
                        } else if (status_ == GameStatus::SET_SIZES){
                            game_.set_ship_fleet_spec();
                            game_.Initialize();
                        } else if (status_ == GameStatus::SET_FIELD){
                            game_.ApplyFieldSize();
                            game_.Initialize();
                        }
                    }
                    break;
                case CommandType::NO:
                    if (CanExecuteCommand(command)){
                        auto status_ = game_.game_status(); 
                        if (status_ == GameStatus::SETTING_SHIPS){
                            game_.set_game_status(GameStatus::PLAYER_TURN);
                        } else if (status_ == GameStatus::ASK_SAVE) {
                            game_.ExitGame();
                        } else if (status_ == GameStatus::ASK_EXIT){
                            game_.set_game_status(last_status_);
                        } else if (status_ == GameStatus::SELECT_SAVE_SLOT || status_ == GameStatus::SELECT_LOAD_SLOT) {
                            game_.set_game_status(last_status_);
                        } else if (status_ == GameStatus::SET_SIZES){
                            game_.set_game_status(last_status_);
                        } else if (status_ == GameStatus::SET_FIELD){
                            game_.set_game_status(last_status_);
                        }
                    }
                    break;
                default:
                    renderer_->ShowMessage("Неизвестная или неподдерживаемая команда!");
                    break;
            }
        } catch (const ShipPlacementException& e) {
            renderer_->ShowMessage(e.what());
        } catch (const AbilityException& e) {
            renderer_->ShowMessage(e.what());
        } catch (const std::exception& e) {
            renderer_->ShowMessage(std::string("Ошибка выполнения команды: ") + e.what());
        }
    }

    Game& game_;
    std::unique_ptr<InputHandlerType> input_handler_;
    std::shared_ptr<RendererType> renderer_;
    GameStatus last_status_  = GameStatus::GAME_OVER;
    std::string file_name_exit_ = "exit_save";
    std::string common_slot_name_ = "slot";
};

#endif