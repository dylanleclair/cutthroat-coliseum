#pragma once

#include <functional>
#include <set>
#include <optional>
#include <string_view>

// references: 
// - https://en.wikipedia.org/wiki/A*_search_algorithm
// 

namespace pathfinding {


	/// <summary>
	/// prints a queue lol
	/// 
	/// STOLEN FROM C++ REFERENCE https://en.cppreference.com/w/cpp/container/priority_queue
	/// </summary>
	/// <typeparam name="Q"></typeparam>
	/// <param name="name"></param>
	/// <param name="q"></param>
	template<typename Q>
	void print_queue(std::string_view name, Q q)
	{
		// NB: q is passed by value because there is no way to traverse
		// priority_queue's content without erasing the queue.
		for (std::cout << name << ": \t"; !q.empty(); q.pop())
			std::cout << q.top() << ' ';
		std::cout << '\n';
	}


	// unlike djikstras, A* only computes the shortest path, NOT the minimum spanning tree
	// A* is a special modification of djikstras. 
	// - works over a much smaller portion of network
	// how exactly does it work? 

	// every edge has a non-negative length
	// we must change the lengths !!
	// - must extend one and shorten another
	//	- do on both sides !?!?
	//	- example: all incoming edges shorter by two, all outgoing longer by two

	// originally for finding least cost paths when the cost of a path is the sum of it's costs, but it has been shown that A& can be used to find optimal paths for any problems satisfying theconditions of a cost algebra

	// is an informed search algorithm, or a "best" first search
	// aims to find a path to the goal node having the smalles tcost (least distance travelled, shortest time) 
	//	- maintaisn a tree of oatgs ioriginating at the start node and extending those paths one edge at a time until it's termination criteria is satisfied.

	// at each iteration of it's main loop, A* needs to determine which of it's paths to extend. it does so based on the cost of the path and an estimate of the cost erequiresd to extend the path a;; the way to the goal. 
	// min function: ( f(n) = g(n) + h(n) ), where n is the next node on the path, g(n) is the cost of the path from start node to n, and h(n) is a heuristic function that estimates the cost of the cheapest path from n to the goal
	// the algorithm terminates when the path it chooses to extend is a path from the start to the goal, or if there are no paths eligible to be extended. 
	// HEURISTIC FUNCTION IS PROBLEM SPECIFIC !!! -> virtual void or lambda function / function pointer

	// ** If the heuristic function is __admissible__ (never overestimates the actual cost to get to the goal), A* is guaranteed to return a __ least cost __ path from start to finish! 

	// typically a priority queue (min/maxheap) is used to perform the repeated selection of minimum cost nodes to expand! 
	// - known as "open set" or fringe

	// at  each step, the node with the lowest f(x) value is removed, ** and the f and g values of ITS NEIGHBOURS are updated accordingly, and these neighbours are added to the queue ** !! 
	// algo continues until a removed node (lowest f value) is a goal node
	// the f value of that goal is then also the cost of the shortest path, since h at the gial is zero in an admissible heuristic

	// must use some mechanism for each node to keep track of it's predecessor (which node was previously shortest path) so that you can foil from goal node to start node and return a path
	// for literal maps, manhattan distance is typically best but might not make sense in a level 

	// consistency: with a consistent heuristic, A* is guaranteed to find an optimal path without processing any node more than once and A* is equivalent to running Djikstras algorithm with <partticular reduced cost function i dont care about > 


	// in this case, assume 2 points in 3d space ? 


	template <typename P>
	struct AStarNode
	{
		AStarNode(P typeVal, std::pair<AStarNode, bool> parent) : value(typeVal), parent(parent) {};
		std::vector<P> reconstructPath();

		const P& getValue() { return value; }

	private:
		P value;
		std::pair<AStarNode, bool> parent;
		float gCost{ std::numeric_limits<float>::max }; // cost of cheapest path from start to this node currently known
		float hCost{ std::numeric_limits<float>::max }; // cost estimate / heuristic from this node to goal
		float fCost{ std::numeric_limits<float>::max }; // fCost = gCost + hCost (best guess as to resulting path cost)


	};


	/// <summary>
	/// Computes the A* algorithm for a given set of points, using given heuristic and comparison functions.
	/// </summary>
	/// <typeparam name="T"></typeparam>
	/// <param name="start"></param>
	/// <param name="goal"></param>
	/// <param name="heuristic"></param>
	/// <param name="compare"></param>
	template <typename T> 
	static void AStar(T start, T goal, std::function<float(T, T)> heuristic, std::function<bool(T&, T&)> compare);




	




}