/*
 * util.cpp
 *
 *  Created on: May 24, 2009
 *      Author: prashant
 */

#include "util.hpp"
#include <boost/tokenizer.hpp>

#include <sstream>

#ifdef USE_OPENSSL_SHA
#include <openssl/sha.h>
#include <openssl/crypto.h>
#include <openssl/evp.h>
#endif


#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>

#include "../lib/tigertree/tiger/tiger.h"

static uint8_t	bits_in_16bits [0x1u << 16];

uint32_t	ip_str_to_num(const std::string&	s)
{
	uint32_t	addr = 0;
	typedef boost::tokenizer<boost::char_separator<char> >	tokenizer;
	boost::char_separator<char> sep(".");
	tokenizer tokens(s, sep);
	for (tokenizer::iterator tok_iter = tokens.begin();	tok_iter != tokens.end(); ++tok_iter)
	{
		uint8_t	val = std::atoi(tok_iter->c_str());
		addr = (addr << 8) | val;
	}
	return addr;
}


std::string	ip_num_to_str(uint32_t	ip)
{
	std::ostringstream stream;
	stream << (uint8_t)(ip & 0xFF000000) << '.'; ip <<= 8;
	stream << (uint8_t)(ip & 0xFF000000) << '.'; ip <<= 8;
	stream << (uint8_t)(ip & 0xFF000000) << '.'; ip <<= 8;
	stream << (uint8_t)(ip & 0xFF000000);
	return stream.str();
}

void	calculate_hash(const std::string&	str, uint8_t	*hash)
{
#ifdef USE_OPENSSL_SHA1
	EVP_Digest(str.c_str(), str.length(), hash, NULL, EVP_sha1(), NULL);
#else
	tiger((uint64_t*)str.c_str(), (uint64_t)str.length(), (uint64_t*)hash);
	std::cout << "hash for " << str << " = " << ((uint64_t*)hash)[0] + ((uint64_t*)hash)[1] + ((uint64_t*)hash)[2] << std::endl;
#endif
}

void	calculate_hash(std::istream	stream, uint8_t	*hash)
{
#ifdef USE_OPENSSL_SHA1

	EVP_MD_CTX	*context = EVP_MD_CTX_create();
	EVP_DigestInit(context, EVP_sha1());

#define	HASH_READ_BUFFER_SIZE	(512*1024)
	char	*buffer = new char[HASH_READ_BUFFER_SIZE];
	while(stream.good())
	{
		size_t	bytes_read = stream.readsome(buffer, HASH_READ_BUFFER_SIZE);
		EVP_DigestUpdate(context, buffer, bytes_read);
	}
#undef	HASH_READ_BUFFER_SIZE
	delete[]	buffer;

	EVP_DigestFinal(context, hash, NULL);

#else

#endif
}

void init_util()
{
	bits_in_16bits[0] = 0;
	for(uint32_t	i = 0; i < (0x1u << 16); i++)
	{
/*
 		// buggy - TODO debug, sync with notebook
 		uint32_t	v = i;
		v = v - ((v >> 1) & 0x55555555u);                    // reuse input as temporary
		v = (v & 0x33333333u) + ((v >> 2) & 0x33333333u);     // temp
		bits_in_16bits[i] = ((v + ((v >> 4) & 0xF0F0F0Fu)) * 0x1010101u) >> 24; // count
*/

/*
		// works, slower
		uint32_t	v = i;
		unsigned int	bits = 0;
		for (bits = 0; v; bits++)
		{
			v &= v - 1;
		}
		bits_in_16bits[i] = bits;
*/

		// fastest of all
		bits_in_16bits[i] = (i & 1) + bits_in_16bits[i >> 1];
	}
}

size_t	bit_distance(const hash&	h1, const hash&	h2)
{
	size_t	dist = 0;
	for(unsigned int i = 0; i < (HASH_BYTES / 4); i++)
	{
		uint32_t	a = *(uint32_t*)(h1.hash + (i*4));
		uint32_t	b = *(uint32_t*)(h2.hash + (i*4));
		uint32_t	v = a ^ b;
		if(v == 0)
			continue;

		dist += bits_in_16bits [v & 0xffffu] + bits_in_16bits [(v >> 16) & 0xffffu] ;
	}
	return dist;
}
