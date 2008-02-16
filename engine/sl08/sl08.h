#ifndef SL08_SIGNALS_CORE_H
#define SL08_SIGNALS_CORE_H

#include <deque>
#include <set>
#include <algorithm>
#include <functional>

#define SLOT(N, tspec, proto, call, signal_template, signal_spec) \
template signal_template class signal##N; \
template tspec \
class slot##N { \
	typedef signal##N signal_spec signal_type; \
	signal_type *signal; \
public: \
	typedef R (CL::*func_t) proto; \
	slot##N(CL *object, func_t func) : signal(NULL), object(object), func(func) {}\
	\
	R operator() proto { \
		return (*object).*func call ;\
	} \
	\
	void connect(signal##N signal_spec &signal_ref) {\
		signal = &signal_ref; \
		signal->connect(this); \
	} \
	\
	void disconnect() {\
		signal->disconnect(this); \
	} \
	~slot##N() { \
		disconnect(); \
	} \
private: \
	CL *object; \
	func_t func;\
}

#define SIGNAL(N, tspec, proto, call, signal_spec, slot_spec) \
template tspec \
class signal##N { \
public: \
	R emit proto { \
	} \
\
	template <class CL> \
	void connect(slot##N slot_spec *slot) {\
	} \
	\
	template <class CL> \
	void disconnect(slot##N slot_spec *slot) {\
	} \
}
#define SLOT_ARG0 <typename R, typename CL>
#define SLOT_ARG1 <typename R, typename A1, typename CL>

#define SIGNAL_ARG0 <typename R>
#define SIGNAL_ARG1 <typename R, typename A1>

#define SIGNAL_TEMPLATE_ARG0 <R>
#define SIGNAL_TEMPLATE_ARG1 <R, A1>
#define SLOT_TEMPLATE_ARG0 <R, CL>
#define SLOT_TEMPLATE_ARG1 <R, A1, CL>

namespace sl08 {
	SLOT(0, SLOT_ARG0, (), (), SIGNAL_ARG0, SIGNAL_TEMPLATE_ARG0);
	SLOT(1, SLOT_ARG1, (A1 a1), (a1), SIGNAL_ARG1, SIGNAL_TEMPLATE_ARG1);
	SIGNAL(0, SIGNAL_ARG0, (), (), TEMPLATE_ARG0, SLOT_TEMPLATE_ARG0);
	SIGNAL(1, SIGNAL_ARG1, (A1 a1), (a1), TEMPLATE_ARG1, SLOT_TEMPLATE_ARG1);
}

#endif


