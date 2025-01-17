//
// Created by aviruk on 1/4/25.
//

#include <cassert>
#include <cstring>
#include <vector>
#include <queue>
#include <limits>
#include <iostream>
#include <iomanip>

#include "constants.h"
#include "utils.h"
#include "states.h"
#include "components/Maze.h"

template<typename T>
void printMatrix(const std::vector<std::vector<T> > &matrix, const std::string &name) {
    for (const std::vector<T> &row: matrix) {
        for (const T &cell: row) {
            if ("mFitnessMaze" == name) {
                if (cell == -1) {
                    std::cout << "\u2588\u2588\u2588\u2588";
                } else {
                    std::cout << std::setw(3) << cell << " ";
                }
            } else {
                if (false == cell) {
                    std::cout << "\u2588\u2588";
                } else {
                    std::cout << "  ";
                }
            }
        }
        std::cout << '\n';
    }
}

void fillFitnessMaze(const std::vector<std::vector<bool> > &mBoolMaze, std::vector<std::vector<int> > &mFitnessMaze) {
    const int size = mBoolMaze.size();
    const int destX = size - 2;
    const int destY = size - 2;

    // Define directions for up, down, left, right
    const std::vector<sf::Vector2i> directions = {
        {-1, 0},
        {+1, 0},
        {0, -1},
        {0, +1}
    };

    // BFS to calculate the shortest path distances
    std::queue<std::pair<int, int> > queue;

    /* Fill cells of mFitnessMaze coreesponding to 0 cells of mBoolMaze
     * with std::numeric_limits<int>::max() */
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            if (false == mBoolMaze.at(i).at(j)) {
                mFitnessMaze.at(i).at(j) = std::numeric_limits<int>::max();
            }
        }
    }

    if (true == mBoolMaze.at(destY).at(destX)) {
        queue.emplace(destX, destY);
        mFitnessMaze.at(destY).at(destX) = 0;
    } else {
        throw std::invalid_argument("Unexpected false at Maze starting cell");
    }

    while (!queue.empty()) {
        auto [x, y] = queue.front();
        queue.pop();

        for (const auto &[dx, dy]: directions) {
            const int nx = x + dx;
            const int ny = y + dy;

            if (nx >= 0 && nx < size &&
                ny >= 0 && ny < size &&
                true == mBoolMaze.at(ny).at(nx) &&
                mFitnessMaze.at(ny).at(nx) > mFitnessMaze.at(y).at(x) + 1
            ) {
                mFitnessMaze.at(ny).at(nx) = mFitnessMaze.at(y).at(x) + 1;
                queue.emplace(nx, ny);
            }
        }
    }

    // Fill cells that are not reachable with a specific value (e.g., -1)
    for (int i = 0; i < size; ++i) {
        for (int j = 0; j < size; ++j) {
            if (mFitnessMaze.at(i).at(j) == std::numeric_limits<int>::max()) {
                // Indicating unreachable cells
                mFitnessMaze.at(i).at(j) = -1;
            }
        }
    }

    // Print matrix
    // printMatrix(mBoolMaze, "mBoolMaze");
    // printMatrix(mFitnessMaze, "mFitnessMaze");
}

ChromosmeFriend::ChromosmeFriend()
    : mWhiteCellCount(0) {
}

EntityFriend::EntityFriend() = default;

Maze::Maze(const int width, const int height)
    : mImgLoadSize(CELLS_PER_DIMENSION),
      mImgDrawSize(std::min(width, height)),
      mBoolMaze(CELLS_PER_DIMENSION, std::vector<bool>(CELLS_PER_DIMENSION, false)),
      mInverseFitnessMaze(CELLS_PER_DIMENSION, std::vector<int>(CELLS_PER_DIMENSION, WORST_INVERSE_FITNESS)) {
    /* Tasks:
     * - Load the IMGSIZE X IMGSIZE image Black / White image
     * - Set mBoolMaze values to mean of 4 channels of the image
     * - Set the mSrcBox and mDestBox
     * - Set mWidth and mHeight
     */
    // load image from file
    mImage.loadFromFile(Utils::pathjoin({
        Constants::ASSETS_PATH, "blueprint", "maze.png"
    }));
    assert(mImage.getSize().x == Maze::CELLS_PER_DIMENSION);
    assert(mImage.getSize().y == Maze::CELLS_PER_DIMENSION);

    // load pixels as an array
    using PixelArray = sf::Uint8[CELLS_PER_DIMENSION][CELLS_PER_DIMENSION][4];
    const PixelArray &pixelArr = *reinterpret_cast<const PixelArray *>(mImage.getPixelsPtr());

    /* load the image information into mBoolMaze vector by taking mean of 4 mage channels
     * and using a threshold to set tru or false */
    for (int i = 0; i < CELLS_PER_DIMENSION; i++) {
        for (int j = 0; j < CELLS_PER_DIMENSION; j++) {
            const sf::Uint8 *pixel = pixelArr[i][j];
            float grayScale = 0.0;
            for (int k = 0; k < 4; k++) {
                grayScale += pixel[k] / 4.0f;
            }
            // If white image cell (or far enough from black in color)
            if (grayScale >= 64) {
                mBoolMaze.at(i).at(j) = true; // Mark cell as true
                mChromosmeFriend.mWhiteCellCount += 1; // Increment count of white cells by 1
                // Set mCellNumberToGeneIndexMapping for (j, i) i.e. (row, col) to the index of gene
                mChromosmeFriend.mCellNumberToGeneIndexMapping.insert({
                    {j, i}, mChromosmeFriend.mWhiteCellCount - 1
                });
            }
        }
    }

    // Ensure (1,1) and (59-1, 59-1) are true
    assert(mBoolMaze.at(1).at(1));
    assert(mBoolMaze.at(CELLS_PER_DIMENSION-2).at(CELLS_PER_DIMENSION-2));

    // Fill the fitness matrix
    fillFitnessMaze(mBoolMaze, mInverseFitnessMaze);

    // Set max mutation count
    States::maxMutationCount = mChromosmeFriend.mWhiteCellCount;
}

Maze::~Maze() = default;

void Maze::handleEvent(const sf::Event &event) {
    if (event.type == sf::Event::MouseMoved) {
        const auto [x, y] = event.mouseMove;
        const sf::Vector2i cellNum = pixelToCellNumber(x, y);
        if (isCellNumberValid(cellNum) && mBoolMaze.at(cellNum.y).at(cellNum.x)) {
            const int fitness = getFitnessOfCellNumber(cellNum);
            const sf::Vector2f pixel = cellNumberToPixel(cellNum);
            // Compute the tooltip text and set state data
            States::mazeCellTooltipData = States::MazeCellToolTipData(
                cellNum, fitness,
                mChromosmeFriend.mCellNumberToGeneIndexMapping.at({cellNum.x, cellNum.y}),
                pixel,
                {
                    pixel.x + getCellSizeInPixels(),
                    pixel.y + getCellSizeInPixels()
                }
            );
        } else {
            States::mazeCellTooltipData = States::MazeCellToolTipData::empty();
        }
    }
}

void Maze::update() {
}

void Maze::draw(sf::RenderTarget &target, const sf::RenderStates states) const {
    sf::Texture texture;
    texture.loadFromImage(mImage);
    sf::Sprite sprite;
    sprite.setTexture(texture);

    const sf::Vector2f targetSize(mImgDrawSize, mImgDrawSize);
    const sf::FloatRect spriteBounds = sprite.getLocalBounds();
    sprite.setScale({
        targetSize.x / spriteBounds.width,
        targetSize.y / spriteBounds.height
    });

    target.draw(sprite, states);
}

sf::Vector2i Maze::pixelToCellNumber(const float pixelX, const float pixelY) const {
    // image size in pixels when loaded from file = mImgLoadSize
    // image size in pixels when drawn = mImgDrawSize
    // total cells per dimension of image / maze (x and y dirn) = CELLS_PER_DIMENSION
    return {
        (int) (pixelX / getCellSizeInPixels()),
        (int) (pixelY / getCellSizeInPixels())
    };
}

sf::Vector2f Maze::cellNumberToPixel(const sf::Vector2i cellNumber) const {
    return {
        cellNumber.x * getCellSizeInPixels(),
        cellNumber.y * getCellSizeInPixels()
    };
}

bool Maze::isValidMoveInPixels(const float yourSize, const float pixelX, const float pixelY, const float dx,
                               const float dy) const {
    const float newX = pixelX + dx;
    const float newY = pixelY + dy;
    const sf::Vector2i cellNumForEntityTL = pixelToCellNumber(newX, newY);
    const sf::Vector2i cellNumForEntityBR = pixelToCellNumber(newX + yourSize, newY + yourSize);
    return isCellNumberValid(cellNumForEntityTL)
           && true == mBoolMaze.at(cellNumForEntityTL.y).at(cellNumForEntityTL.x)
           && isCellNumberValid(cellNumForEntityBR)
           && true == mBoolMaze.at(cellNumForEntityBR.y).at(cellNumForEntityBR.x);
}

bool Maze::isCellNumberValid(sf::Vector2i cellNum) const {
    const auto [j, i] = cellNum;
    return i >= 0 && i < mBoolMaze.size() && j >= 0 && j < mBoolMaze.at(0).size();
}

int Maze::getFitnessOfCellNumber(sf::Vector2i cellNum) const {
    const auto [j, i] = cellNum;
    if (!isCellNumberValid({j, i}) || !mBoolMaze.at(i).at(j)) {
        return 0;
    }
    return WORST_INVERSE_FITNESS - mInverseFitnessMaze.at(i).at(j);
}

sf::Vector2i Maze::getSrcCellNumber() const {
    // ensure (1,1) is true
    assert(mBoolMaze.at(1).at(1));
    return {1, 1};
}

sf::Vector2i Maze::getDestCellNumber() const {
    // ensure (59-1, 59-1) is true
    assert(mBoolMaze.at(CELLS_PER_DIMENSION-2).at(CELLS_PER_DIMENSION-2));
    return {CELLS_PER_DIMENSION - 2, CELLS_PER_DIMENSION - 2};
}

float Maze::getCellSizeInPixels() const {
    // image size in pixels when loaded from file = mImgLoadSize
    // image size in pixels when drawn = mImgDrawSize
    // total cells per dimension of image / maze (x and y dirn) = CELLS_PER_DIMENSION
    return mImgDrawSize / CELLS_PER_DIMENSION;
}
