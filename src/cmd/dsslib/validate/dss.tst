# tests for the dss validate query

TITLE + validate

export TZ=EST5EDT

VIEW data ../bgp/data

TEST 01 'diagnostics'
	EXEC -x bgp '{validate}|{count}' $data/mrt.dat
		OUTPUT - $'19649/19649'
		ERROR - $'dss::validate: warning: no field has constraints or maps'
	EXEC -x bgp-map '{validate --repair}|{count}' $data/mrt.dat
		OUTPUT -
		ERROR - $'dss::validate: reair requires CX_SET callout'
		EXIT 1

TEST 02 'summaries and counts'
	EXEC -x bgp-map '{validate --summary}' $data/mrt.dat
		OUTPUT - $'           FIELD        COUNT  VALUE
          agg_as           32  8347
          agg_as            6  11561
          origin         2909  48
          src_as           10  4608

           FIELD      INVALID   DISCARDED    REPAIRED
          agg_as           38           0           0
          origin         2909           0           0
          src_as           10           0           0'
	EXEC -x bgp-map '{count}' $data/mrt.dat
		OUTPUT - $'19649/19649'
	EXEC -x bgp-map '{scan}|{count}' $data/mrt.dat
	EXEC -x bgp-map '{validate}|{count}' $data/mrt.dat
	EXEC -x bgp-map '{scan}|{validate}|{count}' $data/mrt.dat
	EXEC -x bgp-map '{validate --discard}|{count}' $data/mrt.dat
		OUTPUT - $'16702/19649'
