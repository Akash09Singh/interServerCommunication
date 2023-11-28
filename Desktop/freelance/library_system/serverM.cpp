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
#include <algorithm> 
#include <cctype>    
#include <unordered_map>
#include <sstream>

// mentioned all the server port id as global variable
int UDP_SERVER_M_PORT=44585;
int TCP_SERVER_M_PORT=45585;
int UDP_SERVER_H_PORT=43585;
int UDP_SERVER_L_PORT=42585;
int UDP_SERVER_S_PORT=41585;
int TCP_SERVER_C_PORT;

//this map will load all the book record sent by different servers
std::unordered_map<std::string, std::string> bookRecord;

// this map will store the username and encrypted password
std::unordered_map<std::string, std::string> members;

//authentication helper function to check if username and password pairs are present or not and return accordingly.
int authenticateUser(const std::string& username, const std::string& password) {
    auto it = members.find(username);
    if (it != members.end()) {
        if (it->second == password) {
            return 1;
        } else {
            return 0;
        }
    } else {
        return -1;
    }
}
//load member file helper function, this functional is called on boot of main server to load all the username password stored in file
int loadMemberFromFile(const std::string filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Unable to open file " << filename << std::endl;
        return -1;
    }

    std::string line;
    while (std::getline(file, line)) {
        std::istringstream iss(line);
        std::string username;
        std::string password;

        // Using std::getline with ',' as the delimiter to extract username and password
        if (std::getline(iss, username, ',') && std::getline(iss, password)) {
            // Insert the username-password pair into the unordered map
            members[username] = password;
        } else {
            std::cerr << "Error parsing line: " << line << std::endl;
            return -1;
        }
    }
    
    

    file.close();
    return 1;
}

//helper function to update bookrecord on bootup, this function will take the string of code separated by space and and split
//them into respective book code using space as delimiter and store it into Record mapped with its department
void updateHashMapFromCodes(std::string bookCodesString, std::string department) {
    
    std::istringstream iss(bookCodesString);

    
    std::string code;
    while (iss >> code) {
        
        bookRecord[code] = department;
    }
}
//this is a helper function to send data over udp server
int sendUDPData(const std::string& dataToSend, int portNumber) {
    int udpSocket;
    struct sockaddr_in udpServerAddr;

    // Creating the UDP socket
    udpSocket = socket(AF_INET, SOCK_DGRAM, 0);
    if (udpSocket < 0) {
        std::cerr << "Error creating UDP socket\n";
        return -1;
    }

    // Initializing UDP server address structure
    std::memset(&udpServerAddr, 0, sizeof(udpServerAddr));
    udpServerAddr.sin_family = AF_INET;
    udpServerAddr.sin_addr.s_addr = inet_addr("0.0.0.0"); 
    udpServerAddr.sin_port = htons(portNumber); // UDP Port number

    // Send data
    ssize_t sentBytes = sendto(udpSocket, dataToSend.c_str(), dataToSend.size(), 0,
                               (struct sockaddr *)&udpServerAddr, sizeof(udpServerAddr));
    if (sentBytes < 0) {
        close(udpSocket);
        return -1;
    } else {
        
        close(udpSocket);
        return 1;
    }

   
}

//this is a helper function to send data over tcp network
int sendDataToPort(const std::string& dataToSend, int portNumber) {
    int senderSocket;
    struct sockaddr_in senderAddr;

    // Creating socket 
    senderSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (senderSocket < 0) {
        std::cerr << "Error creating sender socket\n";
        return -1;
    }

    // Initializing sender address structure
    std::memset(&senderAddr, 0, sizeof(senderAddr));
    senderAddr.sin_family = AF_INET;
    senderAddr.sin_addr.s_addr = inet_addr("0.0.0.0");
    senderAddr.sin_port = htons(portNumber); // Sender port

    // Connecting  to receiver
    if (connect(senderSocket, (struct sockaddr *)&senderAddr, sizeof(senderAddr)) < 0) {
        std::cerr << "Connection to receiver failed\n";
        close(senderSocket);
        return -1;
    }

    // Sending data
    ssize_t sentBytes = send(senderSocket, dataToSend.c_str(), dataToSend.size(), 0);
    if (sentBytes < 0) {
        
        close(senderSocket);
        return 1;
    } else {
        
        close(senderSocket);
        return 1;
    }

    
}

//this is a helper function that removes preceding and trailing spaces
//from the string, this has been used to structure the string that reads
//data from files
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

//this is a helper function to respond to quer when client asks for a book
//this function takes the book code and checks from book record to find out 
//which server the book belongs to and then forward the request to that  server
//and if book is not found it will simply respond to the client without proceeding
//further
void bookQueryFromClient(std::string bookCode){
    auto it = bookRecord.find(bookCode);
    if (it != bookRecord.end()) {
        std::string bookQuery = "main|" + bookCode;
        
        std::string server =  it->second; // Return the value if key is found
        std::cout<<"Found "<< bookCode<< " located at "<<server<<". Send to "<< server<<std::endl;
        if(server == std::string("ServerS")){
            sendUDPData(bookQuery, UDP_SERVER_S_PORT);
        }
        else if(server == std::string("ServerH")){
            sendUDPData(bookQuery, UDP_SERVER_H_PORT);
        }
        else if(server == std::string("ServerL")){
            sendUDPData(bookQuery, UDP_SERVER_L_PORT);
        }
    } else {
        std::string queryResponse = "main|Not able to find the book-code "+ bookCode +" in the system.";
        std::cout<<"Did not find "<<bookCode<< " in the book code list.";
        if(sendDataToPort(queryResponse,TCP_SERVER_C_PORT)>0){
            std::cout<<"Main Server sent the book status to the client.\n";
        };
    }
}

//this function creates a tcp connection and keeps it alive also whenever it recieves 
//the data from other servers it takes the decision based on that like passing on the data 
//or displaying on the terminal

void createTCPConnection(int portNumber) {
    int serverSocket;
    struct sockaddr_in serverAddr;

    // Creating socket
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket < 0) {
        std::cerr << "Error creating TCP socket\n";
        return;
    }

    // Initializing server address structure here
    std::memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    serverAddr.sin_port = htons(portNumber); // Port number

    // Binding socket to address and port here
    if (bind(serverSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0) {
        std::cerr << "Binding failed for TCP\n";
        close(serverSocket);
        return;
    }

    // Start listening for incoming connections here
    if (listen(serverSocket, 5) < 0) {
        std::cerr << "Listening failed for TCP\n";
        close(serverSocket);
        return;
    }

    std::cout << "Main Server is up and running.\n";
    // upto above code handles the creation of tcp server 
    //below code handles the data recieved and perform action on that data using logics
    // for example here once this tcp recieves data of authentication from client 
    //it will then call authentication helper function to authenticate the user
    //also functionality of passing on the data to other servers are done here
    //using sendUDPdata and other helper functions
    while (true) {
        int clientSocket;
        struct sockaddr_in clientAddr;
        socklen_t clientAddrLen = sizeof(clientAddr);

        // Accept incoming connections
        clientSocket = accept(serverSocket, (struct sockaddr *)&clientAddr, &clientAddrLen);
        if (clientSocket < 0) {
            std::cerr << "Error accepting connection for TCP\n";
            continue;
        }

        // Handle client connection here (send/receive data)
        char buffer[1024];
        std::memset(buffer, 0, sizeof(buffer));
        ssize_t bytesRead = read(clientSocket, buffer, sizeof(buffer));
        if (bytesRead > 0) {
            if (bytesRead > 0) {
            std::string bufferStr(buffer);

            // Find the position of the delimiter '|'
            size_t delimiterPos = bufferStr.find('|');
            std::string source = bufferStr.substr(0, delimiterPos);
            std::string data = bufferStr.substr(delimiterPos + 1);

            //this could be for testing
            if (source == "main") {
                std::cout << "Data received from main: " << data << std::endl;
            } 
            // Handle the data according to the source if it came with client header it will pass on the query to respective 
            //servers for getting updates and records of book
            else if (source == "client") {
                std::cout << "Main Server received the book query from the client using TCP over 45585." << std::endl;
                bookQueryFromClient(data);
            //if data comes to main with authenticate header then it client authenticate user first before procedding 
            //hence, authentication helper functions will be called
            } else if (source=="authenticate"){
                
                std::cout << "Main Server received the username and password from the client using TCP over port 45585." << std::endl;
                // Find the position of the delimiter '|'
                //here i am bifurcating the string to get the username, password and clients port number 
                
                std::string portNumberString = data.substr(data.length() - 5);
                TCP_SERVER_C_PORT = std::stoi(portNumberString);
                data = data.substr(0, data.length() - 5);
                size_t delimiterPos = data.find(',');
                std::string username = data.substr(0, delimiterPos);
                std::string password = data.substr(delimiterPos + 1);
                int status = authenticateUser(username, password);
                //status == 1 means that username and password has been matched
                if (status==1){
                    std::cout<<"password "<<password<<" matches the username. Send a reply to the Client\n";
                    std::string fromMainToClient = std::string("authenticationResult|") + std::string("true");
                    sendDataToPort(fromMainToClient, TCP_SERVER_C_PORT);
                    
                }
                //status==0 means username has been matched but password is incorrect
                else if (status==0){
                    std::cout<<"password "<<password<<" does not matches the username. Send a reply to the Client\n";
                    std::string fromMainToClient = std::string("authenticationResult|") + std::string("mid");
                    sendDataToPort(fromMainToClient, TCP_SERVER_C_PORT);
                }
                //status==-1 means username was not found in the members record
                else if (status==-1){
                    std::cout<<username<<" is not registered. Send a reply to the Client\n";
                    std::string fromMainToClient = std::string("authenticationResult|") + std::string("false");
                    sendDataToPort(fromMainToClient, TCP_SERVER_C_PORT);
                }

            }
            

        }

        }

        close(clientSocket);
    }

  
    close(serverSocket);
}
//this function creates a UDP connection and keeps it alive also whenever it recieves 
//the data from other servers it takes the decision based on that like passing on the data 
//or displaying on the terminal
void startUDPServer(int portNumber) {
    int udpServerSocket;
    struct sockaddr_in udpServerAddr;

    // Creating UDP socket
    udpServerSocket = socket(AF_INET, SOCK_DGRAM, 0);
    if (udpServerSocket < 0) {
        std::cerr << "Error creating UDP socket\n";
        return;
    }

    // Initializing UDP server address structure
    std::memset(&udpServerAddr, 0, sizeof(udpServerAddr));
    udpServerAddr.sin_family = AF_INET;
    udpServerAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    udpServerAddr.sin_port = htons(portNumber); // UDP Port number

    // Binding UDP socket to address and port
    if (bind(udpServerSocket, (struct sockaddr *)&udpServerAddr, sizeof(udpServerAddr)) < 0) {
        std::cerr << "Binding failed for UDP\n";
        close(udpServerSocket);
        return;
    }
    // upto above code handles the creation of tcp server 
    //below code handles the data recieved and perform action on that data using logics
    //also keeps the connection alive
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
            int status;
            //source variable always contains the information about the sending server like which server is sending the data 
            //so if main server is sending the data then information will look like "main|xyz"
            //as well as it may also contain information about task to be performed on recieved data
            if (source=="ServerS"){
                std::string dataToSend = std::string("main|")+data;
                std::cout<<"Main Server received from server S the book status result using UDP over port 44585"<<std::endl;
                status = sendDataToPort(dataToSend,TCP_SERVER_C_PORT);
                if(status>0){
                    std::cout<<"Main Server sent the book status to the client.\n";
                }
            }
            //this is when source is Server H and sent the response for the book query
            else if (source=="ServerH"){
                std::string dataToSend = std::string("main|")+data;
                std::cout<<"Main Server received from server H the book status result using UDP over port 44585"<<std::endl;
                status = sendDataToPort(dataToSend,TCP_SERVER_C_PORT);
                if(status>0){
                    std::cout<<"Main Server sent the book status to the client.\n";
                }
            }
            //this is when source is Server L and sent the response for the book query
            else if (source=="ServerL"){
                std::string dataToSend = std::string("main|")+data;
                std::cout<<"Main Server received from server L the book status result using UDP over port 44585"<<std::endl;
                status = sendDataToPort(dataToSend,TCP_SERVER_C_PORT);
                if(status>0){
                    std::cout<<"Main Server sent the book status to the client.\n";
                }
            }
            //recieving data from ServerS on boot up and storing it in hashmap
           
            else if(source==std::string("serverS-boot")){
                updateHashMapFromCodes(data, std::string("ServerS"));
                std::cout<<"Main Server received the book code list from server S using UDP over port 44585.\n";
                
            }
            //recieving data from ServerH on boot up and storing it in hashmap
            else if(source==std::string("serverH-boot")){
                updateHashMapFromCodes(data, std::string("ServerH"));
                std::cout<<"Main Server received the book code list from server H using UDP over port 44585.\n";
            }
            //recieving data from ServerL on boot up and storing it in hashmap
            else if(source==std::string("serverL-boot")){
                updateHashMapFromCodes(data, std::string("ServerL"));
                std::cout<<"Main Server received the book code list from server L using UDP over port 44585.\n";
            }
            
        }
    }

    close(udpServerSocket);
}

int main() {
    int tcpPortNumber=45585, udpPortNumber=44585;
    

    if(loadMemberFromFile("members.txt")>0){
        std::cout<<"Main Server loaded the member list.\n";
    };
    // Creating threads for TCP and UDP connections so that they can run independently and share same variables
    std::thread tcpThread(createTCPConnection, tcpPortNumber);
    std::thread udpThread(startUDPServer, udpPortNumber);

    // joining the threads
    tcpThread.join();
    udpThread.join();

    return 0;
}
