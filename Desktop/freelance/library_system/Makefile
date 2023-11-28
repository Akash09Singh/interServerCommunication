CC = g++
CFLAG1 = -std=c++11
CFLAG2 = -lpthread

all: compile

compile: serverM serverL serverH serverS client

serverM: serverM.cpp
$(CC) $(CFLAG1) serverM.cpp -o serverM $(CFLAG2)

serverL: serverL.cpp
$(CC) $(CFLAG1) serverL.cpp -o serverL $(CFLAG2)

serverH: serverH.cpp
$(CC) $(CFLAG1) serverH.cpp -o serverH $(CFLAG2)

serverS: serverS.cpp
$(CC) $(CFLAG1) serverS.cpp -o serverS $(CFLAG2)

client: client.cpp
$(CC) $(CFLAG1) client.cpp -o client $(CFLAG2)

.PHONY: clean

clean:
rm -f serverM serverL serverH serverS client