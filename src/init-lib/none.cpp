#include <filesystem>
#include <iostream>
#include <string>

#include "include/init.h"

using std::string;
using std::filesystem::path;
namespace apdebug::init::none {
class noeditor : public editor {
 public:
  noeditor() : editor("none", "Don't init editor config")
  {
    editors()->append(this);
  }
  compiler* find(const string&) const { return nullptr; }
  void read() override {}
  void print() const override {}
  void load(const path&, const Operate) override {}
  void install(const path&, const path&) override {}
  void init(const path&, const path&) override {}
  void update(const path&, const path&) override {}
  void deinit(const path&) override {}
  void uninstall(const path&) override {}

 private:
  void initImpl(const path&, const path&) override {}
  void deinitImpl(const path&) {}
} noedit;

class nocompiler : public compiler {
 public:
  nocompiler() : compiler("none", "Don't init compiler config")
  {
    shared()->append(this);
  }
  void install(const path&, const path&) override {}
  void init(const path&, const path&) override {}
  void update(const path&, const path&) override {}
  void deinit(const path&) override {}
  void uninstall(const path&) override {}

 private:
  void initImpl(const path&, const path&) override {}
  void deinitImpl(const path&) override {}
} nocompile;
}  // namespace apdebug::init::none