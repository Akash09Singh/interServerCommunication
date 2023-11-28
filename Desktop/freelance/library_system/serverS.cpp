#include <iostream>
#include <string>
#include <cstring>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <thread>
#include <arpa/inet.h>
#include <cstdlib>
#include <fstream>
#include <algorithm> // Required for std::find_if
#include <cctype>    // Required for std::isspace
#include <unordered_map>
#include <sstream>

std::unordered_map<std::string, int> bookRecord;
int udpMain = 44585;

// Trim function to remove leading and trailing spaces from the given string
void trim(std::string &str) {
    // Trim leading spaces:
    // Erase characters from the beginning of the string until a non-space character is encountered
    str.erase(str.begin(), std::find_if(str.begin(), str.end(), [](int ch) {
        // Lambda function used as a predicate to identify non-space characters
        return !std::isspace(ch); // Checks if the character is not a space
    }));

    // Trim trailing spaces:
    // Erase characters from the end of the string until a non-space character is encountered
    str.erase(std::find_if(str.rbegin(), str.rend(), [](int ch) {
        return !std::isspace(ch);
    }).base(), str.end());
}

// Reads data from the "science.txt" file, extracts and processes book codes and counts
// Returns a formatted string containing a list of book codes from the file
std::string bookData(){
                std::ifstream userFile("science.txt"); // Replace "your_file.txt" with your file's path

                if (!userFile.is_open()) {
                    std::cerr << "Unable to open file\n";
                    return std::string("unable to open file");
                }

                std::string line;
                bool valid = false;
                std::string listOfBooks="";
                while (std::getline(userFile, line)) {
                    trim(line);
                    if (!line.empty()) {
                        // Find the position of the delimiter '|'
                        size_t delimiterPos = line.find(' ');
                        std::string bookCode = line.substr(0, delimiterPos);
                        std::string bookCount = line.substr(delimiterPos + 1);
                        listOfBooks+=(bookCode+std::string(" "));
                        // std::cout << "Non-empty line: " << line << data << std::endl;   
                        // Perform operations with non-empty lines here
                    }
                    
                }
                trim(listOfBooks);
                return listOfBooks;
}

// Sends data over UDP to a specified IP address and port number
// Parameters:
// - dataToSend: The string containing the data to be sent
// - portNumber: The port number of the UDP server
// Returns:
// - Returns 1 if the data is successfully sent; otherwise, returns -1
int sendUDPData(const std::string& dataToSend, int portNumber) {
    int udpSocket;
    struct sockaddr_in udpServerAddr;

    // Create UDP socket
    udpSocket = socket(AF_INET, SOCK_DGRAM, 0);
    if (udpSocket < 0) {
        std::cerr << "Error creating UDP socket\n";
        return -1;
    }

    // Initialize UDP server address structure
    std::memset(&udpServerAddr, 0, sizeof(udpServerAddr));
    udpServerAddr.sin_family = AF_INET;
    udpServerAddr.sin_addr.s_addr = inet_addr("127.0.0.1"); // IP address of the UDP server
    udpServerAddr.sin_port = htons(portNumber); // UDP Port number

    // Send data
    ssize_t sentBytes = sendto(udpSocket, dataToSend.c_str(), dataToSend.size(), 0,
                               (struct sockaddr *)&udpServerAddr, sizeof(udpServerAddr));
    if (sentBytes < 0) {
        // Close UDP socket
    close(udpSocket);
        return -1;
    } else {
        // Close UDP socket
    close(udpSocket);
        return 1;
    }

    
}

// Function to decrease count by one for a specific key (if count > 0)
void decreaseCount(const std::string key) {
    auto it = bookRecord.find(key);
    if (it != bookRecord.end() && it->second > 0) {
        // Decrease count by one if the key exists and count > 0
        it->second--;
    } else {
        ;
    }
}
// Updates the count of a specific book code in a file
// Parameters:
// - filename: The name of the file to be updated
// - bookCode: The code of the book whose count needs to be updated
// - newCount: The new count to be assigned to the specified book code
void updateBookCountInFile(const std::string filename, const std::string bookCode, int newCount) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Unable to open file " << filename << std::endl;
        return;
    }

    std::string line;
    std::stringstream updatedData;

    while (std::getline(file, line)) {
        std::istringstream iss(line);
        std::string code;
        int count;

        if (iss >> code >> count) {
            if (code == bookCode) {
                updatedData << code << " " << newCount << '\n'; // Update count for the specific book code
            } else {
                updatedData << code << " " << count << '\n'; // Retain other book codes as they are
            }
        } else {
            std::cerr << "Error parsing line: " << line << std::endl;
        }
    }

    file.close();

    // Write the updated data back to the file
    std::ofstream outFile(filename);
    if (!outFile.is_open()) {
        std::cerr << "Unable to open file for writing: " << filename << std::endl;
        return;
    }

    outFile << updatedData.str();
    outFile.close();
}
// Handles query response for a given book code
// Parameters:
// - bookCode: The code of the book being queried
void queryResponse(std::string bookCode){
    auto it = bookRecord.find(bookCode);
    int status;
    if (it != bookRecord.end()) {
        if( it->second > 0){
            int newCount = it->second-1;
            std::string dataToSend = std::string("ServerS|The requested book ")+bookCode+ " is available in the library.";
            status = sendUDPData(dataToSend, udpMain);
            decreaseCount(bookCode);
            updateBookCountInFile("science.txt", bookCode, newCount);
            
        }else{
            std::string dataToSend = std::string("ServerS|The requested book ")+bookCode+ " is NOT available in the library.";
            status = sendUDPData(dataToSend, udpMain);
        } // Returns true if count > 0, else false
        if(status>0){
            std::cout<<"Server S finished sending the availability status of code "<<bookCode<< " to the Main Server using UDP on port 44585.\n";
        }
    }
    // Book not found in the map
}


// Starts a UDP server on a specified port to receive and process incoming messages
// Parameters:
// - portNumber: The port number on which the UDP server will listen for incoming messages
void startUDPServer(int portNumber) {
    int udpServerSocket;
    struct sockaddr_in udpServerAddr;

    // Create UDP socket
    udpServerSocket = socket(AF_INET, SOCK_DGRAM, 0);
    if (udpServerSocket < 0) {
        std::cerr << "Error creating UDP socket\n";
        return;
    }

    // Initialize UDP server address structure
    std::memset(&udpServerAddr, 0, sizeof(udpServerAddr));
    udpServerAddr.sin_family = AF_INET;
    udpServerAddr.sin_addr.s_addr = INADDR_ANY;
    udpServerAddr.sin_port = htons(portNumber); // UDP Port number

    // Bind UDP socket to address and port
    if (bind(udpServerSocket, (struct sockaddr *)&udpServerAddr, sizeof(udpServerAddr)) < 0) {
        std::cerr << "Binding failed for UDP\n";
        close(udpServerSocket);
        return;
    }

    std::cout<<"Server S is up and running using UDP on port 41585"<<std::endl;

    while (true) {
        char udpBuffer[1024];
        struct sockaddr_in clientAddr;
        socklen_t clientAddrLen = sizeof(clientAddr);
        std::memset(udpBuffer, 0, sizeof(udpBuffer));
        ssize_t udpBytesReceived = recvfrom(udpServerSocket, udpBuffer, sizeof(udpBuffer), 0,
                                            (struct sockaddr *)&clientAddr, &clientAddrLen);
        if (udpBytesReceived > 0) {
            
            std::string bufferStr(udpBuffer);

    // Find the position of the delimiter '|'
            
            size_t delimiterPos = bufferStr.find('|');
            std::string source = bufferStr.substr(0, delimiterPos);
            std::string data = bufferStr.substr(delimiterPos + 1);
            
            if (source=="main"){
                std::cout << "Server S received " << data<<  " code from the Main Server." << std::endl;
                queryResponse(data);
            }

            
        }
    }

    // Close UDP socket
    close(udpServerSocket);
}
//bootup function of ServerS
std::string onBootUp(){
    std::string listOfBook = bookData();
    std::string dataToSend = std::string("serverS-boot|")+listOfBook;
    sendUDPData(dataToSend, udpMain);
    return listOfBook;
}
// Loads book data from a file and populates an unordered map with book codes and counts
// Parameters:
// - filename: The name of the file containing book data to be loaded
void loadDataFromFile(const std::string filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Unable to open file " << filename << std::endl;
        return;
    }

    std::string line;
    while (std::getline(file, line)) {
        std::istringstream iss(line);
        std::string bookCode;
        int bookCount;

        if (iss >> bookCode >> bookCount) {
            // Insert the book code and count into the unordered map
            bookRecord[bookCode] = bookCount;
        } else {
            std::cerr << "Error parsing line: " << line << std::endl;
        }
    }

    file.close();
}
int main() {
    int udpPortNumber=41585;

    onBootUp();
    loadDataFromFile("science.txt");
    // Creating threads for TCP and UDP connections
    std::thread udpThread(startUDPServer, udpPortNumber);

    // Join threads to wait for their completion
    udpThread.join();
    

    return 0;
}

