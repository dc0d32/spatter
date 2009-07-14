/*
 * DHT.cpp
 *
 *  Created on: May 24, 2009
 *      Author: prashant
 */

#include "DHT.hpp"
#include "../util/util.hpp"

#include <boost/lexical_cast.hpp>
#include <algorithm>

DHT::DHT(const std::string	&ip, uint16_t	port)
{
	this_node.ip = ip;
	this_node.port = port;

	using boost::lexical_cast;
	using boost::bad_lexical_cast;

	std::string	str = ip + lexical_cast<std::string>(port) + lexical_cast<std::string>(random());
	calculate_hash(str, this_node.nodeId.hash);
}

DHT::~DHT()
{

}

void	DHT::make_dht(const std::string	&ip, uint16_t	port)
{
	if(!singletonObject)
	{
		singletonObject = new DHT(ip, port);
	}
}

DHT*	DHT::get()
{
	return singletonObject;
}

node_record	DHT::getNode()
{
	return this_node;
}

DHT*	DHT::singletonObject = NULL;






void DHT::insertFileInfoRecord(const file_info_record&	record)
{
	boost::unordered_map<hash, std::list<file_info_record> >::iterator	itr = hashTable.find(record.key);
	if(itr == hashTable.end())
	{
		std::list<file_info_record>	l;
		l.push_back(record);
		hashTable.insert(std::make_pair(record.key, l));
	}
	else
	{
		std::list<file_info_record>	*l = &itr->second;
		if(std::find(l->begin(), l->end(), record) == l->end())
		{
			l->push_back(record);
		}
	}
}

std::list<file_info_record> DHT::query(const hash&	h)
{
	std::list<file_info_record>	l;

	boost::unordered_map<hash, std::list<file_info_record> >::iterator	itr = hashTable.find(h);
	if(itr != hashTable.end())
	{
		l = itr->second;
	}
	return l;
}

