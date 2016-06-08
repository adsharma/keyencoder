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

The table interface is really about schematizing the way keys and values
are encoded. It doesn't necessarily impose the relational data model on
your db. Although you can implement (multiple) relational tables in a
single rocksdb instance, you can also use semi-relational, hierarchical
clustered and graph data models.

If you use the schema-id as the first column, you end up with a (nearly)
relational data model. However, if you use the ID of the entity being
stored as the first column (and schema-id as the second column), you end
up with a hierarchical clustered data model. An example of this usage
model is storing albums and photos such that all photos belonging to
an album can be queried using a prefix scan instead of a joining the
photo table with the album table. This choice largely depends on your
data access patterns.

In the graph data model, you can choose to store all vertices and all
edges separately or you can store edges close to vertices they belong to.

## Installation from source:

Install google test for your distribution. 1.7.0 recommended

```
yum install gtest-devel
apt install libgtest-dev
```

Use a recent toolchain that supports C++11.

CentOS 6:

```
$ DTLS_RPM=rhscl-devtoolset-3-epel-6-x86_64.noarch.rpm
$ DTLS_RPM_URL=https://www.softwarecollections.org/en/scls/rhscl/devtoolset-3/epel-6-x86_64/download/${DTLS_RPM}
$ wget ${DTLS_RPM_URL} -O ${DTLS_RPM}
$ sudo yum install -y scl-utils ${DTLS_RPM}
$ sudo yum install -y devtoolset-3-toolchain
$ scl enable devtoolset-3 bash
```

Ubuntu:

Use a recent distribution such as 16.04

```
mkdir build
cd build; cmake ..
make -j
```
