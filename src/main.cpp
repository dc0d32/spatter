/*
 * tindc.cc
 *
 *  Created on: May 24, 2009
 *      Author: prashant
 */

#include <cstdlib>
#include <iostream>
#include <vector>

#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/array.hpp>
#include <boost/lexical_cast.hpp>

#include "dht/protocol/messages.hpp"
#include "dht/protocol/control_protocol.hpp"
#include "dht/DHT.hpp"
#include "dht/NodeQueue.hpp"
#include "Config.hpp"
#include "util/util.hpp"
#include "ui/console_ui.hpp"


using boost::asio::ip::udp;




int main(int argc, char* argv[])
{
	srand(time(NULL));
	init_util();

	load_config("spatter.conf");

	DHT::make_dht(get_param("network.ip"), boost::lexical_cast<uint16_t>(get_param("network.port")));

	load_friends_list();

	// start console
	boost::thread console_ui_thread(&console);

	size_t	thread_pool_size_ = boost::lexical_cast<size_t>(get_param("network.thread_pool_size"));
	// Create a pool of threads to run all of the io_services.
	std::vector<boost::shared_ptr<boost::thread> > threads;

	boost::asio::io_service io_service;

	try
	{
		control_protocol::create(io_service, boost::lexical_cast<uint16_t>(get_param("network.port")));

		for (std::size_t i = 0; i < thread_pool_size_; ++i)
		{
			boost::shared_ptr<boost::thread> thread(new boost::thread(
					boost::bind(&boost::asio::io_service::run, &io_service)));
			threads.push_back(thread);
		}
	}
	catch (std::exception& e)
	{
		std::cerr << "Exception: " << e.what() << "\n";
	}

	// ping everyone in node queues
	ping_all();

	// wait for console to exit (it should be done by now)
	console_ui_thread.join();

	// Wait for all threads in the pool to exit.
	for (std::size_t i = 0; i < threads.size(); ++i)
		threads[i]->interrupt();

	save_config();

	return 0;
}
