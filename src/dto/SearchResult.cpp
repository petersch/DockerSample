#include "SearchResult.h"

using namespace Search::Dto;

namespace
{
	using StringGetter = size_t(sysearch_result_t, char*, size_t);
	using LocationGetter = int(sysearch_result_t result, sysearch_location_t* location);

	oatpp::String GetString(sysearch_result_t result, StringGetter f)
	{
		auto size = f(result, nullptr, 0);
		std::string str(size, '\0');
		f(result, str.data(), size);
		return str;
	}

	Location::Wrapper GetLocation(sysearch_result_t result, LocationGetter f)
	{
		auto dto = Location::createShared();
		sysearch_location_t location;
		if (0 == f(result, &location) && sysearch_location_is_valid(&location))
		{
			dto->lat = location.lat;
			dto->lon = location.lon;
		}
		return dto;
	}

	void AddResultData(sysearch_result_t result, Result* dto)
	{
		dto->resultType = static_cast<ResultType>(sysearch_result_get_type(result));
		dto->title = GetString(result, &sysearch_result_get_title);
		dto->subtitle = GetString(result, &sysearch_result_get_subtitle);
	}

	void AddAutocompleteResultData(sysearch_result_t result, AutocompleteResult::Wrapper& dto)
	{
		dto->subtitle = GetString(result, &sysearch_result_get_location_id);
	}

	void AddGeocodingResultData(sysearch_result_t result, GeocodingResult::Wrapper& dto)
	{
		dto->location = GetLocation(result, &sysearch_result_get_location);
	}

	AutocompleteResult::Wrapper ConvertAutocompleteResult(sysearch_result_t result)
	{
		auto dto = AutocompleteResult::createShared();
		AddResultData(result, dto.get());
		AddAutocompleteResultData(result, dto);
		return dto;
	}
}

GeocodingResult::Wrapper Search::Dto::ToGeocodingResult(sysearch_result_t result)
{
	auto dto = GeocodingResult::createShared();
	AddResultData(result, dto.get());
	AddGeocodingResultData(result, dto);
	return dto;
}

oatpp::Vector<AutocompleteResult::Wrapper> Search::Dto::ToAutocompleteResults(const sysearch_result_t* results, size_t n_results)
{
	auto dto = oatpp::Vector<AutocompleteResult::Wrapper>::createShared();
	dto->reserve(n_results);

	for (size_t i = 0; i < n_results; ++i)
	{
		dto->push_back(ConvertAutocompleteResult(results[i]));
	}
	return dto;
}