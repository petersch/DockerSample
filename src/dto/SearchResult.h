#pragma once

#include <oatpp/core/Types.hpp>
#include <oatpp/core/data/mapping/type/Enum.hpp>
#include <oatpp/core/macro/codegen.hpp>

#include <sysearch/sysearch.h>

#include OATPP_CODEGEN_BEGIN(DTO)

namespace Search::Dto
{
	ENUM(ResultType, v_int32,
		VALUE(coordinate, sysearch_result_type_e::SYSEARCH_RESULT_COORDINATE, "coord"),
		VALUE(flat_data, sysearch_result_type_e::SYSEARCH_RESULT_FLAT_DATA)
	);

	class Location : public oatpp::DTO
	{
		DTO_INIT(Location, DTO);
		DTO_FIELD(Float64, lat);
		DTO_FIELD(Float64, lon);
	};

	class Result : public oatpp::DTO
	{
		DTO_INIT(Result, DTO);
		DTO_FIELD(String, title);
		DTO_FIELD(String, subtitle);
		DTO_FIELD(Enum<ResultType>, resultType);
	};

	class AutocompleteResult : public Result
	{
		DTO_INIT(AutocompleteResult, Result);
		DTO_FIELD(String, locationId);
	};

	class GeocodingResult : public Result
	{
		DTO_INIT(GeocodingResult, Result);
		DTO_FIELD(Object<Location>, location);
	};

	class AutocompleteResponse : public oatpp::DTO
	{
		DTO_INIT(AutocompleteResponse, DTO);
		DTO_FIELD(Vector<Object<AutocompleteResult>>, results);
	};

	class LocationResponse : public oatpp::DTO
	{
		DTO_INIT(LocationResponse, DTO);
		DTO_FIELD(Object<GeocodingResult>, result);
	};

	GeocodingResult::Wrapper ToGeocodingResult(sysearch_result_t result);
	oatpp::Vector<AutocompleteResult::Wrapper> ToAutocompleteResults(const sysearch_result_t* results, size_t n_results);
}

#include OATPP_CODEGEN_END(DTO)