#pragma once

#include <oatpp/network/tcp/server/ConnectionProvider.hpp>
#include <oatpp/web/server/AsyncHttpConnectionHandler.hpp>
#include <oatpp/parser/json/mapping/ObjectMapper.hpp>
#include <oatpp/core/macro/component.hpp>

class AppComponent
{
public:
	OATPP_CREATE_COMPONENT(std::shared_ptr<oatpp::network::ServerConnectionProvider>, serverConnectionProvider)([] {
		return oatpp::network::tcp::server::ConnectionProvider::createShared({ "0.0.0.0", 80, oatpp::network::Address::IP_4 });
	}());

	OATPP_CREATE_COMPONENT(std::shared_ptr<oatpp::web::server::HttpRouter>, httpRouter)([] {
		return oatpp::web::server::HttpRouter::createShared();
	}());

	OATPP_CREATE_COMPONENT(std::shared_ptr<oatpp::network::ConnectionHandler>, serverConnectionHandler)([] {
		OATPP_COMPONENT(std::shared_ptr<oatpp::web::server::HttpRouter>, router);
		return oatpp::web::server::AsyncHttpConnectionHandler::createShared(router);
	}());

	OATPP_CREATE_COMPONENT(std::shared_ptr<oatpp::data::mapping::ObjectMapper>, apiObjectMapper)([]
	{
		auto serializeConfig = oatpp::parser::json::mapping::Serializer::Config::createShared();
		auto deserializeConfig = oatpp::parser::json::mapping::Deserializer::Config::createShared();

		serializeConfig->useBeautifier = true;

		return oatpp::parser::json::mapping::ObjectMapper::createShared(serializeConfig, deserializeConfig);
	}());
};