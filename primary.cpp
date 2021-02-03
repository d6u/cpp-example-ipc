#include "doc_anonymous_condition_shared_data.hpp"

#include <boost/interprocess/ipc/message_queue.hpp>
#include <boost/interprocess/mapped_region.hpp>
#include <boost/interprocess/shared_memory_object.hpp>
#include <boost/interprocess/sync/scoped_lock.hpp>
#include <cstdio>
#include <iostream>

using namespace boost::interprocess;

const char *kSharedMemoryName = "MySharedMemory";
const char *kMessageQueueName = "message_queue";

int main() {
  // Erase previous shared memory and schedule erasure on exit
  struct shm_remove {
    shm_remove() {
      shared_memory_object::remove(kSharedMemoryName);
      message_queue::remove(kMessageQueueName);
    }
    ~shm_remove() {
      shared_memory_object::remove(kSharedMemoryName);
      message_queue::remove(kMessageQueueName);
    }
  } remover;

  // Create a shared memory object.
  shared_memory_object shm(create_only, kSharedMemoryName, read_write);

  // Create a message_queue.
  message_queue mq(create_only, kMessageQueueName,
                   100,        // max message number
                   sizeof(int) // max message size
  );

  try {
    // Set size
    shm.truncate(sizeof(client_queue));

    // Map the whole shared memory in this process
    mapped_region region(shm, read_write);

    // Get the address of the mapped region
    void *addr = region.get_address();

    // Construct the shared structure in memory
    client_queue *data = new (addr) client_queue;

    int count = 0;

    while (count < 2) {
      count++;

      scoped_lock<interprocess_mutex> lock(data->mutex);

      data->cond_wait_client_connection.wait(lock);

      std::cout << "Client " << count << " has connected." << std::endl;

      // Send 10 numbers
      for (int i = 0; i < 10; ++i) {
        mq.send(&i, sizeof(i), 0);
      }

      data->cond_wait_server_response.notify_one();
    }
  } catch (interprocess_exception &ex) {
    std::cout << ex.what() << std::endl;
    return 1;
  }

  return 0;
}