#ifndef __STACKVM_SINGLETON_H__
#define __STACKVM_SINGLETON_H__

#include "mrt/export_mrt.h"

/* M-runtime for c++
 * Copyright (C) 2005-2008 Vladimir Menshakov
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.

 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
*/


namespace mrt {
	template <class S> class Accessor {
	public:

		inline S* operator->() const {
			static S * p = S::get_instance();
			return p;
		}

		inline const S* get_const() const {
			static S * p = S::get_instance();
			return p;
		}
	};
}

#define SINGLETON(name, class) \
	extern const mrt::Accessor<class> name

#define PUBLIC_SINGLETON(e, name, class) \
	extern e const mrt::Accessor<class> name

#define DECLARE_SINGLETON(class) \
	static class * get_instance()

#define IMPLEMENT_SINGLETON(name, class) \
	class * class::get_instance() { \
		static class instance; \
		return &instance; \
	} \
	\
	const mrt::Accessor<class> name = mrt::Accessor<class>()


#endif
