#ifndef CHAISCRIPT_UTILITY_QUICK_FLAT_MAP_HPP
#define CHAISCRIPT_UTILITY_QUICK_FLAT_MAP_HPP

namespace chaiscript::utility {
  template<typename Key, typename Value, typename Comparator = std::equal_to<>>
  struct QuickFlatMap {
    Comparator comparator;

    template<typename Lookup>
    auto find(const Lookup &s) noexcept {
      return std::find_if(std::begin(data), std::end(data), [&s, this](const auto &d) { return comparator(d.first, s); });
    }

    template<typename Lookup>
    auto find(const Lookup &s) const noexcept {
      return std::find_if(std::cbegin(data), std::cend(data), [&s, this](const auto &d) { return comparator(d.first, s); });
    }

    template<typename Lookup>
    auto find(const Lookup &s, const std::size_t t_hint) const noexcept {
      if (data.size() > t_hint && comparator(data[t_hint].first, s)) {
        const auto begin = std::cbegin(data);
        return std::next(begin, static_cast<typename std::iterator_traits<std::decay_t<decltype(begin)>>::difference_type>(t_hint));
      } else {
        return find(s);
      }
    }

    auto size() const noexcept { return data.size(); }

    auto begin() const noexcept { return data.begin(); }

    auto end() const noexcept { return data.end(); }

    auto begin() noexcept { return data.begin(); }

    auto end() noexcept { return data.end(); }

    auto &back() noexcept { return data.back(); }

    const auto &back() const noexcept { return data.back(); }

    Value &operator[](const Key &s) {
      const auto itr = find(s);
      if (itr != data.end()) {
        return itr->second;
      } else {
        grow();
        return data.emplace_back(s, Value()).second;
      }
    }

    Value &at_index(const std::size_t idx) noexcept { return data[idx].second; }

    const Value &at_index(const std::size_t idx) const noexcept { return data[idx].second; }

    bool empty() const noexcept { return data.empty(); }

    template<typename Itr>
    void assign(Itr begin, Itr end) {
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
    auto insert_or_assign(Key &&key, M &&m) {
      if (auto itr = find(key); itr != data.end()) {
        *itr = std::forward<M>(m);
        return std::pair{itr, false};
      } else {
        grow();
        return std::pair{data.emplace(data.end(), std::move(key), std::forward<M>(m)), true};
      }
    }

    template<typename M>
    auto insert_or_assign(const Key &key, M &&m) {
      if (auto itr = find(key); itr != data.end()) {
        itr->second = std::forward<M>(m);
        return std::pair{itr, false};
      } else {
        grow();
        return std::pair{data.emplace(data.end(), key, std::forward<M>(m)), true};
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

    template<typename Lookup>
    size_t count(const Lookup &s) const noexcept {
      return (find(s) != data.end()) ? 1 : 0;
    }

    std::vector<std::pair<Key, Value>> data;

    using value_type = std::pair<Key, Value>;
    using iterator = typename decltype(data)::iterator;
    using const_iterator = typename decltype(data)::const_iterator;

    std::pair<iterator, bool> insert(value_type &&value) {
      if (const auto itr = find(value.first); itr != data.end()) {
        return std::pair{itr, false};
      } else {
        grow();
        return std::pair{data.insert(data.end(), std::move(value)), true};
      }
    }

    void grow() {
      if ((data.capacity() - data.size()) == 0) {
        data.reserve(data.size() + 2);
      }
    }
  };

} // namespace chaiscript::utility

#endif
