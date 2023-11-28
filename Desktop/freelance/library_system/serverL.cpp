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
int udpMain = 44585;
std::unordered_map<std::string, int> bookRecord;

void trim(std::string &str) {
    // Trim leading spaces
    str.erase(str.begin(), std::find_if(str.begin(), str.end(), [](int ch) {
        return !std::isspace(ch);
    }));

    // Trim trailing spaces
    str.erase(std::find_if(str.rbegin(), str.rend(), [](int ch) {
        return !std::isspace(ch);
    }).base(), str.end());
}

std::string bookData(){
                std::ifstream userFile("literature.txt"); // Replace "your_file.txt" with your file's path

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

void queryResponse(std::string bookCode){
    auto it = bookRecord.find(bookCode);
    int status;
    if (it != bookRecord.end()) {
        if( it->second > 0){
            int newCount = it->second-1;
            std::string dataToSend = std::string("ServerL|The requested book ")+bookCode+ " is available in the library.";
            status = sendUDPData(dataToSend, udpMain);
            decreaseCount(bookCode);
            updateBookCountInFile("literature.txt", bookCode, newCount);
            
        }else{
            std::string dataToSend = std::string("ServerL|The requested book ")+bookCode+ " is NOT available in the library.";
            status = sendUDPData(dataToSend, udpMain);
            
        } // Returns true if count > 0, else false
        if(status>0){
            std::cout<<"Server L finished sending the availability status of code "<<bookCode<< " to the Main Server using UDP on port 44585.\n";
        }
    }
    // Book not found in the map
}

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

    std::cout<<"Server L is up and running using UDP on port 42585"<<std::endl;
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
                std::cout << "Server L received " << data<<  " code from the Main Server." << std::endl;
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
    std::string dataToSend = std::string("serverL-boot|")+listOfBook;
    sendUDPData(dataToSend, udpMain);
    return listOfBook;
}
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
    int udpPortNumber=42585;

    onBootUp();
    loadDataFromFile("literature.txt");
    // Creating threads for TCP and UDP connections
    std::thread udpThread(startUDPServer, udpPortNumber);

    // Join threads to wait for their completion
    udpThread.join();
    

    return 0;
}

