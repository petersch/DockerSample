#include "SearchController.h"

#include <dto/SearchResult.h>

#include <sstream>

namespace
{
	char FromHex(char ch)
	{
		if (ch <= 9) return ch - '0';
		if (ch <= 'Z') return ch - 'A' + 10;
		return ch - 'a' + 10;
	}

	std::string UrlDecode(std::string str)
	{
		std::ostringstream os;
		os.fill('0');

		for (auto begin = str.begin(), end = str.end(); begin != end; ++begin)
		{
			const auto ch = *begin;

			if (ch == '%')
			{
				if (begin + 3 <= end)
				{
					os << (FromHex(begin[1]) << 4 | FromHex(begin[2]));
					std::advance(begin, 2);
				}
			}
			else if (ch == '+')
			{
				os << ' ';
			}
			else
			{
				os << ch;
			}
		}

		return os.str();
	}
}

void SearchTask::onNewItem(oatpp::async::CoroutineWaitList& list)
{
	if (mFinished)
	{
		list.notifyFirst();
	}
}

SearchTask::SearchTask()
{
	mWaitList.setListener(this);
}

void SearchTask::OnAutocompleteResult(const sysearch_result_t* results, size_t n_results, sysearch_callback_data_t callback_data)
{
	auto response = Search::Dto::AutocompleteResponse::createShared();
	oatpp::Vector<Search::Dto::AutocompleteResult::Wrapper> v;
	response->results = Search::Dto::ToAutocompleteResults(results, n_results);

	auto& searchTask = *reinterpret_cast<SearchTask*>(callback_data);
	searchTask.mStatusCode = Status::CODE_200;
	searchTask.mResponseDto = std::move(response);
	searchTask.mFinished.store(true, std::memory_order_release);
	searchTask.mWaitList.notifyFirst();
}

void SearchTask::OnLocationResult(const sysearch_result_t result, sysearch_callback_data_t callback_data)
{
	auto response = Search::Dto::LocationResponse::createShared();
	oatpp::Vector<Search::Dto::AutocompleteResult::Wrapper> v;
	response->result = Search::Dto::ToGeocodingResult(result);

	auto& searchTask = *reinterpret_cast<SearchTask*>(callback_data);
	searchTask.mStatusCode = Status::CODE_200;
	searchTask.mResponseDto = std::move(response);
	searchTask.mFinished.store(true, std::memory_order_release);
	searchTask.mWaitList.notifyFirst();
}

void SearchTask::OnError(sysearch_status_e status, sysearch_callback_data_t callback_data)
{
	auto& searchTask = *reinterpret_cast<SearchTask*>(callback_data);

	switch (status)
	{
	case sysearch_status_e::SYSEARCH_STATUS_INVALID_LOCATION_ID:
		searchTask.mStatusCode = Status::CODE_400;
		searchTask.mResponseDto = oatpp::String{ "Invalid location ID" };
		break;

	case sysearch_status_e::SYSEARCH_STATUS_INVALID_SESSION_HANDLE:
	case sysearch_status_e::SYSEARCH_STATUS_INVALID_SESSION_STATE:
		searchTask.mStatusCode = Status::CODE_400;
		searchTask.mResponseDto = oatpp::String{ "Invalid session" };
		break;
	}

	searchTask.mFinished.store(true, std::memory_order_release);
	searchTask.mWaitList.notifyFirst();
}

SearchTask::Status SearchTask::GetStatusCode()
{
	return mStatusCode;
}

oatpp::Any SearchTask::GetReponse()
{
	return mResponseDto;
}

SearchTask::Action SearchTask::AutocompleteAsync(sysearch_session_t session, sysearch_search_request_t request, Action&& onResultAction)
{
	if (mFinished.load(std::memory_order_acquire))
	{
		return std::forward<Action>(onResultAction);
	}

	sysearch_autocomplete(session, &request, &OnAutocompleteResult, &OnError, reinterpret_cast<sysearch_callback_data_t>(this));

	return Action::createWaitListAction(&mWaitList);

}

SearchTask::Action SearchTask::LocationAsync(sysearch_session_t session, sysearch_location_request_t request, Action&& onResultAction)
{
	if (mFinished.load(std::memory_order_acquire))
	{
		return std::forward<Action>(onResultAction);
	}

	// TODO: remove when session can be replaced with session ID
	sysearch_search_request_t autoreq;
	sysearch_search_request_init(&autoreq);
	sysearch_autocomplete(session, &autoreq, [](const sysearch_result_t*, size_t, sysearch_callback_data_t) {}, [](sysearch_status_e, sysearch_callback_data_t) {}, {});

	sysearch_geocode_location(session, &request, &OnLocationResult, &OnError, reinterpret_cast<sysearch_callback_data_t>(this));

	return Action::createWaitListAction(&mWaitList);
}

SearchController::SearchController(std::shared_ptr<ObjectMapper>& objectMapper)
	: oatpp::web::server::api::ApiController(objectMapper)
{
	sysearch_flat_data_search_create(&mSearch);

	sysearch_flat_data_t flatData;

	flatData.location = { 48.11, 17.12 };
	flatData.title = "Flat Data";
	flatData.subtitle = "Subtitle";
	sysearch_flat_data_search_add_item(mSearch, &flatData);

	flatData.location = { 48.20, 17.15 };
	flatData.title = "Other Flat Data";
	flatData.subtitle = "Something else";
	sysearch_flat_data_search_add_item(mSearch, &flatData);
}

oatpp::async::Action SearchController::Autocomplete::act()
{
	sysearch_search_session_create(controller->mSearch, &mSession);
	sysearch_search_request_init(&mRequest);

	if (auto query = request->getQueryParameter("q"))
	{
		mDecodedQuery = UrlDecode(*query);
		mRequest.input_query = mDecodedQuery.c_str();
	}

	return yieldTo(&Autocomplete::DoSearch);
}

oatpp::async::Action SearchController::Autocomplete::DoSearch()
{
	return mSearchTask.AutocompleteAsync(mSession, mRequest, yieldTo(&Autocomplete::OnSearchTaskFinished));
}

oatpp::async::Action SearchController::Autocomplete::OnSearchTaskFinished()
{
	auto status = mSearchTask.GetStatusCode();
	auto response = mSearchTask.GetReponse();
	return _return(controller->createDtoResponse(status, response));
}

oatpp::async::Action SearchController::GeocodeLocation::act()
{
	sysearch_search_session_create(controller->mSearch, &mSession);
	sysearch_location_request_init(&mRequest);

	if (auto locationId = request->getQueryParameter("id"))
	{
		mDecodedLocationId = UrlDecode(*locationId);
		mRequest.location_id = mDecodedLocationId.c_str();
	}

	return yieldTo(&GeocodeLocation::DoSearch);
}

oatpp::async::Action SearchController::GeocodeLocation::DoSearch()
{
	return mSearchTask.LocationAsync(mSession, mRequest, yieldTo(&GeocodeLocation::OnSearchTaskFinished));
}

oatpp::async::Action SearchController::GeocodeLocation::OnSearchTaskFinished()
{
	auto status = mSearchTask.GetStatusCode();
	auto response = mSearchTask.GetReponse();
	return _return(controller->createDtoResponse(status, response));
}
