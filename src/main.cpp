#include <oatpp/web/server/HttpConnectionHandler.hpp>
#include <oatpp/network/tcp/server/ConnectionProvider.hpp>
#include <oatpp/network/Server.hpp>
#include <oatpp/web/protocol/http/outgoing/StreamingBody.hpp>
#include <oatpp/core/data/stream/FileStream.hpp>
#include <oatpp/parser/json/mapping/ObjectMapper.hpp>
#include <oatpp/web/server/api/ApiController.hpp>
#include <oatpp/core/macro/codegen.hpp>
#include <oatpp/core/macro/component.hpp>

#include <filesystem>

class AppComponent {
public:

	OATPP_CREATE_COMPONENT(std::shared_ptr<oatpp::network::ServerConnectionProvider>, serverConnectionProvider)([] {
		return oatpp::network::tcp::server::ConnectionProvider::createShared({ "0.0.0.0", 80, oatpp::network::Address::IP_4 });
	}());

	OATPP_CREATE_COMPONENT(std::shared_ptr<oatpp::web::server::HttpRouter>, httpRouter)([] {
		return oatpp::web::server::HttpRouter::createShared();
	}());

	OATPP_CREATE_COMPONENT(std::shared_ptr<oatpp::network::ConnectionHandler>, serverConnectionHandler)([] {
		OATPP_COMPONENT(std::shared_ptr<oatpp::web::server::HttpRouter>, router);
		return oatpp::web::server::HttpConnectionHandler::createShared(router);
	}());

	OATPP_CREATE_COMPONENT(std::shared_ptr<oatpp::data::mapping::ObjectMapper>, apiObjectMapper)([] {
		return oatpp::parser::json::mapping::ObjectMapper::createShared();
	}());
};

#include OATPP_CODEGEN_BEGIN(ApiController)

class Controller : public oatpp::web::server::api::ApiController {
public:
	Controller(OATPP_COMPONENT(std::shared_ptr<ObjectMapper>, objectMapper))
		: oatpp::web::server::api::ApiController(objectMapper)
	{}

	ENDPOINT("GET", "/getfile", getFile)
	{
		if (auto filePath = std::getenv("PATH_TO_FILE"))
		{
			if (std::filesystem::exists(filePath))
			{
				auto fileStream = std::make_shared<oatpp::data::stream::FileInputStream>(filePath);
				auto responseBody = std::make_shared<oatpp::web::protocol::http::outgoing::StreamingBody>(fileStream);
				return OutgoingResponse::createShared(Status::CODE_200, responseBody);

			}
			else
			{
				using namespace std::string_literals;
				return createResponse(Status::CODE_404, { "Could not find "s + filePath });
			}
		}
		return createResponse(Status::CODE_404, { "Path to file not set" });
	}
};

#include OATPP_CODEGEN_END(ApiController)

void RunServer()
{
	AppComponent components;

	OATPP_COMPONENT(std::shared_ptr<oatpp::web::server::HttpRouter>, router);
	router->addController(std::make_shared<Controller>());

	OATPP_COMPONENT(std::shared_ptr<oatpp::network::ConnectionHandler>, connectionHandler);
	OATPP_COMPONENT(std::shared_ptr<oatpp::network::ServerConnectionProvider>, connectionProvider);
	oatpp::network::Server server(connectionProvider, connectionHandler);

	OATPP_LOGI("App", "Server running on port %s", connectionProvider->getProperty("port").getData());

	server.run();
}

int main()
{
	oatpp::base::Environment::init();

	RunServer();

	oatpp::base::Environment::destroy();

	return 0;
}
