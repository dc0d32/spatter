/*
 * NodeQueue.cpp
 *
 *  Created on: May 25, 2009
 *      Author: prashant
 */

#include "NodeQueue.hpp"
#include "../util/util.hpp"
#include "DHT.hpp"
#include "protocol/control_protocol.hpp"

#include <boost/thread.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/interprocess/sync/scoped_lock.hpp>
#include <boost/unordered_set.hpp>

/**
 * essentilaly (sizeof(hash::hash) * CHAR_BIT) queues. i'th queue holds nodes distance=i apart, sorted (and eventually evicted) by last communication time
 */
std::list<node_record>	buckets[HASH_BYTES * CHAR_BIT];
boost::mutex	bucket_locks[HASH_BYTES * CHAR_BIT];

// TODO read from config
static unsigned int	bucketSize = 32;
extern unsigned int	alpha;
static uint32_t	next_message_id = 0;

void	mark_seen(const node_record	&node)
{
	assert(node.port);
	size_t	d = bit_distance(DHT::get()->getNode().nodeId, node.nodeId);

	// will be deleted automatically
	boost::unique_lock<boost::mutex>	lock(bucket_locks[d]);

	std::list<node_record>::iterator	itr = std::find(buckets[d].begin(), buckets[d].end(), node);
	if(itr != buckets[d].end())
	{
		buckets[d].erase(itr);
	}

	buckets[d].push_front(node);

	if(buckets[d].size() >= bucketSize)
	{
		node_record	node = buckets[d].back();
		buckets[d].pop_back();
		// ping the node being evicted. helps counter node flooding DoS attacks
		ping_message	ping;
		ping.isReply = false;
		ping.messageId = next_message_id++;
		ping.receiverNode = node;
		ping.senderNode = DHT::get()->getNode();

		control_protocol::get()->ping(ping);
	}
}

std::list<node_record>	get_better_nodes(const	hash&	h, bool	allow_non_euclidian)
{
	typedef std::list<node_record>::iterator	node_list_iterator;
	typedef std::list<std::pair<size_t, node_record> >::iterator	pq_iterator;

	size_t	max_dist_allowed = bit_distance(DHT::get()->getNode().nodeId, h);

	std::list<std::pair<size_t, node_record> >	pq;

	for(unsigned int	i = 0; i < (HASH_BYTES * CHAR_BIT); i++)
	{
		// frees up automatically
		boost::unique_lock<boost::mutex>	lock(bucket_locks[i]);

		node_list_iterator	iter = buckets[i].begin();
		while(iter != buckets[i].end())
		{
			size_t	d = bit_distance(iter->nodeId, h);
			// std::cout << "max_dist_allowed=" << max_dist_allowed << ", d=" << d << ", ip=" << iter->ip << std::endl;
			if(d < max_dist_allowed || allow_non_euclidian){	// should not be <=, as it makes the search non-convergent
				pq_iterator	itr = pq.begin();
				if(!pq.size())
					pq.insert(itr, std::make_pair<size_t, node_record>(d, *iter));
				else
				{
					std::pair<size_t, node_record>	pq_entry = std::make_pair<size_t, node_record>(d, *iter);
					while(itr != pq.end())
					{
						// yes, '<='
						if(d <= itr->first)
						{
							pq.insert(itr, pq_entry);
							if(pq.size() >= alpha)
							{
								pq.pop_back();
							}
							break;
						}
						itr++;
					}
				}
			}

			iter++;
		}

	}

	std::list<node_record>	l;
	pq_iterator	itr = pq.begin();
	while(itr != pq.end())
	{
		l.push_back(itr->second);
		itr++;
	}

	return l;
}


void	ping_all()
{
	for(unsigned int	i = 0; i < (HASH_BYTES * CHAR_BIT); i++)
	{
		// frees up automatically
		boost::unique_lock<boost::mutex>	lock(bucket_locks[i]);
		typedef std::list<node_record>::iterator	node_list_iterator;
		node_list_iterator	iter = buckets[i].begin();
		while(iter != buckets[i].end())
		{
			ping_message	ping;
			ping.isReply = false;
			ping.messageId = next_message_id++;
			ping.receiverNode = *iter;
			ping.senderNode = DHT::get()->getNode();
			control_protocol::get()->ping(ping);

			iter++;
		}

	}

}
