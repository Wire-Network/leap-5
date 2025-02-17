#include <array>
#include <boost/test/unit_test.hpp>
#include <sysio/chain/application.hpp>
#include <sysio/chain_plugin/chain_plugin.hpp>
#include <stdint.h>

BOOST_AUTO_TEST_CASE(chain_plugin_default_tests) {
   fc::temp_directory  tmp;
   appbase::scoped_app app;

   auto tmp_path = tmp.path().string();
   std::array          args = {
       "test_chain_plugin", "--blocks-log-stride", "10", "--data-dir", tmp_path.c_str(),
   };

   BOOST_CHECK(app->initialize<sysio::chain_plugin>(args.size(), const_cast<char**>(args.data())));
   auto& plugin = app->get_plugin<sysio::chain_plugin>();

   auto* config = std::get_if<sysio::chain::partitioned_blocklog_config>(&plugin.chain_config().blog);
   BOOST_REQUIRE(config);
   BOOST_CHECK_EQUAL(config->max_retained_files, UINT32_MAX);
}