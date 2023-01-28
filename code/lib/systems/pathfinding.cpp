#include "pathfinding.h"


namespace pathfinding {
	template <typename T>
	static void AStar(T start, T goal, std::function<float(T, T)> heuristic, std::function<bool(T&, T&)> compare, std::function<std::vector<T>(T)> getNeighbours)
	{

		AStarNode<T> actualStart{ start };
		AStarNode<T> actualGoal{ goal };


		std::set<AStarNode<T>, std::function<bool(T&, T&)>> openSet{};
		openSet.insert(actualStart);


		while (openSet.size > 0)
		{
			auto beginIt = openSet.begin();

			// pop the node from the "priority queue" 
			AStarNode<T>& n = *beginIt; // find the node (deref)
			T& nValue = n.getValue();
			if (nValue == goal)
			{
				return reconstructPath(n);
			}

			openSet.erase(beginIt); // delete it from the set 
			
			// foil the neighbours according to the neighbouring function
			std::vector<T> neighbours = getNeighbours(nValue);
			for (auto& neighbour : neighbours)
			{
				// do the mathy math stuffs & add new neighbours
			}

		}

		/* this some cursed shit imma try and avoid if i can help it
		struct customCompare {
			bool operator() (const T& lhs, const T& rhs)
			{
				return compare(lhs, rhs);
			}
		};
		*/
		// initialize a set of nodes




		// maybe we start with a simpler implementation lol


	}
}
