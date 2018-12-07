#include "capi_helpers.hpp"
#include "catch.hpp"

#include <vector>

using namespace std;

TEST_CASE("Simple In-Memory DB Start Up and Shutdown", "[simplestartup]") {
	duckdb_database database;
	duckdb_connection connection;

	// open and close a database in in-memory mode
	REQUIRE(duckdb_open(NULL, &database) == DuckDBSuccess);
	REQUIRE(duckdb_connect(database, &connection) == DuckDBSuccess);
	REQUIRE(duckdb_disconnect(connection) == DuckDBSuccess);
	REQUIRE(duckdb_close(database) == DuckDBSuccess);
}

TEST_CASE("Multiple In-Memory DB Start Up and Shutdown", "[multiplestartup]") {
	duckdb_database database[10];
	duckdb_connection connection[100];

	// open and close 10 databases
	// and open 10 connections per database
	for (size_t i = 0; i < 10; i++) {
		REQUIRE(duckdb_open(NULL, &database[i]) == DuckDBSuccess);
		for (size_t j = 0; j < 10; j++) {
			REQUIRE(duckdb_connect(database[i], &connection[i * 10 + j]) == DuckDBSuccess);
		}
	}
	for (size_t i = 0; i < 10; i++) {
		for (size_t j = 0; j < 10; j++) {
			REQUIRE(duckdb_disconnect(connection[i * 10 + j]) == DuckDBSuccess);
		}
		REQUIRE(duckdb_close(database[i]) == DuckDBSuccess);
	}
}