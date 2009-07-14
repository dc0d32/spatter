/*
 * console_ui.cpp
 *
 *  Created on: May 27, 2009
 *      Author: prashant
 */

#include "console_ui.hpp"
#include <string>
#include <iostream>

#include <boost/interprocess/sync/scoped_lock.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <boost/tokenizer.hpp>
#include <boost/lexical_cast.hpp>

#include "../dht/protocol/search_manager.hpp"
#include "../dht/protocol/control_protocol.hpp"
#include "../util/util.hpp"

#include <readline/readline.h>
#include <readline/history.h>

bool	console()
{
	bool	quit = false;
	while(!quit)
	{
		std::string	command;
		{
			char* cmd = readline("--spatter--$ ");
			if (cmd && *cmd)
			{
				command = cmd;
				add_history (cmd);
				free(cmd);
			}
		}
		//		std::getline(std::cin, command);

		if(!command.length())
			continue;

		boost::char_separator<char> cmd_sep(" \t");
		boost::tokenizer<boost::char_separator<char> >	line_tokens(command, cmd_sep);
		boost::tokenizer<boost::char_separator<char> >::iterator tok_iter = line_tokens.begin();

		std::string	tok = *tok_iter;
		if(tok == "share")
		{
			// publish

			// TODO FIXME obviously this should be started up as a job in job queue for a pool of threads that
			// do the right node search and send publish message
			tok_iter++;
			std::string	path = *tok_iter;
			boost::char_separator<char> q_sep("`~!@#$%^&*()-_=+\\|[{]};:'\",<.>/? \t\n");
			boost::tokenizer<boost::char_separator<char> >	_tokens(path, q_sep);
			boost::tokenizer<boost::char_separator<char> >::iterator path_tok_iter = _tokens.begin();


			while(path_tok_iter != _tokens.end())
			{
				std::string	tok = *path_tok_iter;
				if(tok.length())
				{
					hash	h;
					calculate_hash(tok, h.hash);
					uint32_t	searchId = start_search(h, NODE_SEARCH);

					bool	done = false;

					while(!done)
					{
						bool	queues_empty = true;
						boost::shared_ptr<search>	s = find_search(searchId);
						if(s)
						{
							{
								boost::unique_lock<boost::mutex>	__lock(s->sub_searches_lock);
								boost::unordered_map<uint32_t, boost::shared_ptr<sub_search> >::iterator	itr = s->sub_searches.begin();
								while(itr != s->sub_searches.end())
								{
									boost::shared_ptr<sub_search>	ss = itr->second;
									std::cout << "status : q=" << ss->queryToken << ", hops=" << ss->hops << ", results=" << ss->results[ss->key].size() << ", suggestions=" << ss->next_nodes_queue.size() << std::endl;

									queues_empty = queues_empty && (ss->next_nodes_queue.size() == 0);
									done = done && ss->hops >= max_hops;

									itr++;
								}
							}

							if(done || queues_empty)
							{
								std::cout << "publishing..." << std::endl;
								// TODO FIXME fill in proper file info, and add file to shared files' list
								publish_message	pm;
								pm.file_info.path = path;
								pm.file_info.size = 4;
								calculate_hash(tok/*pm.file_info.path*/, pm.file_info.key.hash);
								pm.file_info.node = DHT::get()->getNode();

								boost::unordered_map<uint32_t, boost::shared_ptr<sub_search> >::iterator	itr = s->sub_searches.begin();
								while(itr != s->sub_searches.end())
								{
									boost::shared_ptr<sub_search>	ss = itr->second;
									ss->pq.sort();

									int	redundancy = publish_redundancy;

									std::list<std::pair<uint32_t, node_record> >::iterator	targets_iter = ss->pq.begin();
									while(targets_iter != ss->pq.end() && redundancy--)
									{
										control_protocol::get()->udp_send(serialize_message<publish_message>(PUBLISH_MESSAGE, pm), targets_iter->second);
										targets_iter++;
									}

									itr++;
								}

								DHT::get()->insertFileInfoRecord(pm.file_info);

								done = true;
							}

							sleep(1);

						}
						else
						{
							break;
						}
					}

				}

				path_tok_iter++;
			}



		}
		else if(tok == "search")
		{
			// start off a search, return search id
			uint32_t	searchId = start_search(std::string(command.c_str() + strlen("search")));
			std::cout << searchId;
		}
		else if(tok == "search_result")
		{
			// print stats and results so far from search given by search id
			uint32_t	searchId = boost::lexical_cast<uint32_t>(*(++tok_iter));

			boost::shared_ptr<search>	s = find_search(searchId);
			if(s)
			{
				boost::unique_lock<boost::mutex>	__lock(s->sub_searches_lock);
				boost::unordered_map<uint32_t, boost::shared_ptr<sub_search> >::iterator	itr = s->sub_searches.begin();
				while(itr != s->sub_searches.end())
				{
					boost::shared_ptr<sub_search>	ss = itr->second;
					std::cout << "status : q=" << ss->queryToken << ", hops=" << ss->hops << ", results=" << ss->results[ss->key].size() << std::endl;
					itr++;
				}

			}
		}
		else if(tok == "search_kill")
		{
			// print stats and results so far from search given by search id
			uint32_t	searchId = boost::lexical_cast<uint32_t>(*(++tok_iter));
			remove_search(searchId);
		}
		else if(tok == "quit")
		{
			quit = true;
		}
	}
	return quit;
}
