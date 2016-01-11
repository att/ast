/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 2002-2012 AT&T Intellectual Property          *
*                      and is licensed under the                       *
*                 Eclipse Public License, Version 1.0                  *
*                    by AT&T Intellectual Property                     *
*                                                                      *
*                A copy of the License is available at                 *
*          http://www.eclipse.org/org/documents/epl-v10.html           *
*         (with md5 checksum b35adb5213ca9657e911e9befb180842)         *
*                                                                      *
*              Information and Software Systems Research               *
*                            AT&T Research                             *
*                           Florham Park NJ                            *
*                                                                      *
*                Aman Shaikh <ashaikh@research.att.com>                *
*                                                                      *
***********************************************************************/
/**********************************************************************
 * File Name    : utils.h
 * Description  : Utility function prototypes and definitions.
**********************************************************************/

#ifndef	_OSPFMON_UTILS_H_
#define	_OSPFMON_UTILS_H_ 1

#define get_time_as_double(s,u)		((double)((s)*1e0+(u)*1e-6))
#define ipaddr_to_dotted_decimal(a)	fmtip4(a,-1)
#define mask_len_to_ipaddr(b)		((b)?((((ipaddr_t)1)<<(32-(b)))-1):~((ipaddr_t)0))

#include <ospf_rfc.h>
#include <mem_debug.h>

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

/* Length of a dotted current time string (YYYY.MMMDD.HH.MM.DD\0). */
#define	DOTTED_TIME_STR_LEN		20

#ifndef INET_ADDRSTRLEN
#define INET_ADDRSTRLEN		20
#endif

/* String of characters constituting a white space. */
#define WHITE_SPACE_CHAR_STR    " \t\n\v\r"

/* Global string that stores result of calling my_inet_ntoa() */
extern char		addr_str[][INET_ADDRSTRLEN + 1];

/* The tab array */
extern char *tab_strings[];

/* For function pointers. */
#define FUNC_PTR(func, return, proto) return (*func) proto

#ifndef MIN
#define MIN(a, b)   ((a) < (b) ? (a) : (b))
#endif  /* MIN */
#ifndef MAX
#define MAX(a, b)   ((a) > (b) ? (a) : (b))
#endif  /* MAX */
#ifndef ABS
#define ABS(a)      ((a) < 0 ? -(a) : (a))
#endif  /* ABS */

/* Bit manipulation macros. */
#define BIT_SET(f, b)   ((f) |= b)
#define BIT_RESET(f, b) ((f) &= ~(b))
#define BIT_FLIP(f, b)  ((f) ^= (b))
#define BIT_TEST(f, b)  ((f) & (b))
#define BIT_MATCH(f, b) (((f) & (b)) == (b))
#define BIT_COMPARE(f, b1, b2)  (((f) & (b1)) == b2)
#define BIT_MASK_MATCH(f, g, b) (!(((f) ^ (g)) & (b)))

/*
 * Macro to convert a length when the space is extend to four-octet
 * padding alignment.
 * For example:
 *   FOUR_OCTECT_PADDED_LEN(1) = 4
 *   FOUR_OCTECT_PADDED_LEN(2) = 4
 *   FOUR_OCTECT_PADDED_LEN(3) = 4
 *   FOUR_OCTECT_PADDED_LEN(4) = 4
 *   FOUR_OCTECT_PADDED_LEN(5) = 8
 */
#define FOUR_OCTECT_PADDED_LEN(len) \
	(BIT_TEST((len), 0x00000003)  == 0) ? \
    ((((uint32_t)(len)) >> 2) << 2) : \
    ((((uint32_t)(len)) >> 2) << 2) + 4

/* Macro that fills a 32 bit IP address into a struct sockaddr_in */
#define FILL_IP_ADDRESS_INTO_SOCKADDR(sockaddr, ip_addr) \
    (sockaddr)->sin_addr.s_addr = htonl((ip_addr))

#define ROUNDUP(a, size) (((a) & ((size)-1)) ? (1 + ((a) | ((size)-1))) : (a))

/* For storing and extracting data in/from void arrays. */
#define BYTE_ARRAY_TO_DATA_ARRAY(data_type, void_ptr, byte_index) \
    ((data_type *)(void_ptr))[(byte_index) / sizeof(data_type)]

/* 
 * Macros for extracting network order bytes such that
 * memory alignment is not a problem.
 */
#define EXTRACT_NO_16BITS(p) \
    ((uint16_t)*((const uint8_t *)(p) + 0) << 8 | \
    (uint16_t)*((const uint8_t *)(p) + 1))
#define EXTRACT_NO_24BITS(p) \
    ((uint32_t)*((const uint8_t *)(p) + 0) << 16 | \
    (uint32_t)*((const uint8_t *)(p) + 1) << 8 | \
    (uint32_t)*((const uint8_t *)(p) + 2))
#define EXTRACT_NO_32BITS(p) \
    ((uint32_t)*((const uint8_t *)(p) + 0) << 24 | \
    (uint32_t)*((const uint8_t *)(p) + 1) << 16 | \
    (uint32_t)*((const uint8_t *)(p) + 2) << 8 | \
    (uint32_t)*((const uint8_t *)(p) + 3))

/* Extracts a variable from a network ordered buffer of given data_type */
#define EXTRACT_DATA_TYPE_FROM_NO_BUFF(data_type, void_ptr, byte_index, var, \
                                       bytes_left) \
do { \
    if (sizeof(data_type) == sizeof(uint32_t)) { \
        (var) = EXTRACT_NO_32BITS((const uint8_t *)(void_ptr) + (byte_index)); \
    } else if (sizeof(data_type) == sizeof(uint16_t)) { \
        (var) = EXTRACT_NO_16BITS((const uint8_t *)(void_ptr) + (byte_index)); \
    } else { \
        (var) = (void_ptr)[(byte_index)]; \
    } \
    (void_ptr) += sizeof(data_type); \
    (bytes_left) -= sizeof(data_type); \
} while (0)

/* Extracts a variable from a network ordered buffer of given len */
#define EXTRACT_LEN_BYTES_FROM_NO_BUFF(len, void_ptr, byte_index, var, \
                                       bytes_left) \
do { \
	if ((len) == 4) { \
        (var) = EXTRACT_NO_32BITS((const uint8_t *)(void_ptr) + (byte_index)); \
	} else if ((len) == 3) { \
        (var) = EXTRACT_NO_24BITS((const uint8_t *)(void_ptr) + (byte_index)); \
	} else if ((len) == 2) { \
        (var) = EXTRACT_NO_16BITS((const uint8_t *)(void_ptr) + (byte_index)); \
	} else if ((len) == 1) { \
        (var) = (void_ptr)[(byte_index)]; \
	} else { \
		ASSERT(0); \
	} \
    (void_ptr) += len; \
    (bytes_left) -= len; \
} while (0)

/* Storing a variable of a given data_type into a network ordered buffer */
#define STORE_DATA_TYPE_INTO_NO_BUFF(data_type, void_ptr, byte_index, var, \
                                     bytes_left) \
do { \
    if (sizeof(data_type) == sizeof(uint32_t)) { \
        (void_ptr)[byte_index] = ((var) & 0xff000000) >> 24; \
        (void_ptr)[byte_index + 1] = ((var) & 0x00ff0000) >> 16; \
        (void_ptr)[byte_index + 2] = ((var) & 0x0000ff00) >> 8; \
        (void_ptr)[byte_index + 3] = ((var) & 0x000000ff); \
    } else if (sizeof(data_type) == sizeof(uint16_t)) { \
        (void_ptr)[byte_index] = ((var) & 0xff00) >> 8; \
        (void_ptr)[byte_index + 1] = ((var) & 0x00ff); \
    } else { \
        (void_ptr)[byte_index] = (var); \
    } \
    (void_ptr) += sizeof(data_type); \
    (bytes_left) -= sizeof(data_type); \
} while (0)

/* Storing given number of bytes of a variable into a network ordered buffer */
#define STORE_LEN_BYTES_INTO_NO_BUFF(len, void_ptr, byte_index, var, \
                                     bytes_left) \
do { \
    if ((len) == 4) { \
        (void_ptr)[byte_index] = ((var) & 0xff000000) >> 24; \
        (void_ptr)[byte_index + 1] = ((var) & 0x00ff0000) >> 16; \
        (void_ptr)[byte_index + 2] = ((var) & 0x0000ff00) >> 8; \
        (void_ptr)[byte_index + 3] = ((var) & 0x000000ff); \
    } else if ((len) == 3) { \
        (void_ptr)[byte_index] = ((var) & 0xff00) >> 16; \
        (void_ptr)[byte_index + 1] = ((var) & 0x00ff) >> 8; \
        (void_ptr)[byte_index + 2] = ((var) & 0x00ff); \
    } else if ((len) == 2) { \
        (void_ptr)[byte_index] = ((var) & 0xff00) >> 8; \
        (void_ptr)[byte_index + 1] = ((var) & 0x00ff); \
    } else if ((len) == 1) { \
        (void_ptr)[byte_index] = (var); \
    } else { \
		ASSERT(0); \
	} \
    (void_ptr) += (len); \
    (bytes_left) -= (len); \
} while (0)

/*
 * Copies bit pattern of a (32-bit) float variable into a uint32_t
 * variable. NOTE: this is not a cast of float into uint32_t since
 * cast is likely to change the underlying bit pattern.
 */
uint32_t float_to_four_bytes(float f);

/*
 * Copies bit pattern of a uint32_t variable into a (32-bit) float
 * variable. NOTE: this is not a cast of uint32_t into float since
 * cast is likely to change the underlying bit pattern.
 */
float four_bytes_to_float(uint32_t l);

/*
 * Converts bit pattern of a (32-bit) float variable 'f' into a 
 * 32-bit network byte order value.
 */
extern uint32_t htonf(float f);

/* Converts 32-bit network order bit pattern into a float variable */
extern float ntohf(uint32_t l);

/* Returns pointer to dotted decimal string for rtrid (host order) */
extern char *rtid_to_dotted_decimal(rtid_t rtrid);

/* Returns pointer to dotted decimal string for area id (host order) */
extern char *areaid_to_dotted_decimal(areaid_t area_id);

/* 
 * Returns mask length for a mask represented as an IP address (host
 * order).
 * length = number of consecutive 1's starting form MSB.
 */
extern int ipaddr_to_mask_len(ipaddr_t ipaddr);

/* 
 * Returns pointer to dotted decimal string for the prefix which
 * is determined by 'addr' & 'mask'.
 * Both 'addr' and 'mask' should be in host order.
 */
extern char *get_prefix_dotted_decimal(ipaddr_t addr, ipaddr_t mask);

/* 
 * Returns TRUE if prefix range given by 'addr1', 'mask1' overlaps
 * with that given by 'addr2', 'mask2'.
 */
extern int does_prefix_range_overlap(ipaddr_t addr1, ipaddr_t mask1,
			 				  	     ipaddr_t addr2, ipaddr_t mask2);

/*
 * Calculates most specific prefix that covers 'addr1/mask1' and
 * 'addr2/mask2'. Returns address part. If 'mask' is non-NULL,
 * sets *mask to the mask.
 */
extern ipaddr_t get_cover_prefix(ipaddr_t addr1, ipaddr_t mask1,
								 ipaddr_t addr2, ipaddr_t mask2,
								 ipaddr_t *mask);

/* 
 * Returns pointer to dotted decimal string for a numeric IPv4 addr
 * (nw order)
 */
extern char *my_inet_ntoa(struct in_addr);

/* 
 * Returns 1 if 'addr' (host order) is an IP multicast address; 0 otherwise.
 */
extern int is_this_mcast_addr(ipaddr_t addr);

/* Breaks 'val' into second and microsec part. */
extern void get_time_as_sec_microsec(double val, time_t *sec,
							  	     time_t *micro_sec);

/* Returns time string for 'time_val' in local time-zone. */
extern char *get_time_str_local(double time_val);

/* Returns time string for 'time_val' in GMT time-zone. */
extern char *get_time_str_gmt(double time_val);

/*
 * Returns time string for 'time_val' in local time-zone. 
 * Saves time-zone information in '*tz' if 'tz' is non-NULL.
 */
extern char *get_time_str_wtz_local(double time_val, char **tz);

/*
 * Returns time string for 'time_val' in GMT time-zone. 
 * Saves time-zone information (which is 'GMT' in this case)
 * in '*tz' if 'tz' is non-NULL.
 */
extern char *get_time_str_wtz_gmt(double time_val, char **tz);

/* 
 * Gets 'time_val' in dotted format "YYYY.MMM.DD.HH.MM.SS"
 * as per local time-zone. The memory is in static area.
 */
extern char *get_dotted_time_str_local(double time_val);

/* 
 * Gets 'time_val' in dotted format "YYYY.MMM.DD.HH.MM.SS"
 * as per GMT time-zone. The memory is in static area.
 */
extern char *get_dotted_time_str_gmt(double time_val);

/* 
 * Gets 'time_str' in dotted format "YYYY.MMM.DD.HH.MM.SS".
 * The memory is in static area. 'time_str' should be a string 
 * returned by one of the 'get_time_str' function.
 */
extern char *get_dotted_time_str(const char *time_str);

/* Gets current time in sec.microsec format. */
extern double get_curr_time();

/*
 * Gets current time string in local time-zone.
 * The memory is in static area.
 */
extern char *get_curr_time_str_local();

/*
 * Gets current time string in GMT time-zone.
 * The memory is in static area.
 */
extern char *get_curr_time_str_gmt();

/*
 * Gets current time string in local time-zone.
 * Also saves time-zone information in '*tz' if 'tz' is non-NULL.
 */
extern char *get_curr_time_str_wtz_local(char **tz);

/*
 * Gets current time string in GMT time-zone.
 * Also saves time-zone information (which is 'GMT' in this case)
 * in '*tz' if 'tz' is non-NULL.
 */
extern char *get_curr_time_str_wtz_gmt(char **tz);

/* 
 * Gets current time string in dotted format "YYYY.MMM.DD.HH.MM.SS"
 * as per local time-zone. The memory is in static area.
 */
extern char *get_dotted_curr_time_str_local();

/* 
 * Gets current time string in dotted format "YYYY.MMM.DD.HH.MM.SS"
 * as per GMT time-zone. The memory is in static area.
 */
extern char *get_dotted_curr_time_str_gmt();

/*
 * Converts a month-name given in 'mon_name'
 * to a number string and stores it in 'mon_no_str'.
 * User is responsible for allocating space to 'mon_no_str'.
 * 'mon_name_len' specifies the number of chars in 'mon_name'.
 * Comparison is case-insensitive.
 * Returns 1 if successful; 0 otherwise.
 */
extern int mon_name_to_no(const char *mon_name, int mon_name_len,
						  char *mon_no_str);

/* Returns no of times char 'c' occurs in string 'str'. */
extern int str_no_of_chars(char *str, char c);

/*
 * Gets the number of seconds left before the 'interval' such that it
 * falls at the hour boundary. If 'local_tz' is set, local time-zone
 * is used, otherwise GMT is used. Note that 'interval' should
 * be such that it always falls at an hour boundary once set that way.
 * Otherwise, the function just returns 'interval'.
 */
extern int get_sec_bf_hr_bdry(int interval, int local_tz);

/*
 * Gets the number of seconds left before the 'interval' such that it
 * falls at the day boundary. If 'local_tz' is set, local time-zone
 * is used, otherwise GMT is used. Note that 'interval' should
 * be such that it always falls at a day boundary once set that way.
 * Otherwise, the function just returns 'interval'.
 */
extern int get_sec_bf_day_bdry(int interval, int local_tz);

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif		/* _OSPFMON_UTILS_H_ */
