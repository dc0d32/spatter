/*
 * control_protocol.cpp
 *
 *  Created on: May 25, 2009
 *      Author: prashant
 */

#include "control_protocol.hpp"
#include "search_manager.hpp"

static unsigned int	ping_timeout = 60;	//ping timeout

control_protocol*	control_protocol::singleton_object = NULL;

control_protocol*	control_protocol::create(boost::asio::io_service& io_service, short port)
{
	if(!singleton_object)
	{
		singleton_object = new control_protocol(io_service, port);
	}
	return singleton_object;
}


control_protocol*	control_protocol::get()
{
	return singleton_object;
}


control_protocol::control_protocol(boost::asio::io_service& io_service, short port)
: io_service_(io_service),
socket_(io_service, udp::endpoint(udp::v4(), port))
{
	socket_.async_receive_from(
			boost::asio::buffer(buffer_, max_data), sender_endpoint_,
			boost::bind(&control_protocol::handle_receive_from, this,
					boost::asio::placeholders::error,
					boost::asio::placeholders::bytes_transferred));
}

void control_protocol::handle_receive_from(const boost::system::error_code& error,
		size_t bytes_recvd)
{
	uint8_t	localBuffer[max_data];
	if (!error && bytes_recvd > 0)
	{
		// FIXME (maybe wrongly) assumes that entire UDP message has been read.
		// should probably have different data_ per thread
		memcpy(localBuffer, buffer_, max_data);
	}

	socket_.async_receive_from(
			boost::asio::buffer(buffer_, max_data), sender_endpoint_,
			boost::bind(&control_protocol::handle_receive_from, this,
					boost::asio::placeholders::error,
					boost::asio::placeholders::bytes_transferred));

	if (error || bytes_recvd <= 0){
		return;
	}

	// process the data...
	// ...

	if(0)
	{
		using namespace std;	// cout, endl
		cout << " Got : " << bytes_recvd << "bytes" << endl << localBuffer << endl;
	}

	handle_message(localBuffer, bytes_recvd);

}

void	control_protocol::udp_send(const	std::string&	msg, node_record	to)
{
	if(!(to == DHT::get()->getNode()))
	{
		// TODO find better ways of sending udp packets, fast
		udp::resolver resolver(io_service_);
		udp::resolver::query query(udp::v4(), to.ip, boost::lexical_cast<std::string>(to.port));
		udp::endpoint receiver_endpoint = *resolver.resolve(query);

		udp::socket socket(io_service_);
		socket.open(udp::v4());

		socket.send_to(boost::asio::buffer(msg), receiver_endpoint);
	}
}


void	control_protocol::ping(ping_message&	m)
{
	if(!(m.receiverNode == DHT::get()->getNode()))
	{
		unacknowledged_pings.insert(std::make_pair<uint32_t, ping_message>(time(NULL) + ping_timeout, m));
		std::string	s = serialize_message<ping_message>(PING_MESSAGE, m);
		udp_send(s, m.receiverNode);
	}
}


void control_protocol::handle_message(uint8_t	*data, size_t	len)
{
	std::string archive_data((char*)&data[1], len - 1);
	std::istringstream archive_stream(archive_data);
	boost::archive::text_iarchive archive(archive_stream);


	using std::cout;
	using std::endl;

	if(data[0] == PUBLISH_MESSAGE)
	{
		publish_message	pm;

		archive >> pm;
		cout << "got publish message : path=" << pm.file_info.path << ", size=" << pm.file_info.size << endl;
		// add it to local hash table
		DHT::get()->insertFileInfoRecord(pm.file_info);

		mark_seen(pm.file_info.node);
	}
	else if(data[0] == SEARCH_MESSAGE)
	{
		search_message	sm;
		archive >> sm;

		handle_search(sm);

		mark_seen(sm.senderNode);
	}
	else if(data[0] == SEARCH_RESULT_MESSAGE)
	{
		search_result_message	srm;
		archive >> srm;

		handle_search_result(srm);

		mark_seen(srm.senderNode);
	}
	else if(data[0] == PING_MESSAGE)
	{
		ping_message	pm;
		archive >> pm;

		std::cout << "PING message ";

		if(pm.isReply)
		{
			std::cout << "REPLY ";
			boost::unordered_set<std::pair<uint32_t, ping_message> >::iterator	itr = unacknowledged_pings.begin();
			while(itr != unacknowledged_pings.end())
			{
				if(itr->second == pm)
				{
					std::cout << ", ACK_received ";
					unacknowledged_pings.erase(itr);
					break;
				}
				itr++;
			}
		}
		else
		{
			std::cout << ", sending ACK ";
			pm.receiverNode = pm.senderNode;
			pm.isReply = true;
			pm.senderNode = DHT::get()->getNode();
			std::string	s = serialize_message<ping_message>(PING_MESSAGE, pm);
			udp_send(s, pm.receiverNode);
		}
		mark_seen(pm.senderNode);
	}

}

