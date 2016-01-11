# tests for the dss fix query

TITLE + fix

export TZ=EST5EDT

VIEW data ../netflow/data

TEST 01 'basics'
	EXEC -x netflow '{fix --stamp=2003-01-31}' $data/netflow-5.dat
		OUTPUT - $'<METHOD>flat</>
<FLAT>
	<NAME>netflow</>
	<DESCRIPTION>Cisco router netflow dump data fixed width binary format.</>
	<IDENT>@(#)$Id: netflow bin 2003-01-31 $</>
	<MAGIC>
		<STRING>netflow</>
		<VERSION>20030131</>
		<SWAP>be</>
	</>
	<COMPRESS>pzip netflow-bin</>
	<LIBRARY>time_t</>
	<LIBRARY>ip_t</>
	<LIBRARY>num_t</>
	<FIELD>
		<NAME>if_desc</>
		<DESCRIPTION>Full interface name.</>
		<TYPE>string</>
		<PHYSICAL>
			<WIDTH>8</>
		</>
	</>
	<FIELD>
		<NAME>if_name</>
		<DESCRIPTION>Shortened interface name.</>
		<TYPE>string</>
		<PHYSICAL>
			<WIDTH>8</>
		</>
	</>
	<FIELD>
		<NAME>sampler_name</>
		<DESCRIPTION>Flow sampler name.</>
		<TYPE>string</>
		<PHYSICAL>
			<WIDTH>8</>
		</>
	</>
	<FIELD>
		<NAME>dst_prefix</>
		<DESCRIPTION>Destination ip prefix.</>
		<TYPE>ipprefix_t</>
		<PHYSICAL>
			<TYPE>unsigned be_t</>
			<WIDTH>8</>
		</>
	</>
	<FIELD>
		<NAME>dst_prefixv4</>
		<DESCRIPTION>Destination ipv4 prefix.</>
		<TYPE>ipv4prefix_t</>
		<PHYSICAL>
			<TYPE>unsigned be_t</>
			<WIDTH>8</>
		</>
	</>
	<FIELD>
		<NAME>end</>
		<DESCRIPTION>Flow end time in 64 bit nanoseconds since the epoch (synthesized).</>
		<TYPE>ns_t</>
		<PHYSICAL>
			<TYPE>unsigned be_t</>
			<WIDTH>8</>
		</>
	</>
	<FIELD>
		<NAME>src_prefix</>
		<DESCRIPTION>Source ip prefix.</>
		<TYPE>ipprefix_t</>
		<PHYSICAL>
			<TYPE>unsigned be_t</>
			<WIDTH>8</>
		</>
	</>
	<FIELD>
		<NAME>src_prefixv4</>
		<DESCRIPTION>Source ipv4 prefix.</>
		<TYPE>ipv4prefix_t</>
		<PHYSICAL>
			<TYPE>unsigned be_t</>
			<WIDTH>8</>
		</>
	</>
	<FIELD>
		<NAME>start</>
		<DESCRIPTION>Flow start time in 64 bit nanoseconds since the epoch (synthesized).</>
		<TYPE>ns_t</>
		<PHYSICAL>
			<TYPE>unsigned be_t</>
			<WIDTH>8</>
		</>
	</>
	<FIELD>
		<NAME>bytes</>
		<DESCRIPTION>Bytes sent in duration (synthesized).</>
		<TYPE>number</>
		<PHYSICAL>
			<TYPE>unsigned be_t</>
			<WIDTH>4</>
		</>
	</>
	<FIELD>
		<NAME>dst_addr</>
		<DESCRIPTION>Destination ip address.</>
		<TYPE>ipaddr_t</>
		<PHYSICAL>
			<TYPE>unsigned be_t</>
			<WIDTH>4</>
		</>
	</>
	<FIELD>
		<NAME>dst_addrv4</>
		<DESCRIPTION>Destination ipv4 address.</>
		<TYPE>ipv4addr_t</>
		<PHYSICAL>
			<TYPE>unsigned be_t</>
			<WIDTH>4</>
		</>
	</>
	<FIELD>
		<NAME>dst_as32</>
		<DESCRIPTION>Destination BGP AS32 number.</>
		<TYPE>as32_t</>
		<PHYSICAL>
			<TYPE>unsigned be_t</>
			<WIDTH>4</>
		</>
	</>
	<FIELD>
		<NAME>first</>
		<DESCRIPTION>Elapsed milliseconds at flow start.</>
		<TYPE>elapsed_t</>
		<PHYSICAL>
			<TYPE>be_t</>
			<WIDTH>4</>
		</>
	</>
	<FIELD>
		<NAME>hop</>
		<DESCRIPTION>Next hop ip address.</>
		<TYPE>ipaddr_t</>
		<PHYSICAL>
			<TYPE>unsigned be_t</>
			<WIDTH>4</>
		</>
	</>
	<FIELD>
		<NAME>hopv4</>
		<DESCRIPTION>Next hop ipv4 address.</>
		<TYPE>ipv4addr_t</>
		<PHYSICAL>
			<TYPE>unsigned be_t</>
			<WIDTH>4</>
		</>
	</>
	<FIELD>
		<NAME>last</>
		<DESCRIPTION>Elapsed milliseconds at flow end.</>
		<TYPE>elapsed_t</>
		<PHYSICAL>
			<TYPE>be_t</>
			<WIDTH>4</>
		</>
	</>
	<FIELD>
		<NAME>nsec</>
		<DESCRIPTION>Residual nanoseconds (header).</>
		<TYPE>number</>
		<PHYSICAL>
			<TYPE>unsigned be_t</>
			<WIDTH>4</>
		</>
	</>
	<FIELD>
		<NAME>router_sc</>
		<DESCRIPTION>Router shortcut ip address (V7).</>
		<TYPE>ipaddr_t</>
		<PHYSICAL>
			<TYPE>unsigned be_t</>
			<WIDTH>4</>
		</>
	</>
	<FIELD>
		<NAME>router_scv4</>
		<DESCRIPTION>Router shortcut ipv4 address.</>
		<TYPE>ipv4addr_t</>
		<PHYSICAL>
			<TYPE>unsigned be_t</>
			<WIDTH>4</>
		</>
	</>
	<FIELD>
		<NAME>src_addr</>
		<DESCRIPTION>Source ip address.</>
		<TYPE>ipaddr_t</>
		<PHYSICAL>
			<TYPE>unsigned be_t</>
			<WIDTH>4</>
		</>
	</>
	<FIELD>
		<NAME>src_addrv4</>
		<DESCRIPTION>Source ipv4 address.</>
		<TYPE>ipv4addr_t</>
		<PHYSICAL>
			<TYPE>unsigned be_t</>
			<WIDTH>4</>
		</>
	</>
	<FIELD>
		<NAME>src_as32</>
		<DESCRIPTION>Source BGP AS32 number.</>
		<TYPE>as32_t</>
		<PHYSICAL>
			<TYPE>unsigned be_t</>
			<WIDTH>4</>
		</>
	</>
	<FIELD>
		<NAME>time</>
		<DESCRIPTION>Current time in seconds since the epoch (header).</>
		<TYPE>time_t</>
		<PHYSICAL>
			<TYPE>unsigned be_t</>
			<WIDTH>4</>
		</>
	</>
	<FIELD>
		<NAME>uptime</>
		<DESCRIPTION>Elapsed milliseconds since the router booted (header).</>
		<TYPE>elapsed_t</>
		<PHYSICAL>
			<TYPE>be_t</>
			<WIDTH>4</>
		</>
	</>
	<FIELD>
		<NAME>count</>
		<DESCRIPTION>Number of records that follow in packet (header).</>
		<TYPE>number</>
		<PHYSICAL>
			<TYPE>unsigned be_t</>
			<WIDTH>2</>
		</>
	</>
	<FIELD>
		<NAME>direction</>
		<DESCRIPTION>Flow direction: 0 - ingress flow, 1 - egress flow.</>
		<TYPE>number</>
		<PHYSICAL>
			<TYPE>unsigned be_t</>
			<WIDTH>2</>
		</>
	</>
	<FIELD>
		<NAME>dst_as</>
		<DESCRIPTION>Destination BGP AS number.</>
		<TYPE>as_t</>
		<PHYSICAL>
			<TYPE>unsigned be_t</>
			<WIDTH>2</>
		</>
	</>
	<FIELD>
		<NAME>dst_as16</>
		<DESCRIPTION>Destination BGP AS16 number.</>
		<TYPE>as16_t</>
		<PHYSICAL>
			<TYPE>unsigned be_t</>
			<WIDTH>2</>
		</>
	</>
	<FIELD>
		<NAME>dst_mask</>
		<DESCRIPTION>Destination address prefix mask bits.</>
		<TYPE>number</>
		<PHYSICAL>
			<TYPE>unsigned be_t</>
			<WIDTH>2</>
		</>
	</>
	<FIELD>
		<NAME>dst_maskv4</>
		<DESCRIPTION>Destination ipv4 address prefix mask bits.</>
		<TYPE>number</>
		<PHYSICAL>
			<TYPE>unsigned be_t</>
			<WIDTH>2</>
		</>
	</>
	<FIELD>
		<NAME>dst_maskv6</>
		<DESCRIPTION>Destination ipv6 address prefix mask bits.</>
		<TYPE>number</>
		<PHYSICAL>
			<TYPE>unsigned be_t</>
			<WIDTH>2</>
		</>
	</>
	<FIELD>
		<NAME>dst_port</>
		<DESCRIPTION>TCP/UDP destination port number.</>
		<TYPE>number</>
		<PHYSICAL>
			<TYPE>unsigned be_t</>
			<WIDTH>2</>
		</>
	</>
	<FIELD>
		<NAME>dst_tos</>
		<DESCRIPTION>Type of Service on exiting outgoing interface.</>
		<TYPE>number</>
		<PHYSICAL>
			<TYPE>unsigned be_t</>
			<WIDTH>2</>
		</>
	</>
	<FIELD>
		<NAME>dst_vlan</>
		<DESCRIPTION>Virtual LAN identifier associated with egress interface.</>
		<TYPE>number</>
		<PHYSICAL>
			<TYPE>unsigned be_t</>
			<WIDTH>2</>
		</>
	</>
	<FIELD>
		<NAME>engine_id</>
		<DESCRIPTION>ID number of the flow switching engine.</>
		<TYPE>number</>
		<PHYSICAL>
			<TYPE>unsigned be_t</>
			<WIDTH>2</>
		</>
	</>
	<FIELD>
		<NAME>engine_type</>
		<DESCRIPTION>Type of flow switching engine 0: RP, 1: VIP/linecard.</>
		<TYPE>number</>
		<PHYSICAL>
			<TYPE>unsigned be_t</>
			<WIDTH>2</>
		</>
	</>
	<FIELD>
		<NAME>flags</>
		<DESCRIPTION>Reason flow was discarded, etc.</>
		<TYPE>number</>
		<PHYSICAL>
			<TYPE>unsigned be_t</>
			<WIDTH>2</>
		</>
	</>
	<FIELD>
		<NAME>flow_active_timeout</>
		<DESCRIPTION>Timeout value (in seconds) for active flow cache entries.</>
		<TYPE>number</>
		<PHYSICAL>
			<TYPE>unsigned be_t</>
			<WIDTH>2</>
		</>
	</>
	<FIELD>
		<NAME>flow_inactive_timeout</>
		<DESCRIPTION>Timeout value (in seconds) for inactive flow cache entries.</>
		<TYPE>number</>
		<PHYSICAL>
			<TYPE>unsigned be_t</>
			<WIDTH>2</>
		</>
	</>
	<FIELD>
		<NAME>flow_label</>
		<DESCRIPTION>ipv6 RFC 2460 flow label.</>
		<TYPE>number</>
		<PHYSICAL>
			<TYPE>unsigned be_t</>
			<WIDTH>2</>
		</>
	</>
	<FIELD>
		<NAME>flow_sequence</>
		<DESCRIPTION>Flow sequence counter (header).</>
		<TYPE>number</>
		<PHYSICAL>
			<TYPE>unsigned be_t</>
			<WIDTH>2</>
		</>
	</>
	<FIELD>
		<NAME>flows</>
		<DESCRIPTION>Number of flows that were aggregated.</>
		<TYPE>number</>
		<PHYSICAL>
			<TYPE>unsigned be_t</>
			<WIDTH>2</>
		</>
	</>
	<FIELD>
		<NAME>forwarding_code</>
		<DESCRIPTION>Forwarding reason code.</>
		<TYPE>number</>
		<PHYSICAL>
			<TYPE>unsigned be_t</>
			<WIDTH>2</>
		</>
	</>
	<FIELD>
		<NAME>forwarding_status</>
		<DESCRIPTION>Forwarding status 0: unknown, 1: forwarded, 2: dropped, 3: consumed.</>
		<TYPE>number</>
		<PHYSICAL>
			<TYPE>unsigned be_t</>
			<WIDTH>2</>
		</>
	</>
	<FIELD>
		<NAME>fragment_offset</>
		<DESCRIPTION>Fragmented packet fragment-offset.</>
		<TYPE>number</>
		<PHYSICAL>
			<TYPE>unsigned be_t</>
			<WIDTH>2</>
		</>
	</>
	<FIELD>
		<NAME>icmp_type</>
		<DESCRIPTION>Internet Control Message Protocol packet type coded as ((type*256)+code).</>
		<TYPE>number</>
		<PHYSICAL>
			<TYPE>unsigned be_t</>
			<WIDTH>2</>
		</>
	</>
	<FIELD>
		<NAME>ident</>
		<DESCRIPTION>Identification field.</>
		<TYPE>number</>
		<PHYSICAL>
			<TYPE>unsigned be_t</>
			<WIDTH>2</>
		</>
	</>
	<FIELD>
		<NAME>in_bytes</>
		<DESCRIPTION>Incoming counter for the number of bytes associated with an ip Flow.</>
		<TYPE>number</>
		<PHYSICAL>
			<TYPE>unsigned be_t</>
			<WIDTH>2</>
		</>
	</>
	<FIELD>
		<NAME>in_dst_mac</>
		<DESCRIPTION>Incoming destination MAC address.</>
		<TYPE>number</>
		<PHYSICAL>
			<TYPE>unsigned be_t</>
			<WIDTH>2</>
		</>
	</>
	<FIELD>
		<NAME>in_packets</>
		<DESCRIPTION>Incoming counter for the number of packets associated with an ip Flow.</>
		<TYPE>number</>
		<PHYSICAL>
			<TYPE>unsigned be_t</>
			<WIDTH>2</>
		</>
	</>
	<FIELD>
		<NAME>in_permanent_bytes</>
		<DESCRIPTION>Permanent flow byte count.</>
		<TYPE>number</>
		<PHYSICAL>
			<TYPE>unsigned be_t</>
			<WIDTH>2</>
		</>
	</>
	<FIELD>
		<NAME>in_permanent_packets</>
		<DESCRIPTION>Permanent flow packet count.</>
		<TYPE>number</>
		<PHYSICAL>
			<TYPE>unsigned be_t</>
			<WIDTH>2</>
		</>
	</>
	<FIELD>
		<NAME>in_src_mac</>
		<DESCRIPTION>Incoming source MAC address.</>
		<TYPE>number</>
		<PHYSICAL>
			<TYPE>unsigned be_t</>
			<WIDTH>2</>
		</>
	</>
	<FIELD>
		<NAME>input_snmp</>
		<DESCRIPTION>Input interface index.</>
		<TYPE>number</>
		<PHYSICAL>
			<TYPE>unsigned be_t</>
			<WIDTH>2</>
		</>
	</>
	<FIELD>
		<NAME>ip_protocol_version</>
		<DESCRIPTION>ip version 6: ipv6, 4 or not specified: ipv4.</>
		<TYPE>number</>
		<PHYSICAL>
			<TYPE>unsigned be_t</>
			<WIDTH>2</>
		</>
	</>
	<FIELD>
		<NAME>max_packet_length</>
		<DESCRIPTION>Maximum incoming ip packet length.</>
		<TYPE>number</>
		<PHYSICAL>
			<TYPE>unsigned be_t</>
			<WIDTH>2</>
		</>
	</>
	<FIELD>
		<NAME>max_ttl</>
		<DESCRIPTION>Maximum TTL on incoming packets.</>
		<TYPE>number</>
		<PHYSICAL>
			<TYPE>unsigned be_t</>
			<WIDTH>2</>
		</>
	</>
	<FIELD>
		<NAME>min_packet_length</>
		<DESCRIPTION>Minimum incoming ip packet length.</>
		<TYPE>number</>
		<PHYSICAL>
			<TYPE>unsigned be_t</>
			<WIDTH>2</>
		</>
	</>
	<FIELD>
		<NAME>min_ttl</>
		<DESCRIPTION>Minimum TTL on incoming packets.</>
		<TYPE>number</>
		<PHYSICAL>
			<TYPE>unsigned be_t</>
			<WIDTH>2</>
		</>
	</>
	<FIELD>
		<NAME>mpls_label_1</>
		<DESCRIPTION>Stack position 1 MPLS label: 20 bits MPLS label, 3 bits experimental, 1 bit end-of-stack.</>
		<TYPE>number</>
		<PHYSICAL>
			<TYPE>unsigned be_t</>
			<WIDTH>2</>
		</>
	</>
	<FIELD>
		<NAME>mpls_label_10</>
		<DESCRIPTION>Stack position 10 MPLS label: 20 bits MPLS label, 3 bits experimental, 1 bit end-of-stack.</>
		<TYPE>number</>
		<PHYSICAL>
			<TYPE>unsigned be_t</>
			<WIDTH>2</>
		</>
	</>
	<FIELD>
		<NAME>mpls_label_2</>
		<DESCRIPTION>Stack position 2 MPLS label: 20 bits MPLS label, 3 bits experimental, 1 bit end-of-stack.</>
		<TYPE>number</>
		<PHYSICAL>
			<TYPE>unsigned be_t</>
			<WIDTH>2</>
		</>
	</>
	<FIELD>
		<NAME>mpls_label_3</>
		<DESCRIPTION>Stack position 3 MPLS label: 20 bits MPLS label, 3 bits experimental, 1 bit end-of-stack.</>
		<TYPE>number</>
		<PHYSICAL>
			<TYPE>unsigned be_t</>
			<WIDTH>2</>
		</>
	</>
	<FIELD>
		<NAME>mpls_label_4</>
		<DESCRIPTION>Stack position 4 MPLS label: 20 bits MPLS label, 3 bits experimental, 1 bit end-of-stack.</>
		<TYPE>number</>
		<PHYSICAL>
			<TYPE>unsigned be_t</>
			<WIDTH>2</>
		</>
	</>
	<FIELD>
		<NAME>mpls_label_5</>
		<DESCRIPTION>Stack position 5 MPLS label: 20 bits MPLS label, 3 bits experimental, 1 bit end-of-stack.</>
		<TYPE>number</>
		<PHYSICAL>
			<TYPE>unsigned be_t</>
			<WIDTH>2</>
		</>
	</>
	<FIELD>
		<NAME>mpls_label_6</>
		<DESCRIPTION>Stack position 6 MPLS label: 20 bits MPLS label, 3 bits experimental, 1 bit end-of-stack.</>
		<TYPE>number</>
		<PHYSICAL>
			<TYPE>unsigned be_t</>
			<WIDTH>2</>
		</>
	</>
	<FIELD>
		<NAME>mpls_label_7</>
		<DESCRIPTION>Stack position 7 MPLS label: 20 bits MPLS label, 3 bits experimental, 1 bit end-of-stack.</>
		<TYPE>number</>
		<PHYSICAL>
			<TYPE>unsigned be_t</>
			<WIDTH>2</>
		</>
	</>
	<FIELD>
		<NAME>mpls_label_8</>
		<DESCRIPTION>Stack position 8 MPLS label: 20 bits MPLS label, 3 bits experimental, 1 bit end-of-stack.</>
		<TYPE>number</>
		<PHYSICAL>
			<TYPE>unsigned be_t</>
			<WIDTH>2</>
		</>
	</>
	<FIELD>
		<NAME>mpls_label_9</>
		<DESCRIPTION>Stack position 9 MPLS label: 20 bits MPLS label, 3 bits experimental, 1 bit end-of-stack.</>
		<TYPE>number</>
		<PHYSICAL>
			<TYPE>unsigned be_t</>
			<WIDTH>2</>
		</>
	</>
	<FIELD>
		<NAME>mpls_top_label_class</>
		<DESCRIPTION>Forwarding Equivalent Class corresponding to the MPLS Top Label.</>
		<TYPE>number</>
		<PHYSICAL>
			<TYPE>unsigned be_t</>
			<WIDTH>2</>
		</>
	</>
	<FIELD>
		<NAME>mpls_top_label_type</>
		<DESCRIPTION>MPLS top label type: 0x00 UNKNOWN 0x01 TE-MIDPT 0x02 ATOM 0x03 VPN 0x04 BGP 0x05 LDP.</>
		<TYPE>number</>
		<PHYSICAL>
			<TYPE>unsigned be_t</>
			<WIDTH>2</>
		</>
	</>
	<FIELD>
		<NAME>mul_dst_bytes</>
		<DESCRIPTION>Multicast outgoing byte count.</>
		<TYPE>number</>
		<PHYSICAL>
			<TYPE>unsigned be_t</>
			<WIDTH>2</>
		</>
	</>
	<FIELD>
		<NAME>mul_dst_packets</>
		<DESCRIPTION>Multicast outgoing packet count.</>
		<TYPE>number</>
		<PHYSICAL>
			<TYPE>unsigned be_t</>
			<WIDTH>2</>
		</>
	</>
	<FIELD>
		<NAME>mul_igmp_type</>
		<DESCRIPTION>Internet Group Management Protocol packet type coded.</>
		<TYPE>number</>
		<PHYSICAL>
			<TYPE>unsigned be_t</>
			<WIDTH>2</>
		</>
	</>
	<FIELD>
		<NAME>option_headers</>
		<DESCRIPTION>Bit-encoded field identifying ipv6 option headers found in the flow.</>
		<TYPE>number</>
		<PHYSICAL>
			<TYPE>unsigned be_t</>
			<WIDTH>2</>
		</>
	</>
	<FIELD>
		<NAME>out_bytes</>
		<DESCRIPTION>Outgoing counter for the number of bytes associated with an ip Flow.</>
		<TYPE>number</>
		<PHYSICAL>
			<TYPE>unsigned be_t</>
			<WIDTH>2</>
		</>
	</>
	<FIELD>
		<NAME>out_dst_mac</>
		<DESCRIPTION>Outgoing destination MAC address.</>
		<TYPE>number</>
		<PHYSICAL>
			<TYPE>unsigned be_t</>
			<WIDTH>2</>
		</>
	</>
	<FIELD>
		<NAME>out_packets</>
		<DESCRIPTION>Outgoing counter for the number of packets associated with an ip Flow.</>
		<TYPE>number</>
		<PHYSICAL>
			<TYPE>unsigned be_t</>
			<WIDTH>2</>
		</>
	</>
	<FIELD>
		<NAME>out_src_mac</>
		<DESCRIPTION>Outgoing source MAC address.</>
		<TYPE>number</>
		<PHYSICAL>
			<TYPE>unsigned be_t</>
			<WIDTH>2</>
		</>
	</>
	<FIELD>
		<NAME>output_snmp</>
		<DESCRIPTION>Output interface index.</>
		<TYPE>number</>
		<PHYSICAL>
			<TYPE>unsigned be_t</>
			<WIDTH>2</>
		</>
	</>
	<FIELD>
		<NAME>packets</>
		<DESCRIPTION>Number of packets in flow.</>
		<TYPE>number</>
		<PHYSICAL>
			<TYPE>unsigned be_t</>
			<WIDTH>2</>
		</>
	</>
	<FIELD>
		<NAME>prot</>
		<DESCRIPTION>ip protocol, e.g., 6=TCP, 17=UDP, ...</>
		<TYPE>number</>
		<PHYSICAL>
			<TYPE>unsigned be_t</>
			<WIDTH>2</>
		</>
	</>
	<FIELD>
		<NAME>sampler_algorithm</>
		<DESCRIPTION>0x01: deterministic, 0x02: random.</>
		<TYPE>number</>
		<PHYSICAL>
			<TYPE>unsigned be_t</>
			<WIDTH>2</>
		</>
	</>
	<FIELD>
		<NAME>sampler_interval</>
		<DESCRIPTION>Sampling interval.</>
		<TYPE>number</>
		<PHYSICAL>
			<TYPE>unsigned be_t</>
			<WIDTH>2</>
		</>
	</>
	<FIELD>
		<NAME>sampler_mode</>
		<DESCRIPTION>Sampling mode.</>
		<TYPE>number</>
		<PHYSICAL>
			<TYPE>unsigned be_t</>
			<WIDTH>2</>
		</>
	</>
	<FIELD>
		<NAME>src_as</>
		<DESCRIPTION>Source BGP AS number.</>
		<TYPE>as_t</>
		<PHYSICAL>
			<TYPE>unsigned be_t</>
			<WIDTH>2</>
		</>
	</>
	<FIELD>
		<NAME>src_as16</>
		<DESCRIPTION>Source BGP AS16 number.</>
		<TYPE>as16_t</>
		<PHYSICAL>
			<TYPE>unsigned be_t</>
			<WIDTH>2</>
		</>
	</>
	<FIELD>
		<NAME>src_mask</>
		<DESCRIPTION>Source address prefix mask bits.</>
		<TYPE>number</>
		<PHYSICAL>
			<TYPE>unsigned be_t</>
			<WIDTH>2</>
		</>
	</>
	<FIELD>
		<NAME>src_maskv4</>
		<DESCRIPTION>Source ipv4 address prefix mask bits.</>
		<TYPE>number</>
		<PHYSICAL>
			<TYPE>unsigned be_t</>
			<WIDTH>2</>
		</>
	</>
	<FIELD>
		<NAME>src_maskv6</>
		<DESCRIPTION>Source ipv6 address prefix mask bits.</>
		<TYPE>number</>
		<PHYSICAL>
			<TYPE>unsigned be_t</>
			<WIDTH>2</>
		</>
	</>
	<FIELD>
		<NAME>src_port</>
		<DESCRIPTION>TCP/UDP source port number.</>
		<TYPE>number</>
		<PHYSICAL>
			<TYPE>unsigned be_t</>
			<WIDTH>2</>
		</>
	</>
	<FIELD>
		<NAME>src_tos</>
		<DESCRIPTION>ip type-of-service upon entering incoming interface.</>
		<TYPE>number</>
		<PHYSICAL>
			<TYPE>unsigned be_t</>
			<WIDTH>2</>
		</>
	</>
	<FIELD>
		<NAME>src_vlan</>
		<DESCRIPTION>Virtual LAN identifier associated with ingress interface.</>
		<TYPE>number</>
		<PHYSICAL>
			<TYPE>unsigned be_t</>
			<WIDTH>2</>
		</>
	</>
	<FIELD>
		<NAME>tcp_flags</>
		<DESCRIPTION>Cumulative OR of tcp flags for this flow.</>
		<TYPE>number</>
		<PHYSICAL>
			<TYPE>unsigned be_t</>
			<WIDTH>2</>
		</>
	</>
	<FIELD>
		<NAME>tcp_misseq_cnt</>
		<DESCRIPTION>Number of mis-sequenced tcp packets (V1).</>
		<TYPE>number</>
		<PHYSICAL>
			<TYPE>unsigned be_t</>
			<WIDTH>2</>
		</>
	</>
	<FIELD>
		<NAME>tcp_retx_cnt</>
		<DESCRIPTION>Number of mis-seq with delay > 1sec (V1).</>
		<TYPE>number</>
		<PHYSICAL>
			<TYPE>unsigned be_t</>
			<WIDTH>2</>
		</>
	</>
	<FIELD>
		<NAME>tcp_retx_secs</>
		<DESCRIPTION>Number of seconds between mis-sequenced packets (V1).</>
		<TYPE>number</>
		<PHYSICAL>
			<TYPE>unsigned be_t</>
			<WIDTH>2</>
		</>
	</>
	<FIELD>
		<NAME>tos</>
		<DESCRIPTION>ip type-of-service (synthesized).</>
		<TYPE>number</>
		<PHYSICAL>
			<TYPE>unsigned be_t</>
			<WIDTH>2</>
		</>
	</>
	<FIELD>
		<NAME>total_bytes_exp</>
		<DESCRIPTION>The number of bytes exported by the observation domain.</>
		<TYPE>number</>
		<PHYSICAL>
			<TYPE>unsigned be_t</>
			<WIDTH>2</>
		</>
	</>
	<FIELD>
		<NAME>total_flows_exp</>
		<DESCRIPTION>The number of flows exported by the observation domain.</>
		<TYPE>number</>
		<PHYSICAL>
			<TYPE>unsigned be_t</>
			<WIDTH>2</>
		</>
	</>
	<FIELD>
		<NAME>total_packets_exp</>
		<DESCRIPTION>The number of packets exported by the observation domain.</>
		<TYPE>number</>
		<PHYSICAL>
			<TYPE>unsigned be_t</>
			<WIDTH>2</>
		</>
	</>
	<FIELD>
		<NAME>vendor_43</>
		<DESCRIPTION>Vendor private value.</>
		<TYPE>number</>
		<PHYSICAL>
			<TYPE>unsigned be_t</>
			<WIDTH>2</>
		</>
	</>
	<FIELD>
		<NAME>vendor_51</>
		<DESCRIPTION>Vendor private value.</>
		<TYPE>number</>
		<PHYSICAL>
			<TYPE>unsigned be_t</>
			<WIDTH>2</>
		</>
	</>
	<FIELD>
		<NAME>vendor_65</>
		<DESCRIPTION>Vendor private value.</>
		<TYPE>number</>
		<PHYSICAL>
			<TYPE>unsigned be_t</>
			<WIDTH>2</>
		</>
	</>
	<FIELD>
		<NAME>vendor_66</>
		<DESCRIPTION>Vendor private value.</>
		<TYPE>number</>
		<PHYSICAL>
			<TYPE>unsigned be_t</>
			<WIDTH>2</>
		</>
	</>
	<FIELD>
		<NAME>vendor_67</>
		<DESCRIPTION>Vendor private value.</>
		<TYPE>number</>
		<PHYSICAL>
			<TYPE>unsigned be_t</>
			<WIDTH>2</>
		</>
	</>
	<FIELD>
		<NAME>vendor_68</>
		<DESCRIPTION>Vendor private value.</>
		<TYPE>number</>
		<PHYSICAL>
			<TYPE>unsigned be_t</>
			<WIDTH>2</>
		</>
	</>
	<FIELD>
		<NAME>vendor_69</>
		<DESCRIPTION>Vendor private value.</>
		<TYPE>number</>
		<PHYSICAL>
			<TYPE>unsigned be_t</>
			<WIDTH>2</>
		</>
	</>
	<FIELD>
		<NAME>vendor_87</>
		<DESCRIPTION>Vendor private value.</>
		<TYPE>number</>
		<PHYSICAL>
			<TYPE>unsigned be_t</>
			<WIDTH>2</>
		</>
	</>
	<FIELD>
		<NAME>version</>
		<DESCRIPTION>Record version (header).</>
		<TYPE>number</>
		<PHYSICAL>
			<TYPE>unsigned be_t</>
			<WIDTH>2</>
		</>
	</>
	<FIELD>
		<NAME>dst_addrv6</>
		<DESCRIPTION>Destination ipv6 address.</>
		<TYPE>ipv6addr_t</>
		<PHYSICAL>
			<TYPE>buffer</>
			<WIDTH>4</>
		</>
	</>
	<FIELD>
		<NAME>dst_prefixv6</>
		<DESCRIPTION>Destination ipv6 prefix.</>
		<TYPE>ipv6prefix_t</>
		<PHYSICAL>
			<TYPE>buffer</>
			<WIDTH>4</>
		</>
	</>
	<FIELD>
		<NAME>hopv6</>
		<DESCRIPTION>Next hop ipv6 address.</>
		<TYPE>ipv6addr_t</>
		<PHYSICAL>
			<TYPE>buffer</>
			<WIDTH>4</>
		</>
	</>
	<FIELD>
		<NAME>router_scv6</>
		<DESCRIPTION>Router shortcut ipv6 address.</>
		<TYPE>ipv6addr_t</>
		<PHYSICAL>
			<TYPE>buffer</>
			<WIDTH>4</>
		</>
	</>
	<FIELD>
		<NAME>src_addrv6</>
		<DESCRIPTION>Source ipv6 address.</>
		<TYPE>ipv6addr_t</>
		<PHYSICAL>
			<TYPE>buffer</>
			<WIDTH>4</>
		</>
	</>
	<FIELD>
		<NAME>src_prefixv6</>
		<DESCRIPTION>Source ipv6 prefix.</>
		<TYPE>ipv6prefix_t</>
		<PHYSICAL>
			<TYPE>buffer</>
			<WIDTH>4</>
		</>
	</>
	<FIELD>
		<NAME>_HEAP_</>
		<DESCRIPTION>Variable width data heap.</>
		<TYPE>void</>
		<PHYSICAL>
			<WIDTH>692</>
		</>
	</>
</>'
