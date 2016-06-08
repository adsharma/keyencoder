//  Copyright (c) 2016, Facebook, Inc.  All rights reserved.
//  This source code is licensed under the BSD-style license found in the
//  LICENSE file in the root directory of this source tree. An additional grant
//  of patent rights can be found in the PATENTS file in the same directory.
//
#pragma once

#include "rocksdb/ldb_tool.h"
#include "rocksdb/utilities/ldb_cmd.h"

#include <vector>

namespace rocksdb { namespace table {

class LDBTool : public rocksdb::LDBTool {
 public:
  void Run(int argc, char** argv, rocksdb::Options = rocksdb::Options(),
           const rocksdb::LDBOptions& ldb_options = rocksdb::LDBOptions(),
           const std::vector<ColumnFamilyDescriptor>* column_families
             = nullptr);
};

}}  // namespace rocksdb::table
