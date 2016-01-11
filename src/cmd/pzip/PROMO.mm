.H 1 pzip
Fixed length record data is often viewed as a waste of space,
too sparse for production use.
Much effort is then put into optimizing
the data schema, and in the process complicating the data interface.
.B pzip
shows that in many cases this view of fixed length data is wrong.
In fact, variable length data may become more compressible when
converted to a sparse, fixed length format.
Intense semantic schema
analysis can be replaced by an automated record partition, resulting in
compression space improvements of 2 to 10 times and decompression speed
improvements of 2 to 3 times over
.B gzip
for a large class of data.
