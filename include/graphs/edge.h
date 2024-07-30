#ifndef EDGE_H
#define EDGE_H

#include <iomanip>
#include "_graphs.h"

namespace graphs
{
	class edge
	{
	public:
		edge() {};
		edge(const uint& _v, const uint& _w, double _weight) noexcept { v = _v; w = _w; weight = _weight; }
		edge(const edge& e) noexcept { v = e.v; w = e.w; weight = e.weight; }
		edge(edge&& e) noexcept { v = e.v; w = e.w; weight = e.weight; }
		edge& operator=(const edge& e) noexcept { v = e.v; w = e.w; weight = e.weight; return *this; }
		edge& operator=(const edge&& e) noexcept { v = e.v; w = e.w; weight = e.weight; return *this; }
		~edge() {};
		
		inline int from() const { return v; }
		inline int to() const { return w; }
		inline int either() const { return v; }
		inline int other(const uint& vertex) const { if(vertex == v) return w; else return v; }
		int compareTo(const edge&) const;
		
		bool valid() const { return weight != inf; }		// If one of these are invalid, so is the entire class.
		bool operator==(const edge& rhs) const { return weight == rhs.weight; }
		bool operator!=(const edge& rhs) const { return weight != rhs.weight; }
		bool operator<(const edge& rhs) const { return weight < rhs.weight; }
		bool operator<=(const edge& rhs) const { return weight <= rhs.weight; }
		bool operator>(const edge& rhs) const { return weight > rhs.weight; }
		bool operator>=(const edge& rhs) const { return weight >= rhs.weight; }
		std::string str() const;
		
		double weight = inf;
		uint v = undefined_uint;
		uint w = undefined_uint;
	};
}

#endif
