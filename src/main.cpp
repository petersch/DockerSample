#include <oatpp/network/Server.hpp>

#include <sysearch/sysearch.h>
#include <sysearch/sysearchserver.h>

#include <syl/thread_pool.h>
#include <syl/future/synchronization_context.h>
#include <syl/future/executor.h>

#include <AppComponent.h>
#include <controller/SearchController.h>

#include <csignal>

namespace
{
	static volatile sig_atomic_t gIsTerminated = 0;

	void SetTerminatedFlag([[maybe_unused]] int signal)
	{
		gIsTerminated = 1;
	}

	bool IsServerEnabled()
	{
		return !gIsTerminated;
	}

	syl::lf_thread_pool& GetThreadPool()
	{
		static syl::lf_thread_pool threadPool{ "SearchThreadPool", 8 };
		return threadPool;
	}

	syl::synchronization_context& GetExecutor()
	{
		static syl::executor_t executor{ GetThreadPool(), 0 };
		return executor;
	}

	namespace SearchExecutor
	{
		void Async(sysearch_task_callback_t callback, sysearch_callback_data_t callback_data)
		{
			GetExecutor().post([callback, callback_data] { callback(callback_data); });
		}

		void Sync(sysearch_task_callback_t callback, sysearch_callback_data_t callback_data)
		{
			callback(callback_data);
		}
	}
}

void RunServer()
{
	AppComponent components;

	OATPP_COMPONENT(std::shared_ptr<oatpp::web::server::HttpRouter>, router);
	router->addController(std::make_shared<SearchController>());

	OATPP_COMPONENT(std::shared_ptr<oatpp::network::ConnectionHandler>, connectionHandler);
	OATPP_COMPONENT(std::shared_ptr<oatpp::network::ServerConnectionProvider>, connectionProvider);
	oatpp::network::Server server(connectionProvider, connectionHandler);

	OATPP_LOGI("App", "Server running on port %s", connectionProvider->getProperty("port").getData());

	server.run([] { return IsServerEnabled(); });

	OATPP_LOGI("App", "Server stopped");
}

int main()
{
	std::signal(SIGTERM, SetTerminatedFlag);
	std::signal(SIGINT, SetTerminatedFlag);
	std::signal(SIGABRT, SetTerminatedFlag);

	GetThreadPool().try_create();

	oatpp::base::Environment::init();

	sysearch_module_init(SearchExecutor::Async);

	RunServer();

	sysearch_module_deinit();

	oatpp::base::Environment::destroy();

	return 0;
}
