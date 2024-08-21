#ifndef HTTPSERVER_HPP
# define HTTPSERVER_HPP

#include "server.hpp"
#include "log.hpp"
#include "database.hpp"

class HttpServer : public config
{
	private:
		// variables;
		std::unordered_map<int, Server*> servers;
		int			kq;

		struct sockaddr_in	address;

		std::unordered_map<int, ClientInfo> clientInfoMap;
		std::set<int> openSockets;

		Logger& logger;
		Database& database;

		// methods
		void	init();
		void	mainLoop();

		// connection handlers
		void	acceptConnection(int serverSocket);
		void	setupKevent(int client_socket, int timeoutSeconds);
		void	configureSocketNonBlocking(int client_socket);
		void	closeSocket(int client_socket);
		void	modifyEvent(int fd, int filter, int flags);
		void	logSocketAction(const std::string& action, int fd);

		// request
		void	readRequest(int client_socket);
		void	handleRequest(int client_Socket);

		bool	validateServer(int client_socket);
		bool	validateRouteAndMethod(int client_socket, const HttpRequest &request);
		void	decideConnectionPersistence(int client_socket, const HttpRequest &request);
		void	processRequestMethod(int client_socket);
		void	registerWriteEvent(int client_socket);

		// content
		void	writeResponse(int client_socket);
		std::string getFilePath(int server_fd, const std::string &uri);
		std::string readFileContent(const std::string& filePath);

		// parse
		bool	parseHttpRequest(const std::string& requestStream, HttpRequest& request, int client_socket);
		bool	parseHttpRequestBody(std::istringstream& requestStream, HttpRequest& request, int client_socket);
		bool	parseHttpRequestHeaders(std::istringstream& requestStream, HttpRequest& request);

		// response
		void	sendRedirectResponse(int client_socket, const std::string &redirectUrl);
		std::string formatHttpResponse(const std::string& httpVersion, int status_code, const std::string& reasonPhrase,
		const std::string& body, int keepAlive);

		// GET
		void	handleGetRequest(int client_socket);

		// DELETE
		void	handleDeleteRequest(int client_socket);

		// POST
		void	handlePostRequest(int client_socket);
		std::map<std::string, std::string>	parseFormData(const std::string& body);

		//directory listing
		void	handleDirectoryListing(int client_socket, const std::string &dirPath);
		bool	isDirectory(const std::string &path);
		bool	fileExists(const std::string &path);
		std::vector<std::string> listDirectory(const std::string &directoryPath);

		// Error
		void		sendErrorResponse(int client_socket, int statusCode, const std::string &reasonPhrase);
		std::string	getErrorFilePath(int statusCode, int serverFd);
		std::string replacePlaceholder(const std::string &content, const std::string &placeholder, const std::string &value);

		// Generate Profile
		std::string	generateProfilePage(const userProfile& profile);
		void	generateAllProfilesPage(int client_socket);
		bool	findProfileByID(const std::string& uri, int client_socket);

		// Utils
		void	trim(std::string& str);
		int		getMaxClientBodySize(int client_socket);


	public:
		HttpServer(std::string confpath, Logger& loggerRef, Database& databaseRef);
		~HttpServer();
};

#endif
