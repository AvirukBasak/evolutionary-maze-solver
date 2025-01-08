//
// Created by aviruk on 1/8/25.
//

#ifndef STATES_H
#define STATES_H

class States final {
public:
    States() = delete;

    static int simulationSpeedScaler;
    static int pixelMovementSpeedScaler;
    static float mutationProbability;
    static int entityCount;
};

#endif //STATES_H
