/*
 * control_protocol.h
 *
 *  Created on: May 25, 2009
 *      Author: prashant
 */

#ifndef CONTROL_PROTOCOL_H_
#define CONTROL_PROTOCOL_H_

#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/array.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/unordered_set.hpp>

#include "messages.hpp"
#include "../DHT.hpp"
#include "../NodeQueue.hpp"
#include "../../Config.hpp"
#include "../../util/util.hpp"

using boost::asio::ip::udp;


class control_protocol
{
public:
	static control_protocol	*create(boost::asio::io_service& io_service, short port);
	static control_protocol	*get();

	void	handle_receive_from(const boost::system::error_code& error, size_t bytes_recvd);

	void	udp_send(const	std::string&	msg, node_record	to);
	void	ping(ping_message&	m);

private:
	control_protocol(boost::asio::io_service& io_service, short port);

	static control_protocol	*singleton_object;
	boost::asio::io_service& io_service_;
	udp::socket socket_;
	udp::endpoint sender_endpoint_;
	enum {max_data = 262144};	// 256K
	uint8_t	buffer_[max_data];

	boost::unordered_set<std::pair<uint32_t, ping_message> >	unacknowledged_pings;

	void handle_message(uint8_t	*data, size_t	len);
};

#endif /* CONTROL_PROTOCOL_H_ */
