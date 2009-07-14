/*
 * messages.cpp
 *
 *  Created on: May 24, 2009
 *      Author: prashant
 */

#include "messages.hpp"

#include <boost/functional/hash.hpp>

std::size_t hash_value(const hash& h)
{
	size_t	hash = 0;
	size_t	*p = (size_t*)(&h.hash[0]);
	for(unsigned int	i = 0; i < (sizeof(h.hash) / sizeof(size_t)); i++)
	{
		hash ^= *p;
	}

	if(sizeof(h.hash) % sizeof(size_t) != 0)
	{
		p = (size_t*)(&h.hash[sizeof(h.hash) - sizeof(size_t)]);
		hash ^= *p;
	}
	return hash;
}


std::size_t hash_value(const ping_message& p)
{
	boost::hash<uint32_t>	hasher;

	return hasher(p.messageId);
}
