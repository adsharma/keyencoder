// Copyright (c) 2016 Facebook. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#pragma once

#include "rocksdb/utilities/ldb_cmd.h"

#include <map>
#include <string>
#include <utility>
#include <vector>

#include "rocksdb/utilities/table_interface.h"
#include "rocksdb/utilities/table_serialization.h"

namespace rocksdb { namespace table {

class LDBCommand : public rocksdb::LDBCommand {
 public:
  static const std::string ARG_ORDER;
  static const std::string ARG_SCHEMA;

  template <typename Selector>
  static rocksdb::LDBCommand* InitFromCmdLineArgs(
    int argc,
    char** argv,
    const Options& options,
    const LDBOptions& ldb_options,
    const std::vector<ColumnFamilyDescriptor>* column_families,
    Selector selector = SelectCommand) {
    std::vector<std::string> args;
    for (int i = 1; i < argc; i++) {
      args.push_back(argv[i]);
    }
    return rocksdb::LDBCommand::InitFromCmdLineArgs(
      args, options, ldb_options, column_families,
      selector);
  }

  static rocksdb::LDBCommand* SelectCommand(
    const std::string& cmd,
    const std::vector<std::string>& cmdParams,
    const std::map<std::string, std::string>& option_map,
    const std::vector<std::string>& flags);

  LDBCommand(const std::map<std::string, std::string>& options, const std::vector<std::string>& flags,
             bool is_read_only, const std::vector<std::string>& valid_cmd_line_options);

 protected:
  void ParseSchemaFile();
  static void ParseKeyValue(std::vector<rocksdb::Dynamic>& key,
                            std::vector<rocksdb::Dynamic>& val,
                            const std::vector<std::string>& params);

  bool descending_;
  std::string schema_path_;
  std::map<int64_t,
           std::pair<std::vector<Dynamic>, std::vector<Dynamic>>> schema_;
};

class TPutCommand : public LDBCommand {
 public:
  static std::string Name() { return "tput"; }

  TPutCommand(const std::vector<std::string>& params,
              const std::map<std::string, std::string>& options,
              const std::vector<std::string>& flags);

  virtual void DoCommand() override;

  static void Help(std::string& ret);

  virtual Options PrepareOptionsForOpenDB() override;

 private:
  std::vector<rocksdb::Dynamic> key_;
  std::vector<rocksdb::Dynamic> value_;
};

class TLoadCommand : public LDBCommand {
 public:
  static std::string Name() { return "tload"; }

  TLoadCommand(const std::vector<std::string>& params,
              const std::map<std::string, std::string>& options,
              const std::vector<std::string>& flags);

  virtual void DoCommand() override;

  static void Help(std::string& ret);

  virtual Options PrepareOptionsForOpenDB() override;

 private:
  std::vector<rocksdb::Dynamic> key_;
  std::vector<rocksdb::Dynamic> value_;
};

class TScanCommand : public LDBCommand {
 public:
  static std::string Name() { return "tscan"; }

  TScanCommand(const std::vector<std::string>& params,
               const std::map<std::string, std::string>& options,
               const std::vector<std::string>& flags);

  virtual void DoCommand() override;

  static void Help(std::string& ret);

  virtual Options PrepareOptionsForOpenDB() override;

 private:
  std::string start_key_;
  std::string end_key_;
  bool start_key_specified_;
  bool end_key_specified_;
  int max_keys_scanned_;

  std::vector<rocksdb::Dynamic> key_;
  std::vector<rocksdb::Dynamic> value_;
};

inline void PrintVector(const std::vector<Dynamic>& vec) {
  bool first = true;
  for (const auto& r : vec) {
    if (!first) {
      fputs(", ", stdout);
    } else {
      first = false;
    }
    fputs(r.toString().c_str(), stdout);
  }
}

class LDBCommandRunner {
 public:
  static void PrintHelp(const char* exec_name) {
    rocksdb::LDBCommandRunner::PrintHelp(exec_name);

    std::string ret;
    ret.append("\n\n");
    ret.append("Table Access Commands:\n");
    TPutCommand::Help(ret);
    TScanCommand::Help(ret);
    TLoadCommand::Help(ret);

    fprintf(stderr, "%s\n", ret.c_str());
  }

  static void RunCommand(int argc, char** argv, Options options,
                         const LDBOptions& ldb_options,
                         const std::vector<ColumnFamilyDescriptor>*
                           column_families) {
    using rocksdb::table::LDBCommand;

    if (argc <= 2) {
      PrintHelp(argv[0]);
      exit(1);
    }

    auto selector = rocksdb::table::LDBCommand::SelectCommand;
    auto* cmdObj = LDBCommand::InitFromCmdLineArgs(argc, argv, options,
                                                   ldb_options, column_families,
                                                   selector);

    if (cmdObj == nullptr) {
      fprintf(stderr, "Unknown command\n");
      PrintHelp(argv[0]);
      exit(1);
    }

    if (!cmdObj->ValidateCmdLineOptions()) {
      exit(1);
    }

    cmdObj->Run();
    LDBCommandExecuteResult ret = cmdObj->GetExecuteState();
    fprintf(stderr, "%s\n", ret.ToString().c_str());
    delete cmdObj;

    exit(ret.IsFailed());
  }

};

}} // namespace rocksdb::table
