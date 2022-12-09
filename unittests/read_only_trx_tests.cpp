#include <eosio/chain/abi_serializer.hpp>
#include <boost/test/unit_test.hpp>
#include <eosio/testing/tester.hpp>
#include <eosio/chain/global_property_object.hpp>
#include <fc/variant_object.hpp>
#include <contracts.hpp>

#ifdef NON_VALIDATING_TEST
#define TESTER tester
#else
#define TESTER validating_tester
#endif

using namespace eosio;
using namespace eosio::chain;
using namespace eosio::testing;
using namespace fc;

using mvo = fc::mutable_variant_object;

struct read_only_trx_tester : TESTER {
   read_only_trx_tester() {
      produce_block();
      create_accounts( {"noauthtable"_n, "alice"_n} );
      set_code( "noauthtable"_n, contracts::no_auth_table_wasm() );
      set_abi( "noauthtable"_n, contracts::no_auth_table_abi().data() );
      produce_block();

      insert_data = abi_ser.variant_to_binary( "insert", mutable_variant_object()
         ("user", "alice") ("id", 1) ("age", 10),
         abi_serializer::create_yield_function( abi_serializer_max_time ) );
      getage_data = abi_ser.variant_to_binary("getage", mutable_variant_object()
         ("user", "alice"),
         abi_serializer::create_yield_function( abi_serializer_max_time ));
   }

   void send_native_function_transaction(const action& act) {
      signed_transaction trx;
      trx.actions.push_back( act );
      set_transaction_headers( trx );

      push_transaction( trx, fc::time_point::maximum(), DEFAULT_BILLED_CPU_TIME_US, false, transaction_metadata::trx_type::read_only );
   }

   auto send_db_api_transaction( action_name name, bytes data, const vector<permission_level>& auth={{"alice"_n, config::active_name}}, transaction_metadata::trx_type type=transaction_metadata::trx_type::input ) {
      action act;
      signed_transaction trx;

      act.account = "noauthtable"_n;
      act.name = name;
      act.authorization = auth;
      act.data = data;

      trx.actions.push_back( act );
      set_transaction_headers( trx );
      if ( type == transaction_metadata::trx_type::input ) {
         trx.sign(get_private_key("alice"_n, "active"), control->get_chain_id());
      }

      return push_transaction( trx, fc::time_point::maximum(), DEFAULT_BILLED_CPU_TIME_US, false, type );
   }

   void insert_a_record() {
      auto res = send_db_api_transaction("insert"_n, insert_data);
      BOOST_CHECK_EQUAL(res->receipt->status, transaction_receipt::executed);
      produce_block();
   }

   abi_serializer abi_ser{ json::from_string(contracts::no_auth_table_abi().data()).as<abi_def>(), abi_serializer::create_yield_function(abi_serializer_max_time )};
   bytes insert_data;
   bytes getage_data;
};

BOOST_AUTO_TEST_SUITE(read_only_trx_tests)

BOOST_FIXTURE_TEST_CASE(newaccount_test, read_only_trx_tester) { try {
   produce_blocks( 1 );

   action act = {
      {},
      newaccount{
         .creator  = config::system_account_name,
         .name     = "alice"_n,
         .owner    = authority( get_public_key( "alice"_n, "owner" ) ),
         .active   = authority( get_public_key( "alice"_n, "active" ) )
      }
   };

   BOOST_CHECK_THROW( send_native_function_transaction(act), action_validate_exception );
} FC_LOG_AND_RETHROW() }

BOOST_FIXTURE_TEST_CASE(setcode_test, read_only_trx_tester) { try {
   produce_blocks( 1 );

   std::vector<uint8_t> code(10);
   action act = {
      {}, setcode { "eosio"_n, 0, 0, bytes(code.begin(), code.end()) }
   };

   BOOST_CHECK_THROW( send_native_function_transaction(act), action_validate_exception );
} FC_LOG_AND_RETHROW() }

BOOST_FIXTURE_TEST_CASE(setabi_test, read_only_trx_tester) { try {
   produce_blocks( 1 );

   std::vector<uint8_t> abi(10);
   action act = {
      {},
      setabi {
         .account = "alice"_n, .abi = bytes(abi.begin(), abi.end())
      }
   };

   BOOST_CHECK_THROW( send_native_function_transaction(act), action_validate_exception );
} FC_LOG_AND_RETHROW() }

BOOST_FIXTURE_TEST_CASE(updateauth_test, read_only_trx_tester) { try {
   produce_blocks( 1 );

   auto auth = authority( get_public_key( "alice"_n, "test" ) );
   action act = {
      vector<permission_level>{{config::system_account_name,config::active_name}},
      updateauth {
         .account = "alice"_n, .permission = "active"_n, .parent = "owner"_n, .auth  = auth
      }
   };

   BOOST_CHECK_THROW( send_native_function_transaction(act), transaction_exception );
} FC_LOG_AND_RETHROW() }


BOOST_FIXTURE_TEST_CASE(deleteauth_test, read_only_trx_tester) { try {
   produce_blocks( 1 );

   name account = "alice"_n;
   name permission = "active"_n;
   action act = {
      vector<permission_level>{{config::system_account_name,config::active_name}},
      deleteauth { account, permission }
   };

   BOOST_CHECK_THROW( send_native_function_transaction(act), transaction_exception );
} FC_LOG_AND_RETHROW() }

BOOST_FIXTURE_TEST_CASE(linkauth_test, read_only_trx_tester) { try {
   produce_blocks( 1 );

   name account = "alice"_n;
   name code = "eosio_token"_n;
   name type = "transfer"_n;
   name requirement = "first"_n;
   action act = {
      vector<permission_level>{{config::system_account_name,config::active_name}},
      linkauth { account, code, type, requirement }
   };

   BOOST_CHECK_THROW( send_native_function_transaction(act), transaction_exception );
} FC_LOG_AND_RETHROW() }

BOOST_FIXTURE_TEST_CASE(unlinkauth_test, read_only_trx_tester) { try {
   produce_blocks( 1 );

   name account = "alice"_n;
   name code = "eosio_token"_n;
   name type = "transfer"_n;
   action act = {
      vector<permission_level>{{config::system_account_name,config::active_name}},
      unlinkauth { account, code, type }
   };

   BOOST_CHECK_THROW( send_native_function_transaction(act), transaction_exception );
} FC_LOG_AND_RETHROW() }

BOOST_FIXTURE_TEST_CASE(db_read_only_mode_test, read_only_trx_tester) { try {
   control->set_db_read_only_mode();
   BOOST_CHECK_THROW( create_account("bob"_n), std::exception );
   control->unset_db_read_only_mode();
} FC_LOG_AND_RETHROW() }

BOOST_FIXTURE_TEST_CASE(db_insert_test, read_only_trx_tester) { try {
   // verify DB insert is not allowed by read-only transaction
   BOOST_CHECK_THROW(send_db_api_transaction("insert"_n, insert_data, {}, transaction_metadata::trx_type::read_only), table_operation_not_permitted);

   // verify DB insert still works with non-read-only transaction after read-only
   insert_a_record();
   
   // do a read-only transaction and verify the return value (age) is the same as inserted
   auto res = send_db_api_transaction("getage"_n, getage_data, {}, transaction_metadata::trx_type::read_only);
   BOOST_CHECK_EQUAL(res->receipt->status, transaction_receipt::executed);
   BOOST_CHECK_EQUAL(res->action_traces[0].return_value[0], 10);
} FC_LOG_AND_RETHROW() }

BOOST_FIXTURE_TEST_CASE(permission_test, read_only_trx_tester) { try {
   // verify read-only transaction does not allow permissions.
   BOOST_CHECK_THROW(send_db_api_transaction("getage"_n, getage_data, {{"alice"_n, config::active_name}}, transaction_metadata::trx_type::read_only), transaction_exception);
} FC_LOG_AND_RETHROW() }

BOOST_FIXTURE_TEST_CASE(db_modify_test, read_only_trx_tester) { try {
   insert_a_record();

   // verify DB update is not allowed by read-only transaction
   auto modify_data = abi_ser.variant_to_binary("modify", mutable_variant_object()
      ("user", "alice") ("age", 25),
      abi_serializer::create_yield_function( abi_serializer_max_time )
   );
   BOOST_CHECK_THROW(send_db_api_transaction("modify"_n, modify_data, {}, transaction_metadata::trx_type::read_only), table_operation_not_permitted);

   // verify DB update still works in by non-read-only transaction
   auto res = send_db_api_transaction("modify"_n, modify_data);
   BOOST_CHECK_EQUAL(res->receipt->status, transaction_receipt::executed);
   produce_block();

   // verify the value was successfully updated
   res = send_db_api_transaction("getage"_n, getage_data, {}, transaction_metadata::trx_type::read_only);
   BOOST_CHECK_EQUAL(res->receipt->status, transaction_receipt::executed);
   BOOST_CHECK_EQUAL(res->action_traces[0].return_value[0], 25);

   // verify DB update by secondary key is not allowed by read-only transaction
   auto modifybyid_data = abi_ser.variant_to_binary("modifybyid", mutable_variant_object()
      ("id", 1) ("age", 50),
      abi_serializer::create_yield_function( abi_serializer_max_time )
   );
   BOOST_CHECK_THROW(send_db_api_transaction("modifybyid"_n, modifybyid_data, {}, transaction_metadata::trx_type::read_only), table_operation_not_permitted);

   // verify DB update by secondary key still works in by non-read-only transaction
   res = send_db_api_transaction("modifybyid"_n, modifybyid_data);
   BOOST_CHECK_EQUAL(res->receipt->status, transaction_receipt::executed);
   produce_block();

   // verify the value was successfully updated
   res = send_db_api_transaction("getage"_n, getage_data, {}, transaction_metadata::trx_type::read_only);
   BOOST_CHECK_EQUAL(res->receipt->status, transaction_receipt::executed);
   BOOST_CHECK_EQUAL(res->action_traces[0].return_value[0], 50);
} FC_LOG_AND_RETHROW() }

BOOST_FIXTURE_TEST_CASE(db_erase_test, read_only_trx_tester) { try {
   insert_a_record();

   // verify DB erase is not allowed by read-only transaction
   auto erase_data = abi_ser.variant_to_binary("erase", mutable_variant_object()
      ("user", "alice"),
      abi_serializer::create_yield_function( abi_serializer_max_time )
   );
   BOOST_CHECK_THROW(send_db_api_transaction("erase"_n, erase_data, {}, transaction_metadata::trx_type::read_only), table_operation_not_permitted);

   // verify DB erase by secondary key is not allowed by read-only transaction
   auto erasebyid_data = abi_ser.variant_to_binary("erasebyid", mutable_variant_object()
      ("id", 1),
      abi_serializer::create_yield_function( abi_serializer_max_time )
   );
   BOOST_CHECK_THROW(send_db_api_transaction("erasebyid"_n, erasebyid_data, {}, transaction_metadata::trx_type::read_only), table_operation_not_permitted);

   // verify DB erase still works in by non-read-only transaction
   auto res = send_db_api_transaction("erase"_n, erase_data);
   BOOST_CHECK_EQUAL(res->receipt->status, transaction_receipt::executed);
} FC_LOG_AND_RETHROW() }

BOOST_FIXTURE_TEST_CASE(sequence_numbers_test, read_only_trx_tester) { try {
   const auto& p = control->get_dynamic_global_properties();
   auto receiver_account = control->db().find<account_metadata_object,by_name>("noauthtable"_n);
   auto amo = control->db().find<account_metadata_object,by_name>("alice"_n);

   // verify sequence numbers in state increment for non-read-only transactions
   auto prev_global_action_sequence = p.global_action_sequence;
   auto prev_recv_sequence = receiver_account->recv_sequence;
   auto prev_auth_sequence = amo->auth_sequence; 

   auto res = send_db_api_transaction("insert"_n, insert_data);
   BOOST_CHECK_EQUAL(res->receipt->status, transaction_receipt::executed);

   BOOST_CHECK_EQUAL( prev_global_action_sequence + 1, p.global_action_sequence );
   BOOST_CHECK_EQUAL( prev_recv_sequence + 1, receiver_account->recv_sequence );
   BOOST_CHECK_EQUAL( prev_auth_sequence + 1, amo->auth_sequence );
   
   produce_block();

   // verify sequence numbers in state do not change for read-only transactions
   prev_global_action_sequence = p.global_action_sequence;
   prev_recv_sequence = receiver_account->recv_sequence;
   prev_auth_sequence = amo->auth_sequence; 

   send_db_api_transaction("getage"_n, getage_data, {}, transaction_metadata::trx_type::read_only);

   BOOST_CHECK_EQUAL( prev_global_action_sequence, p.global_action_sequence );
   BOOST_CHECK_EQUAL( prev_recv_sequence, receiver_account->recv_sequence );
   BOOST_CHECK_EQUAL( prev_auth_sequence, amo->auth_sequence );
} FC_LOG_AND_RETHROW() }

BOOST_AUTO_TEST_SUITE_END()
