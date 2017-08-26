#ifndef CHAISCRIPT_UTILITY_QUICK_FLAT_MAP_HPP
#define CHAISCRIPT_UTILITY_QUICK_FLAT_MAP_HPP

namespace chaiscript::utility {

  template<typename Key, typename Value>
  struct QuickFlatMap
  {
    auto find(const Key &s) noexcept {
      return std::find_if(std::begin(data), std::end(data), [&s](const auto &d) { return d.first == s; });
    }

    auto find(const Key &s) const noexcept {
      return std::find_if(std::begin(data), std::end(data), [&s](const auto &d) { return d.first == s; });
    }

    auto size() const noexcept {
      return data.size();
    }

    auto begin() const noexcept {
      return data.begin();
    }

    auto end() const noexcept {
      return data.end();
    }


    auto begin() noexcept {
      return data.begin();
    }

    auto end() noexcept {
      return data.end();
    }


    Value &operator[](const Key &s) {
      const auto itr = find(s);
      if (itr != data.end()) {
        return itr->second;
      } else {
        return data.emplace_back(s, Value()).second;
      }
    }

    Value &at(const Key &s) {
      const auto itr = find(s);
      if (itr != data.end()) {
        return itr->second;
      } else {
        throw std::out_of_range("Unknown key: " + s);
      }
    }

    const Value &at(const Key &s) const {
      const auto itr = find(s);
      if (itr != data.end()) {
        return itr->second;
      } else {
        throw std::out_of_range("Unknown key: " + s);
      }
    }

    size_t count(const Key &s) const noexcept {
      return (find(s) != data.end())?1:0;
    }

    std::vector<std::pair<Key, Value>> data;

    using iterator = typename decltype(data)::iterator;
    using const_iterator = typename decltype(data)::const_iterator;


  };

}

#endif

