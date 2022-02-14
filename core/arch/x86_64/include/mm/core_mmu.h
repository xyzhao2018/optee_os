/* SPDX-License-Identifier: BSD-2-Clause */
/*
 * Copyright (c) 2016, Linaro Limited
 * Copyright (c) 2014, STMicroelectronics International N.V.
 */
#ifndef CORE_MMU_H
#define CORE_MMU_H

#ifndef __ASSEMBLER__
#include <assert.h>
#include <compiler.h>
#include <kernel/user_ta.h>
#include <mm/tee_mmu_types.h>
#include <types_ext.h>
#include <util.h>

typedef uint64_t map_addr_t;
typedef uint64_t arch_flags_t;

#endif

#include <platform_config.h>

/* top level defines for the x86 mmu */
/* NOTE: the top part can be included from assembly */
#define KB                (1024UL)
#define MB                (1024UL*1024UL)
#define GB                (1024UL*1024UL*1024UL)

#define X86_PD_PA_POS      12

/* P   Present */
#define X86_MMU_PG_P       0x001
/* R/W Read/Write, 1=enable writes */
#define X86_MMU_PG_RW      0x002
/* U/S User/Supervisor, 1=enable user mode */
#define X86_MMU_PG_U       0x004
/* PWT Page-level write-through */
#define X86_MMU_PG_PWT     0x008
/* PCD Page Cache Disable */
#define X86_MMU_PG_PCD     0x010
/* PS  Page size */
#define X86_MMU_PG_PS      0x080
/* PAT PAT index */
#define X86_MMU_PG_PTE_PAT 0x080
/* G   Global */
#define X86_MMU_PG_G       0x100
/* XD  Execute-disable, 1=disable execution */
#define X86_MMU_PG_NX      (1ul << 63)

#define X86_MMU_CLEAR       0x0
#define X86_DIRTY_ACCESS_MASK   0xf9f

/* default flags for inner page directory entries */
#define X86_KERNEL_PD_FLAGS (X86_MMU_PG_G | X86_MMU_PG_RW | X86_MMU_PG_P)

/* default flags for 2MB/4MB/1GB page directory entries */
#define X86_KERNEL_PD_LP_FLAGS (X86_MMU_PG_G | X86_MMU_PG_PS | X86_MMU_PG_RW | \
				X86_MMU_PG_P)

#define X86_USER_PD_FLAGS (X86_MMU_PG_G | X86_MMU_PG_U | X86_MMU_PG_RW |\
				X86_MMU_PG_P)

#define PAGE_SIZE       4096
#define PAGE_DIV_SHIFT      12

/* PAE mode */
#define X86_PDPT_ADDR_MASK  (0x00000000ffffffe0ul)
#define X86_PG_VA_FRAME     (0x0000fffffffff000ul)
#define X86_PG_PA_FRAME     (0x0000fffffffff000ul)
#define X86_PHY_ADDR_MASK   (0x000ffffffffffffful)
#define X86_FLAGS_MASK      (~X86_PG_PA_FRAME)
#define X86_PTE_NOT_PRESENT (0xFFFFFFFFFFFFFFFEul)
#define X86_2MB_PAGE_FRAME  (0x000fffffffe00000ul)
#define PAGE_OFFSET_MASK_4KB    (0x0000000000000ffful)
#define PAGE_OFFSET_MASK_2MB    (0x00000000001ffffful)

#define X86_PAGING_LEVELS   4
#define PML4_SHIFT      39

#define PDP_SHIFT       30
#define PD_SHIFT        21
#define PT_SHIFT        12
#define ADDR_OFFSET     9
#define ADDR_MASK       ((1ul << ADDR_OFFSET) - 1)
#define PDPT_ADDR_OFFSET    2

/* Memory area which one Page Table maps with 512 Page Table Entries. */
#define AREA_MAPPED_W_ONE_PT  (2 * MB)

#define NO_OF_PML4_ENTRIES    512
#define NO_OF_PDP_ENTRIES     512
#define NO_OF_PD_ENTRIES      512
#define NO_OF_PT_ENTRIES      512
#define NO_OF_PT_TABLES       40

// User mode definitions
#define NO_OF_USER_PD_ENTRIES 512
#define NO_OF_USER_PT_TABLES  (ROUNDUP(TA_RAM_SIZE, AREA_MAPPED_W_ONE_PT) / AREA_MAPPED_W_ONE_PT)

#define ARCH_MMU_FLAG_CACHED            (0<<0)
#define ARCH_MMU_FLAG_UNCACHED          (1<<0)
/* only exists on some arches, otherwise UNCACHED */
#define ARCH_MMU_FLAG_UNCACHED_DEVICE   (2<<0)
#define ARCH_MMU_FLAG_CACHE_MASK        (3<<0)

#define ARCH_MMU_FLAG_PERM_USER         (1<<2)
#define ARCH_MMU_FLAG_PERM_RO           (1<<3)
#define ARCH_MMU_FLAG_PERM_NO_EXECUTE   (1<<4)
#define ARCH_MMU_FLAG_NS                (1<<5) /* NON-SECURE */
/* indicates that flags are not specified */
#define ARCH_MMU_FLAG_INVALID           (1<<7)

#ifndef __ASSEMBLER__
/* Different page table levels in the page table mgmt hirerachy */
enum page_table_levels {
	PF_L,
	PT_L,
	PD_L,
	PDP_L,
	PML4_L
};
#endif

/* A small page is the smallest unit of memory that can be mapped */
#define SMALL_PAGE_SHIFT	12
#define SMALL_PAGE_SIZE		BIT(SMALL_PAGE_SHIFT)
#define SMALL_PAGE_MASK		((paddr_t)SMALL_PAGE_SIZE - 1)
#define IS_PAGE_ALIGNED(addr)   (((addr) & SMALL_PAGE_MASK) == 0)

/* flags for initial mapping struct */
#define MMU_INITIAL_MAPPING_TEMPORARY     (0x1)
#define MMU_INITIAL_MAPPING_FLAG_UNCACHED (0x2)
#define MMU_INITIAL_MAPPING_FLAG_DEVICE   (0x4)
/* entry has to be patched up by platform_reset */
#define MMU_INITIAL_MAPPING_FLAG_DYNAMIC  (0x8)

/*
 * PGDIR is the translation table above the translation table that holds
 * the pages.
 */
#ifdef CFG_WITH_LPAE
#define CORE_MMU_PGDIR_SHIFT	21
#define CORE_MMU_PGDIR_LEVEL	3
#else
#define CORE_MMU_PGDIR_SHIFT	20
#define CORE_MMU_PGDIR_LEVEL	2
#endif
#define CORE_MMU_PGDIR_SIZE		BIT(CORE_MMU_PGDIR_SHIFT)
#define CORE_MMU_PGDIR_MASK		((paddr_t)CORE_MMU_PGDIR_SIZE - 1)

/* TA user space code, data, stack and heap are mapped using this granularity */
#define CORE_MMU_USER_CODE_SHIFT	SMALL_PAGE_SHIFT
#define CORE_MMU_USER_CODE_SIZE		BIT(CORE_MMU_USER_CODE_SHIFT)
#define CORE_MMU_USER_CODE_MASK		((paddr_t)CORE_MMU_USER_CODE_SIZE - 1)

/* TA user space parameters are mapped using this granularity */
#define CORE_MMU_USER_PARAM_SHIFT	SMALL_PAGE_SHIFT
#define CORE_MMU_USER_PARAM_SIZE	BIT(CORE_MMU_USER_PARAM_SHIFT)
#define CORE_MMU_USER_PARAM_MASK	((paddr_t)CORE_MMU_USER_PARAM_SIZE - 1)

#ifdef CFG_WITH_LPAE
/*
 * CORE_MMU_L1_TBL_OFFSET is used when switching to/from reduced kernel
 * mapping. The actual value depends on internals in core_mmu_lpae.c which
 * we rather not expose here. There's a compile time assertion to check
 * that these magic numbers are correct.
 */
#define CORE_MMU_L1_TBL_OFFSET		(CFG_TEE_CORE_NB_CORE * \
					 BIT(CFG_LPAE_ADDR_SPACE_BITS - 30) * 8)
#endif
/*
 * TEE_RAM_VA_START:            The start virtual address of the TEE RAM
 * TEE_TEXT_VA_START:           The start virtual address of the OP-TEE text
 */

/*
 * Identify mapping constraint: virtual base address is the physical start addr.
 * If platform did not set some macros, some get default value.
 */
#ifndef TEE_RAM_VA_SIZE
#define TEE_RAM_VA_SIZE			CORE_MMU_PGDIR_SIZE
#endif

#ifndef TEE_LOAD_ADDR
#define TEE_LOAD_ADDR			TEE_RAM_START
#endif

#define TEE_RAM_VA_START		TEE_RAM_START
#define TEE_TEXT_VA_START		(TEE_RAM_VA_START + \
					 (TEE_LOAD_ADDR - TEE_RAM_START))

#ifndef STACK_ALIGNMENT
#define STACK_ALIGNMENT			(sizeof(long) * 2)
#endif

#ifndef __ASSEMBLER__
struct map_range {
	vaddr_t start_vaddr;
	uint64_t start_paddr;
	size_t size;
};

/*
 * Memory area type:
 * MEM_AREA_END:      Reserved, marks the end of a table of mapping areas.
 * MEM_AREA_TEE_RAM:  core RAM (read/write/executable, secure, reserved to TEE)
 * MEM_AREA_TEE_RAM_RX:  core private read-only/executable memory (secure)
 * MEM_AREA_TEE_RAM_RO:  core private read-only/non-executable memory (secure)
 * MEM_AREA_TEE_RAM_RW:  core private read/write/non-executable memory (secure)
 * MEM_AREA_NEX_RAM_RW: nexus private r/w/non-executable memory (secure)
 * MEM_AREA_TEE_COHERENT: teecore coherent RAM (secure, reserved to TEE)
 * MEM_AREA_TEE_ASAN: core address sanitizer RAM (secure, reserved to TEE)
 * MEM_AREA_IDENTITY_MAP_RX: core identity mapped r/o executable memory (secure)
 * MEM_AREA_TA_RAM:   Secure RAM where teecore loads/exec TA instances.
 * MEM_AREA_NSEC_SHM: NonSecure shared RAM between NSec and TEE.
 * MEM_AREA_RAM_NSEC: NonSecure RAM storing data
 * MEM_AREA_RAM_SEC:  Secure RAM storing some secrets
 * MEM_AREA_IO_NSEC:  NonSecure HW mapped registers
 * MEM_AREA_IO_SEC:   Secure HW mapped registers
 * MEM_AREA_EXT_DT:   Memory loads external device tree
 * MEM_AREA_RES_VASPACE: Reserved virtual memory space
 * MEM_AREA_SHM_VASPACE: Virtual memory space for dynamic shared memory buffers
 * MEM_AREA_TA_VASPACE: TA va space, only used with phys_to_virt()
 * MEM_AREA_DDR_OVERALL: Overall DDR address range, candidate to dynamic shm.
 * MEM_AREA_SEC_RAM_OVERALL: Whole secure RAM
 * MEM_AREA_MAXTYPE:  lower invalid 'type' value
 */
enum teecore_memtypes {
	MEM_AREA_END = 0,
	MEM_AREA_TEE_RAM,
	MEM_AREA_TEE_RAM_RX,
	MEM_AREA_TEE_RAM_RO,
	MEM_AREA_TEE_RAM_RW,
	MEM_AREA_NEX_RAM_RW,
	MEM_AREA_TEE_COHERENT,
	MEM_AREA_TEE_ASAN,
	MEM_AREA_IDENTITY_MAP_RX,
	MEM_AREA_TA_RAM,
	MEM_AREA_NSEC_SHM,
	MEM_AREA_RAM_NSEC,
	MEM_AREA_RAM_SEC,
	MEM_AREA_IO_NSEC,
	MEM_AREA_IO_SEC,
	MEM_AREA_EXT_DT,
	MEM_AREA_RES_VASPACE,
	MEM_AREA_SHM_VASPACE,
	MEM_AREA_TA_VASPACE,
	MEM_AREA_PAGER_VASPACE,
	MEM_AREA_SDP_MEM,
	MEM_AREA_DDR_OVERALL,
	MEM_AREA_SEC_RAM_OVERALL,
	MEM_AREA_MAXTYPE
};

static inline const char *teecore_memtype_name(enum teecore_memtypes type)
{
	static const char * const names[] = {
		[MEM_AREA_END] = "END",
		[MEM_AREA_TEE_RAM] = "TEE_RAM_RWX",
		[MEM_AREA_TEE_RAM_RX] = "TEE_RAM_RX",
		[MEM_AREA_TEE_RAM_RO] = "TEE_RAM_RO",
		[MEM_AREA_TEE_RAM_RW] = "TEE_RAM_RW",
		[MEM_AREA_NEX_RAM_RW] = "NEX_RAM_RW",
		[MEM_AREA_TEE_ASAN] = "TEE_ASAN",
		[MEM_AREA_IDENTITY_MAP_RX] = "IDENTITY_MAP_RX",
		[MEM_AREA_TEE_COHERENT] = "TEE_COHERENT",
		[MEM_AREA_TA_RAM] = "TA_RAM",
		[MEM_AREA_NSEC_SHM] = "NSEC_SHM",
		[MEM_AREA_RAM_NSEC] = "RAM_NSEC",
		[MEM_AREA_RAM_SEC] = "RAM_SEC",
		[MEM_AREA_IO_NSEC] = "IO_NSEC",
		[MEM_AREA_IO_SEC] = "IO_SEC",
		[MEM_AREA_EXT_DT] = "EXT_DT",
		[MEM_AREA_RES_VASPACE] = "RES_VASPACE",
		[MEM_AREA_SHM_VASPACE] = "SHM_VASPACE",
		[MEM_AREA_TA_VASPACE] = "TA_VASPACE",
		[MEM_AREA_PAGER_VASPACE] = "PAGER_VASPACE",
		[MEM_AREA_SDP_MEM] = "SDP_MEM",
		[MEM_AREA_DDR_OVERALL] = "DDR_OVERALL",
		[MEM_AREA_SEC_RAM_OVERALL] = "SEC_RAM_OVERALL",
	};

	COMPILE_TIME_ASSERT(ARRAY_SIZE(names) == MEM_AREA_MAXTYPE);
	return names[type];
}

#ifdef CFG_CORE_RWDATA_NOEXEC
#define MEM_AREA_TEE_RAM_RW_DATA	MEM_AREA_TEE_RAM_RW
#else
#define MEM_AREA_TEE_RAM_RW_DATA	MEM_AREA_TEE_RAM
#endif

struct core_mmu_phys_mem {
	const char *name;
	enum teecore_memtypes type;
	__extension__ union {
#if __SIZEOF_LONG__ != __SIZEOF_PADDR__
		struct {
			uint32_t lo_addr;
			uint32_t hi_addr;
		};
#endif
		paddr_t addr;
	};
	__extension__ union {
#if __SIZEOF_LONG__ != __SIZEOF_PADDR__
		struct {
			uint32_t lo_size;
			uint32_t hi_size;
		};
#endif
		paddr_size_t size;
	};
};

struct mmu_initial_mapping {
	paddr_t phys;
	vaddr_t virt;
	size_t size;
	unsigned int flags;
	const char *name;
};

#define __register_memory(_name, _type, _addr, _size, _section) \
	SCATTERED_ARRAY_DEFINE_ITEM(_section, struct core_mmu_phys_mem) = \
		{ .name = (_name), .type = (_type), .addr = (_addr), \
		  .size = (_size) }

#if __SIZEOF_LONG__ != __SIZEOF_PADDR__
#define __register_memory_ul(_name, _type, _addr, _size, _section) \
	SCATTERED_ARRAY_DEFINE_ITEM(_section, struct core_mmu_phys_mem) = \
		{ .name = (_name), .type = (_type), .lo_addr = (_addr), \
		  .lo_size = (_size) }
#else
#define __register_memory_ul(_name, _type, _addr, _size, _section) \
		__register_memory(_name, _type, _addr, _size, _section)
#endif

#define register_phys_mem(type, addr, size) \
		__register_memory(#addr, (type), (addr), (size), \
				  phys_mem_map)

#define register_phys_mem_ul(type, addr, size) \
		__register_memory_ul(#addr, (type), (addr), (size), \
				     phys_mem_map)

/* Same as register_phys_mem() but with PGDIR_SIZE granularity */
#define register_phys_mem_pgdir(type, addr, size) \
	register_phys_mem(type, ROUNDDOWN(addr, CORE_MMU_PGDIR_SIZE), \
		ROUNDUP(size + addr - ROUNDDOWN(addr, CORE_MMU_PGDIR_SIZE), \
			CORE_MMU_PGDIR_SIZE))

#ifdef CFG_SECURE_DATA_PATH
#define register_sdp_mem(addr, size) \
		__register_memory(#addr, MEM_AREA_SDP_MEM, (addr), (size), \
				  phys_sdp_mem)
#else
#define register_sdp_mem(addr, size) \
		static int CONCAT(__register_sdp_mem_unused, __COUNTER__) \
			__unused
#endif

/* register_dynamic_shm() is deprecated, please use register_ddr() instead */
#define register_dynamic_shm(addr, size) \
		__register_memory(#addr, MEM_AREA_DDR_OVERALL, (addr), (size), \
				  phys_ddr_overall_compat)

/*
 * register_ddr() - Define a memory range
 * @addr: Base address
 * @size: Length
 *
 * This macro can be used multiple times to define disjoint ranges. While
 * initializing holes are carved out of these ranges where it overlaps with
 * special memory, for instance memory registered with register_sdp_mem().
 *
 * The memory that remains is accepted as non-secure shared memory when
 * communicating with normal world.
 *
 * This macro is an alternative to supply the memory description with a
 * devicetree blob.
 */
#define register_ddr(addr, size) \
		__register_memory(#addr, MEM_AREA_DDR_OVERALL, (addr), \
				  (size), phys_ddr_overall)

#define phys_ddr_overall_begin \
	SCATTERED_ARRAY_BEGIN(phys_ddr_overall, struct core_mmu_phys_mem)

#define phys_ddr_overall_end \
	SCATTERED_ARRAY_END(phys_ddr_overall, struct core_mmu_phys_mem)

#define phys_ddr_overall_compat_begin \
	SCATTERED_ARRAY_BEGIN(phys_ddr_overall_compat, struct core_mmu_phys_mem)

#define phys_ddr_overall_compat_end \
	SCATTERED_ARRAY_END(phys_ddr_overall_compat, struct core_mmu_phys_mem)

#define phys_sdp_mem_begin \
	SCATTERED_ARRAY_BEGIN(phys_sdp_mem, struct core_mmu_phys_mem)

#define phys_sdp_mem_end \
	SCATTERED_ARRAY_END(phys_sdp_mem, struct core_mmu_phys_mem)

#define phys_mem_map_begin \
	SCATTERED_ARRAY_BEGIN(phys_mem_map, struct core_mmu_phys_mem)

#define phys_mem_map_end \
	SCATTERED_ARRAY_END(phys_mem_map, struct core_mmu_phys_mem)

#ifdef CFG_CORE_RESERVED_SHM
/* Default NSec shared memory allocated from NSec world */
extern unsigned long default_nsec_shm_paddr;
extern unsigned long default_nsec_shm_size;
#endif

/*
 * Assembly code in enable_mmu() depends on the layout of this struct.
 */
struct core_mmu_config {
#if defined(ARM64)
	uint64_t tcr_el1;
	uint64_t mair_el1;
	uint64_t ttbr0_el1_base;
	uint64_t ttbr0_core_offset;
	uint64_t load_offset;
#elif defined(CFG_WITH_LPAE)
	uint32_t ttbcr;
	uint32_t mair0;
	uint32_t ttbr0_base;
	uint32_t ttbr0_core_offset;
	uint32_t load_offset;
#else
	uint32_t prrr;
	uint32_t nmrr;
	uint32_t dacr;
	uint32_t ttbcr;
	uint32_t ttbr;
	uint32_t load_offset;
#endif
};

void core_init_mmu_map(unsigned long seed, struct core_mmu_config *cfg);
void core_init_mmu_regs(struct core_mmu_config *cfg);

/*
 * struct core_mmu_user_map - current user mapping register state
 * @cr3:	physical address of user map translation table
 *
 * Note that this struct should be treated as an opaque struct since
 * the content depends on descriptor table format.
 */
struct core_mmu_user_map {
	uint64_t cr3;
};

#ifdef CFG_WITH_LPAE
bool core_mmu_user_va_range_is_defined(void);
#else
static inline bool __noprof core_mmu_user_va_range_is_defined(void)
{
	return true;
}
#endif

/*
 * struct mmu_partition - stores MMU partition.
 *
 * Basically it	represent whole MMU mapping. It is possible
 * to create multiple partitions, and change them in runtime,
 * effectively changing how OP-TEE sees memory.
 * This is opaque struct which is defined differently for
 * v7 and LPAE MMUs
 *
 * This structure used mostly when virtualization is enabled.
 * When CFG_VIRTUALIZATION==n only default partition exists.
 */
struct mmu_partition;

/*
 * core_mmu_get_user_va_range() - Return range of user va space
 * @base:	Lowest user virtual address
 * @size:	Size in bytes of user address space
 */
void core_mmu_get_user_va_range(vaddr_t *base, size_t *size);

/*
 * enum core_mmu_fault - different kinds of faults
 * @CORE_MMU_FAULT_ALIGNMENT:		alignment fault
 * @CORE_MMU_FAULT_DEBUG_EVENT:		debug event
 * @CORE_MMU_FAULT_TRANSLATION:		translation fault
 * @CORE_MMU_FAULT_WRITE_PERMISSION:	Permission fault during write
 * @CORE_MMU_FAULT_READ_PERMISSION:	Permission fault during read
 * @CORE_MMU_FAULT_ASYNC_EXTERNAL:	asynchronous external abort
 * @CORE_MMU_FAULT_ACCESS_BIT:		access bit fault
 * @CORE_MMU_FAULT_OTHER:		Other/unknown fault
 */
enum core_mmu_fault {
	CORE_MMU_FAULT_ALIGNMENT,
	CORE_MMU_FAULT_DEBUG_EVENT,
	CORE_MMU_FAULT_TRANSLATION,
	CORE_MMU_FAULT_WRITE_PERMISSION,
	CORE_MMU_FAULT_READ_PERMISSION,
	CORE_MMU_FAULT_ASYNC_EXTERNAL,
	CORE_MMU_FAULT_ACCESS_BIT,
	CORE_MMU_FAULT_OTHER,
};

/*
 * core_mmu_get_fault_type() - get fault type
 * @fault_descr:	Content of fault status or exception syndrome register
 * @returns an enum describing the content of fault status register.
 */
enum core_mmu_fault core_mmu_get_fault_type(uint32_t fault_descr);

/*
 * core_mm_type_to_attr() - convert memory type to attribute
 * @t: memory type
 * @returns an attribute that can be passed to core_mm_set_entry() and friends
 */
uint32_t core_mmu_type_to_attr(enum teecore_memtypes t);

/*
 * core_mmu_create_user_map() - Create user mode mapping
 * @uctx:	Pointer to user mode context
 * @map:	MMU configuration to use when activating this VA space
 */
void core_mmu_create_user_map(struct user_mode_ctx *uctx,
			      struct core_mmu_user_map *map);
/*
 * core_mmu_get_user_map() - Reads current MMU configuration for user VA space
 * @map:	MMU configuration for current user VA space.
 */
void core_mmu_get_user_map(struct core_mmu_user_map *map);

/*
 * core_mmu_set_user_map() - Set new MMU configuration for user VA space
 * @map:	User context MMU configuration or NULL to set core VA space
 *
 * Activate user VA space mapping and set its ASID if @map is not NULL,
 * otherwise activate core mapping and set ASID to 0.
 */
void core_mmu_set_user_map(struct core_mmu_user_map *map);

/*
 * struct core_mmu_table_info - Properties for a translation table
 * @table:	Pointer to translation table
 * @va_base:	VA base address of the transaltion table
 * @level:	Translation table level
 * @shift:	The shift of each entry in the table
 * @num_entries: Number of entries in this table.
 */
struct core_mmu_table_info {
	void *table;
	vaddr_t va_base;
	unsigned level;
	unsigned shift;
	unsigned num_entries;
#ifdef CFG_VIRTUALIZATION
	struct mmu_partition *prtn;
#endif
};

/*
 * core_mmu_find_table() - Locates a translation table
 * @prtn:	MMU partition where search should be performed
 * @va:		Virtual address for the table to cover
 * @max_level:	Don't traverse beyond this level
 * @tbl_info:	Pointer to where to store properties.
 * @return true if a translation table was found, false on error
 */
bool core_mmu_find_table(struct mmu_partition *prtn, vaddr_t va,
			 unsigned max_level,
			 struct core_mmu_table_info *tbl_info);

/*
 * core_mmu_entry_to_finer_grained() - divide mapping at current level into
 *     smaller ones so memory can be mapped with finer granularity
 * @tbl_info:	table where target record located
 * @idx:	index of record for which a pdgir must be setup.
 * @secure:	true/false if pgdir maps secure/non-secure memory (32bit mmu)
 * @return true on successful, false on error
 */
bool core_mmu_entry_to_finer_grained(struct core_mmu_table_info *tbl_info,
				     unsigned int idx, bool secure);

void core_mmu_set_entry_primitive(void *table, size_t level, size_t idx,
				  paddr_t pa, uint32_t attr);

/*
 * core_mmu_set_entry() - Set entry in translation table
 * @tbl_info:	Translation table properties
 * @idx:	Index of entry to update
 * @pa:		Physical address to assign entry
 * @attr:	Attributes to assign entry
 */
void core_mmu_set_entry(struct core_mmu_table_info *tbl_info, unsigned idx,
			paddr_t pa, uint32_t attr);

void core_mmu_get_entry_primitive(const void *table, size_t level, size_t idx,
				  paddr_t *pa, uint32_t *attr);

/*
 * core_mmu_get_entry() - Get entry from translation table
 * @tbl_info:	Translation table properties
 * @idx:	Index of entry to read
 * @pa:		Physical address is returned here if pa is not NULL
 * @attr:	Attributues are returned here if attr is not NULL
 */
void core_mmu_get_entry(struct core_mmu_table_info *tbl_info, unsigned idx,
			paddr_t *pa, uint32_t *attr);

/*
 * core_mmu_va2idx() - Translate from virtual address to table index
 * @tbl_info:	Translation table properties
 * @va:		Virtual address to translate
 * @returns index in transaltion table
 */
static inline unsigned core_mmu_va2idx(struct core_mmu_table_info *tbl_info,
			vaddr_t va)
{
	return (va - tbl_info->va_base) >> tbl_info->shift;
}

/*
 * core_mmu_idx2va() - Translate from table index to virtual address
 * @tbl_info:	Translation table properties
 * @idx:	Index to translate
 * @returns Virtual address
 */
static inline vaddr_t core_mmu_idx2va(struct core_mmu_table_info *tbl_info,
			unsigned idx)
{
	return (idx << tbl_info->shift) + tbl_info->va_base;
}

/*
 * core_mmu_get_block_offset() - Get offset inside a block/page
 * @tbl_info:	Translation table properties
 * @pa:		Physical address
 * @returns offset within one block of the translation table
 */
static inline size_t core_mmu_get_block_offset(
			struct core_mmu_table_info *tbl_info, paddr_t pa)
{
	return pa & ((1 << tbl_info->shift) - 1);
}

/*
 * core_mmu_is_dynamic_vaspace() - Check if memory region belongs to
 *  empty virtual address space that is used for dymanic mappings
 * @mm:		memory region to be checked
 * @returns result of the check
 */
static inline bool core_mmu_is_dynamic_vaspace(struct tee_mmap_region *mm)
{
	return mm->type == MEM_AREA_RES_VASPACE ||
		mm->type == MEM_AREA_SHM_VASPACE;
}

/*
 * core_mmu_map_pages() - map list of pages at given virtual address
 * @vstart:	Virtual address where mapping begins
 * @pages:	Array of page addresses
 * @num_pages:	Number of pages
 * @memtype:	Type of memmory to be mapped
 *
 * Note: This function asserts that pages are not mapped executeable for
 * kernel (privileged) mode.
 *
 * @returns:	TEE_SUCCESS on success, TEE_ERROR_XXX on error
 */
TEE_Result core_mmu_map_pages(vaddr_t vstart, paddr_t *pages, size_t num_pages,
			      enum teecore_memtypes memtype);

/*
 * core_mmu_unmap_pages() - remove mapping at given virtual address
 * @vstart:	Virtual address where mapping begins
 * @num_pages:	Number of pages to unmap
 */
void core_mmu_unmap_pages(vaddr_t vstart, size_t num_pages);

/*
 * core_mmu_user_mapping_is_active() - Report if user mapping is active
 * @returns true if a user VA space is active, false if user VA space is
 *          inactive.
 */
bool core_mmu_user_mapping_is_active(void);

/*
 * core_mmu_mattr_is_ok() - Check that supplied mem attributes can be used
 * @returns true if the attributes can be used, false if not.
 */
bool core_mmu_mattr_is_ok(uint32_t mattr);

void core_mmu_get_mem_by_type(enum teecore_memtypes type, vaddr_t *s,
			      vaddr_t *e);

enum teecore_memtypes core_mmu_get_type_by_pa(paddr_t pa);

/* routines to retreive shared mem configuration */
static inline bool core_mmu_is_shm_cached(void)
{
	return core_mmu_type_to_attr(MEM_AREA_NSEC_SHM) &
		(TEE_MATTR_CACHE_CACHED << TEE_MATTR_CACHE_SHIFT);
}

bool core_mmu_add_mapping(enum teecore_memtypes type, paddr_t addr, size_t len);

/*
 * tlbi_mva_range() - Invalidate TLB for virtual address range
 * @va:		start virtual address, must be a multiple of @granule
 * @len:	length in bytes of range, must be a multiple of @granule
 * @granule:	granularity of mapping, supported values are
 *		CORE_MMU_PGDIR_SIZE or SMALL_PAGE_SIZE. This value must
 *		match the actual mappings.
 */
void tlbi_mva_range(vaddr_t va, size_t len, size_t granule);

/*
 * tlbi_mva_range_asid() - Invalidate TLB for virtual address range for
 *			   a specific ASID
 * @va:		start virtual address, must be a multiple of @granule
 * @len:	length in bytes of range, must be a multiple of @granule
 * @granule:	granularity of mapping, supported values are
 *		CORE_MMU_PGDIR_SIZE or SMALL_PAGE_SIZE. This value must
 *		match the actual mappings.
 * @asid:	Address space identifier
 */
void tlbi_mva_range_asid(vaddr_t va, size_t len, size_t granule, uint32_t asid);

/* Cache maintenance operation type */
enum cache_op {
	DCACHE_CLEAN,
	DCACHE_AREA_CLEAN,
	DCACHE_INVALIDATE,
	DCACHE_AREA_INVALIDATE,
	ICACHE_INVALIDATE,
	ICACHE_AREA_INVALIDATE,
	DCACHE_CLEAN_INV,
	DCACHE_AREA_CLEAN_INV,
	DCACHE_TLB_INVALIDATE,
};

/* L1/L2 cache maintenance */
TEE_Result cache_op_inner(enum cache_op op, void *va, size_t len);

/* Check cpu mmu enabled or not */
bool cpu_mmu_enabled(void);

/* Do section mapping, not support on LPAE */
void map_memarea_sections(const struct tee_mmap_region *mm, uint32_t *ttb);

#ifdef CFG_CORE_DYN_SHM
/*
 * Check if platform defines nsec DDR range(s).
 * Static SHM (MEM_AREA_NSEC_SHM) is not covered by this API as it is
 * always present.
 */
bool core_mmu_nsec_ddr_is_defined(void);

void core_mmu_set_discovered_nsec_ddr(struct core_mmu_phys_mem *start,
				      size_t nelems);
#endif

/* Initialize MMU partition */
void core_init_mmu_prtn(struct mmu_partition *prtn, struct tee_mmap_region *mm);

unsigned int asid_alloc(void);
void asid_free(unsigned int asid);

#ifdef CFG_SECURE_DATA_PATH
/* Alloc and fill SDP memory objects table - table is NULL terminated */
struct mobj **core_sdp_mem_create_mobjs(void);
#endif

#ifdef CFG_VIRTUALIZATION
size_t core_mmu_get_total_pages_size(void);
struct mmu_partition *core_alloc_mmu_prtn(void *tables);
void core_free_mmu_prtn(struct mmu_partition *prtn);
void core_mmu_set_prtn(struct mmu_partition *prtn);
void core_mmu_set_default_prtn(void);
void core_mmu_set_default_prtn_tbl(void);

void core_mmu_init_virtualization(void);
#endif

/* init some allocation pools */
void core_mmu_init_ta_ram(void);

#endif /*__ASSEMBLER__*/

#endif /* CORE_MMU_H */
