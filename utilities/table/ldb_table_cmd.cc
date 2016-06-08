//  Copyright (c) 2013, Facebook, Inc.  All rights reserved.
//  This source code is licensed under the BSD-style license found in the
//  LICENSE file in the root directory of this source tree. An additional grant
//  of patent rights can be found in the PATENTS file in the same directory.
//
#ifndef ROCKSDB_LITE
#include "utilities/table/ldb_table_cmd.h"

#include <fstream>
#include <iostream>
#include <sstream>
#include <rocksdb/comparator.h>

namespace rocksdb { namespace table {

using rocksdb::RecursiveDynamic;
using namespace std;

const string LDBCommand::ARG_ORDER = "order";
const string LDBCommand::ARG_SCHEMA = "schema";

LDBCommand::LDBCommand(const map<string, string>& options,
                       const vector<string>& flags,
                       bool is_read_only,
                       const vector<string>& valid_cmd_line_options) :
  rocksdb::LDBCommand(options, flags, is_read_only, valid_cmd_line_options) {
  map<string, string>::const_iterator itr = options.find(ARG_ORDER);
  descending_ = true;
  if (itr != options.end()) {
    if (itr->second == "asc") {
      descending_ = false;
    }
  }
  itr = options.find(ARG_SCHEMA);
  if (itr != options.end()) {
    schema_path_ = itr->second;
  }
}

// Please keep this in-sync with Dynamic.h
static std::map<std::string, rocksdb::Dynamic::Type> typeNameMap = {
  { "blank", rocksdb::Dynamic::T_BLANK},
  { "bool", rocksdb::Dynamic::T_BOOL},
  { "double", rocksdb::Dynamic::T_DOUBLE},
  { "int", rocksdb::Dynamic::T_INT},
  { "string", rocksdb::Dynamic::T_STRING},
  { "slice", rocksdb::Dynamic::T_SLICE},
};

void LDBCommand::ParseSchemaFile() {
  std::ifstream schema_file(schema_path_);
  std::string line;
  bool has_complete_line = false;
  while (true) {
    bool seen_delim = false;
    if (std::getline(schema_file, line)) {
      has_complete_line = !schema_file.eof();
    } else {
      has_complete_line = false;
    }
    int64_t schema;
    std::vector<rocksdb::Dynamic> kvec;
    std::vector<rocksdb::Dynamic> vvec;
    if (has_complete_line) {
      std::stringstream ss(line);
      std::string type_name;
      ss >> schema;
      std::string delim = DELIM;
      delim = delim.substr(1, delim.size() - 2);
      while (ss >> type_name) {
        if (type_name == delim) {
          seen_delim = true;
          continue;
        }
        auto typeEnum = typeNameMap.at(type_name);
        if (seen_delim) {
          vvec.emplace_back(Dynamic(typeEnum));
        } else {
          kvec.emplace_back(Dynamic(typeEnum));
        }
      }
      schema_[schema] = {std::move(kvec), std::move(vvec)};
    } else {
      break;
    }
  }
}

void LDBCommand::ParseKeyValue(std::vector<Dynamic>& key,
                               std::vector<Dynamic>& val,
                               const vector<string>& params) {
  std::string delim = DELIM;
  delim = delim.substr(1, delim.size() - 2);
  auto sep = std::find(params.begin(), params.end(), delim);
  auto kIt = params.begin();
  for (; kIt != sep; kIt++) {
    if (kIt->at(0) == '"') {
      const auto& str = kIt->substr(1, kIt->size() - 2);
      key.emplace_back(str);
    } else {
      // Assumed to be an int
      key.emplace_back(std::atoi(kIt->c_str()));
    }
  }
  kIt++;  // skip over sep
  for (auto vIt = kIt; vIt != params.end(); vIt++) {
    if (vIt->at(0) == '"') {
      const auto& str = vIt->substr(1, vIt->size() - 2);
      val.emplace_back(str);
    } else {
      // Assumed to be an int
      val.emplace_back(std::atoi(vIt->c_str()));
    }
  }
}

TPutCommand::TPutCommand(const vector<string>& params,
      const map<string, string>& options, const vector<string>& flags) :
  LDBCommand(options, flags, false,
             BuildCmdLineOptions({ARG_CREATE_IF_MISSING, ARG_ORDER,
                                  ARG_SCHEMA})) {
  if (params.size() < 2) {
    exec_state_ = LDBCommandExecuteResult::Failed(
        "<key> and <value> must be specified for the put command");
  } else {
    ParseKeyValue(key_, value_, params);
  }
  ParseSchemaFile();
}

void TPutCommand::Help(string& ret) {
  ret.append("  ");
  ret.append(TPutCommand::Name());
  ret.append("<keys> ==> <values> ");
  ret.append(" [--" + ARG_TTL + "]");
  ret.append(" [--" + ARG_SCHEMA + "]");
  ret.append(" [--" + ARG_ORDER + "]");
  ret.append("\n");
  ret.append("Strings should be enclosed in double quotes\n");
}

void TPutCommand::DoCommand() {
  Status st = TableInterface::Put(db_, WriteOptions(), key_, value_);
  if (st.ok()) {
    fprintf(stdout, "OK\n");
  } else {
    exec_state_ = LDBCommandExecuteResult::Failed(st.ToString());
  }
}

Options TPutCommand::PrepareOptionsForOpenDB() {
  Options opt = LDBCommand::PrepareOptionsForOpenDB();
  if (!descending_) {
    opt.comparator = rocksdb::BytewiseComparator();
  }
  opt.create_if_missing = IsFlagPresent(flags_, ARG_CREATE_IF_MISSING);
  return opt;
}

TScanCommand::TScanCommand(const vector<string>& params,
      const map<string, string>& options, const vector<string>& flags) :
    LDBCommand(options, flags, true,
               BuildCmdLineOptions({ARG_TO, ARG_FROM, ARG_TIMESTAMP,
                                    ARG_MAX_KEYS, ARG_ORDER, ARG_SCHEMA})),
    start_key_specified_(false),
    end_key_specified_(false),
    max_keys_scanned_(-1) {
  map<string, string>::const_iterator itr = options.find(ARG_FROM);
  if (itr != options.end()) {
    start_key_ = itr->second;
    if (is_key_hex_) {
      start_key_ = HexToString(start_key_);
    }
    start_key_specified_ = true;
  }
  itr = options.find(ARG_TO);
  if (itr != options.end()) {
    end_key_ = itr->second;
    if (is_key_hex_) {
      end_key_ = HexToString(end_key_);
    }
    end_key_specified_ = true;
  }

  itr = options.find(ARG_MAX_KEYS);
  if (itr != options.end()) {
    try {
#if defined(CYGWIN)
      max_keys_scanned_ = strtol(itr->second.c_str(), 0, 10);
#else
      max_keys_scanned_ = stoi(itr->second);
#endif
    } catch(const std::invalid_argument&) {
      exec_state_ = LDBCommandExecuteResult::Failed(ARG_MAX_KEYS +
                                                    " has an invalid value");
    } catch(const std::out_of_range&) {
      exec_state_ = LDBCommandExecuteResult::Failed(
          ARG_MAX_KEYS + " has a value out-of-range");
    }
  }
  ParseSchemaFile();
}

void TScanCommand::Help(string& ret) {
  ret.append("  ");
  ret.append(TScanCommand::Name());
  ret.append(HelpRangeCmdArgs());
  ret.append(" [--" + ARG_TIMESTAMP + "]");
  ret.append(" [--" + ARG_ORDER + "]");
  ret.append(" [--" + ARG_SCHEMA + "]");
  ret.append(" [--" + ARG_MAX_KEYS + "=<N>q] ");
  ret.append("\n");
}

void TScanCommand::DoCommand() {

  int num_keys_scanned = 0;
  ReadOptions read_options;

  read_options.pin_data = true;
  Iterator* it = db_->NewIterator(read_options);
  if (start_key_specified_) {
    it->Seek(start_key_);
  } else {
    it->SeekToFirst();
  }
  for ( ;
        it->Valid() && (!end_key_specified_ || it->key().ToString() < end_key_);
        it->Next()) {

    auto table_it = new TableIterator(it);
    // TODO: generalize this to the case where the schema id is a different
    // column. For now, we assume the first column is schema id.
    std::vector<Dynamic> key = {
      Dynamic(Dynamic::T_INT),
    };
    std::vector<Dynamic> value;
    table_it->key(&key);
    const auto& p = schema_.at(key[0].getInt());
    key = p.first;
    value = p.second;
    table_it->key(&key);
    table_it->value(&value);
    PrintVector(key);
    fprintf(stdout, DELIM);
    PrintVector(value);
    fprintf(stdout, "\n");

    num_keys_scanned++;
    if (max_keys_scanned_ >= 0 && num_keys_scanned >= max_keys_scanned_) {
      break;
    }
  }
  if (!it->status().ok()) {  // Check for any errors found during the scan
    exec_state_ = LDBCommandExecuteResult::Failed(it->status().ToString());
  }
  delete it;
}

Options TScanCommand::PrepareOptionsForOpenDB() {
  Options opt = LDBCommand::PrepareOptionsForOpenDB();
  if (!descending_) {
    opt.comparator = rocksdb::BytewiseComparator();
  }
  return opt;
}

TLoadCommand::TLoadCommand(const vector<string>& params,
      const map<string, string>& options, const vector<string>& flags) :
  LDBCommand(options, flags, false,
             BuildCmdLineOptions({ARG_CREATE_IF_MISSING, ARG_ORDER,
                                  ARG_SCHEMA})) {
  ParseSchemaFile();
}

void TLoadCommand::Help(string& ret) {
  ret.append("  ");
  ret.append(TLoadCommand::Name());
  ret.append(" [--" + ARG_TTL + "]");
  ret.append(" [--" + ARG_SCHEMA + "]");
  ret.append(" [--" + ARG_ORDER + "]");
  ret.append("\n");
}

void TLoadCommand::DoCommand() {
  std::string line;
  size_t count = 1;
  while (getline(std::cin, line, '\n')) {
    stringstream ss(line);
    std::vector<string> params;

    while (true) {
      std::string one;
      if (!std::getline(ss, one, ' ')) {
        break;
      }
      params.push_back(one);
    }
    key_.clear();
    value_.clear();
    ParseKeyValue(key_, value_, params);

    Status st = TableInterface::Put(db_, WriteOptions(), key_, value_);
    if (!st.ok()) {
      exec_state_ = LDBCommandExecuteResult::Failed(st.ToString());
    }
    if (count++ % 10000 == 0) {
      fprintf(stdout, ".");
    }
  }
  fprintf(stdout, "\nOK\n");
}

Options TLoadCommand::PrepareOptionsForOpenDB() {
  Options opt = LDBCommand::PrepareOptionsForOpenDB();
  if (!descending_) {
    opt.comparator = rocksdb::BytewiseComparator();
  }
  opt.create_if_missing = IsFlagPresent(flags_, ARG_CREATE_IF_MISSING);
  return opt;
}

rocksdb::LDBCommand* LDBCommand::SelectCommand(
    const std::string& cmd,
    const vector<string>& cmdParams,
    const map<string, string>& option_map,
    const vector<string>& flags
  ) {
  if (cmd == TPutCommand::Name()) {
    return new TPutCommand(cmdParams, option_map, flags);
  } else if (cmd == TScanCommand::Name()) {
    return new TScanCommand(cmdParams, option_map, flags);
  } else if (cmd == TLoadCommand::Name()) {
    return new TLoadCommand(cmdParams, option_map, flags);
  }

  auto cmdPtr = rocksdb::LDBCommand::SelectCommand(cmd, cmdParams, option_map,
                                                   flags);

  if (cmdPtr) {
    return cmdPtr;
  }

  return nullptr;
}

}}   // namespace rocksdb::table
#endif  // ROCKSDB_LITE
