#ifndef ONIDEV_CORE_SIGNAL_H_INCLUDED
#define ONIDEV_CORE_SIGNAL_H_INCLUDED

#ifndef ONIDEV_LOOP_PREVENT
#define ONIDEV_LOOP_PREVENT 1
#endif

#include <functional>
#include <utility>
#include <list>
#include <set>
#include "signal.incl"

namespace od
{
	typedef size_t TypeID;
#if ONIDEV_LOOP_PREVENT
	extern std::set<void*> _emitted;
	extern void* _first_emitted;
#endif
template <typename... Args>
class Signal
{
    // http://stackoverflow.com/questions/21192659/variadic-templates-and-stdbind
    template<class T, int... Is>
    void connect_member(void (T::*field)(Args...), T* t, int_sequence<Is...>)
    {
        _slot.push_back(std::bind(field, t, placeholder_template<Is>{}...));
		//_slot = (std::bind(field, t, placeholder_template<Is>{}...));
    }
    
public:
    typedef std::function<void(Args...)> Slot;
    
	void swap(Signal& sign)
	{
		_slot.swap(sign._slot);
	}
    template<class T>
    void connect(void (T::*field)(Args...), T* t)
    {
        connect_member(field, t, make_int_sequence<sizeof...(Args)>{});
    }
    void connect(Slot const& slot) const
    {
        _slot.push_back(slot);
		//_slot = slot;
    }
    void disconnect() const
    {
		_slot.clear();
		//_slot = Slot();
    }
    template <typename... Params>
    void emit(Params&&... args)
    {
#if ONIDEV_LOOP_PREVENT
		if (_emitted.find(this) == _emitted.end())
		{
			_add_emitted_items();
#endif //ONIDEV_LOOP_PREVENT
		for (auto& slot : _slot)
			slot(std::forward<Args>(args)...);
#if ONIDEV_LOOP_PREVENT
		}
		_clear_emitted_items();
#endif //ONIDEV_LOOP_PREVENT
        /*if(_slot)
            _slot(std::forward<Args>(args)...);*/
    }
    Signal& operator=(Signal const& other)
    {
        disconnect();
		return *this;
    }

private:
	
    mutable std::list<Slot> _slot;
#if ONIDEV_LOOP_PREVENT
	void _add_emitted_items()
	{
		if (_first_emitted == nullptr)
			_first_emitted = this;
		_emitted.insert(this);
	}
	void _clear_emitted_items()
	{
		if (this == _first_emitted)
		{
			_emitted.clear();
			_first_emitted = nullptr;
		}
	}
#endif //ONIDEV_LOOP_PREVENT
};

}

#endif
