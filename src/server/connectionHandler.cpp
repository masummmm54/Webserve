#include "HttpServer.hpp"

// setup to read kevent events for sockets
void HttpServer::setupKevent(int client_socket)
{
	struct kevent change;
	EV_SET(&change, client_socket, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL);
	if (kevent(kq, &change, 1, NULL, 0, NULL) == -1)
	{
		std::string error_msg = "Kevent registration failed for socket " + std::to_string(client_socket)
			+ ": " + std::string(strerror(errno));
		log("ERROR", error_msg, NOSTATUS);
		throw std::runtime_error(error_msg);
	}
	log("INFO", "Successfully registered kevent for socket: " + std::to_string(client_socket), NOSTATUS);
}

// configure a given socket for non-blocking mode
void HttpServer::configureSocketNonBlocking(int client_socket)
{
	if (fcntl(client_socket, F_SETFL, O_NONBLOCK) < 0)
	{
		std::string errMsg = "Failed to set non-blocking mode for socket: " + std::to_string(client_socket);
		log("ERROR", errMsg, NOSTATUS);
		throw std::runtime_error(errMsg);
	}
	log("Info", "Socket configured to non-blocking mode: " + std::to_string(client_socket), NOSTATUS);
}

void HttpServer::acceptConnection()
{
	struct sockaddr_in client_address;
	socklen_t client_addrlen = sizeof(client_address);
	int client_socket = -1;
	try
	{
		client_socket = accept(server_fd, (struct sockaddr *)&client_address, &client_addrlen);
		if (client_socket < 0)
		{
			if (errno != EAGAIN && errno != EWOULDBLOCK)
				throw std::runtime_error("Accept failed: " + std::string(strerror(errno)));
			return ; // just exit when no critical error
		}
		openSockets.insert(client_socket);
		// log IP address of accepted connection
		char client_ip[INET_ADDRSTRLEN];
		inet_ntop(AF_INET, &client_address.sin_addr, client_ip, INET_ADDRSTRLEN);
		log("INFO", "Accepted connection from IP: " + std::string(client_ip) + " on socket: " + std::to_string(client_socket), NOSTATUS);
		configureSocketNonBlocking(client_socket);
		setupKevent(client_socket);
		clientInfoMap[client_socket] = ClientInfo();
		std::cout << "Check boolean in accept connection: " << clientInfoMap[client_socket].shouldclose << std::endl;
	}
	catch(const std::exception& e)
	{
		log("ERROR", e.what(), NOSTATUS);
		if (client_socket >= 0) // if opened succesfully but an error occured
		{
			closeSocket(client_socket);
			//close(client_socket);
		}
	}
}