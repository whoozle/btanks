#ifndef __STACKVM_SINGLETON_H__
#define __STACKVM_SINGLETON_H__

namespace mrt {
	template <class S> class Accessor {
	public:
		inline S* operator->() const {
			static S * p = S::get_instance();
			return p;
		}
		inline const S* get_const() const {
			static const S * p = S::get_instance();
			return p;
		}
	};
}

#define SINGLETON(name, class) \
	extern mrt::Accessor<class> name

#define DECLARE_SINGLETON(class) \
	static inline class * get_instance() { \
		static class instance; \
		return &instance; \
	} 

#define IMPLEMENT_SINGLETON(name, class) \
	mrt::Accessor<class> name; \


#endif
