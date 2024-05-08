#pragma once

#include <cassert>
#include <cinttypes>
#include <cstdlib>
#include <memory>
#include <type_traits>

#include "cista/allocator.h"
#include "cista/containers/ptr.h"
#include "cista/exception.h"
#include "cista/is_iterable.h"
#include "cista/next_power_of_2.h"
#include "cista/strong.h"
#include "cista/unused_param.h"
#include "cista/verify.h"

namespace cista {

template <typename T, template <typename> typename Ptr,
          typename TemplateSizeType = std::uint32_t,
          class Allocator = allocator<T, Ptr>>
struct basic_list {
  using size_type = base_t<TemplateSizeType>;
  using difference_type = std::ptrdiff_t;
  using reference = T&;
  using const_reference = T const&;
  using pointer = Ptr<T>;
  using const_pointer = Ptr<T const>;
  using value_type = T;
  using allocator_type = Allocator;

  struct node_base {
    node_base* next_;
    node_base* prev_;

    void insert(node_base* next) noexcept {
      assert(next);
      assert(next->next_);
      assert(next->prev_);

      next_ = next;
      prev_ = next->prev_;
      next->prev_->next_ = this;
      next->prev_ = this;
    }

    void remove() noexcept {
      assert(next_);
      assert(prev_);

      next_->prev_ = prev_;
      prev_->next_ = next_;
    }
  };

  struct node : public node_base {
    value_type value_;
  };

  template <typename Pointer, typename Reference>
  struct list_iterator {
    using iterator_type = list_iterator<Pointer, Reference>;

    node_base* node_;

    list_iterator() noexcept : node_() {}
    list_iterator(const node_base* node) noexcept
        : node_(const_cast<node_base*>(node)) {}
    list_iterator(const list_iterator& it) noexcept
        : node_(const_cast<node_base*>(it.node_)) {}

    iterator_type next() const noexcept { return list_iterator(node_->next_); }
    iterator_type prev() const noexcept { return list_iterator(node_->prev_); }

    reference operator*() const noexcept {
      assert(node_);
      return static_cast<node*>(node_)->value_;
    }

    pointer operator->() const noexcept {
      assert(node_);
      &static_cast<node*>(node_)->value_;
    }

    iterator_type& operator++() noexcept {
      assert(node_);
      assert(node_->next_);
      node_ = node_->next_;
      return *this;
    }

    iterator_type operator++(int) noexcept {
      assert(node_);
      assert(node_->next_);
      list_iterator temp(*this);
      node_ = node_->next_;
      return temp;
    }

    iterator_type& operator--() noexcept {
      assert(node_);
      assert(node_->prev_);
      node_ = node_->prev_;
      return *this;
    }

    iterator_type operator--(int) noexcept {
      assert(node_);
      assert(node_->prev_);
      list_iterator temp(*this);
      node_ = node_->prev_;
      return temp;
    }

    friend bool operator==(const list_iterator& lhs,
                           const list_iterator& rhs) noexcept {
      return lhs.node_ == rhs.node_;
    }

    friend bool operator!=(const list_iterator& lhs,
                           const list_iterator& rhs) noexcept {
      return lhs.node_ != rhs.node_;
    }
  };

  using iterator = list_iterator<pointer, reference>;
  using const_iterator = list_iterator<const_pointer, const_reference>;

  basic_list() noexcept {
    node_.next_ = &node_;
    node_.prev_ = &node_;
  }

  explicit basic_list(allocator_type const&) noexcept : basic_list() {}

  explicit basic_list(size_type const size, value_type init = value_type{},
                      Allocator const& alloc = Allocator{})
      : basic_list() {
    CISTA_UNUSED_PARAM(alloc)
    resize(size, std::move(init));
  }

  template <class InputIt>
  basic_list(InputIt first, InputIt last, Allocator const& alloc = Allocator{})
      : basic_list() {
    CISTA_UNUSED_PARAM(alloc);
    for (; first != last; ++first) insert(&node_, *first);
  }

  basic_list(std::initializer_list<value_type> init,
             Allocator const& alloc = Allocator{})
      : basic_list(init.begin(), init.end(), alloc) {}

  basic_list(basic_list const& other, Allocator const& alloc = Allocator{})
      : basic_list(other.cbegin(), other.cend(), alloc) {}

  basic_list& operator=(basic_list const& other) {
    if (&other != this) {
      clear();
      auto first = other.cbegin();
      auto last = other.cend();
      for (; first != last; ++first) insert(&node_, *first);
    }
    return *this;
  }

  ~basic_list() { clear(); }

  allocator_type get_allocator() const noexcept { return {}; }

  iterator begin() const noexcept { return iterator(node_.next_); }
  iterator end() const noexcept { return iterator(&node_); }
  const_iterator cbegin() const noexcept { return const_iterator(node_.next_); }
  const_iterator cend() const noexcept { return const_iterator(&node_); }

  friend const_iterator begin(const basic_list& other) noexcept {
    return other.begin();
  }

  friend const_iterator end(const basic_list& other) noexcept {
    return other.end();
  }

  friend iterator begin(basic_list& other) noexcept { return other.begin(); }
  friend iterator end(basic_list& other) noexcept { return other.end(); }

  const_reference back() const noexcept {
    assert(node_.prev_);
    static_cast<node*>(node_.prev_)->value_;
  }

  reference back() noexcept {
    assert(node_.prev_);
    static_cast<node*>(node_.prev_)->value_;
  }

  reference front() noexcept {
    assert(node_.next_);
    static_cast<node*>(node_.next_)->value_;
  }

  const_reference front() const noexcept {
    assert(node_.next_);
    static_cast<node*>(node_.next_)->value_;
  }

  size_type max_size() const { return std::numeric_limits<size_type>::max(); }
  size_type size() const noexcept { return size_; }
  bool empty() const noexcept { return size() == 0U; }

  void clear() noexcept {
    node_base* clear_node = node_.next_;

    while (clear_node != &node_) {
      node* temp = static_cast<node*>(clear_node);
      clear_node = clear_node->next_;

      temp->~node();
      std::free(temp);
    }

    size_ = 0;

    node_.next_ = &node_;
    node_.prev_ = &node_;
  }

  iterator insert(const_iterator pos, value_type init = value_type{}) {
    node* create_node = static_cast<node*>(std::malloc(sizeof(node)));
    if (create_node == nullptr) {
      throw_exception(std::bad_alloc());
    }

    new (&create_node->value_) value_type(std::move(init));
    static_cast<node_base*>(create_node)->insert(pos.node_);
    ++size_;

    return create_node;
  }

  template <typename... Args>
  iterator insert(const_iterator pos, Args&&... args) {
    return insert(pos, std::forward<Args>(args)...);
  }

  // TODO: should be simplified as insert-loop
  iterator insert(const_iterator pos, size_type count,
                  value_type init = value_type{}) {
    iterator prev(pos.node_);
    --prev;

    const auto node_size = sizeof(node);

    for (size_type i = count; i > 0; --i) {
      node* create_node = static_cast<node*>(std::malloc(node_size));
      if (create_node == nullptr) {
        throw_exception(std::bad_alloc());
      }

      new (&create_node->value_) value_type(init);
      static_cast<node_base*>(create_node)->insert(pos.node_);
      ++size_;
    }

    return ++prev;
  }

  template <typename... Args>
  iterator emplace(const_iterator pos, Args&&... args) {
    return insert(pos.node_, std::forward<Args>(args)...);
  }

  template <typename... Args>
  reference emplace_front(Args&&... args) {
    return *emplace(node_.next_, std::forward<Args>(args)...);
  }

  template <typename... Args>
  reference emplace_back(Args&&... args) {
    return *emplace(&node_, std::forward<Args>(args)...);
  }

  // TODO: as const_iterator
  iterator erase(iterator pos) {
    ++pos;

    assert(pos.node_);
    assert(pos.node_->prev_);
    auto erase_node = pos.node_->prev_;
    erase_node->remove();

    node* temp = static_cast<node*>(erase_node);
    temp->~node();
    std::free(temp);

    --size_;

    return pos.node_;
  }

  // TODO: as const_iterator
  iterator erase(iterator first, iterator last) {
    while (first != last) first = erase(first);
    return last.node_;
  }

  void resize(size_type const size, value_type init = value_type{}) {
    verify(size >= 0 && size <= std::numeric_limits<size_type>::max(),
           "cista::list::resize: invalid range");

    iterator it(node_.next_);
    size_type count = 0;

    while ((it.node_ != &node_) && (count < size)) {
      ++it;
      ++count;
    }

    if (count == size)
      erase(it, &node_);
    else
      insert(&node_, size - count, std::move(init));
  }

  node_base node_;
  size_type size_{0U};

  std::uint8_t __fill_0__{0U};
  std::uint16_t __fill_1__{0U};
  std::uint32_t __fill_2__{0U};
};

namespace raw {
// using cista::list;
template <typename T>
using list = basic_list<T, ptr>;
}  // namespace raw

namespace offset {
// using cista::list;
template <typename T>
using list = basic_list<T, ptr>;
}  // namespace offset

}  // namespace cista