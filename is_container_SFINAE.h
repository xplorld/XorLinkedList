//https://gist.github.com/louisdx/1076849

#include <utility>


template<typename T>
struct has_const_iterator
{
private:
  typedef char                      one;
  typedef struct { char array[2]; } two;

  template<typename C> static one test(typename C::const_iterator*);
  template<typename C> static two  test(...);
public:
  static const bool value = sizeof(test<T>(0)) == sizeof(one);
  typedef T type;
};

template <typename T>
struct has_begin_end
{
  struct Dummy { typedef void const_iterator; };
  typedef typename std::conditional<has_const_iterator<T>::value, T, Dummy>::type TType;
  typedef typename TType::const_iterator iter;

  struct Fallback { iter begin() const; iter end() const; };
  struct Derived : TType, Fallback { };

  template<typename C, C> struct ChT;

  template<typename C> static char (&f(ChT<iter (Fallback::*)() const, &C::begin>*))[1];
  template<typename C> static char (&f(...))[2];
  template<typename C> static char (&g(ChT<iter (Fallback::*)() const, &C::end>*))[1];
  template<typename C> static char (&g(...))[2];

  static bool const beg_value = sizeof(f<Derived>(0)) == 2;
  static bool const end_value = sizeof(g<Derived>(0)) == 2;
};

template <typename T>
struct is_container
{
  static const bool value = has_const_iterator<T>::value &&
    has_begin_end<T>::beg_value && has_begin_end<T>::end_value;
};
