#include <boost/interprocess/sync/interprocess_condition.hpp>
#include <boost/interprocess/sync/interprocess_mutex.hpp>

struct client_queue {
  client_queue() {}

  boost::interprocess::interprocess_mutex mutex;
  boost::interprocess::interprocess_condition cond_wait_server_response;
  boost::interprocess::interprocess_condition cond_wait_client_connection;
};