/*
 * Config.hpp
 *
 *  Created on: May 25, 2009
 *      Author: prashant
 */

#ifndef CONFIG_HPP_
#define CONFIG_HPP_

#include <string>

#include <boost/unordered_set.hpp>
#include <boost/unordered_map.hpp>

#include "dht/protocol/messages.hpp"

extern std::string							configFile;
extern boost::unordered_map<std::string, std::string>	props;
extern boost::unordered_set<node_record>	friend_nodes;


bool	load_config(std::string	file);
std::string	get_param(std::string	key);

bool	save_config();

bool	load_friends_list();

#endif /* CONFIG_HPP_ */
