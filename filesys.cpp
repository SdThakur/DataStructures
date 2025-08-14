// CMSC 341 - Fall 2024 - Project 4
#include "filesys.h"
#include <cmath>
#include <iostream>
#include <stdexcept>

#define MINPRIME 101
#define MAXPRIME 1009
// Constructor
FileSys::FileSys(int size, hash_fn hash, prob_t probing) 
    : m_currentCap(size), m_hash(hash), m_probeType(probing), m_currentSize(0), m_oldTable(nullptr), m_oldCap(0) {
    m_currentTable = new File*[m_currentCap]();
}

// Destructor
FileSys::~FileSys() {
    for (int i = 0; i < m_currentCap; ++i) {
        delete m_currentTable[i];
    }
    delete[] m_currentTable;

    // Clean up old table if it exists
    if (m_oldTable != nullptr) {
        for (int i = 0; i < m_oldCap; ++i) {
            delete m_oldTable[i];
        }
        delete[] m_oldTable;
    }
}

// Change the probing policy
void FileSys::changeProbPolicy(prob_t policy) {
    m_probeType = policy;
}

// Insert a file into the table
bool FileSys::insert(File file) {

    if (m_currentSize == m_currentCap) {
        return false; // Table full
    }

    // Trigger rehashing if load factor exceeds threshold
    if (lambda() > 0.75) {
        rehash();
    }

    int index = m_hash(file.getName()) % m_currentCap;
    int step = 1;

    while (m_currentTable[index] != nullptr && m_currentTable[index] != reinterpret_cast<File*>(-1)) {
        if (m_currentTable[index]->getName() == file.getName() &&
            m_currentTable[index]->getDiskBlock() == file.getDiskBlock()) {
            return false; // Duplicate entry
        }

        index = (index + step * step) % m_currentCap; // Quadratic probing
        step++;
        if (step > m_currentCap) {
            return false; // Probing exhausted
        }
    }

    m_currentTable[index] = new File(file);
    m_currentSize++;
    return true;
}



// Remove a file from the table
bool FileSys::remove(File file) {
    int index = m_hash(file.getName()) % m_currentCap;
    int step = 1;

    while (m_currentTable[index] != nullptr) {
        if (m_currentTable[index] != reinterpret_cast<File*>(-1) && // Skip tombstones
            m_currentTable[index]->getName() == file.getName() &&
            m_currentTable[index]->getDiskBlock() == file.getDiskBlock()) {
            delete m_currentTable[index];
            m_currentTable[index] = reinterpret_cast<File*>(-1); // Mark as tombstone
            m_currentSize--;
            return true;
        }

        index = (index + step * step) % m_currentCap; // Quadratic probing
        step++;
        if (step > m_currentCap) {
            break; // Probing exhausted
        }
    }
    return false; // File not found
}

int FileSys::hash(std::string name, int block) const {
    int nameHash = std::hash<std::string>{}(name);
    int blockHash = std::hash<int>{}(block);
    return (nameHash ^ (blockHash << 1)) % m_currentCap; // Combine hashes
}


// Retrieve a file by name and block
const File FileSys::getFile(std::string name, int block) const {
    int index = m_hash(name) % m_currentCap;
    int step = 1;

    while (m_currentTable[index] != nullptr) {
        if (m_currentTable[index] != reinterpret_cast<File*>(-1) &&
            m_currentTable[index]->getName() == name &&
            m_currentTable[index]->getDiskBlock() == block) {
            return *m_currentTable[index];
        }

        index = (index + step * step) % m_currentCap; // Quadratic probing
        step++;
        if (step > m_currentCap) {
            break; // Probing exhausted
        }
    }

    throw std::runtime_error("File not found");
}



// Update the disk block of a file
bool FileSys::updateDiskBlock(File file, int block) {
    int index = m_hash(file.getName()) % m_currentCap;
    int step = 1;

    while (m_currentTable[index]) {
        if (m_currentTable[index]->getName() == file.getName() &&
            m_currentTable[index]->getDiskBlock() == file.getDiskBlock()) {
            m_currentTable[index]->setDiskBlock(block);
            return true;
        }
        index = (index + step * step) % m_currentCap;
        step++;
    }

    return false; // File not found
}

// Calculate the load factor
float FileSys::lambda() const {
    return static_cast<float>(m_currentSize) / m_currentCap;
}

// Calculate the deleted ratio
float FileSys::deletedRatio() const {
    int deletedCount = 0;
    for (int i = 0; i < m_currentCap; ++i) {
        if (m_currentTable[i] == nullptr) {
            deletedCount++;
        }
    }
    return static_cast<float>(deletedCount) / m_currentCap;
}

// Dump the current table
void FileSys::dump() const {
    std::cout << "Dump for the current table: " << std::endl;
    if (m_currentTable != nullptr) {
        for (int i = 0; i < m_currentCap; i++) {
            if (m_currentTable[i]) {
                std::cout << "[" << i << "] : " << m_currentTable[i]->getName() << ", Block: " << m_currentTable[i]->getDiskBlock() << std::endl;
            } else {
                std::cout << "[" << i << "] : Empty" << std::endl;
            }
        }
    }

    std::cout << "Dump for the old table: " << std::endl;
    if (m_oldTable != nullptr) {
        for (int i = 0; i < m_oldCap; i++) {
            if (m_oldTable[i]) {
                std::cout << "[" << i << "] : " << m_oldTable[i]->getName() << ", Block: " << m_oldTable[i]->getDiskBlock() << std::endl;
            } else {
                std::cout << "[" << i << "] : Empty" << std::endl;
            }
        }
    }
}

// Check if a number is prime
bool FileSys::isPrime(int number) {
    if (number <= 1) return false;
    for (int i = 2; i <= sqrt(number); ++i) {
        if (number % i == 0) {
            return false;
        }
    }
    return true;
}

// Find the next prime number after the current number
int FileSys::findNextPrime(int current) {
    if (current < MINPRIME) current = MINPRIME - 1;
    for (int i = current + 1; i < MAXPRIME; ++i) {
        if (isPrime(i)) {
            return i;
        }
    }
    return MAXPRIME; // If no prime found, return MAXPRIME
}

void FileSys::rehash() {
    int newCap = findNextPrime(m_currentCap * 2); // Double the capacity and find next prime
    File** newTable = new File*[newCap]();

    // Save the old table for dumping
    File** oldTable = m_currentTable;
    int oldCap = m_currentCap;

    // Rehash all non-null, non-tombstone entries
    for (int i = 0; i < oldCap; ++i) {
        if (oldTable[i] != nullptr && oldTable[i] != reinterpret_cast<File*>(-1)) {
            int index = m_hash(oldTable[i]->getName()) % newCap;
            int step = 1;

            while (newTable[index] != nullptr) {
                index = (index + step * step) % newCap;
                step++;
            }
            newTable[index] = oldTable[i];
        }
    }

    // Clean up the old table
    delete[] oldTable;

    // Update to the new table
    m_currentTable = newTable;
    m_currentCap = newCap;
}
