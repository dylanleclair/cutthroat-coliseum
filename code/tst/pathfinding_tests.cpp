#include "gtest/gtest.h"
#include "systems/pathfinding.h"

struct DummyData
{
    int hello0{ 0 };
    int hello1{ 1 };
};

struct DummyDataAlternate
{
    int why{ 1 };
    int oh{ 0 };
};

struct Transform
{
    float posX, posY, posZ;
};

TEST(pathfinding, do_nothing)
{

    std::cout << "yay";
    ASSERT_TRUE(true);

}
