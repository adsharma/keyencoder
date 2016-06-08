This repo contains some algorithms to encode some
primitive data types (int64_t, double, strings) to
other strings such that comparing the encoded
strings has the same outcome as 
comparing std::tuple<..> of typed values.

Such a library would be useful to build a database
on top of a key value store such as RocksDB.

Common problems many rocksdb users solve to build a
table abstraction:

1. Writing a reverse lexicographic comparator
2. Prefix scan with such a comparator
3. Skip and scan query plans
4. Types - lazy conversion of bytes to typed tuples
5. Storing variable length strings without affecting ordering
6. Iterators templatized by key/value types
7. Delete keys by predicate

See the github wiki for more info on RocksDB.
