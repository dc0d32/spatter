/*
 * search_manager.hpp
 *
 *  Created on: May 25, 2009
 *      Author: prashant
 */

#ifndef SEARCH_MANAGER_HPP_
#define SEARCH_MANAGER_HPP_

#include <list>
#include <string>
#include <queue>

#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/array.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/unordered_set.hpp>
#include <boost/ptr_container/ptr_container.hpp>

#include "messages.hpp"
#include "../DHT.hpp"
#include "../NodeQueue.hpp"
#include "../../Config.hpp"
#include "../../util/util.hpp"

extern unsigned int		max_hops;
extern unsigned int		publish_redundancy;

enum search_status
{
	RUNNING,
	TERMINATED,
	KILLED
};

class search;

class sub_search : boost::noncopyable
{
	// promoted to class just because functions can not be
	// implemented in another cc file
public:
	std::string	queryToken;
	hash		key;
	uint32_t	searchId;
	search_status	status;
	unsigned int	hops;

	std::list<std::pair<uint32_t, node_record> >	next_nodes_queue;
	std::list<std::pair<uint32_t, node_record> >	pq;	// TODO convert to first class priority queue

	size_t					next_nodes_queue_max_size;
	boost::unordered_map<hash, std::list<file_info_record> >	results;
 	boost::mutex						_lock;

	boost::shared_ptr<search>	master_search;

	boost::shared_ptr<boost::thread> 	thread;


	sub_search(std::string	token, hash&	_key, boost::shared_ptr<search>	master);
	~sub_search();

	void	add_response(const	search_result_message&	srm);

	void	start();
private:
	void	run();
	bool	giveUpSearch();

	int	next_deadline;

	std::list<search_result_message>	responses;
	boost::condition_variable cond;
	boost::mutex _mut;
	bool response_queue_empty;



	bool	kill_search;
};


class search : boost::noncopyable

{
	// promoted to class just because functions can not be
	// implemented in another cc file
public:
	std::string	fullQuery;
	search_type	type;
	uint32_t	masterSearchId;

	float_t		percent_completed;

	boost::mutex	sub_searches_lock;
	boost::unordered_map<uint32_t,boost::shared_ptr<sub_search> >	sub_searches;

	~search();
	void	advance(sub_search	*ss);
	void	sub_search_finished(boost::shared_ptr<sub_search>	&ss);
};


void	handle_search(const	search_message&	sm);
void	handle_search_result(const	search_result_message&	srm);

uint32_t	start_search(hash	h, search_type	type);
uint32_t	start_search(std::string	query);

boost::shared_ptr<search>	find_search(uint32_t	masterSearchId);

bool	remove_search(uint32_t	masterSearchId);

namespace std
{
bool operator<(const std::pair<uint32_t, node_record>&	r1, const std::pair<uint32_t, node_record>&	r2);
}

#endif /* SEARCH_MANAGER_HPP_ */
