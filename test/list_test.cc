#include "doctest.h"

#ifdef SINGLE_HEADER
#include "cista.h"
#else
#include "cista/serialization.h"
#endif

// namespace data = cista::offset;
using namespace cista;

TEST_CASE("list initialization") {
  using cista::raw::list;

  {
    auto l = list<int>();
    CHECK(l.empty());
    CHECK(l.size() == 0);
    CHECK(l.begin() == l.end());
    CHECK(l.cbegin() == l.cend());
  }

  {
    auto l = list<int>(42);
    CHECK(false == l.empty());
    CHECK(l.size() == 42);
    CHECK(l.begin() != l.end());
    CHECK(l.cbegin() != l.cend());
  }

  {
    auto l_a = list<int>{111, 222, 333};
    list<int> l_b(l_a);

    //  CHECK(l_a == l_b);
    CHECK(l_a.size() == l_b.size());
    CHECK(l_a.begin() != l_b.begin());
    CHECK(l_a.end() != l_b.end());
  }

  return;
}

TEST_CASE("list emplace_back on empty") {
  using cista::raw::list;

  auto l = list<int>();

  CHECK(l.empty());
  CHECK(l.size() == 0);

  auto begin = l.begin();
  auto cbegin = l.cbegin();
  auto end = l.end();
  auto cend = l.cend();

  CHECK(begin == end);
  CHECK(cbegin == cend);
  CHECK(l.begin() == l.end());
  CHECK(l.cbegin() == l.cend());

  auto& last_ref = l.emplace_back(42);
  auto list_it = l.end().prev();
  const auto const_list_it = l.cend().prev();

  CHECK(last_ref == 42);
  CHECK(list_it != l.end());
  CHECK(const_list_it != l.cend());

  CHECK((*list_it) == last_ref);
  CHECK((*const_list_it) == last_ref);
}

TEST_CASE("list emplace_back on already filled") {
  using cista::raw::list;

  auto l = list<int>{111, 222, 333};

  auto& last_ref = l.emplace_back(444);
  auto list_it = l.end().prev();
  const auto const_list_it = l.cend().prev();

  CHECK(last_ref == 444);
  CHECK(list_it != l.end());
  CHECK((*list_it) == last_ref);
  CHECK((*const_list_it) == last_ref);
}

TEST_CASE("list clear") {
  using cista::raw::list;

  {
    auto l = list<int>();
    l.clear();

    CHECK(l.empty());
    CHECK(l.size() == 0);
    CHECK(l.begin() == l.end());

    l.clear();
    l.clear();

    CHECK(l.empty());
    CHECK(l.size() == 0);
    CHECK(l.begin() == l.end());
  }

  {
    auto l = list<int>();
    auto l_a = l;
    auto l_b = l;

    CHECK(l.empty());
    CHECK(l_a.empty());
    CHECK(l_b.empty());

    CHECK(l.begin() == l.end());
    CHECK(l.cbegin() == l.cend());
    CHECK(l_a.begin() == l_a.end());
    CHECK(l_a.cbegin() == l_a.cend());
    CHECK(l_b.begin() == l_b.end());
    CHECK(l_b.cbegin() == l_b.cend());

    l_a.clear();
    l_b.clear();

    CHECK(l_a.empty());
    CHECK(l_b.empty());
    CHECK(l_a.begin() == l_a.end());
    CHECK(l_a.cbegin() == l_a.cend());
    CHECK(l_b.begin() == l_b.end());
    CHECK(l_b.cbegin() == l_b.cend());

    l_a.emplace_back(111);
    l_b.emplace_back(111);

    l_a.clear();
    CHECK(l_a.empty());
    CHECK(false == l_b.empty());
    CHECK(l_a.begin() == l_a.end());
    CHECK(l_a.cbegin() == l_a.cend());
    CHECK(l_b.begin() != l_b.end());
    CHECK(l_b.cbegin() != l_b.cend());

    l_a.emplace_back(111);
    l_b.clear();

    CHECK(false == l_a.empty());
    CHECK(l_b.empty());
    CHECK(l_a.begin() != l_a.end());
    CHECK(l_a.cbegin() != l_a.cend());
    CHECK(l_b.begin() == l_b.end());
    CHECK(l_b.cbegin() == l_b.cend());
  }

  {
    auto l = list<int>{111, 222};

    auto some_it = l.end().prev();
    auto& last_ref = l.emplace_back(333);
    auto list_it = l.end().prev();

    CHECK((*some_it) == 222);
    CHECK(last_ref == 333);
    CHECK(list_it != l.end());

    l.clear();
    l.clear();

    CHECK(l.empty());
    CHECK(l.size() == 0);
    CHECK(l.begin() == l.end());
    CHECK(l.cbegin() == l.cend());
  }
}

TEST_CASE("list itors stability, basic") {
  using cista::raw::list;

  auto l = list<int>{111, 222, 333};

  auto l_begin = l.begin();

  auto ones_it = l_begin++;
  auto twos_it = l_begin++;
  auto threes_it = l_begin++;

  CHECK((*ones_it++) == 111);
  CHECK((*twos_it++) == 222);
  CHECK((*threes_it++) == 333);

  CHECK(l_begin == l.end());

  CHECK((*ones_it) == (*twos_it) - 111);
  CHECK(threes_it == l.end());

  ones_it--;
  twos_it--;
  threes_it--;

  CHECK((*ones_it) == 111);
  CHECK((*twos_it) == 222);
  CHECK((*threes_it) == 333);
}

TEST_CASE("list itors stability, moderate") {
  using cista::raw::list;

  auto l = list<int>{111, 222, 333};

  auto l_begin = l.begin();
  auto l_end = l.end();

  auto& ref444 = l.emplace_back(444);
  auto it_444 = l.end().prev();

  auto& ref555 = l.emplace_back(555);
  auto it_555 = l.end().prev();

  auto ones_it_from_begin = l_begin++;
  auto twos_it_from_begin = l_begin++;
  auto threes_it_from_begin = l_begin++;

  l_end--;
  l_end--;

  auto threes_it_from_end = --l_end;
  auto twos_it_from_end = --l_end;
  auto ones_it_from_end = --l_end;

  CHECK((*ones_it_from_begin) == 111);
  CHECK((*twos_it_from_begin) == 222);
  CHECK((*threes_it_from_begin) == 333);

  CHECK((*ones_it_from_end) == 111);
  CHECK((*twos_it_from_end) == 222);
  CHECK((*threes_it_from_end) == 333);
}

TEST_CASE("list erase, iterator ending on filled") {
  using cista::raw::list;

  auto l = list<int>{111, 222};

  auto some_it = l.end().prev();
  auto erase_it = l.erase(some_it);

  CHECK(erase_it == l.end());
  CHECK(false == l.empty());
  CHECK(l.begin() != l.end());
}

TEST_CASE("list erase, iterator endining on one element") {
  using cista::raw::list;

  auto l = list<int>{42};

  auto some_it = l.end().prev();
  auto erase_it = l.erase(some_it);

  CHECK(erase_it == l.end());
  CHECK(l.empty());
  CHECK(l.size() == 0);
  CHECK(l.begin() == l.end());
}

TEST_CASE("list erase, iterator 'next' onto endining") {
  using cista::raw::list;

  auto l = list<int>{111, 222};

  auto some_it = l.begin().next();
  auto erase_it = l.erase(some_it);

  CHECK(erase_it == l.end());
  CHECK(false == l.empty());
  CHECK(l.begin() != l.end());
}

TEST_CASE("list erase, iterator begining") {
  using cista::raw::list;

  auto l = list<int>{111, 222};

  auto last_it = l.end().prev();

  auto some_it = l.begin();
  auto erase_it = l.erase(some_it);

  CHECK(erase_it == last_it);
  CHECK(false == l.empty());
  CHECK(l.begin() != l.end());
}

TEST_CASE("list itors stability, erase") {
  using cista::raw::list;

  auto l = list<int>{111, 222};

  auto& ref333 = l.emplace_back(333);
  auto it_333 = l.end().prev();

  auto& ref444 = l.emplace_back(444);
  auto it_444 = l.end().prev();

  auto& ref555 = l.emplace_back(555);
  auto it_555 = l.end().prev();

  {
    auto erase_it = l.erase(it_333);
    CHECK(erase_it == it_444);
    CHECK((*it_444) == ref444);
  }

  l.emplace_back(000);

  CHECK((*it_444) == ref444);
  CHECK((*it_555) == ref555);

  {
    auto erase_it = l.erase(it_555);
    CHECK((*erase_it) == 000);
    CHECK((*it_444) == ref444);
  }

  {
    auto erase_it = l.erase(it_444);
    CHECK((*erase_it) == 000);
  }
}

TEST_CASE("list ctors") {
  using cista::raw::list;

  auto l = list<int>{111, 222, 333};

  auto l_a = l;
  l.erase(l.end().prev());
  auto l_b = l;

  CHECK(l.size() == 2);
  CHECK(l_a.size() == 3);
  CHECK(l_b.size() == 2);

  l_a.emplace_front(121212);
  l_b.emplace_front(212121);
  
  l.emplace_front(123456);
  l.emplace_back(654321);

  //TODO: move ctor

  return;
}