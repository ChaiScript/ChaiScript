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

    auto &back() noexcept {
      return data.back();
    }

    const auto &back() const noexcept {
      return data.back();
    }


    Value &operator[](const Key &s) {
      const auto itr = find(s);
      if (itr != data.end()) {
        return itr->second;
      } else {
        return data.emplace_back(s, Value()).second;
      }
    }

    Value &at_index(const std::size_t idx) noexcept
    {
      return data[idx].second;
    }

    const Value &at_index(const std::size_t idx) const noexcept
    {
      return data[idx].second;
    }

    bool empty() const noexcept 
    {
      return data.empty();
    }

    template<typename Itr>
    void assign(Itr begin, Itr end)
    {
      data.assign(begin, end);
    }

    Value &at(const Key &s) {
      const auto itr = find(s);
      if (itr != data.end()) {
        return itr->second;
      } else {
        throw std::out_of_range("Unknown key: " + s);
      }
    }


    template<typename M>
    auto insert_or_assign(Key &&key, M &&m)
    {
      if (auto itr = find(key); itr != data.end()) {
        *itr = std::forward<M>(m);
        return std::pair{itr, false};
      } else {
        return std::pair{data.emplace(itr, std::move(key), std::forward<M>(m)), true};
      }
    }


    template<typename M>
    auto insert_or_assign(const Key &key, M &&m)
    {
      if (auto itr = find(key); itr != data.end()) {
        *itr = std::forward<M>(m);
        return std::pair{itr, false};
      } else {
        return std::pair{data.emplace(itr, key, std::forward<M>(m)), true};
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

    using value_type = std::pair<Key, Value>;
    using iterator = typename decltype(data)::iterator;
    using const_iterator = typename decltype(data)::const_iterator;

    std::pair<iterator,bool> insert( value_type&& value )
    {
      if (const auto itr = find(value.first); itr != data.end()) {
        return std::pair{itr, false};
      } else {
        return std::pair{data.insert(itr, std::move(value)), true};
      }
    }

  };

}

#endif

