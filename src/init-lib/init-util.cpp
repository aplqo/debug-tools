#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <string>

#include "include/init-list.h"
#include "include/init.h"
#include "include/utility.h"
using apdebug::Input::readFileVal;
using apdebug::Output::writeFile;
using std::cout;
using std::string;
using std::unique_ptr;
using std::filesystem::path;
namespace fs = std::filesystem;

namespace apdebug::init {
template class list<compiler*>;
list<compiler*>* shared()
{
  static unique_ptr<list<compiler*>> obj(new list<compiler*>("Shared"));
  return obj.get();
}
template class list<editor*>;
list<editor*>* editors()
{
  static unique_ptr<list<editor*>> obj(new list<editor*>("Editors"));
  return obj.get();
}
CommandList* preScript()
{
  static std::unique_ptr<CommandList> obj(new CommandList);
  return obj.get();
}
CommandList* postScript()
{
  static std::unique_ptr<CommandList> obj(new CommandList);
  return obj.get();
}

string readFileLn(const path& p)
{
  string ret;
  std::ifstream f(p);
  std::getline(f, ret);
  return ret;
}
template <bool front>
void readCommandFile(const path& p, CommandList* lst)
{
  CommandList cur;
  std::ifstream f(p);
  if (!f.is_open()) return;
  while (f) {
    std::string s;
    getline(f, s);
    if (!s.empty()) cur.push_back([v = std::move(s)]() { system(v.c_str()); });
  }
  if constexpr (front) {
    cur.splice(cur.end(), std::move(*lst));
    std::swap(cur, *lst);
  }
  else
    lst->splice(lst->end(), std::move(cur));
}
template void readCommandFile<true>(const path&, CommandList*);
template void readCommandFile<false>(const path&, CommandList*);

compiler* editor::find(const string& c) const
{
  compiler* ptr = com.find(c);
  return ptr ? ptr : shared()->find(c);
}
void editor::read()
{
  unsigned int sel;
  print();
  std::cout << "Enter compiler: ";
  std::cout.flush();
  std::cin >> sel;
  while (sel >= com.size() && sel - com.size() > shared()->size()) {
    std::cout << "No such config!" << std::endl;
    std::cout << "Enter compiler: ";
    std::cout.flush();
    std::cin >> sel;
  }
  std::cout << std::endl;
  c = sel >= com.size() ? shared()->lst[sel - com.size()] : com.lst[sel];
  c->read();
}
void editor::add(compiler* c) { com.append(c); }
void editor::load(const path& dest, const Operate op)
{
  c = find(readFileLn(dest / ".config" / "compiler"));
  c->load(dest, op);
}
void editor::install(const path& src, const path& dest)
{
  writeFile(dest / ".config" / "compiler", c->name);
  initImpl(src, dest);
  c->install(src, dest);
}
void editor::init(const path& src, const path& dest)
{
  initImpl(src, dest);
  c->init(src, dest);
}
void editor::print() const
{
  com.print();
  std::cout << std::endl;
  shared()->print(com.lst.size());
}
void editor::update(const path& src, const path& dest)
{
  deinitImpl(dest);
  initImpl(src, dest);
  c->update(src, dest);
}
void editor::deinit(const path& dest)
{
  c->deinit(dest);
  deinitImpl(dest);
}
void editor::uninstall(const path& dest)
{
  c->uninstall(dest);
  deinitImpl(dest);
}

void compiler::install(const path& src, const path& dest)
{
  initImpl(src, dest);
}
void compiler::init(const path& src, const path& dest) { initImpl(src, dest); }
void compiler::update(const path& src, const path& dest)
{
  deinitImpl(dest);
  initImpl(src, dest);
}
void compiler::deinit(const path& dest) { deinitImpl(dest); }
void compiler::uninstall(const path& dest) { deinitImpl(dest); }
}  // namespace apdebug::init
