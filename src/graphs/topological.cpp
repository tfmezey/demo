#include "topological.h"

namespace graphs
{
	topological::topological(const digraph& dg)
	{
		// The largest path within a graph is a loop of ALL the verticies.  From start to start = V() + 1.
		_order = new path();
		directed_cycle cyclefinder(dg);
		if(cyclefinder.hasCycle() == false)
		{
			depth_first_order dfo(dg);
			rpost temp = dfo.ReversePost();
			
			for(rpost_citer i = temp.cbegin(); i != temp.cend(); i++)
				_order->add(*i);
		}
	}
	
	void topological::operator()(const digraph& dg)
	{
		_order->clear();
		
		directed_cycle cyclefinder(dg);
		if(cyclefinder.hasCycle() == false)
		{
			depth_first_order dfo(dg);
			rpost temp = dfo.ReversePost();
			for(rpost_citer i = temp.cbegin(); i != temp.cend(); i++)
				_order->add(*i);
		}
	}
	
	topological::topological(const edge_weighted_digraph& dg)
	{
		// The largest path within a graph is a loop of ALL the verticies.  From start to start = V() + 1.
		_order = new path();
		directed_cycle cyclefinder(dg);
		if(cyclefinder.hasCycle() == false)
		{
			depth_first_order dfo(dg);
			rpost temp = dfo.ReversePost();
			
			for(rpost_citer i = temp.cbegin(); i != temp.cend(); i++)
				_order->add(*i);
		}
	}
	
	void topological::operator()(const edge_weighted_digraph& dg)
	{
		_order->clear();
		
		directed_cycle cyclefinder(dg);
		if(cyclefinder.hasCycle() == false)
		{
			depth_first_order dfo(dg);
			rpost temp = dfo.ReversePost();
			for(rpost_citer i = temp.cbegin(); i != temp.cend(); i++)
				_order->add(*i);
		}
	}
}
