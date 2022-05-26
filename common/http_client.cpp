
#include "http_client.h"

ECHAINS::HttpClient::HttpClient()
{
}

ECHAINS::HttpClient::~HttpClient()
{
}

ECHAINS::HttpClient::RecvMessage ECHAINS::HttpClient::http_request(HTTP_REQUEST sendtype,std::string path, std::string content)
{
	try
	{
		RecvMessage rec;
		rec.status_code = 1;
		std::ostringstream retSS;
		
		boost::asio::io_service io_service;
		boost::asio::ip::tcp::socket socket(io_service);
		
		boost::asio::ip::tcp::endpoint endpoint(boost::asio::ip::address::from_string(ip_),
			port_);
	
		socket.connect(endpoint);
		
		// Form the request. We specify the "Connection: close" header so that
		// the
		// server will close the socket after transmitting the response. This
		// will
		// allow us to treat all data up until the EOF as the content.
		boost::asio::streambuf request;
		std::ostream request_stream(&request);
				
	//	NBDLog("request path is %s!\n", path.c_str());
		
		if(sendtype == HTTP_POST)
		{
			request_stream << "POST " << path << " HTTP/1.1\r\n";
			request_stream << "Host: " << ip_ << "\r\n";
			request_stream << "Accept: " << "*/*" << "\r\n";
			request_stream << "User-Agent: Mozilla/4.0 (compatible; MSIE 6.0; Windows NT 5.1; SV1)";
			request_stream << "Content-Type: application/json\r\n";
			request_stream << "Content-Length: "<<content.size()<<"\r\n";
			request_stream << "Connection: close\r\n";
			request_stream << "\r\n";
			request_stream << content;
		}
		else if(sendtype == HTTP_GET)
		{
			
			request_stream << "GET " << path << " HTTP/1.0\r\n";
			request_stream << "Host: " << ip_ << "\r\n";
			request_stream << "Accept: */*\r\n";
			request_stream << "Connection: close\r\n\r\n";
		}
		else
		{
			return rec;
		}
		
		int request_size = request.size();  
    //将boost::asio::streambuf转为std::string  
    boost::asio::streambuf::const_buffers_type cbt1 = request.data();  
    std::string request_data(boost::asio::buffers_begin(cbt1), boost::asio::buffers_end(cbt1));  
    //  
  //  std::cout << request_data << std::endl;  
		
		
		//Send the request.
		boost::asio::write(socket, request);
		//socket.send(request);
		// Read the response status line. The response streambuf will
		// automatically
		// grow to accommodate the entire line. The growth may be limited by
		// passing
		// a maximum size to the streambuf constructor.
		boost::asio::streambuf response;
		boost::asio::read_until(socket, response, "\r\n");
		//socket.read_some(response);
		// Check that response is OK.
		
		int response_size = response.size();  
    //将boost::asio::streambuf转为std::string  
    boost::asio::streambuf::const_buffers_type cbt2 = response.data();  
    std::string response_data(boost::asio::buffers_begin(cbt2), boost::asio::buffers_end(cbt2));  
    	
		std::istream response_stream(&response);
		response_stream >> rec.http_version;
		response_stream >> rec.status_code;
		std::string status_message;
		std::getline(response_stream, status_message);
		if (!response_stream || rec.http_version.substr(0, 5) != "HTTP/")
		{
			 std::cout << "Invalid response ";
			return rec;
		}
		if (rec.status_code != 200)
		{
			std::cout << "Response returned with status code "<<rec.status_code;
			return rec;
		}
		
		// Read the response headers, which are terminated by a blank line.
		boost::asio::read_until(socket, response, "\r\n\r\n");

		std::string header;
		
		// Process the response headers.
		
		while (std::getline(response_stream, header) && header != "\r"){}

		if (response.size() > 0)
			retSS << &response;

		// Read until EOF, writing data to output as we go.
		boost::system::error_code error;
		while (boost::asio::read(socket, response, boost::asio::transfer_at_least(1), error))
			retSS << &response;
		if (error != boost::asio::error::eof)
			throw boost::system::system_error(error);
	//	std::cout << response_data << std::endl;  
		rec.context = retSS.str();
		return rec;
	}
	catch (std::exception& e)
	{
	//	std::cout << "\nException " << e.what()<<std::endl;
		RecvMessage rec;
		rec.status_code = 1;
		printf("1010101011010101\n");	
		return rec;
	}
}

bool ECHAINS::HttpClient::Initialize(std::string address){
	do
	{
		if (address.empty())break;

		utils::StringVector ip_array = utils::String::Strtok(address, ':');

		ip_ = address;
		port_ = 19333;
		if (ip_array.size() > 1){
			port_ = utils::String::Stoui(ip_array[1]);
			ip_ = ip_array[0];
		}
		return true;
	} while (true);
	return false;
}