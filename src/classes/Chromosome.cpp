//
// Created by aviruk on 1/8/25.
//

#include "classes/Chromosome.h"

#include <random>
#include <algorithm>
#include <cassert>

// Random number generation setup (see https://en.cppreference.com/w/cpp/numeric/random/uniform_int_distribution)
static std::random_device rd;
static std::mt19937 gen(rd());

Chromosome::Chromosome(const Maze &maze)
    : mMaze(maze), mChromoString(maze.mChromosmeFriend.mWhiteCellCount) {
    // Initialize with random moves (8 possible moves)
    std::uniform_int_distribution<> distrib(0, 7);

    for (auto &gene: mChromoString) {
        gene = static_cast<GeneticMoveInfo>(distrib(gen));
    }
}

Chromosome::GeneticMoveInfo Chromosome::getGeneticMoveInfoByCellNumber(const sf::Vector2i cellNum) const {
    const auto geneLocation = mMaze.mChromosmeFriend.mCellNumberToGeneIndexMapping.at(cellNum);
    return mChromoString[geneLocation];
}

void Chromosome::mutateRandom(int mutationCount) {
    // If mutationCount is 0 or exceeds chromosome length, mutate all genes
    if (mutationCount <= 0 || mutationCount > mChromoString.size()) {
        mutationCount = static_cast<int>(mChromoString.size());
    }

    // Create a uniform distribution for selecting random moves
    std::uniform_int_distribution<> moveDistrib(0, 7);

    // Create a uniform distribution for selecting gene positions
    std::uniform_int_distribution<> positionDistrib(0, static_cast<int>(mChromoString.size()) - 1);

    // Perform random mutations
    for (int i = 0; i < mutationCount; ++i) {
        // Select a random position to mutate
        const int position = positionDistrib(gen);
        // Generate a new random move
        const auto newMove = static_cast<GeneticMoveInfo>(moveDistrib(gen));
        // Apply mutation
        this->mChromoString[position] = newMove;
    }
}

std::pair<Chromosome, Chromosome> Chromosome::crossoverSinglePoint(const Chromosome &other) const {
    Chromosome child1(this->mMaze), child2(this->mMaze);

    // Ensure children have same size as parents
    assert(child1.mChromoString.size() == this->mChromoString.size());
    assert(child1.mChromoString.size() == other.mChromoString.size());
    assert(child2.mChromoString.size() == this->mChromoString.size());
    assert(child2.mChromoString.size() == other.mChromoString.size());

    // Select random crossover point for this parent.
    std::uniform_int_distribution<> distrib(0, static_cast<int>(mChromoString.size()) - 1);
    const int crossoverPoint = distrib(gen);

    // Do child1 = this[:cp] + other[cp:]
    std::copy(
        mChromoString.begin(),
        mChromoString.begin() + crossoverPoint,
        child1.mChromoString.begin()
    );
    std::copy(
        other.mChromoString.begin() + crossoverPoint,
        other.mChromoString.end(),
        child1.mChromoString.begin() + crossoverPoint
    );

    // Do child1 = other[:cp] + this[cp:]
    std::copy(
        other.mChromoString.begin(),
        other.mChromoString.begin() + crossoverPoint,
        child2.mChromoString.begin()
    );
    std::copy(
        mChromoString.begin() + crossoverPoint,
        mChromoString.end(),
        child2.mChromoString.begin() + crossoverPoint
    );

    return {child1, child2};
}
