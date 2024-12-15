# Lock-Free Hash Map

## Overview

This repository provides an implementation of a lock-free hash map using atomic operations. The implementation is encapsulated in a shared library: `liblockfreehashmap.so`.

Lock-free data structures ensure thread-safe access without requiring locks, improving performance in highly concurrent environments.

## Features

- **Lock-Free Design**: Ensures high performance and scalability.
- **Thread-Safe**: Multiple threads can safely read/write concurrently.

## Requirements

- A C-compatible compiler (e.g., GCC or Clang).
- POSIX-compliant system (for atomic operations support).

## Building the Library

1. Clone the repository:
   ```bash
   git clone <repository-url>
   cd <repository-name>
   ```

2. Build the library using Make:
   ```bash
   make
   ```

3. After a successful build, the shared library `liblockfreehashmap.so` will be located in the same directory.

## Using the Library

1. Link the library to your project during compilation:
   ```bash
   gcc -o your_program your_program.c -L/path/to/lib -llockfreehashmap -pthread
   ```
   Replace `/path/to/lib` with the directory containing `liblockfreehashmap.so`.

2. Include the header file in your source code:
   ```c
   #include "chashmap.h"
   ```

3. Example usage:
   ```c
   #include "chashmap.h"
   #include <stdio.h>

   int main() {
       HM* hashmap = alloc_hashmap(10);

       if (insert_item(hashmap, 42) == 0) {
           printf("Inserted value 42 into the hashmap.\n");
       }

       if (lookup_item(hashmap, 42) == 0) {
           printf("Value 42 exists in the hashmap.\n");
       }

       if (remove_item(hashmap, 42) == 0) {
           printf("Removed value 42 from the hashmap.\n");
       }

       free_hashmap(hashmap);
       return 0;
   }
   ```

## API Reference

### `HM* alloc_hashmap(size_t n_buckets)`

Allocates a hashmap with the given number of buckets.

- **Parameters**: `n_buckets` - Number of buckets to allocate.
- **Returns**: A pointer to the allocated hashmap.

### `void free_hashmap(HM* hm)`

Frees the memory associated with the given hashmap.

- **Parameters**: `hm` - Pointer to the hashmap to free.

### `int insert_item(HM* hm, long val)`

Inserts a value into the hashmap.

- **Parameters**: 
  - `hm` - Pointer to the hashmap.
  - `val` - Value to insert.
- **Returns**: 
  - `0` if successful.
  - `1` if memory allocation fails or other errors occur.

### `int remove_item(HM* hm, long val)`

Removes a value from the hashmap.

- **Parameters**: 
  - `hm` - Pointer to the hashmap.
  - `val` - Value to remove.
- **Returns**: 
  - `0` if the value is successfully removed.
  - `1` if the value is not found.

### `int lookup_item(HM* hm, long val)`

Checks if a value exists in the hashmap.

- **Parameters**: 
  - `hm` - Pointer to the hashmap.
  - `val` - Value to look up.
- **Returns**: 
  - `0` if the value exists.
  - `1` if the value is not found.

### `void print_hashmap(HM* hm)`

Prints all elements in the hashmap in the following format:
```
Bucket 1 - val1 - val2 - val3 ...
Bucket 2 - val4 - val5 - val6 ...
...
```

- **Parameters**: `hm` - Pointer to the hashmap to print.