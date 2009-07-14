/*
 * NodeQueue.hpp
 *
 *  Created on: May 25, 2009
 *      Author: prashant
 */

#ifndef NODEQUEUE_HPP_
#define NODEQUEUE_HPP_

#include <list>

#include "protocol/messages.hpp"

// void	mark_seen(node_record	node);
void	mark_seen(const node_record	&node);
std::list<node_record>	get_better_nodes(const	hash&	h, bool	allow_non_euclidian = false);

void	ping_all();

#endif /* NODEQUEUE_HPP_ */
