#include "doc_anonymous_condition_shared_data.hpp"

#include <boost/interprocess/ipc/message_queue.hpp>
#include <boost/interprocess/mapped_region.hpp>
#include <boost/interprocess/shared_memory_object.hpp>
#include <boost/interprocess/sync/scoped_lock.hpp>
#include <cstring>
#include <iostream>

using namespace boost::interprocess;

const char *kSharedMemoryName = "MySharedMemory";
const char *kMessageQueueName = "message_queue";

int main() {
  // Create a shared memory object.
  shared_memory_object shm(open_only, kSharedMemoryName, read_write);

  // Open a message queue.
  message_queue mq(open_only, kMessageQueueName);

  try {
    // Map the whole shared memory in this process
    mapped_region region(shm,       // What to map
                         read_write // Map it as read-write
    );

    // Get the address of the mapped region
    void *addr = region.get_address();

    // Obtain a pointer to the shared structure
    client_queue *data = static_cast<client_queue *>(addr);

    data->cond_wait_client_connection.notify_one();

    {
      scoped_lock<interprocess_mutex> lock(data->mutex);

      data->cond_wait_server_response.wait(lock);

      unsigned int priority;
      message_queue::size_type recvd_size;

      // Receive 10 numbers
      for (int i = 0; i < 10; ++i) {
        int number;
        mq.receive(&number, sizeof(number), recvd_size, priority);
        if (number != i || recvd_size != sizeof(number)) {
          return 1;
        }
        std::cout << "Receiving message: " << number << std::endl;
      }
    }
  } catch (interprocess_exception &ex) {
    std::cout << ex.what() << std::endl;
    return 1;
  }

  return 0;
}