#ifndef LV1_H
#define LV1_H

#include "../Types.h"
#include "LV1Calls.h"

namespace lv1 {

enum class status_code : int32_t 
{
    LV1_SUCCESS                     =  0,
    LV1_RESOURCE_SHORTAGE           = -2,
    LV1_NO_PRIVILEGE                = -3,
    LV1_DENIED_BY_POLICY            = -4,
    LV1_ACCESS_VIOLATION            = -5,
    LV1_NO_ENTRY                    = -6,
    LV1_DUPLICATE_ENTRY             = -7,
    LV1_TYPE_MISMATCH               = -8,
    LV1_BUSY                        = -9,
    LV1_EMPTY                       = -10,
    LV1_WRONG_STATE                 = -11,
    LV1_NO_MATCH                    = -13,
    LV1_ALREADY_CONNECTED           = -14,
    LV1_UNSUPPORTED_PARAMETER_VALUE = -15,
    LV1_CONDITION_NOT_SATISFIED     = -16,
    LV1_ILLEGAL_PARAMETER_VALUE     = -17,
    LV1_BAD_OPTION                  = -18,
    LV1_IMPLEMENTATION_LIMITATION   = -19,
    LV1_NOT_IMPLEMENTED             = -20,
    LV1_INVALID_CLASS_ID            = -21,
    LV1_CONSTRAINT_NOT_SATISFIED    = -22,
    LV1_ALIGNMENT_ERROR             = -23,
    LV1_HARDWARE_ERROR              = -24,
    LV1_INVALID_DATA_FORMAT         = -25,
    LV1_INVALID_OPERATION           = -26,
    LV1_INTERNAL_ERROR              = -32768,
};

union ps3_firmware_version 
{  
    struct 
    {
        uint16_t pad;
        uint16_t major;
        uint16_t minor;
        uint16_t revision;
    };

    uint64_t packed;

    ps3_firmware_version(uint16_t major, uint16_t minor, uint16_t revision) : pad(0), major(major), minor(minor), revision(revision) {}
    ps3_firmware_version() : packed(0) {}

    bool operator>(const ps3_firmware_version& Other)  const { return this->packed > Other.packed; }
    bool operator<(const ps3_firmware_version& Other)  const { return this->packed < Other.packed; }
    bool operator==(const ps3_firmware_version& Other) const { return this->packed == Other.packed; }
};

enum 
{
    PAGE_SHIFT_4K = 12U,
    PAGE_SHIFT_64K = 16U,
    PAGE_SHIFT_16M = 24U,
};

enum 
{
    HTAB_SIZE_MAX = 20U, /* HV limit of 1MB */
    HTAB_SIZE_MIN = 18U, /* CPU limit of 256KB */
};

enum 
{
    ALLOCATE_MEMORY_TRY_ALT_UNIT = 0X04,
    ALLOCATE_MEMORY_ADDR_ZERO = 0X08,
};

namespace hypercall 
{
    #define LV1_CALL(name, inNum, outNum, hyperCallIndex) constexpr auto _##name = HyperCall<hyperCallIndex, inNum, outNum>{};

    LV1_CALL( allocate_memory,                                  4, 2,   0 )
    LV1_CALL( write_htab_entry,                                 4, 0,   1 )
    LV1_CALL( construct_virtual_address_space,                  3, 2,   2 )
    LV1_CALL( invalidate_htab_entries,                          5, 0,   3 )
    LV1_CALL( get_virtual_address_space_id_of_ppe,              1, 1,   4 )
    LV1_CALL( query_logical_partition_address_region_info,      1, 5,   6 )
    LV1_CALL( select_virtual_address_space,                     1, 0,   7 )
    LV1_CALL( pause,                                            1, 0,   9 )
    LV1_CALL( destruct_virtual_address_space,                   1, 0,  10 )
    LV1_CALL( configure_irq_state_bitmap,                       3, 0,  11 )
    LV1_CALL( connect_irq_plug_ext,                             5, 0,  12 )
    LV1_CALL( release_memory,                                   1, 0,  13 )
    LV1_CALL( put_iopte,                                        5, 0,  15 )
    LV1_CALL( disconnect_irq_plug_ext,                          3, 0,  17 )
    LV1_CALL( construct_event_receive_port,                     0, 1,  18 )
    LV1_CALL( destruct_event_receive_port,                      1, 0,  19 )
    LV1_CALL( send_event_locally,                               1, 0,  24 )
    LV1_CALL( end_of_interrupt,                                 1, 0,  27 )
    LV1_CALL( connect_irq_plug,                                 2, 0,  28 )
    LV1_CALL( disconnect_irq_plug,                              1, 0,  29 )
    LV1_CALL( end_of_interrupt_ext,                             3, 0,  30 )
    LV1_CALL( did_update_interrupt_mask,                        2, 0,  31 )
    LV1_CALL( shutdown_logical_partition,                       1, 0,  44 )
    LV1_CALL( destruct_logical_spe,                             1, 0,  54 )
    LV1_CALL( construct_logical_spe,                            7, 6,  57 )
    LV1_CALL( set_spe_interrupt_mask,                           3, 0,  61 )
    LV1_CALL( undocumented_function_62, 			 			5, 0,  62 )
    LV1_CALL( set_spe_transition_notifier,                      3, 0,  64 )
    LV1_CALL( disable_logical_spe,                              2, 0,  65 )
    LV1_CALL( clear_spe_interrupt_status,                       4, 0,  66 )
    LV1_CALL( get_spe_interrupt_status,                         2, 1,  67 )
    LV1_CALL( get_logical_ppe_id,                               0, 1,  69 )
    LV1_CALL( set_interrupt_mask,                               5, 0,  73 )
    LV1_CALL( get_logical_partition_id,                         0, 1,  74 )
    LV1_CALL( configure_execution_time_variable,                1, 0,  77 )
    LV1_CALL( get_spe_irq_outlet,                               2, 1,  78 )
    LV1_CALL( set_spe_privilege_state_area_1_register,          3, 0,  79 )
    LV1_CALL( create_repository_node,                           6, 0,  90 )
    LV1_CALL( get_repository_node_value,                        5, 2,  91 )
    LV1_CALL( modify_repository_node_value,                     6, 0,  92 )
    LV1_CALL( remove_repository_node,                           4, 0,  93 )
    LV1_CALL( read_htab_entries,                                2, 5,  95 )
    LV1_CALL( set_dabr,                                         2, 0,  96 )
    LV1_CALL( undocumented_function_99, 			 			2, 0,  99 )
    LV1_CALL( get_total_execution_time,                         2, 1, 103 )
    LV1_CALL( allocate_io_segment,                              3, 1, 116 )
    LV1_CALL( release_io_segment,                               2, 0, 117 )
    LV1_CALL( construct_io_irq_outlet,                          1, 1, 120 )
    LV1_CALL( destruct_io_irq_outlet,                           1, 0, 121 )
    LV1_CALL( map_htab,                                         1, 1, 122 )
    LV1_CALL( unmap_htab,                                       1, 0, 123 )
    LV1_CALL( get_version_info,                                 0, 1, 127 )
    LV1_CALL( insert_htab_entry,                                6, 3, 158 )
    LV1_CALL( read_virtual_uart,                                3, 1, 162 )
    LV1_CALL( write_virtual_uart,                               3, 1, 163 )
    LV1_CALL( set_virtual_uart_param,                           3, 0, 164 )
    LV1_CALL( get_virtual_uart_param,                           2, 1, 165 )
    LV1_CALL( configure_virtual_uart_irq,                       1, 1, 166 )
    LV1_CALL( open_device,                                      3, 0, 170 )
    LV1_CALL( close_device,                                     2, 0, 171 )
    LV1_CALL( map_device_mmio_region,                           5, 1, 172 )
    LV1_CALL( unmap_device_mmio_region,                         3, 0, 173 )
    LV1_CALL( allocate_device_dma_region,                       5, 1, 174 )
    LV1_CALL( free_device_dma_region,                           3, 0, 175 )
    LV1_CALL( map_device_dma_region,                            6, 0, 176 )
    LV1_CALL( unmap_device_dma_region,                          4, 0, 177 )
    LV1_CALL( net_add_multicast_address,                        4, 0, 185 )
    LV1_CALL( net_remove_multicast_address,                     4, 0, 186 )
    LV1_CALL( net_start_tx_dma,                                 4, 0, 187 )
    LV1_CALL( net_stop_tx_dma,                                  3, 0, 188 )
    LV1_CALL( net_start_rx_dma,                                 4, 0, 189 )
    LV1_CALL( net_stop_rx_dma,                                  3, 0, 190 )
    LV1_CALL( net_set_interrupt_status_indicator,               4, 0, 191 )
    LV1_CALL( net_set_interrupt_mask,                           4, 0, 193 )
    LV1_CALL( net_control,                                      6, 2, 194 )
    LV1_CALL( connect_interrupt_event_receive_port,             4, 0, 197 )
    LV1_CALL( disconnect_interrupt_event_receive_port,          4, 0, 198 )
    LV1_CALL( get_spe_all_interrupt_statuses,                   1, 1, 199 )
    LV1_CALL( deconfigure_virtual_uart_irq,                     0, 0, 202 )
    LV1_CALL( enable_logical_spe,                               2, 0, 207 )
    LV1_CALL( gpu_open,                                         1, 0, 210 )
    LV1_CALL( gpu_close,                                        0, 0, 211 )
    LV1_CALL( gpu_device_map,                                   1, 2, 212 )
    LV1_CALL( gpu_device_unmap,                                 1, 0, 213 )
    LV1_CALL( gpu_memory_allocate,                              5, 2, 214 )
    LV1_CALL( gpu_memory_free,                                  1, 0, 216 )
    LV1_CALL( gpu_context_allocate,                             2, 5, 217 )
    LV1_CALL( gpu_context_free,                                 1, 0, 218 )
    LV1_CALL( gpu_context_iomap,                                5, 0, 221 )
    LV1_CALL( gpu_context_attribute,                            6, 0, 225 )
    LV1_CALL( gpu_context_intr,                                 1, 1, 227 )
    LV1_CALL( gpu_attribute,                                    5, 0, 228 )
    LV1_CALL( get_rtc,                                          0, 2, 232 )
    LV1_CALL( set_ppe_periodic_tracer_frequency,                1, 0, 240 )
    LV1_CALL( start_ppe_periodic_tracer,                        5, 0, 241 )
    LV1_CALL( stop_ppe_periodic_tracer,                         1, 1, 242 )
    LV1_CALL( storage_read,                                     6, 1, 245 )
    LV1_CALL( storage_write,                                    6, 1, 246 )
    LV1_CALL( storage_send_device_command,                      6, 1, 248 )
    LV1_CALL( storage_get_async_status,                         1, 2, 249 )
    LV1_CALL( storage_check_async_status,                       2, 1, 254 )
    LV1_CALL( panic,                                            1, 0, 255 )
    LV1_CALL( construct_lpm,                                    6, 3, 140 )
    LV1_CALL( destruct_lpm,                                     1, 0, 141 )
    LV1_CALL( start_lpm,                                        1, 0, 142 )
    LV1_CALL( stop_lpm,                                         1, 1, 143 )
    LV1_CALL( copy_lpm_trace_buffer,                            3, 1, 144 )
    LV1_CALL( add_lpm_event_bookmark,                           5, 0, 145 )
    LV1_CALL( delete_lpm_event_bookmark,                        3, 0, 146 )
    LV1_CALL( set_lpm_interrupt_mask,                           3, 1, 147 )
    LV1_CALL( get_lpm_interrupt_status,                         1, 1, 148 )
    LV1_CALL( set_lpm_general_control,                          5, 2, 149 )
    LV1_CALL( set_lpm_interval,                                 3, 1, 150 )
    LV1_CALL( set_lpm_trigger_control,                          3, 1, 151 )
    LV1_CALL( set_lpm_counter_control,                          4, 1, 152 )
    LV1_CALL( set_lpm_group_control,                            3, 1, 153 )
    LV1_CALL( set_lpm_debug_bus_control,                        3, 1, 154 )
    LV1_CALL( set_lpm_counter,                                  5, 2, 155 )
    LV1_CALL( set_lpm_signal,                                   7, 0, 156 )
    LV1_CALL( set_lpm_spr_trigger,                              2, 0, 157 )
    LV1_CALL( undocumented_function_114,                        3, 1, 114 )
    LV1_CALL( undocumented_function_115,                        1, 0, 115 )
    LV1_CALL( undocumented_function_138, 			            2, 0, 138 )
    LV1_CALL( undocumented_function_167, 					    2, 1, 167 )
    LV1_CALL( undocumented_function_168,			 			3, 0, 168 )

    LV1_CALL( cfw_peek,					 					    1, 1, 182 )
    LV1_CALL( cfw_poke,					 					    2, 0, 183 )

    static lv1::status_code allocate_memory(uint64_t size, uint64_t pageShift, uint64_t flags, uint64_t* address, uint64_t* muid) 
    { 
        const auto result = _allocate_memory(size, pageShift, 0, flags);

        *address = result.GetReturnValue(0);
        *muid    = result.GetReturnValue(1);

        return result.statusCode;
    }

    static lv1::status_code write_htab_entry(uint64_t VASID, uint64_t slot, uint64_t VA, uint64_t PA)                             
    { 
        return _write_htab_entry(VASID, slot, VA, PA ).statusCode;
    }

    static lv1::status_code construct_virtual_address_space(size_t htabSize, uint64_t numPageSizes, uint64_t pageSizesBitArray, uint64_t* VASID, uint64_t* HVPTEGCount)
    { 
        const auto result = _construct_virtual_address_space(htabSize, numPageSizes, pageSizesBitArray);

        *VASID       = result.GetReturnValue( 0 );
        *HVPTEGCount = result.GetReturnValue( 1 );

        return result.statusCode;
    }

    static lv1::status_code get_virtual_address_space_id_of_ppe(size_t PPEID, uint64_t* VASID)
    { 
        const auto result = _get_virtual_address_space_id_of_ppe(PPEID);

        *VASID = result.GetReturnValue(0);

        return result.statusCode;
    }

    // inline lv1::status_code select_virtual_address_space( uint64_t VASID )
    // { 
    //     return (lv1::status_code)_select_virtual_address_space( (uint64_t)VASID );
    // }

    static lv1::status_code get_firmware_version(ps3_firmware_version* firmwareVersion)
    { 
        const auto result = _get_version_info();

        firmwareVersion->packed = result.GetReturnValue(0);

        return result.statusCode;
    }
}


}

#endif // !LV1_H