/*
 * DHT.hpp
 *
 *  Created on: May 24, 2009
 *      Author: prashant
 */

#ifndef DHT_HPP_
#define DHT_HPP_

#include <sys/types.h>

#include <boost/unordered_map.hpp>
#include <string>
#include "protocol/messages.hpp"


class DHT {
private:
	DHT(const std::string	&ip, uint16_t	port);
	static DHT	*singletonObject;
	node_record	this_node;

	boost::unordered_map<hash, std::list<file_info_record> >	hashTable;

public:
	static void make_dht(const std::string	&ip, uint16_t	port);
	static DHT*	get();
	node_record	getNode();
	virtual ~DHT();

	void insertFileInfoRecord(const file_info_record&	record);
	std::list<file_info_record> query(const hash&	h);
};

#endif /* DHT_HPP_ */
