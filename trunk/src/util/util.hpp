/*
 * util.hpp
 *
 *  Created on: May 24, 2009
 *      Author: prashant
 */

#ifndef UTIL_H_
#define UTIL_H_

#include <sys/types.h>
#include <string>

#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <sstream>

#include "../dht/protocol/messages.hpp"

uint32_t	ip_str_to_num(const std::string&	s);
std::string	ip_num_to_str(uint32_t	ip);

void	calculate_hash(const std::string&	str, uint8_t	*hash);
void	calculate_hash(std::istream	stream, uint8_t	*hash);

template <typename T>
std::string	serialize_message(uint8_t	message_type, T&	o)
{
    std::ostringstream archive_stream;
    archive_stream << message_type;
    boost::archive::text_oarchive archive(archive_stream);
    archive << o;
    return archive_stream.str();
}


void init_util();
size_t	bit_distance(const hash&	h1, const hash&	h2);

#endif /* UTIL_H_ */
