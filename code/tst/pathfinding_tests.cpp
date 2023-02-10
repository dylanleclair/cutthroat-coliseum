#include "gtest/gtest.h"
#include "systems/pathfinding.h"
#include <math.h>
#include <iostream>

struct Transform
{
    float posX{ 0.0f }, posY{ 0.0f };

    bool operator==(Transform& rhs)
    {
        return (posX == rhs.posX && posY == rhs.posY);
    }

    void print()
    {
        printf("transform: %f,%f\n", posX, posY);
    }
};

float euclidean2D(Transform a, Transform b)
{
    float deltaX = b.posX - a.posX;
    float deltaY = b.posY - a.posY;

    float dist = pow(deltaX, 2) + pow(deltaY,2);
    return sqrt(dist);
}

std::vector<std::pair<Transform, float>> generateNearby(Transform initial)
{

    std::vector<std::pair<Transform, float>> output;

    // four cardinal directions (x marks start position)
    // 0a0
    // dxb
    // 0c0
    output.push_back({ { initial.posX + 1, initial.posY }, 1 });
    output.push_back({ { initial.posX - 1, initial.posY }, 1 });
    output.push_back({ { initial.posX, initial.posY + 1 }, 1 });
    output.push_back({ { initial.posX, initial.posY - 1 }, 1 });

    // the other directions (x marks start position)
    // d0a
    // 0x0
    // b0c
    output.push_back({ { initial.posX + 1, initial.posY + 1 }, sqrt(2.f) });
    output.push_back({ { initial.posX - 1, initial.posY - 1 }, sqrt(2.f) });
    output.push_back({ { initial.posX + 1, initial.posY - 1 }, sqrt(2.f) });
    output.push_back({ { initial.posX - 1, initial.posY + 1 }, sqrt(2.f) });

    return output;

}

TEST(pathfinding, a_star_2d)
{

    Transform a{ 0.f, 0.f };
    Transform b{ 12.f, 12.f};

    // must perform a rounding step in case the "goal" is not actually on the coordinate grid
    // OR, find some way to force the goal to be on the grid (in this case rounding?)
    
    printf("finding a path from point a to b\n");
    printf("point a: ");
    a.print();
    printf("point b: ");
    b.print();
    printf("\n");

    std::vector<Transform> result = pathfinding::AStar<Transform>(a, b, euclidean2D, generateNearby);

    printf("path from a to b generated using A* algorithm using euclidean distance as heuristic:");
    for (auto& pos : result)
    {
        pos.print();
    }
    
    ASSERT_TRUE(result[result.size() - 1] == b);

}

TEST(pathfinding, a_star_2d_no_print)
{

    Transform a{ 0.f, 0.f };
    Transform b{ 12.f, 12.f };

    // must perform a rounding step in case the "goal" is not actually on the coordinate grid
    // OR, find some way to force the goal to be on the grid (in this case rounding?)


    std::vector<Transform> result = pathfinding::AStar<Transform>(a, b, euclidean2D, generateNearby);

    ASSERT_TRUE(result[result.size() - 1] == b);

}



TEST(pathfinding, a_star_2d_no_print_far)
{

    Transform a{ 0.f, 0.f };
    Transform b{ 200.f, 200.f };

    // must perform a rounding step in case the "goal" is not actually on the coordinate grid
    // OR, find some way to force the goal to be on the grid (in this case rounding?)


    std::vector<Transform> result = pathfinding::AStar<Transform>(a, b, euclidean2D, generateNearby);

    ASSERT_TRUE(result[result.size() - 1] == b);

}

TEST(pathfinding, a_star_2d_no_print_far_rect)
{

    Transform a{ 0.f, 0.f };
    Transform b{ 100.f, 200.f };

    // must perform a rounding step in case the "goal" is not actually on the coordinate grid
    // OR, find some way to force the goal to be on the grid (in this case rounding?)

    std::vector<Transform> result = pathfinding::AStar<Transform>(a, b, euclidean2D, generateNearby);

    ASSERT_TRUE(result[result.size() - 1] == b);

}
