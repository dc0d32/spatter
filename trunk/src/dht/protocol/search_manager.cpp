/*
 * search_manager.cpp
 *
 *  Created on: May 25, 2009
 *      Author: prashant
 */

#include "search_manager.hpp"
#include "control_protocol.hpp"

#include <boost/interprocess/sync/scoped_lock.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <boost/tokenizer.hpp>
#include <boost/shared_ptr.hpp>

#include <boost/unordered_map.hpp>

boost::unordered_map<uint32_t, boost::shared_ptr<search> >			searches;
boost::mutex	searches_lock;

// TODO link these with config-read values
unsigned int		max_hops = 7;
unsigned int		search_timeout = 5;	// seconds to give up after
unsigned int		alpha = 5;
unsigned int		publish_redundancy = 1;

static uint32_t		next_master_search_id = 0;
static uint32_t		next_sub_search_id = 0;

bool	sub_search::giveUpSearch()
{
	return (hops > max_hops);
}

void	sub_search::add_response(const	search_result_message&	srm)
{
	if(!kill_search)
	{
		responses.push_back(srm);
		{
			boost::lock_guard<boost::mutex> lock(_mut);
			response_queue_empty = false;
		}
		cond.notify_one();
	}
}

namespace std
{
bool operator<(const std::pair<uint32_t, node_record>&	r1, const std::pair<uint32_t, node_record>&	r2)
{
	return r1.first < r2.first;
}
}

void sub_search::run()
{
	while(!giveUpSearch() && !kill_search)
	{
		{
			master_search->advance(this);
		}

		{
			copy(next_nodes_queue.begin(), next_nodes_queue.end(), std::back_inserter(pq));
			pq.unique();

			// send search queries and wait for responses
			search_message	sm;

			sm.key = key;
			sm.searchId = master_search->masterSearchId;
			sm.subSearchId = searchId;
			sm.senderNode = DHT::get()->getNode();
			sm.type = master_search->type;
			std::string	msg = serialize_message<search_message>(SEARCH_MESSAGE, sm);

			std::list<std::pair<uint32_t, node_record> >::iterator	itr = next_nodes_queue.begin();

			while(itr != next_nodes_queue.end())
			{
				control_protocol::get()->udp_send(msg, itr->second);
				itr++;
			}

			next_nodes_queue.clear();
		}

		while(time(NULL) < next_deadline && !kill_search)
		{
			std::list<search_result_message>	some_responses;

			{
				boost::unique_lock<boost::mutex> lock(_mut);

				boost::xtime xt;
				xtime_get(&xt, boost::TIME_UTC);
				xt.sec += search_timeout;

				while(response_queue_empty)
				{
					if(!cond.timed_wait(lock, xt))
						break;
				}

				if(response_queue_empty)
					continue;

				some_responses = responses;
				responses.clear();
				next_deadline = time(NULL) + search_timeout;
				response_queue_empty = true;

			}

			if(kill_search)
				break;

			std::list<search_result_message>::iterator	itr = some_responses.begin();
			while(itr != some_responses.end())
			{
				std::list<node_record>::iterator	it = itr->node_list.begin();
				while(it != itr->node_list.end())
				{
					uint32_t	d = bit_distance(key, it->nodeId);

					// update next_nodes_queue
					{
						boost::unique_lock<boost::mutex>	__lock(_lock);
						std::cout << "updating next_nodes_queue" << std::endl;
						std::pair<uint32_t, node_record>	entry = std::make_pair<uint32_t, node_record>(d, *it);
						std::list<std::pair<uint32_t, node_record> >::iterator	iter = next_nodes_queue.begin();
						if(iter == next_nodes_queue.end())
						{
							next_nodes_queue.push_back(entry);
						}
						else
						{
							while(iter != next_nodes_queue.end())
							{
								if(d <= iter->first)
								{
									next_nodes_queue.insert(iter, entry);
									break;
								}
								iter++;
							}
							while(iter != next_nodes_queue.end())
							{
								if(*iter == entry)
									next_nodes_queue.erase(iter);
								iter++;
							}

							if(next_nodes_queue.size() > next_nodes_queue_max_size)
							{
								// splill last node if size overclaimed
								next_nodes_queue.pop_back();
							}
						}
					}

					it++;
				}

				// update file records if any
				if(master_search->type == KEY_SEARCH)
				{
					std::list<file_info_record>::iterator	it = itr->file_list.begin();
					while(it != itr->file_list.end())
					{
						boost::unique_lock<boost::mutex>	__lock(_lock);

						boost::unordered_map<hash, std::list<file_info_record> >::iterator	iter = results.find(it->key);
						if(iter == results.end())
						{
							std::list<file_info_record>	l;
							l.push_back(*it);
							results[it->key] = l;
						}
						else
						{
							iter->second.push_back(*it);
						}
						it++;
					}

				}

				itr++;
			}

		}
	}

	this->status = TERMINATED;

	std::cout << "exiting sub_search::run..... " << std::endl;
}

sub_search::~sub_search()
{
	kill_search = true;
	thread->join();
}

void	sub_search::start()
{
	thread = boost::shared_ptr<boost::thread> (new boost::thread(boost::bind(&sub_search::run, this)));
}

sub_search::sub_search(std::string	token, hash&	_key, boost::shared_ptr<search>		master)
{
	master_search = master;

	searchId = next_sub_search_id++;
	status = RUNNING;
	next_deadline = time(NULL) + search_timeout;
	hops = 0;

	next_nodes_queue_max_size = alpha;

	if(token.length())
	{
		queryToken = token;
		calculate_hash(queryToken, key.hash);
	}
	else
	{
		queryToken = "undefined-XX";
		key = _key;
	}

	std::list<node_record>	l = get_better_nodes(key);
	l.unique();
	if(l.size() < next_nodes_queue_max_size)
	{
		l = get_better_nodes(key, true);
		l.unique();
	}
	l.sort();
	std::list<node_record>::iterator	iter = l.begin();
	std::advance(iter, next_nodes_queue_max_size);
	l.erase(iter, l.end());


	std::list<node_record>::iterator	itr = l.begin();
	while(itr != l.end())
	{
		next_nodes_queue.push_back(std::make_pair<uint32_t, node_record>(bit_distance(key, itr->nodeId),*itr));
		itr++;
	}

	std::list<file_info_record> local_file_info_records = DHT::get()->query(key);
	results[key] = local_file_info_records;

	kill_search = false;
	response_queue_empty = true;
}


search::~search()
{
	sub_searches_lock.unlock();
	{
		boost::unique_lock<boost::mutex>	__lock(sub_searches_lock);
		sub_searches.clear();
	}
}

void	search::advance(sub_search	*ss)
{
	boost::unique_lock<boost::mutex>	__lock(sub_searches_lock);

	// TODO fix this. Find way to remove all subsearches for ss->searchId from ptr_multimap sub_searches
	boost::shared_ptr<sub_search>	subsrch = sub_searches.find(ss->searchId)->second;
	assert(ss == subsrch.get());
	sub_searches.erase(ss->searchId);

	// each round has unique search id
	ss->searchId = next_sub_search_id++;
	sub_searches[ss->searchId] = subsrch;

	assert(ss == subsrch.get());

	subsrch->hops++;
}


void	search::sub_search_finished(boost::shared_ptr<sub_search>	&ss)
{
	boost::unique_lock<boost::mutex>	__lock(sub_searches_lock);

}

void	handle_search(const	search_message&	sm)
{
	search_result_message	srm;
	if(sm.type == KEY_SEARCH)
	{
		srm.file_list = DHT::get()->query(sm.key);
		if(true){
			using namespace std;
			std::list<file_info_record>::iterator	itr = srm.file_list.begin();
			while(itr != srm.file_list.end())
			{
				cout << itr->path << " ";
				itr++;
			}
			cout << endl;
		}
	}

	{
		srm.node_list = get_better_nodes(sm.key);

		srm.node_list.remove(DHT::get()->getNode());
		srm.node_list.remove(sm.senderNode);

		if(true){

			using namespace std;
			std::list<node_record>::iterator	itr = srm.node_list.begin();
			while(itr != srm.node_list.end())
			{
				cout << itr->ip << ":" << itr->port << ", ";

				itr++;
			}
			cout << endl;
		}
	}

	srm.senderNode = DHT::get()->getNode();
	srm.searchId = sm.searchId;
	srm.subSearchId = sm.subSearchId;

	std::string	msg = serialize_message<search_result_message>(SEARCH_RESULT_MESSAGE, srm);
	control_protocol::get()->udp_send(msg, sm.senderNode);

	std::cout << "found locally " << (sm.type == KEY_SEARCH ? "KEY" : "NODE") << " search message ->results = " << srm.file_list.size() << " and " << srm.node_list.size() << " suggestion(s)" << std::endl;
}



void	handle_search_result(const	search_result_message&	srm)
{
	// find the search process, and register response
	{
		boost::unique_lock<boost::mutex>	__lock(searches_lock);

		boost::unordered_map<uint32_t, boost::shared_ptr<search> >::iterator	iter = searches.find(srm.searchId);
		if(iter != searches.end())
		{
			boost::shared_ptr<search>	s = iter->second;

			// TODO acquire sub_searches_lock

			boost::unordered_map<uint32_t, boost::shared_ptr<sub_search> >::iterator	itr = s->sub_searches.find(srm.subSearchId);
			if(itr != iter->second->sub_searches.end())
			{
				boost::shared_ptr<sub_search>	sr = itr->second;
				sr->add_response(srm);

				/*
				std::list<node_record>::const_iterator	node_list_iterator = srm.node_list.begin();
				while(node_list_iterator != srm.node_list.end())
				{
					sr->next_nodes_queue.push_back(std::make_pair<uint32_t, node_record>(bit_distance(node_list_iterator->nodeId, sr->key), *node_list_iterator));
				}
				sr->next_nodes_queue.unique();
				sr->next_nodes_queue.sort();
				std::list<std::pair<uint32_t, node_record> >::iterator	next_node_queue_iter = sr->next_nodes_queue.begin();
				std::advance(next_node_queue_iter, alpha);
				sr->next_nodes_queue.erase(next_node_queue_iter, sr->next_nodes_queue.end());
				 */
			}
		}
	}

	// update
	{

	}

	std::cout << "got search results = " << srm.file_list.size() << " and " << srm.node_list.size() << "suggestion" << std::endl;
}



uint32_t	start_search(hash	h, search_type	type)
{

	boost::shared_ptr<search>	s(new search());
	s->fullQuery = "";
	s->masterSearchId = next_master_search_id++;
	s->percent_completed = 0;
	s->type = type;

	sub_search	*ss = new sub_search(std::string(), h, s);
	s->sub_searches[ss->searchId] = boost::shared_ptr<sub_search>(ss);

	{
		boost::unique_lock<boost::mutex>	__lock(searches_lock);
		searches[s->masterSearchId] = s;
	}

	ss->start();
	return s->masterSearchId;
}

uint32_t	start_search(std::string	query)
{


	boost::shared_ptr<search>	s(new search());
	s->fullQuery = query;
	s->masterSearchId = next_master_search_id++;
	s->percent_completed = 0;
	s->type = KEY_SEARCH;

	boost::char_separator<char> q_sep("`~!@#$%^&*()-_=+\\|[{]};:'\",<.>/? \t\n");
	boost::tokenizer<boost::char_separator<char> >	_tokens(query, q_sep);
	boost::tokenizer<boost::char_separator<char> >::iterator tok_iter = _tokens.begin();

	while(tok_iter != _tokens.end())
	{
		std::string	tok = *tok_iter;
		boost::algorithm::trim(tok);

		if(tok.length())
		{
			hash	h;
			sub_search	*ss = new sub_search(tok, h, s);
			s->sub_searches[ss->searchId] = boost::shared_ptr<sub_search>(ss);
			ss->start();
		}

		tok_iter++;
	}

	{
		boost::unique_lock<boost::mutex>	__lock(searches_lock);
		searches[s->masterSearchId] = s;
	}


	return s->masterSearchId;
}

boost::shared_ptr<search>	find_search(uint32_t	masterSearchId)
{
	boost::unique_lock<boost::mutex>	__lock(searches_lock);

	boost::unordered_map<uint32_t, boost::shared_ptr<search> >::iterator	iter = searches.find(masterSearchId);
	if(iter != searches.end())
	{
		return	iter->second;
	}
	return boost::shared_ptr<search>();
}


bool	remove_search(uint32_t	masterSearchId)
{
	boost::unique_lock<boost::mutex>	__lock(searches_lock);
	return (searches.erase(masterSearchId) > 0);
}
