Analysis of the File System Hash Table Project
This project implements a file system using a hash table with various collision resolution strategies. Here's a breakdown of what it does:

Core Components
File Class:

Represents files with name, disk block number, and usage status

Provides getters/setters and overloaded operators for comparison

FileSys Class:

Implements a hash table to store File objects

Supports three collision resolution methods:

Quadratic probing

Double hashing

Linear probing

Handles dynamic resizing through rehashing when load factor exceeds 0.75

Key Features
Hash Table Operations:

Insertion of files with unique (name, block) pairs

Removal of files with tombstone marking

Retrieval of files by name and block number

Updating disk block numbers

Collision Handling:

The system supports three probing methods configurable at runtime

Uses quadratic probing as the default method

Rehashing:

Automatically resizes the table when load factor > 0.75

Finds the next prime number for table size within defined bounds

Transfers data from old to new table during rehashing

Testing Framework:

The test file verifies:

Basic insert/retrieve operations

Handling of collisions

Removal operations

Rehashing behavior

Mixed operations (insert, remove, retrieve)

Purpose
This project demonstrates:

Implementation of a hash table with different collision resolution strategies

Dynamic resizing of hash tables

Handling of file system metadata (file names and disk blocks)

Proper memory management

Robust error handling
