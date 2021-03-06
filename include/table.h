#ifndef TABLE_H
#define TABLE_H

#include <array>
#include <concepts>
#include <cstring>
#include <list>
#include <ostream>
#include <string>

#include "include/color.h"

namespace apdebug::Table {
template <class T>
concept ColumnId = std::convertible_to<T, unsigned int> || requires
{
  typename std::is_enum<T>::value_type;
};
template <class T>
concept ColumnData = std::convertible_to<T, std::string>;
template <size_t siz>
class Table {
 public:
  template <ColumnData T>
  Table(const std::array<const char*, siz>& col, T&& fmts)
  {
    header[0] = fmts;
    for (unsigned int i = 0; i < siz; ++i) {
      update(i, std::strlen(col[i]));
      header[i + 1] = col[i];
    }
  }
  Table() = default;
  template <ColumnId T>
  inline void update(const T col, size_t val)
  {
    const unsigned int id = static_cast<unsigned int>(col);
    width[id + 1] = std::max(width[id + 1], val + 2);
  }
  void printHeader(std::ostream& os)
  {
    printRow(header, os);
    os << header[0];
    for (unsigned int i = 1; i <= siz; ++i) {
      for (unsigned int j = 0; j < width[i]; ++j)
        os << "─";
      os << "  ";
    }
    os << Output::Escape::None << "\n";
  }
  template <ColumnData T>
  inline void newColumn(T&& col)
  {
    q.emplace_back();
    q.back()[0] = col;
  }
  template <ColumnId U, ColumnData T>
  void writeColumn(U&& col, T&& str)
  {
    q.back()[static_cast<unsigned int>(col) + 1] = str;
    update(static_cast<unsigned int>(col), q.back()[col + 1].size());
  }
  template <ColumnId U, ColumnData T>
  void writeColumnList(std::initializer_list<std::pair<U, T>> ini)
  {
    for (auto&& [col, str] : ini) {
      const unsigned int pos = static_cast<unsigned int>(col);
      q.back()[pos + 1] = str;
      update(pos, q.back()[pos + 1].size());
    }
  }
  void printAll(std::ostream& os)
  {
    while (!q.empty()) {
      printRow(q.front(), os);
      q.pop_front();
    }
  }
  void mergeData(Table&& other)
  {
    for (unsigned int i = 1; i <= siz; ++i)
      width[i] = std::max(width[i], other.width[i]);
    q.splice(q.end(), std::move(other.q));
  }

 private:
  template <class T>
  void printRow(const std::array<T, siz + 1>& r, std::ostream& os)
  {
    os << r[0];  // write color code
    for (unsigned int i = 1; i <= siz; ++i) {
      os.width(width[i]);
      os << r[i] << "  ";
    }
    os << Output::Escape::None << "\n";
  }

  using row = std::array<std::string, siz + 1>;
  row header;
  size_t width[siz + 1] = {};
  std::list<row> q;
};
template <size_t siz, ColumnData T>
Table(const std::array<const char*, siz>&, T&&) -> Table<siz>;
}  // namespace apdebug::Table

#endif