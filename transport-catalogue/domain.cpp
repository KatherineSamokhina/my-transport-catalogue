#include "domain.h"

bool BusComparator::operator()(std::shared_ptr<Bus> lhs,
	std::shared_ptr<Bus> rhs) const {
	return lexicographical_compare(
		lhs->number.begin(), lhs->number.end(),
		rhs->number.begin(), rhs->number.end());
}

bool StopComparator::operator()(std::shared_ptr<Stop> lhs, std::shared_ptr<Stop> rhs) const {
	return lexicographical_compare(
		lhs->name.begin(), lhs->name.end(),
		rhs->name.begin(), rhs->name.end());
}

size_t detail::DistanceHasher::operator()(const std::pair<std::shared_ptr<Stop>, std::shared_ptr<Stop>>& p) const {
	std::hash<void*> hasher;
	size_t h1 = hasher(p.first.get());
	size_t h2 = hasher(p.second.get()) * 37 ^ 3;
	return h1 + h2;
}