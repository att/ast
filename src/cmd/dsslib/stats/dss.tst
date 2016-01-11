# tests for the dss stats query

TITLE + stats

export TZ=EST5EDT

VIEW data ../netflow/data

TEST 01 'default output'
	EXEC -x netflow '{stats --count --sum}' $data/netflow-5.dat
		OUTPUT - $'                FIELD           COUNT             SUM
                bytes             247        30393693
                count             247             997
            direction             247               0
             dst_addr             247    332121641119
           dst_addrv4             247    332121641119
               dst_as             247         2775870
             dst_as16             247         2775870
             dst_as32             247               0
             dst_mask             247            6156
           dst_maskv4             247            6156
           dst_maskv6             247               0
             dst_port             247          877684
           dst_prefix             247  21255785037772
         dst_prefixv4             247  21255785037772
              dst_tos             247            4224
             dst_vlan             247               0
                  end             247  2.50434663e+20
            engine_id             247            1242
          engine_type             247             138
                first             247     38181216616
                flags             247               0
  flow_active_timeout             247               0
flow_inactive_timeout             247               0
           flow_label             247               0
        flow_sequence             247         9032815
                flows             247               0
      forwarding_code             247               0
    forwarding_status             247               0
      fragment_offset             247               0
                  hop             247     28896210442
                hopv4             247     28896210442
            icmp_type             247               0
                ident             247               0
             in_bytes             247               0
           in_dst_mac             247               0
   in_permanent_bytes             247               0
 in_permanent_packets             247               0
           in_packets             247           22691
           in_src_mac             247               0
           input_snmp             247            1943
  ip_protocol_version             247               0
                 last             247     38184764492
    max_packet_length             247               0
              max_ttl             247               0
    min_packet_length             247               0
              min_ttl             247               0
         mpls_label_1             247               0
         mpls_label_2             247               0
         mpls_label_3             247               0
         mpls_label_4             247               0
         mpls_label_5             247               0
         mpls_label_6             247               0
         mpls_label_7             247               0
         mpls_label_8             247               0
         mpls_label_9             247               0
        mpls_label_10             247               0
 mpls_top_label_class             247               0
  mpls_top_label_type             247               0
        mul_dst_bytes             247               0
      mul_dst_packets             247               0
        mul_igmp_type             247               0
                 nsec             247    166142393718
       option_headers             247               0
            out_bytes             247               0
          out_dst_mac             247               0
          out_packets             247           22691
          out_src_mac             247               0
          output_snmp             247             992
              packets             247           22691
                 prot             247            1533
            router_sc             247               0
          router_scv4             247               0
    sampler_algorithm             247               0
     sampler_interval             247               0
         sampler_mode             247               0
             src_addr             247    656138212302
           src_addrv4             247    656138212302
               src_as             247         1725584
             src_as16             247         1725584
             src_as32             247               0
             src_mask             247            4979
           src_maskv4             247            4979
           src_maskv6             247               0
             src_port             247         2208170
           src_prefix             247  41992845592307
         src_prefixv4             247  41992845592307
              src_tos             247            4224
             src_vlan             247               0
                start             247  2.50434660e+20
            tcp_flags             247            5038
       tcp_misseq_cnt             247               0
         tcp_retx_cnt             247               0
        tcp_retx_secs             247               0
                 time             247    250434664657
                  tos             247            4224
      total_bytes_exp             247               0
      total_flows_exp             247               0
    total_packets_exp             247               0
               uptime             247     38186327004
            vendor_43             247               0
            vendor_51             247               0
            vendor_65             247               0
            vendor_66             247               0
            vendor_67             247               0
            vendor_68             247               0
            vendor_69             247               0
            vendor_87             247               0
              version             247            1235'
	EXEC -x netflow '{stats --group=prot --range packets}' $data/netflow-5.dat
		OUTPUT - $'  FIELD             MIN             MAX            prot
packets               1               6               1
packets               1           20902               6
packets               1              24              17'
	EXEC -x netflow '{stats --group=prot --group=tos --count packets}' $data/netflow-5.dat
		OUTPUT - $'  FIELD           COUNT            prot             tos
packets              69               1               0
packets             120               6               0
packets              22               6             192
packets              36              17               0'
	EXEC -x netflow '{stats --group=prot --group=tos --count --deviation packets}' $data/netflow-5.dat
		OUTPUT - $'  FIELD           COUNT       DEVIATION            prot             tos
packets              69  2.01468134e+00               1               0
packets             120  1.89836045e+03               6               0
packets              22  1.67334945e+01               6             192
packets              36  5.91008651e+00              17               0'

TEST 02 'print output'
	EXEC -x netflow '{stats --group=prot --group=tos --print="%(prot)u|%(tos)u|%(COUNT)u|%(DEVIATION)f" packets}' $data/netflow-5.dat
	OUTPUT - $'1|0|69|2.014681
6|0|120|1898.360454
6|192|22|16.733495
17|0|36|5.910087'
