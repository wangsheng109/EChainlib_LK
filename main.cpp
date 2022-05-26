//#include <gtest/gtest.h>
#include <time.h>
#include <sys/resource.h>

#include "api_echain_func.h"

#include <string>
#include <string.h>

//class FooEnvironment :public testing::Environment
//{
//public:
//	virtual void SetUp(){
//		//init operation
//		websocket_server_ = new echain::WebSocketServer();
//		websocket_server_->Initialize();
//	}
//	virtual void TearDown(){
//		while (true)
//		{
//			utils::Sleep(1);
//		}
//		//end operation
//		delete websocket_server_;
//	}
//
//	echain::WebSocketServer *websocket_server_;
//};

int main(int argc, char **argv)
{
	//testing::AddGlobalTestEnvironment(new FooEnvironment);
//	testing::GTEST_FLAG(output) = "xml:gtest_result.xml";
//	testing::InitGoogleTest(&argc, argv);
	
	if(ApiInitSDK() < 0)
	{
		printf("ApiInitSDK failed!\n");
		return -1;
	}
//	std::string destAddress = argv[1];
	
	ApiGetTransactionBlobTest();
	
	return 1;
}
