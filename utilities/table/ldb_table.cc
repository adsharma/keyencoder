// Copyright (c) 2016 Facebook. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if 0
#include "utilities/table/ldb_table.h"

#include "utilities/table/ldb_table_cmd.h"
#include "rocksdb/comparator.h"

using rocksdb::Dynamic;

namespace rocksdb { namespace table {

void LDBTool::Run(int argc, char** argv, rocksdb::Options options,
                  const rocksdb::LDBOptions& ldb_options,
                  const std::vector<ColumnFamilyDescriptor>* column_families) {
  options.comparator = rocksdb::ReverseBytewiseComparator();
  rocksdb::LDBOptions ldbOptions = ldb_options;
  rocksdb::table::LDBCommandRunner::RunCommand(argc, argv, options, ldbOptions,
                                               column_families);
}
}}  // namespace rocksdb::table

int main(int argc, char** argv) {
  rocksdb::table::LDBTool tool;
  tool.Run(argc, argv);
  return 0;
}
#else
int main(int argc, char** argv) {
  return 0;
}
#endif

