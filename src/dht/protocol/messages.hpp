/*
 * messages.hpp
 *
 *  Created on: May 24, 2009
 *      Author: prashant
 */

#ifndef MESSAGES_HPP_
#define MESSAGES_HPP_

#include <sys/types.h>
#include <boost/serialization/list.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>

#include <boost/unordered_map.hpp>

#include <cstring>

#define		PUBLISH_MESSAGE			((uint8_t)	1)
#define		SEARCH_MESSAGE			((uint8_t)	2)
#define		SEARCH_RESULT_MESSAGE	((uint8_t)	3)
#define		PING_MESSAGE			((uint8_t)	4)

#define HASH_BYTES	(24)

typedef struct __hash
{
	uint8_t		hash[24];

	__hash()
	{
		memset(hash, 0, HASH_BYTES);
	}

	void randomize()
	{
		for(unsigned int	i = 0; i < HASH_BYTES; i++)
		{
			hash[i] = rand() % 256;
		}
	}

	template<class Archive>
	void serialize(Archive & ar, const unsigned int version)
	{
		ar & hash;
	}

	bool	operator==(const __hash&	h2) const
	{
		return (memcmp(hash, h2.hash, HASH_BYTES) == 0);
	}

	bool	operator<(const __hash&	h2) const
	{
		return (memcmp(hash, h2.hash, HASH_BYTES) < 0);
	}

}hash;

std::size_t hash_value(const hash& h);

typedef struct __node_record
{
	hash		nodeId;
	std::string	ip;
	uint16_t	port;

	__node_record()
	{
		ip = "invalid-ip-address";
		port = 0;
	}

	template<class Archive>
	void serialize(Archive & ar, const unsigned int version)
	{
		ar & nodeId;
		ar & ip;
		ar & port;
	}

	bool	operator==(const __node_record&	n) const
	{
		return ( /* (nodeId == n.nodeId)		// TODO FIXME maybe should be removed after testing phase is over, we do not want to support multiple instances per machine
				&& */ (ip == n.ip)
				&&(port == n.port));
	}

	bool	operator<(const __node_record&	n) const
	{
		return ( /* (nodeId < n.nodeId)		// TODO FIXME maybe should be removed after testing phase is over, we do not want to support multiple instances per machine
				&& */ (ip < n.ip)
				&&(port < n.port));
	}

}node_record;

enum file_type
{
	file,
	dir
	// and anything special like the (planned) hosts file (one that will be circulated periodically, and will contain all hosts and their stats)
};

typedef struct __file_info_record
{
	hash		key;
	node_record	node;

	std::string	path;
	uint64_t	size;
	file_type	type;
	uint64_t	modTS;

	uint32_t	owner_publish_ts;
	uint32_t	valid_till;	// time to live, if this record is found in local table after it has expired, it will be deleted.

	// maybe add more file info like perceived popularity/number of downloads, community rating (needs a separate message type) etc.

	__file_info_record()
	{
		path = "invalid-path";
		size = 0;
		type = file;
		modTS = 0;
	}

	template<class Archive>
	void serialize(Archive & ar, const unsigned int version)
	{
		ar & key;
		ar & node;

		ar & path;
		ar & size;
		ar & type;
		ar & modTS;

		ar & owner_publish_ts;
		ar & valid_till;
	}

	bool	operator==(const __file_info_record&	f) const
	{
		return (  (key == f.key)
				&&(node == f.node)
				&&(path == f.path)
				&&(size == f.size)
				&&(type == f.type)
				&&(modTS == f.modTS)
				&&(owner_publish_ts == f.owner_publish_ts)		/* same record when sent again, should not match the older one. On similar lines, the client should choose latest one over stale ones. */
				);
	}

}file_info_record;

struct publish_message
{
	file_info_record	file_info;

	template<class Archive>
	void serialize(Archive & ar, const unsigned int version)
	{
		ar & file_info;
	}
};

enum search_type
{
	NODE_SEARCH,
	KEY_SEARCH
};

struct search_message
{
	hash			key;
	node_record		senderNode;
	uint32_t		searchId;
	uint32_t		subSearchId;
	search_type		type;

	// maybe some more constraints to filter out data being sent back
	// constraints on file type, size, date, junkness-rating etc

	template<class Archive>
	void serialize(Archive & ar, const unsigned int version)
	{
		ar & key;
		ar & senderNode;
		ar & searchId;
		ar & subSearchId;
		ar & type;
	}

};

struct search_result_message
{
	node_record						senderNode;
	std::list<file_info_record>		file_list;
	std::list<node_record>			node_list;
	uint32_t		searchId;
	uint32_t		subSearchId;


	template<class Archive>
	void serialize(Archive & ar, const unsigned int version)
	{
		ar & senderNode;
		ar & file_list;
		ar & node_list;
		ar & searchId;
		ar & subSearchId;
	}
};


struct ping_message
{
	node_record		senderNode;
	node_record		receiverNode;
	uint8_t			isReply;
	uint32_t		messageId;

	ping_message()
	{
		isReply = false;
		messageId = 0;
	}

	template<class Archive>
	void serialize(Archive & ar, const unsigned int version)
	{
		ar & senderNode;
		ar & receiverNode;
		ar & isReply;
		ar & messageId;
	}

	bool	operator==(const ping_message&	p) const
	{
		return (  (messageId == p.messageId)
				&&
					(
						((senderNode == p.receiverNode)&&(receiverNode == p.senderNode)&&(isReply == !p.isReply))
						||
						((senderNode == p.senderNode)&&(receiverNode == p.receiverNode)&&(isReply == p.isReply))
					)
				);
	}

};

std::size_t hash_value(const ping_message& p);


// update these as we add/remove fields from message structures
BOOST_CLASS_VERSION(publish_message, 2)
BOOST_CLASS_VERSION(search_message, 1)
BOOST_CLASS_VERSION(search_result_message, 1)

#endif /* MESSAGES_HPP_ */
