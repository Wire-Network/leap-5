#include <sysio/contract.hpp>

extern "C" __attribute__((sysio_wasm_import)) void set_wasm_parameters_packed(const void*, std::size_t);
#if defined(USE_SYSIO_CDT_1_7_X) || defined(USE_SYSIO_CDT_1_8_X)
extern "C" __attribute__((sysio_wasm_import)) uint32_t read_action_data( void* msg, uint32_t len );
extern "C" __attribute__((sysio_wasm_import))    uint32_t action_data_size();
#endif

struct wasm_config {
   std::uint32_t max_mutable_global_bytes;
   std::uint32_t max_table_elements;
   std::uint32_t max_section_elements;
   std::uint32_t max_linear_memory_init;
   std::uint32_t max_func_local_bytes;
   std::uint32_t max_nested_structures;
   std::uint32_t max_symbol_bytes;
   std::uint32_t max_module_bytes;
   std::uint32_t max_code_bytes;
   std::uint32_t max_pages;
   std::uint32_t max_call_depth;
};

struct internal_config {
   uint32_t version;
   wasm_config config;
};

class [[sysio::contract]] wasm_config_bios : public sysio::contract {
 public:
   using contract::contract;
   [[sysio::action]] void setwparams(const wasm_config& cfg) {
      internal_config config{0, cfg};
      set_wasm_parameters_packed(&config, sizeof(config));
   }
};
