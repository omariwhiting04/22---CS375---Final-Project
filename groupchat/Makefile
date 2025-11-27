CXX = g++
CXXFLAGS = -std=c++17 -pthread

SERVER_SRC = server/chat_server.cpp
CLIENT_SRC = client/client.cpp

SERVER_OUT = server.out
CLIENT_OUT = client.out

all: $(SERVER_OUT) $(CLIENT_OUT)

$(SERVER_OUT): $(SERVER_SRC)
	$(CXX) $(CXXFLAGS) -o $(SERVER_OUT) $(SERVER_SRC)

$(CLIENT_OUT): $(CLIENT_SRC)
	$(CXX) $(CXXFLAGS) -o $(CLIENT_OUT) $(CLIENT_SRC)

clean:
	rm -f $(SERVER_OUT) $(CLIENT_OUT)
