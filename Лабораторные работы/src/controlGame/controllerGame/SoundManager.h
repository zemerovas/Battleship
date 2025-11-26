#ifndef BATTLESHIP_CONTROLGAME_CONTROLLERGAME_SOUNDMANAGER_H_
#define BATTLESHIP_CONTROLGAME_CONTROLLERGAME_SOUNDMANAGER_H_

#include <SFML/Audio.hpp>
#include <string>

class SoundManager {
public:
    static SoundManager& getInstance();
    bool LoadSounds();
    void PlayExplosion();
    void PlayWaterSplash();
    void PlayAbilitySound(const std::string& ability_name);
    void PlayBackgroundMusic();
    void StopBackgroundMusic();

private:
    SoundManager() = default;
    sf::SoundBuffer explosion_buffer_;
    sf::SoundBuffer water_splash_buffer_;
    sf::SoundBuffer scanner_buffer_;
    sf::Sound explosion_sound_;
    sf::Sound water_splash_sound_;
    sf::Sound scanner_sound_;
    sf::Music background_music_;
};

#endif