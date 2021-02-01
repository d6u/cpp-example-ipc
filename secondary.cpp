#include "doc_anonymous_condition_shared_data.hpp"
#include <boost/interprocess/mapped_region.hpp>
#include <boost/interprocess/shared_memory_object.hpp>
#include <boost/interprocess/sync/scoped_lock.hpp>
#include <cstring>
#include <iostream>

using namespace boost::interprocess;

int main() {
  // Create a shared memory object.
  shared_memory_object shm(open_only,        // only create
                           "MySharedMemory", // name
                           read_write        // read-write mode
  );

  try {
    // Map the whole shared memory in this process
    mapped_region region(shm,       // What to map
                         read_write // Map it as read-write
    );

    // Get the address of the mapped region
    void *addr = region.get_address();

    // Obtain a pointer to the shared structure
    trace_queue *data = static_cast<trace_queue *>(addr);

    // Print messages until the other process marks the end
    bool end_loop = false;
    do {
      scoped_lock<interprocess_mutex> lock(data->mutex);
      if (!data->message_in) {
        data->cond_empty.wait(lock);
      }
      if (std::strcmp(data->items, "last message") == 0) {
        end_loop = true;
      } else {
        // Print the message
        std::cout << data->items << std::endl;
        // Notify the other process that the buffer is empty
        data->message_in = false;
        data->cond_full.notify_one();
      }
    } while (!end_loop);
  } catch (interprocess_exception &ex) {
    std::cout << ex.what() << std::endl;
    return 1;
  }

  return 0;
}