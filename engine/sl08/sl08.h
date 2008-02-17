#ifndef SL08_SIGNALS_CORE_H
#define SL08_SIGNALS_CORE_H

#include <deque>
#include <set>
#include <algorithm>
#include <functional>

#define SLOT(N, tspec, proto, call, signal_template, signal_spec, arglist, arglist2, declist, declist2) \
template signal_template class signal##N; \
template signal_template \
class base_slot##N {\
protected: \
	typedef signal##N signal_spec signal_type; \
	signal_type *signal; \
\
public: \
	virtual R operator() proto = 0; \
	base_slot##N () : signal(NULL) {} \
	void connect(signal##N signal_spec &signal_ref) {\
		signal = &signal_ref; \
		signal->connect(this); \
	} \
	\
	void disconnect() {\
		signal->disconnect(this); \
		signal = NULL; \
	} \
	virtual ~base_slot##N() { \
		disconnect(); \
	} \
}; \
\
template tspec \
class slot##N : public base_slot##N signal_spec { \
public: \
	typedef R (CL::*func_t) proto; \
	slot##N(CL *object, func_t func) : object(object), func(func) {}\
	\
	R operator() proto { \
		return (object->*func) call ;\
	} \
	\
private: \
	CL *object; \
	func_t func;\
}; \
template <declist class CL> \
class slot##N <void arglist CL> : public base_slot##N <void arglist2> { \
public: \
	typedef void (CL::*func_t) proto ;\
	slot##N (CL *object, func_t func) : object(object), func(func) {}\
	\
	void operator() proto { \
		(object->*func) call; \
	} \
	\
private: \
	CL *object; \
	func_t func; \
}

#define SIGNAL(N, tspec, proto, call, signal_spec, slot_spec, declist, arglist) \
template tspec \
class base_signal##N { \
protected: \
	typedef base_slot##N signal_spec slot_type; \
	typedef std::deque<slot_type *> slots_type; \
	slots_type slots;\
public: \
	R emit proto { \
		R r; \
		for(typename slots_type::iterator i = slots.begin(); i != slots.end(); ++i) { \
			r = (*i)->operator() call ; \
		} \
		return r; \
	} \
\
	void connect(base_slot##N signal_spec *slot) {\
		slots.push_back(slot);\
	} \
	\
	void disconnect(base_slot##N signal_spec *slot) {\
		for(typename slots_type::iterator i = slots.begin(); i != slots.end(); ) { \
			if (slot != *i) \
				++i; \
			else \
				i = slots.erase(i); \
		} \
	} \
};\
template tspec \
class signal##N : public base_signal##N signal_spec { \
}

#define SIGNAL_SPECIAL(N, typename_helper, proto, call, tspec, tpar) \
template tspec \
class signal##N tpar : public base_signal##N tpar { \
	typedef base_signal##N tpar parent_t; \
public: \
	void emit proto {  \
		for(typename_helper parent_t::slots_type::iterator i = parent_t::slots.begin(); i != parent_t::slots.end(); ++i) { \
			(*i)->operator() call ; \
		} \
	} \
}


#define SLOT_ARG0 <typename R, class CL>
#define SIGNAL_ARG0 <typename R>
#define SIGNAL_TEMPLATE_ARG0 <R>
#define SLOT_TEMPLATE_ARG0_ARGLIST ,
#define SLOT_TEMPLATE_ARG0_ARGLIST2
#define SLOT_TEMPLATE_ARG0 <R, ##SLOT_TEMPLATE_ARG0_ARGLIST## CL>
#define SIGNAL_SPECIAL0_TEMPLATE <>
#define SIGNAL_SPECIAL0_TPARAM <void>

#define SLOT_ARG1 <typename R, typename A1, class CL>
#define SIGNAL_ARG1 <typename R, typename A1>
#define SIGNAL_TEMPLATE_ARG1 <R, A1>
#define SLOT_TEMPLATE_ARG1_ARGLIST2 , A1 
#define SLOT_TEMPLATE_ARG1_DECLIST2 typename A1
#define SLOT_TEMPLATE_ARG1_DECLIST SLOT_TEMPLATE_ARG1_DECLIST2,
#define SLOT_TEMPLATE_ARG1_ARGLIST SLOT_TEMPLATE_ARG1_ARGLIST2, 
#define SLOT_TEMPLATE_ARG1 <R, ##SLOT_TEMPLATE_ARG1_ARGLIST## CL>

#define SIGNAL_SPECIAL1_TEMPLATE <typename A1>
#define SIGNAL_SPECIAL1_TPARAM <void, A1>

namespace sl08 {
	SLOT(0, SLOT_ARG0, (), (), SIGNAL_ARG0, SIGNAL_TEMPLATE_ARG0, \
		SLOT_TEMPLATE_ARG0_ARGLIST, SLOT_TEMPLATE_ARG0_ARGLIST2 ,,);
	SLOT(1, SLOT_ARG1, (A1 a1), (a1), SIGNAL_ARG1, SIGNAL_TEMPLATE_ARG1, \
		SLOT_TEMPLATE_ARG1_ARGLIST, SLOT_TEMPLATE_ARG1_ARGLIST2, SLOT_TEMPLATE_ARG1_DECLIST, SLOT_TEMPLATE_ARG1_DECLIST2);
	SIGNAL(0, SIGNAL_ARG0, (), (), SIGNAL_TEMPLATE_ARG0, SLOT_TEMPLATE_ARG0, SIGNAL_TEMPLATE_ARG0_DECLIST, SLOT_TEMPLATE_ARG0_ARGLIST2);
	SIGNAL(1, SIGNAL_ARG1, (A1 a1), (a1), SIGNAL_TEMPLATE_ARG1, SLOT_TEMPLATE_ARG1, SIGNAL_TEMPLATE_ARG1_DECLIST, SLOT_TEMPLATE_ARG1_ARGLIST2);

	SIGNAL_SPECIAL(0, , 		 (), (), SIGNAL_SPECIAL0_TEMPLATE, SIGNAL_SPECIAL0_TPARAM);
	SIGNAL_SPECIAL(1, typename , (A1 a1), (a1), SIGNAL_SPECIAL1_TEMPLATE, SIGNAL_SPECIAL1_TPARAM);
}

#endif


