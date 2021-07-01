// This file is part of Empirical, https://github.com/mercere99/Empirical/, and is
// Copyright (C) Michigan State University, 2015. It is licensed
// under the MIT Software license; see doc/LICENSE

#define CATCH_CONFIG_MAIN
#include "third-party/Catch/single_include/catch2/catch.hpp"

#include <iostream>
#include <map>
#include <optional>

#include "emp/base/assert.hpp"
#include "emp/base/vector.hpp"
#include "emp/config/ArgManager.hpp"
#include "emp/config/command_line.hpp"
#include "emp/config/config.hpp"
#include "assets/config_setup.hpp"

TEST_CASE("Test config", "[config]"){

  // test config class template
  {

    MyConfig config;
    config.Read("assets/test.cfg");

    std::cout << "Random seed = " << config.RANDOM_SEED() << std::endl;

    REQUIRE(config.RANDOM_SEED() == 333);
    REQUIRE(config.TEST_STRING() == "default");

    config.RANDOM_SEED(123);

    std::cout << "Random seed = " << config.RANDOM_SEED() << std::endl;

    REQUIRE(config.RANDOM_SEED() == 123);
    REQUIRE(config.TEST_STRING_SPACE() == "abc def   ghi");
    REQUIRE(config.TEST_STRING_QUOTE() == "\"Quote\"andonemore\"soit'sodd");

    std::stringstream query_stream;
    config.WriteUrlQueryString(query_stream);
    std::string query(query_stream.str());
    REQUIRE(query[0] == '?');
    REQUIRE(query.find("DEBUG_MODE=0") != std::string::npos);
    REQUIRE(query.find("RANDOM_SEED=123") != std::string::npos);
    REQUIRE(query.find("&TEST_BOOL=0") != std::string::npos);
    REQUIRE(query.find("TEST_CONST=91") != std::string::npos);
    REQUIRE(query.find("TEST_STRING_SPACE=%22abc%20def%20%20%20ghi%22") != std::string::npos);
    REQUIRE(query.find("TEST_STRING_QUOTE=%22%22Quote%22andonemore%22soit%27sodd%22") != std::string::npos);
    REQUIRE(query.find("MUTATION_RATE=0.025") != std::string::npos);
  }

}
