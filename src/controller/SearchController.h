#pragma once

#include <oatpp/parser/json/mapping/ObjectMapper.hpp>
#include <oatpp/web/server/api/ApiController.hpp>
#include <oatpp/core/macro/codegen.hpp>
#include <oatpp/core/macro/component.hpp>
#include <oatpp/core/async/CoroutineWaitList.hpp>
#include <oatpp/core/Types.hpp>

#include <sysearch/sysearch.h>

class SearchTask : private oatpp::async::CoroutineWaitList::Listener
{
	using Action = oatpp::async::Action;
	using Status = oatpp::web::protocol::http::Status;

public:
	SearchTask();

	Action AutocompleteAsync(sysearch_session_t session, sysearch_search_request_t request, Action&& onResultAction);
	Action LocationAsync(sysearch_session_t session, sysearch_location_request_t request, Action&& onResultAction);

	Status GetStatusCode();
	oatpp::Any GetReponse();

private:
	std::atomic<bool> mFinished{ false };
	oatpp::async::CoroutineWaitList mWaitList;
	Status mStatusCode{ Status::CODE_500 };
	oatpp::Any mResponseDto;

	void onNewItem(oatpp::async::CoroutineWaitList& list) override;

	static void OnAutocompleteResult(const sysearch_result_t* results, size_t n_results, sysearch_callback_data_t callback_data);
	static void OnLocationResult(const sysearch_result_t result, sysearch_callback_data_t callback_data);
	static void OnError(sysearch_status_e status, sysearch_callback_data_t callback_data);
};

#include OATPP_CODEGEN_BEGIN(ApiController)

class SearchController : public oatpp::web::server::api::ApiController {
public:
	using __ControllerType = SearchController;

	SearchController(OATPP_COMPONENT(std::shared_ptr<ObjectMapper>, objectMapper));

	ENDPOINT_ASYNC("GET", "/autocomplete", Autocomplete)
	{
		ENDPOINT_ASYNC_INIT(Autocomplete);

		Action act() override;
		Action DoSearch();
		Action OnSearchTaskFinished();

	private:
		sysearch_session_t mSession;
		sysearch_search_request_t mRequest;
		std::string mDecodedQuery;
		SearchTask mSearchTask;
	};

	ENDPOINT_ASYNC("GET", "/location", GeocodeLocation)
	{
		ENDPOINT_ASYNC_INIT(GeocodeLocation);

		Action act() override;
		Action DoSearch();
		Action OnSearchTaskFinished();

	private:
		sysearch_session_t mSession;
		sysearch_location_request_t mRequest;
		std::string mDecodedLocationId;
		SearchTask mSearchTask;
	};

private:
	sysearch_t mSearch{};
};

#include OATPP_CODEGEN_END(ApiController)