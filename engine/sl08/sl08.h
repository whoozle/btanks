#ifndef SL08_SIGNALS_CORE_H
#define SL08_SIGNALS_CORE_H

#include <deque>
#include <algorithm>
#include <functional>

#define SIGNAL(N, tspec, proto, call) \
template tspec \
class signal##N { \
	std::deque<void *> slots; \
	struct caller : public std::unary_function<R, void *> { \
		caller proto {} \
		R operator()(void *a) {} \
	    }; \
public: \
\
	R emit proto { \
		std::for_each(slots.begin(), slots.end(), caller call); \
	} \
}

#define SIGNAL_ARG0 <typename R>
#define SIGNAL_ARG1 <typename R, typename A1>

namespace sl08 {
	SIGNAL(0, SIGNAL_ARG0, (), ());
	SIGNAL(1, SIGNAL_ARG1, (A1 a1), (a1));
}

#endif


