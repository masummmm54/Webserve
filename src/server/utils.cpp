#include "HttpServer.hpp"

std::string HttpServer::getErrorFilePath(int statusCode)
{
	std::string path = "errors/";
	switch (statusCode)
	{
	case 404:
		return (path + "404.html");
	case 500:
		return (path + "500.html");
	default:
		return (path + "default.html");
	}
}

void HttpServer::sendErrorResponse(int client_socket, int statusCode, const std::string &reasonPhrase)
{
	std::string errorFilePath = getErrorFilePath(statusCode);
	std::string htmlContent = readFileContent(errorFilePath, client_socket);

	if (htmlContent.empty())
	{
		htmlContent = "<html><head><title>Error</title></head><body><h1>" + std::to_string(statusCode) + " " + reasonPhrase + "</h1><p>The requested method is not supported.</p></body></html>";
	}

	std::string response = formatHttpResponse(statusCode, reasonPhrase, htmlContent);
	clientInfoMap[client_socket].response = response;
	writeResponse(client_socket);
}

void	HttpServer::log(const std::string& level, const std::string& msg, int client_socket)
{
	std::time_t currentTime = std::time(0);
	std::tm* localTime = std::localtime(&currentTime);

	char timestamp[20];
	std::strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", localTime);

	(void)client_socket;
	std::ofstream logFile("log.txt", std::ios_base::app);
	if (logFile.is_open())
	{
		logFile << timestamp << " [" << level << "]" << " - " << msg << std::endl;
		logFile.close();
	}
	else
		std::cerr << "Unable to open log file" << std::endl;
	std::cout << timestamp << " [" << level << "]" << " - " << msg  << std::endl;
	logFile.close();
}

std::string HttpServer::getFilePath(const std::string& uri)
{
	return ("html" + uri);
}
