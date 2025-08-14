// CMSC 341 - Fall 2024 - Project 4
#include "filesys.h"
#include <math.h>
#include <algorithm>
#include <random>
#include <vector>
using namespace std;
enum RANDOM {UNIFORMINT, UNIFORMREAL, NORMAL, SHUFFLE};
class Random {
public:
    Random(){}
    Random(int min, int max, RANDOM type=UNIFORMINT, int mean=50, int stdev=20) : m_min(min), m_max(max), m_type(type)
    {
        if (type == NORMAL){
            //the case of NORMAL to generate integer numbers with normal distribution
            m_generator = std::mt19937(m_device());
            //the data set will have the mean of 50 (default) and standard deviation of 20 (default)
            //the mean and standard deviation can change by passing new values to constructor 
            m_normdist = std::normal_distribution<>(mean,stdev);
        }
        else if (type == UNIFORMINT) {
            //the case of UNIFORMINT to generate integer numbers
            // Using a fixed seed value generates always the same sequence
            // of pseudorandom numbers, e.g. reproducing scientific experiments
            // here it helps us with testing since the same sequence repeats
            m_generator = std::mt19937(10);// 10 is the fixed seed value
            m_unidist = std::uniform_int_distribution<>(min,max);
        }
        else if (type == UNIFORMREAL) { //the case of UNIFORMREAL to generate real numbers
            m_generator = std::mt19937(10);// 10 is the fixed seed value
            m_uniReal = std::uniform_real_distribution<double>((double)min,(double)max);
        }
        else { //the case of SHUFFLE to generate every number only once
            m_generator = std::mt19937(m_device());
        }
    }
    void setSeed(int seedNum){
        // we have set a default value for seed in constructor
        // we can change the seed by calling this function after constructor call
        // this gives us more randomness
        m_generator = std::mt19937(seedNum);
    }
    void init(int min, int max){
        m_min = min;
        m_max = max;
        m_type = UNIFORMINT;
        m_generator = std::mt19937(10);// 10 is the fixed seed value
        m_unidist = std::uniform_int_distribution<>(min,max);
    }
    void getShuffle(vector<int> & array){
        // this function provides a list of all values between min and max
        // in a random order, this function guarantees the uniqueness
        // of every value in the list
        // the user program creates the vector param and passes here
        // here we populate the vector using m_min and m_max
        for (int i = m_min; i<=m_max; i++){
            array.push_back(i);
        }
        shuffle(array.begin(),array.end(),m_generator);
    }

    void getShuffle(int array[]){
        // this function provides a list of all values between min and max
        // in a random order, this function guarantees the uniqueness
        // of every value in the list
        // the param array must be of the size (m_max-m_min+1)
        // the user program creates the array and pass it here
        vector<int> temp;
        for (int i = m_min; i<=m_max; i++){
            temp.push_back(i);
        }
        std::shuffle(temp.begin(), temp.end(), m_generator);
        vector<int>::iterator it;
        int i = 0;
        for (it=temp.begin(); it != temp.end(); it++){
            array[i] = *it;
            i++;
        }
    }

    int getRandNum(){
        // this function returns integer numbers
        // the object must have been initialized to generate integers
        int result = 0;
        if(m_type == NORMAL){
            //returns a random number in a set with normal distribution
            //we limit random numbers by the min and max values
            result = m_min - 1;
            while(result < m_min || result > m_max)
                result = m_normdist(m_generator);
        }
        else if (m_type == UNIFORMINT){
            //this will generate a random number between min and max values
            result = m_unidist(m_generator);
        }
        return result;
    }

    double getRealRandNum(){
        // this function returns real numbers
        // the object must have been initialized to generate real numbers
        double result = m_uniReal(m_generator);
        // a trick to return numbers only with two deciaml points
        // for example if result is 15.0378, function returns 15.03
        // to round up we can use ceil function instead of floor
        result = std::floor(result*100.0)/100.0;
        return result;
    }

    string getRandString(int size){
        // the parameter size specifies the length of string we ask for
        // to use ASCII char the number range in constructor must be set to 97 - 122
        // and the Random type must be UNIFORMINT (it is default in constructor)
        string output = "";
        for (int i=0;i<size;i++){
            output = output + (char)getRandNum();
        }
        return output;
    }
    
    int getMin(){return m_min;}
    int getMax(){return m_max;}
    private:
    int m_min;
    int m_max;
    RANDOM m_type;
    std::random_device m_device;
    std::mt19937 m_generator;
    std::normal_distribution<> m_normdist;//normal distribution
    std::uniform_int_distribution<> m_unidist;//integer uniform distribution
    std::uniform_real_distribution<double> m_uniReal;//real uniform distribution

};

unsigned int hashCode(const string str) {
   unsigned int val = 0 ;
   const unsigned int thirtyThree = 33 ;  // magic number from textbook
   for (unsigned int i = 0 ; i < str.length(); i++)
      val = val * thirtyThree + str[i] ;
   return val ;
}

string namesDB[6] = {"driver.cpp", "test.cpp", "test.h", "info.txt", "mydocument.docx", "tempsheet.xlsx"};

class Tester{
};

// A helper function to generate colliding keys
std::string generateCollidingKey(const std::string& base, int modifier, int tableSize, hash_fn hash) {
    std::string key = base + std::to_string(modifier);
    while ((hash(key) % tableSize) != (hash(base) % tableSize)) {
        key = base + std::to_string(++modifier);
    }
    return key;
}



int main() {
    vector<File> dataList;
    Random RndID(DISKMIN, DISKMAX); // Disk block randomizer
    Random RndName(0, 5);           // Name randomizer (choosing from the namesDB)
    FileSys filesys(MINPRIME, hashCode, DOUBLEHASH); // File system with double hashing
    bool result = true;
    
    // Test 1: Insert files with non-colliding keys (unique disk blocks)
    cout << "Testing with non-colliding keys:\n";
    for (int i = 0; i < 10; i++) {
        // Generate a file object with a unique disk block
        File dataObj = File(namesDB[RndName.getRandNum()], RndID.getRandNum(), true);
        
        // Ensure the generated disk block is unique
        bool collision = false;
        for (const auto& file : dataList) {
            if (file.getDiskBlock() == dataObj.getDiskBlock()) {
                collision = true;
                break;
            }
        }

        // If a collision is detected, retry generating a new file
        if (collision) {
            i--; // Decrement counter to retry this iteration
            continue;
        }

        // Save data for later use
        dataList.push_back(dataObj);
        
        // Insert the file into the FileSys object
        if (!filesys.insert(dataObj)) {
            cout << "Failed to insert file: " << dataObj.getName() << endl;
            result = false; // Mark the test as failed
        }
    }

    // Dump the file system contents after inserting non-colliding files
    filesys.dump();
    
    // Test 1: Check if all data are correctly inserted and can be retrieved
    cout << "Checking retrieval for non-colliding keys:\n";
    for (const auto& file : dataList) {
        try {
            // Retrieve the file from the file system
            const File& retrievedFile = filesys.getFile(file.getName(), file.getDiskBlock());
            
            // Verify if the retrieved file matches the original file
            if (!(retrievedFile == file)) {
                cout << "Mismatch found for file: " << file.getName() << endl;
                result = false; // Mark the test as failed
            }
        }
        catch (const std::runtime_error& e) {
            cout << "Error retrieving file: " << file.getName() << " Block: " << file.getDiskBlock() << endl;
            result = false; // Mark the test as failed
        }
    }
    
    // Test 1 Result
    if (result) {
        cout << "\nAll non-colliding files were successfully inserted and retrieved.\n";
    } else {
        cout << "\nSome non-colliding files failed to insert or retrieve.\n";
    }

    // Reset result for the next test
    result = true;

    // Test 2: Insert files with colliding keys (same disk block but different names)
    cout << "\nTesting with colliding keys (same disk block but different names):\n";
    // Force collision by manually using the same disk block for two files
    string name1 = "collision1.txt";
    string name2 = "collision2.txt";
    
    File file1(name1, DISKMIN, true);  // Both files will use the same disk block
    File file2(name2, DISKMIN, true);  // Same disk block as file1

    // Insert colliding files
    if (!filesys.insert(file1)) {
        cout << "Failed to insert file: " << name1 << endl;
        result = false; // Mark the test as failed
    } else {
        dataList.push_back(file1);
    }

    if (!filesys.insert(file2)) {
        cout << "Failed to insert file: " << name2 << endl;
        result = false; // Mark the test as failed
    } else {
        dataList.push_back(file2);
    }

    // Test 2: Check if colliding files can be correctly retrieved
    cout << "Checking retrieval for colliding keys:\n";
    for (const auto& file : dataList) {
        try {
            // Retrieve the file from the file system
            const File& retrievedFile = filesys.getFile(file.getName(), file.getDiskBlock());
            
            // Verify if the retrieved file matches the original file
            if (!(retrievedFile == file)) {
                cout << "Mismatch found for file: " << file.getName() << endl;
                result = false; // Mark the test as failed
            }
        }
        catch (const std::runtime_error& e) {
            cout << "Error retrieving file: " << file.getName() << " Block: " << file.getDiskBlock() << endl;
            result = false; // Mark the test as failed
        }
    }
    
    // Test 2 Result
    if (result) {
        cout << "\nAll colliding files were successfully inserted and retrieved.\n";
    } else {
        cout << "\nSome colliding files failed to insert or retrieve.\n";
    }

    // Reset result for the next test
    result = true;

    // Test 3: Remove files with non-colliding keys (unique disk blocks)
cout << "\nTesting remove operation with non-colliding keys:\n";
for (int i = 0; i < 5; i++) {
    File dataObj = File(namesDB[RndName.getRandNum()], RndID.getRandNum(), true);

    // Ensure unique disk blocks
    bool collision = false;
    for (const auto& file : dataList) {
        if (file.getDiskBlock() == dataObj.getDiskBlock()) {
            collision = true;
            break;
        }
    }

    if (collision) {
        i--;
        continue;
    }

    // Insert the file and remove it
    if (!filesys.insert(dataObj)) {
        cout << "Failed to insert file: " << dataObj.getName() << endl;
        result = false;
    } else {
        dataList.push_back(dataObj);
    }

    // Remove the file (pass the whole File object)
    if (!filesys.remove(dataObj)) {
        cout << "Failed to remove file: " << dataObj.getName() << endl;
        result = false;
    }

    // Verify the file is removed
    try {
        filesys.getFile(dataObj.getName(), dataObj.getDiskBlock());
        cout << "Error: file was not removed properly: " << dataObj.getName() << endl;
        result = false;
    } catch (const std::runtime_error& e) {
        // File is successfully removed, expected behavior
    }
}

// Test 3 Result
if (result) {
    cout << "\nAll non-colliding files were successfully removed.\n";
} else {
    cout << "\nSome non-colliding files failed to be removed.\n";
}

// Reset result for the next test
result = true;

// Test 4: Remove files with colliding keys
    cout << "\nTesting remove operation with colliding keys:\n";

    // Step 1: Generate colliding keys
    string baseName = "baseKey";
    int baseDiskBlock = 100;
    vector<File> collidingFiles;
    
    for (int i = 0; i < 5; i++) {
        string collidingName = generateCollidingKey(baseName, i, MINPRIME, hashCode);
        File collidingFile(collidingName, baseDiskBlock + i, true);
        collidingFiles.push_back(collidingFile);
        if (!filesys.insert(collidingFile)) {
            cout << "Failed to insert file: " << collidingFile.getName() << endl;
            result = false; // Mark the test as failed
        }
    }
    
    // Step 2: Remove colliding keys
    for (const auto& file : collidingFiles) {
        if (!filesys.remove(file)) {
            result = false; // Mark the test as failed
        } 

        // Verify the file is removed
        try {
            filesys.getFile(file.getName(), file.getDiskBlock());
            cout << "Error: file was not removed properly: " << file.getName() << endl;
            result = false; // Mark the test as failed
        } catch (const std::runtime_error& e) {
            // File is successfully removed, expected behavior
        }
    }

    // Test 4 Result
    if (result) {
        cout << "\nTEST 4 PASSED: All colliding keys were successfully removed.\n";
    } else {
        cout << "\nTEST 4 FAILED: Some colliding keys failed to be removed.\n";
    }

    // === TEST 5: Random Data Insert and Retrieval with Rehashing ===
cout << "\nTEST 5: Insert and retrieve random data points with rehashing check\n";
result = true;

cout << "Initial capacity: " << MINPRIME << endl;

for (int i = 0; i < 10; i++) {
    // Generating random data
    File dataObj = File(namesDB[RndName.getRandNum()], RndID.getRandNum(), true);
    // Saving data for later use
    dataList.push_back(dataObj);
    // Inserting data into the FileSys object
    if (!filesys.insert(dataObj)) {
        result = false; // Mark test as failed if insertion fails
    }

    // Check if rehashing occurred
    if (filesys.lambda() > 0.75) {
        cout << "Rehash triggered at load factor: " << filesys.lambda() << endl;
    }
}

// Verify all data points are retrievable
for (const auto& file : dataList) {
    try {
        if (!(file == filesys.getFile(file.getName(), file.getDiskBlock()))) {
            result = false; // Mark test as failed
        }
    } catch (const std::runtime_error& e) {
        result = false; // Mark test as failed
    }
}

if (result) {
    cout << "\nTEST 5 PASSED: All data points were inserted and retrieved successfully!\n";
} else {
    cout << "\nTEST 5 FAILED: Some data points were not handled correctly.\n";
}

// Test 6: Mixed Operations (Insert, Remove, Retrieve)
    cout << "\nTest 6 Testing Mixed Operations (Insert, Remove, Retrieve):\n";
    result = true;
    // Step 1: Insert 5 files with unique disk blocks
    for (int i = 0; i < 5; i++) {
        File dataObj = File(namesDB[RndName.getRandNum()], RndID.getRandNum(), true);
        dataList.push_back(dataObj);
        if (!filesys.insert(dataObj)) {
            result = false;
        }
    }

    // Step 2: Remove 2 files from the inserted files
    if (!filesys.remove(dataList[1])) {
        result = false;
    }
    if (!filesys.remove(dataList[3])) {
        result = false;
    }

    for (int i = 0; i < dataList.size(); ++i) {
        if (i != 1 && i != 3) {  // Skip removed files
            try {
                const File& retrievedFile = filesys.getFile(dataList[i].getName(), dataList[i].getDiskBlock());
                if (!(retrievedFile == dataList[i])) {
                    result = false;
                }
            } catch (const std::runtime_error& e) {
                result = false;
            }
        }
    }

    // Step 4: Ensure removed files are no longer accessible
    try {
        filesys.getFile(dataList[1].getName(), dataList[1].getDiskBlock());
        result = false;
    } catch (const std::runtime_error& e) {
        // Expected behavior, file was removed
    }

    try {
        filesys.getFile(dataList[3].getName(), dataList[3].getDiskBlock());
        result = false;
    } catch (const std::runtime_error& e) {
        // Expected behavior, file was removed
    }

    // Test 6 Result
    if (result) {
        cout << "\nTEST 6 PASSED: All operations executed successfully!\n";
    } else {
        cout << "\nTEST 6 FAILED: Some operations did not work as expected.\n";
    }

return 0;

}

