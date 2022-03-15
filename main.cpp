#include <iostream>
#include <sys/types.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <string.h>
#include <string>
#include <memory>

using namespace std;

class Server
{
public:
    Server(std::string ipAdress, int port){
        m_ipAdress = &*ipAdress.begin();
        m_port = port;
    }

    void createConnection(){
        createSocket();
        acceptConnection();
    }
    void closeConnection(){
        // Close the socket
        if(m_clientSocket==-1)
        {
            cerr << "Cant close connection! Quitting" << endl;
            exit(EXIT_FAILURE);
        }
        else{
            close(m_clientSocket);
        }
    }

    int handleMessage(){
        memset(buf, 0, 4096);

        // Wait for client to send data
        int bytesReceived = recv(m_clientSocket, buf, 4096, 0);
        if (bytesReceived == -1)
        {
            cerr << "Error in recv(). Quitting" << endl;
            return 0;
        }

        if (bytesReceived == 0)
        {
            cout << "Client disconnected " << endl;
            return 0;
        }

        cout << string(buf, 0, bytesReceived) << endl;

        // Echo message back to client
        send(m_clientSocket, buf, bytesReceived + 1, 0);
        return 1;
    }
    int getFD(){
        if(m_gotFD){
            return m_clientSocket;
        }
        return -1;
    }
private:

    void createSocket(){
        // Create a socket
        m_listening = socket(AF_INET, SOCK_STREAM, 0);
        if (m_listening == -1)
        {
            cerr << "Can't create a socket! Quitting" << endl;
            exit(EXIT_FAILURE);
        }

        // Bind the ip address and port to a socket
        sockaddr_in hint;
        hint.sin_family = AF_INET;
        hint.sin_port = htons(m_port);
        inet_pton(AF_INET, m_ipAdress, &hint.sin_addr);

        bind(m_listening, (sockaddr*)&hint, sizeof(hint));

        // Tell Winsock the socket is for listening
        listen(m_listening, SOMAXCONN);
    }
    void acceptConnection(){

        // Wait for a connection
        sockaddr_in client;
        socklen_t clientSize = sizeof(client);

        m_clientSocket = accept(m_listening, (sockaddr*)&client, &clientSize);
        if (m_clientSocket == -1)
        {
            cerr << "Cant accept client connection! Quitting" << endl;
            exit(EXIT_FAILURE);
        }

        m_gotFD=true;

        char host[NI_MAXHOST];      // Client's remote name
        char service[NI_MAXSERV];   // Service (i.e. port) the client is connect on

        memset(host, 0, NI_MAXHOST); // same as memset(host, 0, NI_MAXHOST);
        memset(service, 0, NI_MAXSERV);

        if (getnameinfo((sockaddr*)&client, sizeof(client), host, NI_MAXHOST, service, NI_MAXSERV, 0) == 0)
        {
            cout << host << " connected on port " << service << endl;
        }
        else
        {
            inet_ntop(AF_INET, &client.sin_addr, host, NI_MAXHOST);
            cout << host << " connected on port " << ntohs(client.sin_port) << endl;
        }
        // Close listening socket
        close(m_listening);
    }

    char buf[4096];
    char* m_ipAdress;
    int m_port;
    int m_listening;
    int m_clientSocket;
    bool m_gotFD=false;
};

int main()
{
    std::unique_ptr<Server> server(new Server("192.168.150.1",1500));
    server->createConnection();
    int runServer=1;
    while (runServer)
    {
        runServer=server->handleMessage();

    }
    server->closeConnection();
    return 0;
}
