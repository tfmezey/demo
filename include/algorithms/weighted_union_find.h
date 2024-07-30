#ifndef WEIGHTED_UNION_FIND_H
#define WEIGHTED_UNION_FIND_H

#include "_algorithms.h"

namespace algorithms
{
	/*
	 * Union find class to address the connectivity problem.  Connections are added
	 * with formUnion().  A connection between two integers can be querried with the
	 * connected() boolean.  A component of an integer can be obtained via find().
	 * 
	 * The algorithm implements a collapsed binary tree, producing worst case performance
	 * of log N for N items.  This is also the height of the binary tree.
	*/
	
	class weighted_union_find
	{
	private:
		
		uint* _id = nullptr;					// parent link (site indexed)
		uint* _component_size = nullptr;		// size of component for roots (site indexed)
		uint _count = 0;						// The number of components.
		uint _size = 0;							// Size of memory allocation.
	public:
		weighted_union_find(const uint&) noexcept;
		~weighted_union_find() noexcept { delete[] _id; delete[] _component_size; }
		
		uint count() const { return _count; }
		
		bool connected(const uint& p, const uint& q) const { return find(p) == find(q); }
		uint find(const uint& p) const;
		void formUnion(const uint& p, const uint& q);
	};
}

#endif
