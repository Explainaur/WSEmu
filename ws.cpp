#include <map>
#include <list>
#include <stack>
#include <utility>
#include <vector>
#include <fstream>
#include <iostream>

#define TRACE0() \
  do { \
    printf("%s:%d:%s\n", __FILE__, __LINE__, __FUNCTION__); \
  } while (0)

#define DBG_ASSERT(cond, fmt, ...) \
  do { \
    if (!(cond)) { \
      printf("error: %s:%d:%s\n  ", __FILE__, __LINE__, __FUNCTION__); \
      printf(fmt, ##__VA_ARGS__); \
      abort(); \
    } \
  } while (0)

using namespace std;

enum class Opcode {
  push = 0,
  pop = 1,
  dup = 2,
  store = 3,
  retrieve = 4,
  add = 5,
  sub = 6,
  mul = 7,
  div = 8,
  mod = 9,
  jmp = 10,
  jz = 11,
  jn = 12,
  putchar = 13,
  getchar = 14,
  halt = 15,
  exit = 16,
  discard = 17,
};

class Inst;

class BasicBlock;

using Tokens = vector<std::string>;
using InstPtr = shared_ptr<Inst>;
using BlockPtr = shared_ptr<BasicBlock>;
using Insts = vector<InstPtr>;


class Inst {
private:
  int _line;
  Opcode _opcode;
  Tokens _oprands;
public:
  Inst(int line, Opcode opcode, Tokens tokens)
      : _line(line), _opcode(opcode), _oprands(std::move(tokens)) {}

  void dump();

  Opcode opcode() { return _opcode; }

  const Tokens &oprands() { return _oprands; }
};

InstPtr MakeInst(int line, Opcode opcode, const Tokens &tokens) {
  return std::make_shared<Inst>(line, opcode, tokens);
}

class BasicBlock {
private:
  int _label;
  Insts _insts;

  BlockPtr _pred, _succ;

public:
  BasicBlock(int label)
      : _label(label) {}

  const Insts &insts() { return _insts; }

  int label() const { return _label; }

  void InsertInst(const InstPtr &inst) {
    _insts.push_back(inst);
  }


  void SetPred(const BlockPtr &pred) {
    _pred = pred;
  }

  void SetSucc(const BlockPtr &succ) { _succ = succ; }
};

void DumpInst(const Tokens &tokens);

class Module {
private:
  deque<int> _stack;
  int *_heap;
  vector<BlockPtr> _cfg;
  map<int, BlockPtr> _symtab;
  BlockPtr _current;

  // used for eval
  Insts::const_iterator _pc;
  vector<BlockPtr>::const_iterator _cur_BB_it;

public:

  int pop() {
    int value = _stack.back();
    _stack.pop_back();
    return value;
  }

  BlockPtr SetCFG(int name) {
    auto cfg = std::make_shared<BasicBlock>(name);
    _cfg.push_back(cfg);
    _current = cfg;
    _symtab.insert(make_pair(name, cfg));
    return cfg;
  }

  InstPtr ParseInst(const Tokens &tokens, int lineNumber);

  void Eval();

  Tokens Parse(const string &line) {
    string token;
    Tokens tokens;
    for (const auto &it : line) {
      if (it != ' ') {
        token += it;
      } else {
        if (token.empty()) continue;
        tokens.push_back(token);
        token = "";
      }
    }

    if (!token.empty()) tokens.push_back(token);

    return tokens;
  }

  void DumpCFG() {
    for (const auto &BB : _cfg) {
      cout << BB->label() << ":" << endl;
      for (const auto &it : BB->insts()) {
        it->dump();
      }
      cout << endl;
    }
  }

  const vector<BlockPtr> &CFG() { return _cfg; }

  void EvalInit() {
    _heap = new int[1000000000];
    _cur_BB_it = _cfg.begin();
    _pc = _symtab[99999]->insts().begin();
  }

  void DumpState();

  // run passes

  // build pred-succ
  void BlockInfo() {
    for (const auto &it : _cfg) {

    }
  }
};

int main() {
  fstream file;
  file.open("1.asm", std::fstream::in);

  std::string line;

  int lineNumber = 1;
  Tokens tokens;
  Module module;

  while (getline(file, line)) {

    tokens = module.Parse(line);
    module.ParseInst(tokens, lineNumber);
    lineNumber++;
  }

//  module.DumpCFG();

//  auto cfg = module.CFG();
//  for (const auto &it : cfg[0]->insts()) {
//    it->dump();
//  }
  module.EvalInit();
  while (true) {
    module.Eval();
  }
}

void DumpInst(const Tokens &tokens) {
  for (const auto &it : tokens) {
    cout << it << " ";
  }
  cout << endl;
}

void Inst::dump() {
  string opcode;
  Tokens tokens;
  switch (_opcode) {
    case Opcode::push:
      opcode = "push";
      break;
    case Opcode::pop:
      opcode = "pop";
      break;
    case Opcode::dup:
      opcode = "dup";
      break;
    case Opcode::store:
      opcode = "store";
      break;
    case Opcode::retrieve:
      opcode = "retrieve";
      break;
    case Opcode::add:
      opcode = "add";
      break;
    case Opcode::sub:
      opcode = "sub";
      break;
    case Opcode::mul:
      opcode = "mul";
      break;
    case Opcode::div:
      opcode = "div";
      break;
    case Opcode::mod:
      opcode = "mod";
      break;
    case Opcode::jmp:
      opcode = "jmp";
      break;
    case Opcode::jz:
      opcode = "jz";
      break;
    case Opcode::jn:
      opcode = "jn";
      break;
    case Opcode::putchar:
      opcode = "putchar";
      break;
    case Opcode::getchar:
      opcode = "getchar";
      break;
    case Opcode::halt:
      opcode = "halt";
      break;
    case Opcode::exit:
      opcode = "exit";
      break;
    case Opcode::discard:
      opcode = "discard";
      break;
  }

  tokens.push_back(opcode);
  for (const auto &it : _oprands) {
    tokens.push_back(it);
  }
  DumpInst(tokens);
}


InstPtr Module::ParseInst(const Tokens &tokens, int lineNumber) {
  Opcode op;
  if (tokens[0] == "push") {
    op = Opcode::push;
  } else if (tokens[0] == "pop") {
    op = Opcode::pop;
  } else if (tokens[0] == "dup") {
    op = Opcode::dup;
  } else if (tokens[0] == "store") {
    op = Opcode::store;
  } else if (tokens[0] == "retrieve") {
    op = Opcode::retrieve;
  } else if (tokens[0] == "add") {
    op = Opcode::add;
  } else if (tokens[0] == "sub") {
    op = Opcode::sub;
  } else if (tokens[0] == "mul") {
    op = Opcode::mul;
  } else if (tokens[0] == "div") {
    op = Opcode::div;
  } else if (tokens[0] == "mod") {
    op = Opcode::mod;
  } else if (tokens[0] == "jump") {
    op = Opcode::jmp;
  } else if (tokens[0] == "jz") {
    op = Opcode::jz;
  } else if (tokens[0] == "jn") {
    op = Opcode::jn;
  } else if (tokens[0] == "outchar") {
    op = Opcode::putchar;
  } else if (tokens[0] == "readchar") {
    op = Opcode::getchar;
  } else if (tokens[0] == "halt") {
    op = Opcode::halt;
  } else if (tokens[0] == "exit") {
    op = Opcode::exit;
  } else if (tokens[0] == "discard") {
    op = Opcode::discard;
  } else {
    if (tokens[0] == "label") {
      SetCFG(stoi(tokens[1]));
      return nullptr;
    }
    cout << tokens[0] << endl;
    DBG_ASSERT(0, "unknown opcode");
  }

  Tokens operand;
  for (int i = 1; i < tokens.size(); i++) {
    operand.push_back(tokens[i]);
  }

  auto inst = MakeInst(lineNumber, op, operand);

  _current->InsertInst(inst);
  return inst;
}

void Module::Eval() {
  auto &inst = *_pc;
//  inst->dump();
  _pc++;
  if (_pc == (*_cur_BB_it)->insts().end()) {
    _cur_BB_it++;
    _pc = (*_cur_BB_it)->insts().begin();
  }
  switch (inst->opcode()) {
    case Opcode::push: {
      _stack.push_back(stoi(inst->oprands()[0]));
      break;
    }
    case Opcode::pop: {
      _stack.pop_back();
      break;
    }
    case Opcode::dup: {
      int top = _stack.back();
      _stack.push_back(top);
      break;
    }
    case Opcode::store: {
      int value = pop();
      int addr = pop();
      _heap[addr] = value;
      break;
    }
    case Opcode::retrieve: {
      int addr = pop();
      int value = _heap[addr];
      _stack.push_back(value);
      break;
    }
    case Opcode::add: {
      int rhs = pop();
      int lhs = pop();
      _stack.push_back(lhs + rhs);
      break;
    }
    case Opcode::sub: {
      int rhs = pop();
      int lhs = pop();
      _stack.push_back(lhs - rhs);
      break;
    }
    case Opcode::mul: {
      int rhs = pop();
      int lhs = pop();
      _stack.push_back(lhs * rhs);
      break;
    }
    case Opcode::div: {
      int rhs = pop();
      int lhs = pop();
      _stack.push_back(lhs / rhs);
      break;
    }
    case Opcode::mod: {
      int rhs = pop();
      int lhs = pop();
      _stack.push_back(lhs % rhs);
      break;
    }
    case Opcode::jmp: {
      int addr = stoi(inst->oprands()[0]);
      auto target = _symtab[addr];
      _pc = target->insts().begin();
      _cur_BB_it = find(_cfg.begin(), _cfg.end(), target);
      break;
    }
    case Opcode::jz: {
      int value = pop();
      if (value == 0) {
        int addr = stoi(inst->oprands()[0]);
        auto target = _symtab[addr];
        _pc = target->insts().begin();
        _cur_BB_it = find(_cfg.begin(), _cfg.end(), target);

      }
      break;
    }
    case Opcode::jn: {
      int value = pop();
      if (value < 0) {
        int addr = stoi(inst->oprands()[0]);
        auto target = _symtab[addr];
        _pc = target->insts().begin();
        _cur_BB_it = find(_cfg.begin(), _cfg.end(), target);
      }
      break;
    }
    case Opcode::putchar: {
      char ch = (char) (_stack.back());
      cout << ch;
      break;
    }
    case Opcode::getchar: {
      long ch;
      int addr = pop();
      _heap[addr] = ch = cin.get();
      break;
    }
    case Opcode::halt: {

      break;
    }
    case Opcode::exit: {
      exit(0);
    }
    case Opcode::discard: {
      _stack.pop_back();
      break;
    }
  }
//  DumpState();
}

void Module::DumpState() {
  cout << "Heap[0]: " << _heap[0]
       << " stack: ";
  int i = 0;
  for (const auto &it : _stack) {
    cout << it;
    if (i != _stack.size() -1 ) cout << " -> ";
    i++;
  }
  cout << endl;
}