#ifndef REACTOR_GENERATOR_HXX
# define REACTOR_GENERATOR_HXX

namespace reactor
{
  template <typename T>
  Generator<T>::Generator(std::function<void (yielder const&)> const& driver)
    : _results()
    , _thread()
  {
    auto yield = [this] (T elt) { this->_results.put(std::move(elt)); };
    this->_thread.reset(
      new Thread("generator",
                 [this, driver, yield]
                 {
                   try
                   {
                     driver(yield);
                   }
                   catch (...)
                   {
                     this->_exception = std::current_exception();
                   }
                   this->_results.put({});
                 }));
  }

  template <typename T>
  Generator<T>::Generator(Generator<T>&& generator)
    : _results(std::move(generator._results))
    , _thread(std::move(generator._thread))
  {}

  template <typename T>
  Generator<T>::iterator::iterator()
    : _generator(nullptr)
    , _value()
    , _fetch(true)
  {}

  template <typename T>
  Generator<T>::iterator::iterator(Generator<T>& generator)
    : _generator(&generator)
    , _fetch(true)
  {}

  template <typename T>
  bool
  Generator<T>::iterator::operator !=(iterator const& other)
  {
    assert(other._generator == nullptr);
    if (this->_fetch)
    {
      this->_value = this->_generator->_results.get();
      this->_fetch = false;
    }
    if (this->_value)
      return true;
    else
      if (this->_generator->_exception)
        std::rethrow_exception(this->_generator->_exception);
      else
        return false;
  }

  template <typename T>
  typename Generator<T>::iterator&
  Generator<T>::iterator::operator ++()
  {
    this->_fetch = true;
    this->_value.reset();
    return *this;
  }

  template <typename T>
  T
  Generator<T>::iterator::operator *()
  {
    assert(!this->_fetch);
    return std::move(this->_value.get());
  }


  template <typename T>
  Generator<T>
  generator(
    std::function<void (typename yielder<T>::type const&)> const& driver)
  {
    return Generator<T>(driver);
  }

  template <typename T>
  typename Generator<T>::iterator
  Generator<T>::begin()
  {
    return typename Generator<T>::iterator(*this);
  }

  template <typename T>
  typename Generator<T>::iterator
  Generator<T>::end() const
  {
    return typename Generator<T>::iterator();
  }

  template <typename T>
  typename Generator<T>::iterator
  begin(Generator<T>& g)
  {
    return g.begin();
  }

  template <typename T>
  typename Generator<T>::iterator
  end(Generator<T> const& g)
  {
    return g.end();
  }
}

#endif
