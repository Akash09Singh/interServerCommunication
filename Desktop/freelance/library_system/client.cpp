#include <iostream>
#include <cstdlib>
#include <cstring>
#include <string>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <thread>
#include <chrono>
#include <mutex>

using namespace std;
//this is a helper function to generate a dynamic port number except the port number which are already in use
int generatePortNumber() {
        int portNumber = 0;
        while (portNumber == 0 || portNumber == 44585 || portNumber == 45585 || portNumber == 43585 || portNumber == 42585 || portNumber == 41585) {
            portNumber = rand() % 65536;
        }
        return portNumber;
    }
//this variable stores the port number for client
int clientPortNumber = generatePortNumber();
//this variable store the port number of main server
int serverMTcpPort = 45585;
//this variable stores the username of user who is trying to login or logged in
string user;
//this flag vriable is for identifying wheter user is authenticated or not
bool authenticated = false;
std::mutex authMutex; // Mutex for synchronization

string message;
//this is a helper function to send data over tcp network
int sendDataToPort(const std::string& dataToSend, int portNumber) {
    int senderSocket;
    struct sockaddr_in senderAddr;

    // Creating the  socket for sender
    senderSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (senderSocket < 0) {
        std::cerr << "Error creating sender socket\n";
        return -1;
    }

    // Initializing the sender address structure
    std::memset(&senderAddr, 0, sizeof(senderAddr));
    senderAddr.sin_family = AF_INET;
    senderAddr.sin_addr.s_addr = inet_addr("0.0.0.0"); 
    senderAddr.sin_port = htons(portNumber);

    // Connecting to receiver
    if (connect(senderSocket, (struct sockaddr *)&senderAddr, sizeof(senderAddr)) < 0) {
        std::cerr << "Connection to receiver failed\n";
        close(senderSocket);
        return -1;
    }

    // Sending data to reciever
    ssize_t sentBytes = send(senderSocket, dataToSend.c_str(), dataToSend.size(), 0);
    if (sentBytes < 0) {
        
        close(senderSocket);
        return 1;
    } else {
        
        close(senderSocket);
        return 1;
    }

    
}
//this function creates a tcp connection and keeps it alive also whenever it recieves 
//the data from other servers it takes the decision based on that like passing on the data 
//or displaying on the terminal

int createTCPConnection(int portNumber) {
    int serverSocket, clientSocket;
    socklen_t clientAddrLen;
    struct sockaddr_in serverAddr, clientAddr;

    // Create socket
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket < 0) {
        std::cerr << "Error creating socket\n";
        return -1;
    }

    // Initialize server address structure
    std::memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    serverAddr.sin_port = htons(portNumber); // Port number

    // Bind socket to address and port
    if (bind(serverSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0) {
        std::cerr << "Binding failed\n";
        close(serverSocket);
        return -1;
    }

    // Start listening for incoming connections
    if (listen(serverSocket, 5) < 0) {
        std::cerr << "Listening failed\n";
        close(serverSocket);
        return -1;
    }

    std::cout << "Client is up and running.\n";

    // Continuously accept incoming connections and data requests
    while (true) {
        // Accept incoming connections
        clientAddrLen = sizeof(clientAddr);
        clientSocket = accept(serverSocket, (struct sockaddr *)&clientAddr, &clientAddrLen);
        if (clientSocket < 0) {
            std::cerr << "Error accepting connection\n";
            continue; 
        }

        
        char buffer[1024];
        std::memset(buffer, 0, sizeof(buffer));
        ssize_t bytesRead = read(clientSocket, buffer, sizeof(buffer));
        // starts if data is recieved 
        if (bytesRead > 0) {
            std::string bufferStr(buffer);

    // Find the position of the delimiter '|'
    //spits the data recieved based on delimiter

            
            size_t delimiterPos = bufferStr.find('|');
            std::string source = bufferStr.substr(0, delimiterPos);
            std::string data = bufferStr.substr(delimiterPos + 1);
        
            //if source of data is main then it will display the data recieved from main
            
            if (source=="main"){
                std::cout<<"Response received from the Main Server on TCP port: "<<clientPortNumber<<std::endl;
                std::cout<< data << std::endl;
            }
            // for testing 
            else if(source == "client"){
                std::string fromClientToMain = "client|" + data;
                int sent = sendDataToPort(fromClientToMain, serverMTcpPort);
                if(sent>0){
                    std::cout<<user<<" sent the request to the Main Server.\n";
                }
            }
            // again it is also for testing do not pay attention
            else if(source == "authenticate"){
                std::string fromClientToMain = "authenticate|" + data;
                if (sendDataToPort(fromClientToMain, serverMTcpPort)>0){std::cout<<user<<" sent an authentication request to the Main Server."<<std::endl;}
            }
            //this block handles the response from main after the main server verifies the data and sends response
            //back to client this code block is responsible for displaying of authentication message
            else if(source == "authenticationResult"){
                if(data=="true"){
                    authenticated=true;
                    message = user + " received the result of authentication from Main Server using TCP over port " + std::to_string(clientPortNumber) + "\nAuthentication is successful."; // Print the message if needed

                }else if (data == "mid"){
                    message = user + " received the result of authentication from Main Server using TCP over port " + std::to_string(clientPortNumber) + "\nAuthentication failed: Password does not match.";

                }else{
                    message=user+ " received the result of authentication from Main Server using TCP over port "+ std::to_string(clientPortNumber)+ " \nAuthentication failed:  Username not found.";
                }
                
            }
            
        }

        // Close client socket (not the server socket)
        close(clientSocket);
    }

    // Close server socket (this part will not be reached in this loop)
    close(serverSocket);
    return 0;
}
// this is a helper function to encrypt the password entered by user which will
//later be matched durinng authentication
std::string encryptString(const std::string& input) {
    std::string encrypted = input;

    for (char& c : encrypted) {
        if (isalpha(c)) {
            char base = (isupper(c)) ? 'A' : 'a';
            c = (((c - base) + 5) % 26) + base;
        } else if (isdigit(c)) {
            c = (((c - '0') + 5) % 10) + '0';
        }
    }

    return encrypted;
}
//this an authentication function responsible to take username and password 
//as input and will be running on separate thread
// so you can say your authentication system is running as a different process
void authentication(){
    while (true) {
        std::this_thread::sleep_for(std::chrono::seconds(1));

        // Lock the mutex to safely access and modify 'authenticated'
        std::lock_guard<std::mutex> lock(authMutex);
        std::cout << message << std::endl;
        if (authenticated) {
            break; // Exit the loop if authenticated is true
        }

        std::string username;
        std::string password;
        std::cout << "Please enter the username : ";
        std::cin >> user;
        std::cout << "Please enter the password : ";
        std::cin >> password;
        std::string encryptPassword = encryptString(password);
        std::string dataToSend = "authenticate|" + user + "," + encryptPassword+std::to_string(clientPortNumber);

        // Sending data to port (assuming it updates 'authenticated' in response)
        if(sendDataToPort(dataToSend, serverMTcpPort)!=-1){std::cout<<user<<" sent an authentication request to the Main Server."<<std::endl;};
    }

    while(true){
        std::this_thread::sleep_for(std::chrono::seconds(1));
        std::string bookCode;
        std::cout << "Please enter book code to query : ";
        std::cin >> bookCode;
        std::string dataToSend = "client|"+bookCode;
        if(sendDataToPort(dataToSend,serverMTcpPort)!=-1){std::cout<<user<<" sent the request to the Main Server\n";};
    }
    
}

int main() {
    
    // Create a new thread to handle TCP connection creation for the client using specified port number
    std::thread serverThread(createTCPConnection, clientPortNumber);
    // Create a separate thread for authentication verification
    std::thread verificationThread(authentication);
    // Wait for the server thread to complete its task before proceeding
    serverThread.join();
    // Wait for the verification thread to complete its task before continuing execution
    verificationThread.join();

    return 0;
}