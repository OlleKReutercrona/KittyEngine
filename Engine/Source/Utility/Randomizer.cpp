#include "stdafx.h"
#include "Randomizer.h"

#include <random>

const int KE::GetRandomUniformInt(const int aMin, const int aMax)
{
    static std::default_random_engine generator((unsigned int(KE_GLOBAL::trueDeltaTime * 10000.0f)));

    std::uniform_int_distribution<int> dist(aMin, aMax);

    return dist(generator);
}

const float KE::GetRandomUniformFloat(const float aMin, const float aMax)
{
    static std::default_random_engine generator((unsigned int(KE_GLOBAL::trueDeltaTime * 10000.0f)));

    std::uniform_real_distribution<float> dist(aMin * 1000.0f, aMax * 1000.0f);

    return dist(generator) / 1000.0f;
}