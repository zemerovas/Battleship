#ifndef BATTLESHIP_CONTROLGAME_GAMESETTINGS_H_
#define BATTLESHIP_CONTROLGAME_GAMESETTINGS_H_
#include <string>
#include <vector>

enum class InterfaceType { 
    CONSOLE, 
    GUI 
};

enum class FleetBuildMode { 
    STANDARD, 
    CUSTOM
};


enum class PlacementMode { 
    AUTO, 
    MANUAL 
};


class GameSettings {
public:    
    GameSettings();
    void ShowSettingsDialog(); 
    void ConfigureGameSettings();
    void AddShipSize(int size);
    void ClearTempFleetSpec();
    void ApplyFleetSpec();
    void ApplyFieldSize();
    void ResetFieldAndShipSize();
    
    void set_fleet_mode(bool custom = false);
    int temp_field_size() const;
    void set_temp_field_size(int size);
    
    InterfaceType interface_type() const;

    int cell_size() const;

    int field_size() const;
    void set_field_size(int size); 

    PlacementMode placement_mode() const;
    void set_placement_mode(PlacementMode mode);

    const std::string& player_name() const;
    void set_player_name(const std::string& name);

    const std::vector<int>& fleet_spec() const;
    const std::vector<int>& temp_fleet_spec() const;

private:
    InterfaceType interface_type_ = InterfaceType::GUI;
    int field_size_ = 10; 
    int cell_size_ = 40;  
    std::string player_name_ = "Player";
    FleetBuildMode fleet_mode_ = FleetBuildMode::STANDARD;
    PlacementMode placement_mode_ = PlacementMode::AUTO;
    std::vector<int> fleet_spec_;
    std::vector<int> temp_fleet_spec_;
    int temp_field_size_ = 10;
};

#endif
