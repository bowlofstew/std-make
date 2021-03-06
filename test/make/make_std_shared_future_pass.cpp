// Copyright (C) 2014 Vicente J. Botet Escriba
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// <experimental/make.hpp>

//  template <template <class ...> class M, class X>
//  auto make(X&& x);
//  template <class M, class X>
//  auto make(X&& x);
//  template <class M, class ...Args>
//  auto make(Args&& ...args);


#include <experimental/make.hpp>
#include <future>
#include <boost/detail/lightweight_test.hpp>

namespace std {

  // customization point for template (needed because std::shared_future<X> doesn't has a constructor from X)
  template <class T, class X>
  shared_future<T> make(experimental::type<shared_future<T>>, X&& x)
  {
    promise<T> p;
    p.set_value(forward<X>(x));
    return p.get_future().share();
  }

  // customization point for template (needed because std::shared_future doesn't use experimental::in_place_t)
  template <class T, class ...Args>
  shared_future<T> make(experimental::type<shared_future<T>>, experimental::in_place_t, Args&& ...args)
  {
    promise<T> p;
    p.set_value(T(forward<Args>(args)...));
    return p.get_future().share();
  }

  // Holder specialization
  template <>
  struct shared_future<experimental::_t> : experimental::lift<shared_future> {};

  template <>
  struct shared_future<experimental::_t&> : experimental::type_constructor_tag
  {
    template<class T>
    using apply = shared_future<T&>;
  };

  namespace experimental
  {
    // type_constructor customization
    template <class T>
    struct type_constructor<shared_future<T>> : identity<shared_future<_t>> {};
  }
}


struct A
{
  int v;
  A(): v(3) {}
  A(int v): v(std::move(v)) {}
  A(int v1, int v2): v(v1+v2) {}
};

int main()
{
  namespace stde = std::experimental;
  {
    int v=0;
    std::shared_future<int> x = stde::make<std::shared_future>(v);
    BOOST_TEST(x.get() == 0);
  }
  {
    int v=0;
    std::shared_future<int> x = stde::make<std::shared_future<int>>(v);
    BOOST_TEST(x.get() == 0);
  }
  {
    int v=1;
    std::shared_future<A> x = stde::make<std::shared_future<A>>(v,v);
    BOOST_TEST(x.get().v == 2);
  }
  {
    std::shared_future<int> x = stde::make<std::shared_future<int>>();
    BOOST_TEST_EQ(x.get(),  0);
  }
  {
    int v=0;
    std::shared_future<int&> x = stde::make<std::shared_future<int&>>(v);
    BOOST_TEST(&x.get() == &v);
  }
  {
    int v=0;
    std::shared_future<int> x = stde::make<std::shared_future<stde::_t>>(v);
    BOOST_TEST(x.get() == 0);
  }
  {
    int v=0;
    auto x = stde::make<std::shared_future<stde::_t&>>(v);
    BOOST_TEST(&x.get() == &v);
  }
  return ::boost::report_errors();
}
