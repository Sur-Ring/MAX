#ifndef __MAX_HSA_DEF_H__
#define __MAX_HSA_DEF_H__

#include "def.h"

/**
 * @brief Status codes.
 */
typedef enum {
  /**
   * The function has been executed successfully.
   */
  HSA_STATUS_SUCCESS = 0x0,
  /**
   * A traversal over a list of elements has been interrupted by the
   * application before completing.
   */
  HSA_STATUS_INFO_BREAK = 0x1,
  /**
   * A generic error has occurred.
   */
  HSA_STATUS_ERROR = 0x1000,
  /**
   * One of the actual arguments does not meet a precondition stated in the
   * documentation of the corresponding formal argument.
   */
  HSA_STATUS_ERROR_INVALID_ARGUMENT = 0x1001,
  /**
   * The requested queue creation is not valid.
   */
  HSA_STATUS_ERROR_INVALID_QUEUE_CREATION = 0x1002,
  /**
   * The requested allocation is not valid.
   */
  HSA_STATUS_ERROR_INVALID_ALLOCATION = 0x1003,
  /**
   * The agent is invalid.
   */
  HSA_STATUS_ERROR_INVALID_AGENT = 0x1004,
  /**
   * The memory region is invalid.
   */
  HSA_STATUS_ERROR_INVALID_REGION = 0x1005,
  /**
   * The signal is invalid.
   */
  HSA_STATUS_ERROR_INVALID_SIGNAL = 0x1006,
  /**
   * The queue is invalid.
   */
  HSA_STATUS_ERROR_INVALID_QUEUE = 0x1007,
  /**
   * The HSA runtime failed to allocate the necessary resources. This error
   * may also occur when the HSA runtime needs to spawn threads or create
   * internal OS-specific events.
   */
  HSA_STATUS_ERROR_OUT_OF_RESOURCES = 0x1008,
  /**
   * The AQL packet is malformed.
   */
  HSA_STATUS_ERROR_INVALID_PACKET_FORMAT = 0x1009,
  /**
   * An error has been detected while releasing a resource.
   */
  HSA_STATUS_ERROR_RESOURCE_FREE = 0x100A,
  /**
   * An API other than ::hsa_init has been invoked while the reference count
   * of the HSA runtime is 0.
   */
  HSA_STATUS_ERROR_NOT_INITIALIZED = 0x100B,
  /**
   * The maximum reference count for the object has been reached.
   */
  HSA_STATUS_ERROR_REFCOUNT_OVERFLOW = 0x100C,
  /**
   * The arguments passed to a functions are not compatible.
   */
  HSA_STATUS_ERROR_INCOMPATIBLE_ARGUMENTS = 0x100D,
  /**
   * The index is invalid.
   */
  HSA_STATUS_ERROR_INVALID_INDEX = 0x100E,
  /**
   * The instruction set architecture is invalid.
   */
  HSA_STATUS_ERROR_INVALID_ISA = 0x100F,
  /**
   * The instruction set architecture name is invalid.
   */
  HSA_STATUS_ERROR_INVALID_ISA_NAME = 0x1017,
  /**
   * The code object is invalid.
   */
  HSA_STATUS_ERROR_INVALID_CODE_OBJECT = 0x1010,
  /**
   * The executable is invalid.
   */
  HSA_STATUS_ERROR_INVALID_EXECUTABLE = 0x1011,
  /**
   * The executable is frozen.
   */
  HSA_STATUS_ERROR_FROZEN_EXECUTABLE = 0x1012,
  /**
   * There is no symbol with the given name.
   */
  HSA_STATUS_ERROR_INVALID_SYMBOL_NAME = 0x1013,
  /**
   * The variable is already defined.
   */
  HSA_STATUS_ERROR_VARIABLE_ALREADY_DEFINED = 0x1014,
  /**
   * The variable is undefined.
   */
  HSA_STATUS_ERROR_VARIABLE_UNDEFINED = 0x1015,
  /**
   * An HSAIL operation resulted in a hardware exception.
   */
  HSA_STATUS_ERROR_EXCEPTION = 0x1016,
  /**
   * The code object symbol is invalid.
   */
  HSA_STATUS_ERROR_INVALID_CODE_SYMBOL = 0x1018,
  /**
   * The executable symbol is invalid.
   */
  HSA_STATUS_ERROR_INVALID_EXECUTABLE_SYMBOL = 0x1019,
  /**
   * The file descriptor is invalid.
   */
  HSA_STATUS_ERROR_INVALID_FILE = 0x1020,
  /**
   * The code object reader is invalid.
   */
  HSA_STATUS_ERROR_INVALID_CODE_OBJECT_READER = 0x1021,
  /**
   * The cache is invalid.
   */
  HSA_STATUS_ERROR_INVALID_CACHE = 0x1022,
  /**
   * The wavefront is invalid.
   */
  HSA_STATUS_ERROR_INVALID_WAVEFRONT = 0x1023,
  /**
   * The signal group is invalid.
   */
  HSA_STATUS_ERROR_INVALID_SIGNAL_GROUP = 0x1024,
  /**
   * The HSA runtime is not in the configuration state.
   */
  HSA_STATUS_ERROR_INVALID_RUNTIME_STATE = 0x1025,
  /**
  * The queue received an error that may require process termination.
  */
  HSA_STATUS_ERROR_FATAL = 0x1026
} hsa_status_t;

typedef struct hsa_agent_s {
  /**
   * Opaque handle. Two handles reference the same object of the enclosing type
   * if and only if they are equal.
   */
  uint64_t handle;
} hsa_agent_t;

typedef struct hsa_region_s {
  /**
   * Opaque handle. Two handles reference the same object of the enclosing type
   * if and only if they are equal.
   */
  uint64_t handle;
} hsa_region_t;

typedef int64_t hsa_signal_value_t;

typedef struct hsa_signal_s {
  /**
   * Opaque handle. Two handles reference the same object of the enclosing type
   * if and only if they are equal. The value 0 is reserved.
   */
  uint64_t handle;
} hsa_signal_t;

typedef uint32_t hsa_queue_type32_t;

typedef struct hsa_queue_s {
  hsa_queue_type32_t type;
  uint32_t features;
  void* base_address;
  hsa_signal_t doorbell_signal;
  uint32_t size;
  uint32_t reserved1;
  uint64_t id;
} hsa_queue_t;

typedef struct hsa_executable_s {
  /**
   * Opaque handle. Two handles reference the same object of the enclosing type
   * if and only if they are equal.
   */
  uint64_t handle;
} hsa_executable_t;

typedef enum {
  /**
   * Vendor-specific packet.
   */
  HSA_PACKET_TYPE_VENDOR_SPECIFIC = 0,
  /**
   * The packet has been processed in the past, but has not been reassigned to
   * the packet processor. A packet processor must not process a packet of this
   * type. All queues support this packet type.
   */
  HSA_PACKET_TYPE_INVALID = 1,
  /**
   * Packet used by agents for dispatching jobs to kernel agents. Not all
   * queues support packets of this type (see ::hsa_queue_feature_t).
   */
  HSA_PACKET_TYPE_KERNEL_DISPATCH = 2,
  /**
   * Packet used by agents to delay processing of subsequent packets, and to
   * express complex dependencies between multiple packets. All queues support
   * this packet type.
   */
  HSA_PACKET_TYPE_BARRIER_AND = 3,
  /**
   * Packet used by agents for dispatching jobs to agents.  Not all
   * queues support packets of this type (see ::hsa_queue_feature_t).
   */
  HSA_PACKET_TYPE_AGENT_DISPATCH = 4,
  /**
   * Packet used by agents to delay processing of subsequent packets, and to
   * express complex dependencies between multiple packets. All queues support
   * this packet type.
   */
  HSA_PACKET_TYPE_BARRIER_OR = 5
} hsa_packet_type_t;

typedef enum {
  /**
   * Number of dimensions of the grid. Valid values are 1, 2, or 3.
   *
   */
   HSA_KERNEL_DISPATCH_PACKET_SETUP_DIMENSIONS = 0
} hsa_kernel_dispatch_packet_setup_t;

 typedef enum {
  /**
   * Packet type. The value of this sub-field must be one of
   * ::hsa_packet_type_t. If the type is ::HSA_PACKET_TYPE_VENDOR_SPECIFIC, the
   * packet layout is vendor-specific.
   */
   HSA_PACKET_HEADER_TYPE = 0,
  /**
   * Barrier bit. If the barrier bit is set, the processing of the current
   * packet only launches when all preceding packets (within the same queue) are
   * complete.
   */
   HSA_PACKET_HEADER_BARRIER = 8,
  /**
   * Acquire fence scope. The value of this sub-field determines the scope and
   * type of the memory fence operation applied before the packet enters the
   * active phase. An acquire fence ensures that any subsequent global segment
   * or image loads by any unit of execution that belongs to a dispatch that has
   * not yet entered the active phase on any queue of the same kernel agent,
   * sees any data previously released at the scopes specified by the acquire
   * fence. The value of this sub-field must be one of ::hsa_fence_scope_t.
   */
   HSA_PACKET_HEADER_SCACQUIRE_FENCE_SCOPE = 9,
   /**
    * @deprecated Renamed as ::HSA_PACKET_HEADER_SCACQUIRE_FENCE_SCOPE.
    */
   HSA_PACKET_HEADER_ACQUIRE_FENCE_SCOPE = 9,
  /**
   * Release fence scope, The value of this sub-field determines the scope and
   * type of the memory fence operation applied after kernel completion but
   * before the packet is completed. A release fence makes any global segment or
   * image data that was stored by any unit of execution that belonged to a
   * dispatch that has completed the active phase on any queue of the same
   * kernel agent visible in all the scopes specified by the release fence. The
   * value of this sub-field must be one of ::hsa_fence_scope_t.
   */
   HSA_PACKET_HEADER_SCRELEASE_FENCE_SCOPE = 11,
   /**
    * @deprecated Renamed as ::HSA_PACKET_HEADER_SCRELEASE_FENCE_SCOPE.
    */
   HSA_PACKET_HEADER_RELEASE_FENCE_SCOPE = 11
} hsa_packet_header_t;

typedef enum {
  /**
   * No scope (no fence is applied). The packet relies on external fences to
   * ensure visibility of memory updates.
   */
  HSA_FENCE_SCOPE_NONE = 0,
  /**
   * The fence is applied with agent scope for the global segment.
   */
  HSA_FENCE_SCOPE_AGENT = 1,
  /**
   * The fence is applied across both agent and system scope for the global
   * segment.
   */
  HSA_FENCE_SCOPE_SYSTEM = 2
} hsa_fence_scope_t;

typedef enum {
  /**
   * Queue supports multiple producers. Use of multiproducer queue mechanics is
   * required.
   */
  HSA_QUEUE_TYPE_MULTI = 0,
  /**
   * Queue only supports a single producer. In some scenarios, the application
   * may want to limit the submission of AQL packets to a single agent. Queues
   * that support a single producer may be more efficient than queues supporting
   * multiple producers. Use of multiproducer queue mechanics is not supported.
   */
  HSA_QUEUE_TYPE_SINGLE = 1,
  /**
   * Queue supports multiple producers and cooperative dispatches. Cooperative
   * dispatches are able to use GWS synchronization. Queues of this type may be
   * limited in number. The runtime may return the same queue to serve multiple
   * ::hsa_queue_create calls when this type is given. Callers must inspect the
   * returned queue to discover queue size. Queues of this type are reference
   * counted and require a matching number of ::hsa_queue_destroy calls to
   * release. Use of multiproducer queue mechanics is required. See
   * ::HSA_AMD_AGENT_INFO_COOPERATIVE_QUEUES to query agent support for this
   * type.
   */
  HSA_QUEUE_TYPE_COOPERATIVE = 2
} hsa_queue_type_t;

typedef enum {
  /**
   * The application thread may be rescheduled while waiting on the signal.
   */
  HSA_WAIT_STATE_BLOCKED = 0,
  /**
   * The application thread stays active while waiting on a signal.
   */
  HSA_WAIT_STATE_ACTIVE = 1
} hsa_wait_state_t;

typedef enum {
  /**
   * The two operands are equal.
   */
  HSA_SIGNAL_CONDITION_EQ = 0,
  /**
   * The two operands are not equal.
   */
  HSA_SIGNAL_CONDITION_NE = 1,
  /**
   * The first operand is less than the second operand.
   */
  HSA_SIGNAL_CONDITION_LT = 2,
  /**
   * The first operand is greater than or equal to the second operand.
   */
  HSA_SIGNAL_CONDITION_GTE = 3
} hsa_signal_condition_t;

typedef struct hsa_kernel_dispatch_packet_s {
  uint16_t header;
  uint16_t setup;
  uint16_t workgroup_size_x;
  uint16_t workgroup_size_y;
  uint16_t workgroup_size_z;
  uint16_t reserved0;
  uint32_t grid_size_x;
  uint32_t grid_size_y;
  uint32_t grid_size_z;
  uint32_t private_segment_size;
  uint32_t group_segment_size;
  uint64_t kernel_object;
  void* kernarg_address;
  uint64_t reserved2;
  hsa_signal_t completion_signal;
} hsa_kernel_dispatch_packet_t;

typedef struct hsa_amd_memory_pool_s {
  /**
   * Opaque handle.
   */
  uint64_t handle;
} hsa_amd_memory_pool_t;

#endif  // header guard
