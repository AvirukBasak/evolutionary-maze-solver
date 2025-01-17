//
// Created by aviruk on 1/6/25.
//

#ifndef CLASSES_GAME_H
#define CLASSES_GAME_H

#include "SFML/Graphics.hpp"

#include "components/Entity.h"
#include "components/Maze.h"
#include "components/CounterText.h"
#include "components/MazeCellToolTip.h"

class Game {
    sf::RenderWindow &mWindow;

    Maze mMaze;
    MazeCellToolTip mMazeCellToolTip;
    CounterText mCounterText;
    std::vector<Entity> mEntities{};

    int mPrevGeneration;

public:
    explicit Game(sf::RenderWindow &window);

    ~Game();

    /**
     * Receives an event from main method using window.pollEvent.
     * All IGameComponent components receive every event.
     * @param event An event that needs to be handled.
     */
    void handleEvent(const sf::Event &event);

    /**
     * Runs on every iteration of game loop
     */
    void update();

    /**
     * Runs RENDER_FPS times per second in game loop.
     */
    void render() const;

    void startSelectionAndReproduction();
};

#endif // CLASSES_GAME_H
