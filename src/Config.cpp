/*
 * Config.cpp
 *
 *  Created on: May 25, 2009
 *      Author: prashant
 */

#include "Config.hpp"
#include <fstream>
#include <string>

#include <boost/tokenizer.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <boost/foreach.hpp>
#include <boost/lexical_cast.hpp>

#include "dht/NodeQueue.hpp"
#include "dht/DHT.hpp"
#include "dht/protocol/control_protocol.hpp"


std::string										configFile;
boost::unordered_map<std::string, std::string>	props;
boost::unordered_set<node_record>				friend_nodes;

bool	load_config(std::string	file)
{
	// TODO check for exceptions
	configFile = file;
	std::ifstream	f(configFile.c_str(), std::ios::in | std::ios::binary);
	char	buff[512];
	std::string	confString;
	while(f.read(buff, sizeof(buff)).gcount() > 0)
	{
		buff[f.gcount()] = 0;
		confString += buff;
	}

	typedef boost::tokenizer<boost::char_separator<char> >	tokenizer;
	boost::char_separator<char> line_sep("\n");
	tokenizer line_tokens(confString, line_sep);
	for (tokenizer::iterator tok_iter = line_tokens.begin();	tok_iter != line_tokens.end(); ++tok_iter)
	{
		std::string	line = *tok_iter;
		boost::algorithm::trim(line);

		if(line.c_str()[0] == '#')
			continue;

		size_t	splitPos = line.find_first_of('=', 0);
		if(splitPos > 0)
		{
			std::string	param = std::string(line.c_str(), splitPos);
			boost::algorithm::trim(param);
			std::string	value = std::string(line.c_str() + splitPos + 1, line.length() - splitPos - 1);
			boost::algorithm::trim(value);

			props[param] = value;
		}
	}
	f.close();
	return (props.size() > 0);
}

std::string	get_param(std::string	key)
{
	std::string	val;
	try
	{
		val = props[key];
	}
	catch(std::out_of_range	e)
	{
		val = "";
	}
	return	val;
}

bool	save_config()
{
	std::ofstream	f(configFile.c_str(), std::ios::out);

	boost::unordered_map<std::string, std::string>::const_iterator	itr = props.begin();
	while(itr != props.end())
	{
		f << itr->first << "=" << itr->second << std::endl;
		itr++;
	}
	f.close();
	return true;
}


bool	load_friends_list()
{
	// TODO check for exceptions
	std::string	file = get_param("dht.friendlist");
	if(!file.length())
		return false;

	std::ifstream	f(file.c_str(), std::ios::in | std::ios::binary);
	char	buff[512];
	std::string	confString;
	while(f.read(buff, sizeof(buff)).gcount() > 0)
	{
		buff[f.gcount()] = 0;
		confString += buff;
	}

	typedef boost::tokenizer<boost::char_separator<char> >	tokenizer;
	boost::char_separator<char> line_sep("\n");
	tokenizer line_tokens(confString, line_sep);
	for (tokenizer::iterator tok_iter = line_tokens.begin();	tok_iter != line_tokens.end(); ++tok_iter)
	{
		std::string	line = *tok_iter;
		boost::algorithm::trim(line);

		if(line.c_str()[0] == '#')
			continue;

		std::vector<std::string>	v;
		boost::algorithm::split(v, line, boost::algorithm::is_any_of(" \t"));

		node_record	n;
		n.nodeId.randomize();	// this makes little/no difference, compared to saving old nodeIds and restoring
		n.ip = v[0];
		n.port = boost::lexical_cast<uint16_t>(v[1]);

		mark_seen(n);
	}

	f.close();
	return true;
}
