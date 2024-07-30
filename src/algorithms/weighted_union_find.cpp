#include "weighted_union_find.h"

namespace algorithms
{
	weighted_union_find::weighted_union_find(const uint& N) noexcept
	{
		// At initialiation, every element is in its own component.  Thus _count == _size == N.
		_count = N;
		_size = _count;
		_id = new uint[_size];
		_component_size = new uint[_size];
		
		for(int i = 0; i < _size; i++)
		{
			_id[i] = i;
			_component_size[i] = 1;
		}
	}
	
	uint weighted_union_find::find(const uint& q) const
	{
		uint p = q;
		
		// Follow links to find root.
		while(p != _id[p])
			p = _id[p];
		
		return p;
	}
	
	void weighted_union_find::formUnion(const uint& p, const uint& q)
	{
		uint i = find(p);
		uint j = find(q);
		if(i == j)
			return;
		
		// Make smaller root point to larger one
		if(_component_size[i] < _component_size[j])
		{
			_id[i] = j;
			_component_size[j] += _component_size[i];
		}
		else
		{
			_id[j] = i;
			_component_size[i] += _component_size[j];
		}
		
		_count--;
	}
}
