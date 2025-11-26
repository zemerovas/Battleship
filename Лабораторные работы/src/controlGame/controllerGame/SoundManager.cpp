#include "SoundManager.h"
#include <iostream>

SoundManager& SoundManager::getInstance() {
    static SoundManager instance;
    return instance;
}

bool SoundManager::LoadSounds() {
    if (!explosion_buffer_.loadFromFile("resources/sounds/explosion.wav")) {
        std::cerr << "Предупреждение: не удалось загрузить resources/sounds/explosion.wav\n";
    } else {
        explosion_sound_.setBuffer(explosion_buffer_);
    }

    if (!water_splash_buffer_.loadFromFile("resources/sounds/water_splash.wav")) {
        std::cerr << "Предупреждение: не удалось загрузить resources/sounds/water_splash.wav\n";
    } else {
        water_splash_sound_.setBuffer(water_splash_buffer_);
    }

    if (!scanner_buffer_.loadFromFile("resources/sounds/scanner.wav")) {
        std::cerr << "Предупреждение: не удалось загрузить resources/sounds/scanner.wav\n";
    } else {
        scanner_sound_.setBuffer(scanner_buffer_);
    }

    bool background_active = background_music_.openFromFile("resources/sounds/background_music.ogg");
    if (!background_active) {
        std::cerr << "Ошибка загрузки фоновой музыки: resources/sounds/background_music.ogg\n";
    }
    return background_active;
}


void SoundManager::PlayExplosion() {
    explosion_sound_.play();
}

void SoundManager::PlayWaterSplash() {
    water_splash_sound_.play();
}

void SoundManager::PlayAbilitySound(const std::string& ability_name) {
    if (ability_name == "Scanner") {
        scanner_sound_.play();
    } else {
        explosion_sound_.play();
    }
}

void SoundManager::PlayBackgroundMusic() {
    background_music_.setLoop(true);
    background_music_.play();
}

void SoundManager::StopBackgroundMusic() {
    background_music_.stop();
}