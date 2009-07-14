AC_INIT([spatter], [0.1], [prashantb@cse.iitb.ac.in])
AM_INIT_AUTOMAKE([-Wall -Werror foreign])
AC_PROG_CXX
AC_PROG_CC

# check libs

#crypto
#AC_CHECK_LIB([crypto], [EVP_DigestFinal], [], AC_MSG_ERROR([crypto library is needed to build. Install openssl-devel.]))

#boost
AC_CHECK_LIB([boost_thread-mt], [_ZN5boost16thread_exceptionC2Ev], [], AC_MSG_ERROR([boost_thread-mt library is needed to build.]))

AC_CHECK_LIB([boost_system-mt], [_ZN5boost6system20get_generic_categoryEv], [], AC_MSG_ERROR([boost_system-mt library is needed to build.]))

AC_CHECK_LIB([boost_serialization-mt], [_ZN5boost7archive6detail14basic_iarchive25register_basic_serializerERKNS1_17basic_iserializerE], [], AC_MSG_ERROR([boost_serialization-mt library is needed to build.]))

AC_CHECK_LIB([readline], [_rl_init_line_state], [], AC_MSG_ERROR([readline library is needed to build.]))

# check header missing.
AC_LANG_PUSH([C++])

AC_CHECK_HEADER([boost/algorithm/string/classification.hpp], [], AC_MSG_ERROR([boost/algorithm/string/classification.hpp header missing.], [1]))
AC_CHECK_HEADER([boost/algorithm/string/split.hpp], [], AC_MSG_ERROR([boost/algorithm/string/split.hpp header missing.], [1]))
AC_CHECK_HEADER([boost/algorithm/string/trim.hpp], [], AC_MSG_ERROR([boost/algorithm/string/trim.hpp header missing.], [1]))
AC_CHECK_HEADER([boost/archive/text_iarchive.hpp], [], AC_MSG_ERROR([boost/archive/text_iarchive.hpp header missing.], [1]))
AC_CHECK_HEADER([boost/archive/text_oarchive.hpp], [], AC_MSG_ERROR([boost/archive/text_oarchive.hpp header missing.], [1]))
AC_CHECK_HEADER([boost/array.hpp], [], AC_MSG_ERROR([boost/array.hpp header missing.], [1]))
AC_CHECK_HEADER([boost/asio.hpp], [], AC_MSG_ERROR([boost/asio.hpp header missing.], [1]))
AC_CHECK_HEADER([boost/asio.hpp], [], AC_MSG_ERROR([boost/asio.hpp header missing.], [1]))
AC_CHECK_HEADER([boost/bind.hpp], [], AC_MSG_ERROR([boost/bind.hpp header missing.], [1]))
AC_CHECK_HEADER([boost/bind.hpp], [], AC_MSG_ERROR([boost/bind.hpp header missing.], [1]))
AC_CHECK_HEADER([boost/functional/hash.hpp], [], AC_MSG_ERROR([boost/functional/hash.hpp header missing.], [1]))
AC_CHECK_HEADER([boost/interprocess/sync/scoped_lock.hpp], [], AC_MSG_ERROR([boost/interprocess/sync/scoped_lock.hpp header missing.], [1]))
AC_CHECK_HEADER([boost/interprocess/sync/scoped_lock.hpp], [], AC_MSG_ERROR([boost/interprocess/sync/scoped_lock.hpp header missing.], [1]))
AC_CHECK_HEADER([boost/lexical_cast.hpp], [], AC_MSG_ERROR([boost/lexical_cast.hpp header missing.], [1]))
AC_CHECK_HEADER([boost/ptr_container/ptr_container.hpp], [], AC_MSG_ERROR([boost/ptr_container/ptr_container.hpp header missing.], [1]))
AC_CHECK_HEADER([boost/serialization/list.hpp], [], AC_MSG_ERROR([boost/serialization/list.hpp header missing.], [1]))
AC_CHECK_HEADER([boost/shared_ptr.hpp], [], AC_MSG_ERROR([boost/shared_ptr.hpp header missing.], [1]))
AC_CHECK_HEADER([boost/thread.hpp], [], AC_MSG_ERROR([boost/thread.hpp header missing.], [1]))
AC_CHECK_HEADER([boost/thread/mutex.hpp], [], AC_MSG_ERROR([boost/thread/mutex.hpp header missing.], [1]))
AC_CHECK_HEADER([boost/tokenizer.hpp], [], AC_MSG_ERROR([boost/tokenizer.hpp header missing.], [1]))
AC_CHECK_HEADER([boost/tokenizer.hpp], [], AC_MSG_ERROR([boost/tokenizer.hpp header missing.], [1]))
AC_CHECK_HEADER([boost/unordered_map.hpp], [], AC_MSG_ERROR([boost/unordered_map.hpp header missing.], [1]))
AC_CHECK_HEADER([boost/unordered_set.hpp], [], AC_MSG_ERROR([boost/unordered_set.hpp header missing.], [1]))
AC_CHECK_HEADER([cstring], [], AC_MSG_ERROR([cstring header missing.], [1]))
AC_CHECK_HEADER([list], [], AC_MSG_ERROR([list header missing.], [1]))
AC_CHECK_HEADER([sstream], [], AC_MSG_ERROR([sstream header missing.], [1]))
AC_CHECK_HEADER([string], [], AC_MSG_ERROR([string header missing.], [1]))

AC_LANG_POP

#AC_CHECK_HEADER([openssl/crypto.h], [], AC_MSG_ERROR([openssl/crypto.h header missing.], [1]))
#AC_CHECK_HEADER([openssl/evp.h], [], AC_MSG_ERROR([openssl/evp.h header missing.], [1]))
#AC_CHECK_HEADER([openssl/sha.h], [], AC_MSG_ERROR([openssl/sha.h header missing.], [1]))
AC_CHECK_HEADER([sys/types.h], [], AC_MSG_ERROR([sys/types.h header missing.], [1]))
AC_CHECK_HEADER([readline/readline.h], [], AC_MSG_ERROR([readline.h header missing.], [1]))

AC_CONFIG_HEADERS([config.h])
AC_CONFIG_FILES([Makefile src/Makefile])
AC_OUTPUT

