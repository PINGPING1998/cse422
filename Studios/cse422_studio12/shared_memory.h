#define SHARED_MEMORY_REGION "/sharedmemory"
#define SIZE_SHARED_ARRAY 10

struct shared_data_t {
    volatile int write_guard;
    volatile int read_guard;
    volatile int delete_guard;
    volatile int data[SIZE_SHARED_ARRAY];
} shared_data;
