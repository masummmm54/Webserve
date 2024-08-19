#include "HttpServer.hpp"

bool	HttpServer::validateServer(int client_socket)
{
	auto serverIt = servers.find(clientInfoMap[client_socket]->server_fd);
	if (serverIt == servers.end())
	{
		sendErrorResponse(client_socket, 500, "Internal Server Error");
		return (false);
	}
	return (true);
}

bool	HttpServer::validateRouteAndMethod(int client_socket, const HttpRequest& request)
{
	auto serverIt = servers.find(clientInfoMap[client_socket]->server_fd);
	const serverInfo &srv = serverIt->second->getServerInfo();
	auto routeIt = srv.routes.find(request.uri);

	if (routeIt == srv.routes.end())
	{
		sendErrorResponse(client_socket, 404, "Not Found");
		return (false);
	}
	const routeConfig &route = routeIt->second;
	if (std::find(route.allowedMethods.begin(), route.allowedMethods.end(), request.method) == route.allowedMethods.end())
	{
		sendErrorResponse(client_socket, 405, "Method Not Allowed");
		return (false);
	}
	return (true);
}

void	HttpServer::decideConnectionPersistence(int client_socket, const HttpRequest& request)
{
	std::string connectionValue;
	// Check if the connection header exists
	auto header = request.headers.find("connection");
	if (header != request.headers.end())
	{
		connectionValue = header->second;
		trim(connectionValue);
		std::transform(connectionValue.begin(), connectionValue.end(), connectionValue.begin(), ::tolower);
		clientInfoMap[client_socket]->shouldclose = (connectionValue != "keep-alive");
	}
	else
		clientInfoMap[client_socket]->shouldclose = true;
}
void	HttpServer::registerWriteEvent(int client_socket)
{
	struct kevent change;
	EV_SET(&change, static_cast<uintptr_t>(client_socket), EVFILT_WRITE, EV_ADD | EV_ENABLE, 0, 0, NULL);
	if (kevent(kq, &change, 1, NULL, 0, NULL) == -1)
		logger.logMethod("ERROR", "Kevent registration failure for writing: " + std::string(strerror(errno)));
	else
		logger.logMethod("INFO", "Successfully registered kevent for socket: " + std::to_string(client_socket));
}

void	HttpServer::processRequestMethod(int client_socket)
{
	HttpRequest& request = clientInfoMap[client_socket]->request;
	if (request.method == "GET")
		handleGetRequest(client_socket);
	else if (request.method == "POST")
		handlePostRequest(client_socket);
	else
		sendErrorResponse(client_socket, 501, "Not Implemented");
}

void HttpServer::handleRequest(int client_socket)
{
	if (!validateServer(client_socket))
		return;
	HttpRequest &request = clientInfoMap[client_socket]->request;
	if (!validateRouteAndMethod(client_socket, request))
		return;
	decideConnectionPersistence(client_socket, request);
	processRequestMethod(client_socket);
	registerWriteEvent(client_socket);
}

// // first iterator points to beginning of the file
// // second used as end marker, correct syntax

// void HttpServer::handleRequest(int client_socket)
// {
// 	HttpRequest &request = clientInfoMap[client_socket]->request;

// 	auto serverIt = servers.find(clientInfoMap[client_socket]->server_fd);
// 	if (serverIt == servers.end())
// 	{
// 		sendErrorResponse(client_socket, 500, "Internal Server Error");
// 		return;
// 	}
// 	const serverInfo &srv = serverIt->second->getServerInfo();
// 	std::string requestPath = request.uri;
// 	auto routeIt = srv.routes.find(requestPath);

// 	if (routeIt != srv.routes.end())
// 	{
// 		const routeConfig &route = routeIt->second;

// 		if (std::find(route.allowedMethods.begin(), route.allowedMethods.end(), request.method) == route.allowedMethods.end())
// 		{
// 			sendErrorResponse(client_socket, 405, "Method Not Allowed");
// 			return;
// 		}
// 	}

// 	// decide to keep connection open based on HTTP response
// 	if (request.headers.find("connection") != request.headers.end())
// 	{
// 		std::string connectionValue = request.headers["connection"];
// 		trim(connectionValue);
// 		std::transform(connectionValue.begin(), connectionValue.end(), connectionValue.begin(), ::tolower);
// 		if (connectionValue == "keep-alive")
// 			clientInfoMap[client_socket]->shouldclose = false;
// 		else
// 			clientInfoMap[client_socket]->shouldclose = true;
// 	}
// 	else
// 		clientInfoMap[client_socket]->shouldclose = true;
// 	if (request.method == "GET")
// 		handleGetRequest(client_socket);
// 	else if (request.method == "POST")
// 		handlePostRequest(client_socket);
// 	else
// 		sendErrorResponse(client_socket, 501, "Not Implemented");

// 	// set up write event for client response
// 	struct kevent change;
// 	EV_SET(&change, static_cast<uintptr_t>(client_socket), EVFILT_WRITE, EV_ADD | EV_ENABLE, 0, 0, NULL);

// 	// validate fd before using
// 	if (fcntl(client_socket, F_GETFL) != -1)
// 	{
// 		if (kevent(kq, &change, 1, NULL, 0, NULL) == -1)
// 			logger.logMethod("ERROR", "Kevent registration failure for writing: " + std::string(strerror(errno)));
// 		else
// 			logger.logMethod("INFO", "Succesfully registered kevent for socket: " + std::to_string(client_socket));
// 	}
// 	else
// 	{
// 		logger.logMethod("ERROR", "Attempted to register kevent for invalid FD: " + std::to_string(client_socket));
// 		closeSocket(client_socket);
// 	}
// }
